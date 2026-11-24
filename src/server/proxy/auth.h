#ifndef AUTH_H
#define AUTH_H

/**
 * auth.h - parser de la sub-negociación usuario/contraseña (RFC 1929).
 *
 * Cliente -> servidor:
 *
 *   +----+------+----------+------+----------+
 *   |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
 *   +----+------+----------+------+----------+
 *   | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
 *   +----+------+----------+------+----------+
 *
 * Servidor -> cliente:
 *
 *   +----+--------+
 *   |VER | STATUS |
 *   +----+--------+
 *   | 1  |   1    |
 *   +----+--------+
 *
 * Al terminar, `username' y `password' quedan como strings terminados en '\0'.
 */
#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"
#include "socks5.h"

enum auth_state {
    AUTH_VERSION,
    AUTH_ULEN,
    AUTH_UNAME,
    AUTH_PLEN,
    AUTH_PASSWD,
    AUTH_DONE,
    AUTH_ERROR_UNSUPPORTED_VERSION,
    /** ULEN o PLEN en 0 */
    AUTH_ERROR_INVALID_LENGTH,
};

struct auth_parser {
    char username[SOCKS5_AUTH_MAX_LEN + 1];
    char password[SOCKS5_AUTH_MAX_LEN + 1];

    /* ---- privado ---- */
    enum auth_state state;
    /** largo del campo que se está leyendo */
    uint8_t len;
    /** bytes leídos del campo actual */
    uint8_t read;
};

void auth_parser_init(struct auth_parser *p);

enum auth_state auth_parser_feed(struct auth_parser *p, uint8_t b);

enum auth_state auth_consume(buffer *b, struct auth_parser *p, bool *errored);

bool auth_is_done(enum auth_state state, bool *errored);

const char *auth_error(const struct auth_parser *p);

void auth_parser_close(struct auth_parser *p);

/**
 * serializa la respuesta (VER, STATUS) en `b'.
 * `status' es SOCKS5_AUTH_SUCCESS o SOCKS5_AUTH_FAILURE.
 *
 * @return cantidad de bytes escritos, o -1 si no hay lugar en el buffer.
 */
int auth_marshall(buffer *b, uint8_t status);

#endif
