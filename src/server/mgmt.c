/**
 * mgmt.c - servicio de monitoreo y configuración.
 *
 * STUB: por ahora acepta y cierra la conexión.
 *
 * TODO: diseñar e implementar el protocolo.
 */
#include <sys/socket.h>
#include <unistd.h>

#include "mgmt.h"

void
mgmt_passive_accept(struct selector_key *key) {
    const int client = accept(key->fd, NULL, NULL);
    if (client >= 0) {
        close(client);
    }
}
