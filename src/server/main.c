/**
 * main.c - servidor proxy SOCKSv5 concurrente.
 *
 * Interpreta los argumentos de línea de comandos, monta los sockets
 * pasivos (SOCKS y management) y atiende todas las conexiones en un único
 * hilo usando el selector (I/O no bloqueante multiplexada).
 *
 * Basado en ejemplos/main.c provisto por la cátedra (Juan F. Codagnone).
 */
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "args.h"
#include "mgmt.h"
#include "selector.h"
#include "socks5nio.h"

#define PORT_STR_LEN      6     /* "65535" + '\0' */
#define LISTEN_BACKLOG    SOMAXCONN
#define SELECTOR_INITIAL  1024
#define SELECT_TIMEOUT_S  10

static volatile sig_atomic_t done = false;

static void
sigterm_handler(const int signal) {
    (void) signal;
    done = true;
}

/**
 * Crea un socket pasivo TCP no bloqueante escuchando en `addr':`port'.
 * `addr' debe ser una dirección numérica IPv4 o IPv6 (no se resuelve DNS).
 *
 * Retorna el fd, o -1 ante error (dejando un mensaje en `*err_msg').
 */
static int
create_passive_socket(const char *addr, const unsigned short port, const char **err_msg) {
    char port_str[PORT_STR_LEN];
    snprintf(port_str, sizeof(port_str), "%hu", port);

    struct addrinfo hints = {
        .ai_family   = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
        .ai_flags    = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV,
    };
    struct addrinfo *res = NULL;
    if (getaddrinfo(addr, port_str, &hints, &res) != 0 || res == NULL) {
        errno    = EINVAL;
        *err_msg = "invalid listening address";
        return -1;
    }

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        *err_msg = "unable to create socket";
        goto fail;
    }

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
    if (res->ai_family == AF_INET6) {
        // "::" acepta también IPv4 (dual stack)
        setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &(int){ 0 }, sizeof(int));
    }

    if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
        *err_msg = "unable to bind socket";
        goto fail;
    }
    if (listen(fd, LISTEN_BACKLOG) < 0) {
        *err_msg = "unable to listen";
        goto fail;
    }
    if (selector_fd_set_nio(fd) == -1) {
        *err_msg = "unable to set socket as non-blocking";
        goto fail;
    }

    freeaddrinfo(res);
    return fd;

fail:
    if (fd >= 0) {
        close(fd);
    }
    freeaddrinfo(res);
    return -1;
}

int
main(const int argc, char **argv) {
    struct socks5args args;
    parse_args(argc, argv, &args);

    // no tenemos nada que leer de stdin
    close(STDIN_FILENO);

    const char      *err_msg  = NULL;
    selector_status  ss       = SELECTOR_SUCCESS;
    fd_selector      selector = NULL;
    int              socks_fd = -1;
    int              mgmt_fd  = -1;
    int              ret      = 0;

    // TODO: inicializar módulos (usuarios desde args.users, métricas, logger)

    // registrar SIGTERM/SIGINT permite terminar limpiamente (útil con valgrind).
    // SIGPIPE se ignora: escribir en un socket cerrado debe dar EPIPE, no matar el proceso.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);
    signal(SIGPIPE, SIG_IGN);

    socks_fd = create_passive_socket(args.socks_addr, args.socks_port, &err_msg);
    if (socks_fd < 0) {
        goto finally;
    }
    fprintf(stdout, "SOCKSv5 listening on %s:%hu\n", args.socks_addr, args.socks_port);

    mgmt_fd = create_passive_socket(args.mng_addr, args.mng_port, &err_msg);
    if (mgmt_fd < 0) {
        goto finally;
    }
    fprintf(stdout, "Management listening on %s:%hu\n", args.mng_addr, args.mng_port);

    const struct selector_init conf = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec  = SELECT_TIMEOUT_S,
            .tv_nsec = 0,
        },
    };
    if (selector_init(&conf) != SELECTOR_SUCCESS) {
        err_msg = "initializing selector";
        goto finally;
    }

    selector = selector_new(SELECTOR_INITIAL);
    if (selector == NULL) {
        err_msg = "unable to create selector";
        goto finally;
    }

    const struct fd_handler socks_handler = {
        .handle_read  = socksv5_passive_accept,
        .handle_write = NULL,
        .handle_close = NULL, // nada que liberar
    };
    ss = selector_register(selector, socks_fd, &socks_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        err_msg = "registering SOCKS fd";
        goto finally;
    }

    const struct fd_handler mgmt_handler = {
        .handle_read  = mgmt_passive_accept,
        .handle_write = NULL,
        .handle_close = NULL, // nada que liberar
    };
    ss = selector_register(selector, mgmt_fd, &mgmt_handler, OP_READ, NULL);
    if (ss != SELECTOR_SUCCESS) {
        err_msg = "registering management fd";
        goto finally;
    }

    while (!done) {
        err_msg = NULL;
        ss = selector_select(selector);
        if (ss != SELECTOR_SUCCESS) {
            err_msg = "serving";
            goto finally;
        }
    }
    fprintf(stdout, "Signal received, cleaning up and exiting\n");

finally:
    if (ss != SELECTOR_SUCCESS) {
        fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "" : err_msg,
                ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
        ret = 2;
    } else if (err_msg != NULL) {
        perror(err_msg);
        ret = 1;
    }

    if (selector != NULL) {
        selector_destroy(selector);
    }
    selector_close();

    socksv5_pool_destroy();
    // TODO: liberar módulos (usuarios, métricas, logger)

    if (socks_fd >= 0) {
        close(socks_fd);
    }
    if (mgmt_fd >= 0) {
        close(mgmt_fd);
    }
    return ret;
}
