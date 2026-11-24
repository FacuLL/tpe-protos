#ifndef HELLO_H
#define HELLO_H

/**
 * hello.h - parser del mensaje de negociación de método (RFC 1928, sección 3).
 *
 * Cliente -> servidor:
 *
 *   +----+----------+----------+
 *   |VER | NMETHODS | METHODS  |
 *   +----+----------+----------+
 *   | 1  |    1     | 1 to 255 |
 *   +----+----------+----------+
 *
 * Servidor -> cliente:
 *
 *   +----+--------+
 *   |VER | METHOD |
 *   +----+--------+
 *   | 1  |   1    |
 *   +----+--------+
 *
 * El parser consume de a un byte, por lo que tolera lecturas parciales.
 * Por cada método recibido llama a `on_authentication_method'; quien lo usa
 * decide cuál elegir.
 */
#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"

enum hello_state {
    HELLO_VERSION,
    HELLO_NMETHODS,
    HELLO_METHODS,
    HELLO_DONE,
    HELLO_ERROR_UNSUPPORTED_VERSION,
};

struct hello_parser {
    /** invocado por cada método que ofrece el cliente */
    void (*on_authentication_method)(struct hello_parser *p, uint8_t method);

    /** dato de uso libre para el callback */
    void *data;

    /* ---- privado ---- */
    enum hello_state state;
    /** métodos que faltan leer */
    uint8_t remaining;
};

/** inicializa el parser (los campos públicos se setean antes o después) */
void hello_parser_init(struct hello_parser *p);

/** entrega un byte al parser; retorna el nuevo estado */
enum hello_state hello_parser_feed(struct hello_parser *p, uint8_t b);

/**
 * consume bytes de `b' hasta terminar el mensaje o vaciar el buffer.
 * Si termina con error deja `*errored' en true.
 */
enum hello_state hello_consume(buffer *b, struct hello_parser *p, bool *errored);

/**
 * indica si el estado es terminal. Si `errored' no es NULL indica
 * además si terminó por error.
 */
bool hello_is_done(enum hello_state state, bool *errored);

/** descripción humana del error en que quedó el parser */
const char *hello_error(const struct hello_parser *p);

/** libera recursos internos del parser */
void hello_parser_close(struct hello_parser *p);

/**
 * serializa la respuesta (VER, METHOD) en `b'.
 *
 * @return cantidad de bytes escritos, o -1 si no hay lugar en el buffer.
 */
int hello_marshall(buffer *b, uint8_t method);

#endif
