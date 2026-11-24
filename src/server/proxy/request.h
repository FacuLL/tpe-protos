#ifndef REQUEST_H
#define REQUEST_H

/**
 * request.h - parser del pedido SOCKSv5 (RFC 1928, sección 4) y
 * serialización de la respuesta (sección 6).
 *
 * Cliente -> servidor:
 *
 *   +----+-----+-------+------+----------+----------+
 *   |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
 *   +----+-----+-------+------+----------+----------+
 *   | 1  |  1  | X'00' |  1   | Variable |    2     |
 *   +----+-----+-------+------+----------+----------+
 *
 * Servidor -> cliente:
 *
 *   +----+-----+-------+------+----------+----------+
 *   |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
 *   +----+-----+-------+------+----------+----------+
 *   | 1  |  1  | X'00' |  1   | Variable |    2     |
 *   +----+-----+-------+------+----------+----------+
 *
 * El parser NO falla ante un CMD desconocido: termina de leer el pedido y
 * quien lo usa responde SOCKS5_REPLY_COMMAND_NOT_SUPPORTED. Sí falla ante un
 * ATYP desconocido, porque no se puede saber el largo de DST.ADDR.
 */
#include <stdbool.h>
#include <stdint.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include "buffer.h"
#include "socks5.h"

/** dirección destino; el miembro válido lo indica `dest_atyp' */
union socks5_addr {
    /** FQDN terminado en '\0' (largo máximo 255) */
    char                fqdn[256];
    struct sockaddr_in  ipv4;
    struct sockaddr_in6 ipv6;
};

struct request {
    uint8_t           cmd;
    uint8_t           dest_atyp;
    union socks5_addr dest_addr;
    /** en network byte order */
    in_port_t         dest_port;
};

enum request_state {
    REQUEST_VERSION,
    REQUEST_CMD,
    REQUEST_RSV,
    REQUEST_ATYP,
    REQUEST_DSTADDR_FQDN_LEN,
    REQUEST_DSTADDR,
    REQUEST_DSTPORT,
    REQUEST_DONE,
    REQUEST_ERROR_UNSUPPORTED_VERSION,
    REQUEST_ERROR_UNSUPPORTED_ATYP,
    /** FQDN de largo 0 */
    REQUEST_ERROR_INVALID_LENGTH,
};

struct request_parser {
    /** dónde se guarda el pedido; lo provee quien usa el parser */
    struct request *request;

    /* ---- privado ---- */
    enum request_state state;
    /** largo del campo que se está leyendo */
    uint8_t len;
    /** bytes leídos del campo actual */
    uint8_t read;
};

/** inicializa el parser. `p->request' debe estar seteado */
void request_parser_init(struct request_parser *p);

enum request_state request_parser_feed(struct request_parser *p, uint8_t b);

enum request_state request_consume(buffer *b, struct request_parser *p, bool *errored);

bool request_is_done(enum request_state state, bool *errored);

const char *request_error(const struct request_parser *p);

void request_parser_close(struct request_parser *p);

/**
 * serializa la respuesta en `b'.
 *
 * @param bnd dirección local del socket hacia el origen (BND.ADDR/BND.PORT).
 *            NULL serializa 0.0.0.0:0 (p. ej. en respuestas de error).
 *
 * @return cantidad de bytes escritos, o -1 si no hay lugar en el buffer.
 */
int request_marshall(buffer *b, enum socks5_reply reply, const struct sockaddr *bnd);

/** traduce un errno de connect()/getsockopt(SO_ERROR) al código REP */
enum socks5_reply errno_to_socks5_reply(int e);

#endif
