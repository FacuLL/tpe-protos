#ifndef SOCKS5NIO_H
#define SOCKS5NIO_H

#include "selector.h"

/**
 * socks5nio - atención de conexiones SOCKSv5 (RFC 1928 / RFC 1929)
 * usando sockets no bloqueantes y el selector.
 */

/** handler de lectura del socket pasivo SOCKS: acepta una conexión nueva */
void socksv5_passive_accept(struct selector_key *key);

/** libera los recursos globales (pool de conexiones) al terminar */
void socksv5_pool_destroy(void);

#endif
