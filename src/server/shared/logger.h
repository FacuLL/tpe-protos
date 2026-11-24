#ifndef LOGGER_H
#define LOGGER_H

/**
 * logger.h - registro de accesos y de credenciales capturadas.
 *
 * Escribir en un archivo/stdout puede bloquear, así que el logger encola las
 * líneas en memoria y las vuelca registrando su fd en el selector (OP_WRITE).
 * Cada línea lleva timestamp (lo agrega el logger).
 */
#include <stdbool.h>
#include <stdint.h>

#include <sys/socket.h>

#include "selector.h"

/**
 * inicializa el logger.
 *
 * @param path archivo donde se escribe el registro; NULL usa stdout.
 */
bool logger_init(fd_selector s, const char *path);

/** vuelca lo pendiente (puede bloquear: solo al terminar) y libera recursos */
void logger_destroy(void);

/**
 * registra un acceso a través del proxy.
 *
 * @param user      usuario autenticado, o NULL si no hubo autenticación
 * @param client    dirección del cliente SOCKS
 * @param dest_host destino pedido (FQDN o IP en texto)
 * @param dest_port puerto destino en host byte order
 * @param reply     código REP con que se respondió (enum socks5_reply)
 */
void log_access(const char *user, const struct sockaddr *client,
                const char *dest_host, uint16_t dest_port, uint8_t reply);

/**
 * registra credenciales capturadas por un disector (p. ej. POP3).
 *
 * @param protocol nombre del protocolo ("POP3")
 */
void log_credentials(const char *user, const struct sockaddr *client,
                     const char *dest_host, uint16_t dest_port,
                     const char *protocol,
                     const char *username, const char *password);

#endif
