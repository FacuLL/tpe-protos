/**
 * socks5nio.c - atención de conexiones SOCKSv5.
 *
 * STUB: por ahora acepta y cierra la conexión, para que el servidor
 * compile y el socket pasivo no quede eternamente "listo para leer".
 *
 * TODO: máquina de estados (hello, auth, request, resolve, connect, copy).
 */
#include <sys/socket.h>
#include <unistd.h>

#include "socks5nio.h"

void
socksv5_passive_accept(struct selector_key *key) {
    const int client = accept(key->fd, NULL, NULL);
    if (client >= 0) {
        close(client);
    }
}

void
socksv5_pool_destroy(void) {
    // nada que liberar todavía
}
