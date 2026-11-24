#ifndef USERS_H
#define USERS_H

/**
 * users.h - usuarios habilitados para usar el proxy.
 *
 * Lo usan el proxy (autenticación RFC 1929) y management (alta, baja, listado
 * en tiempo de ejecución). Todo se invoca desde el hilo del selector.
 */
#include <stdbool.h>
#include <stddef.h>

/** largos máximos (los impone RFC 1929: campo de 1 byte) */
#define USERS_MAX_NAME_LEN 255
#define USERS_MAX_PASS_LEN 255

enum users_status {
    USERS_OK,
    /** ya existe un usuario con ese nombre */
    USERS_EXISTS,
    /** no existe un usuario con ese nombre */
    USERS_NOT_FOUND,
    /** no hay lugar para más usuarios */
    USERS_FULL,
    /** nombre o contraseña vacíos, demasiado largos o con caracteres inválidos */
    USERS_INVALID,
};

/** inicializa el módulo sin usuarios */
bool users_init(void);

/** libera todos los recursos */
void users_destroy(void);

/** agrega un usuario; copia los strings */
enum users_status users_add(const char *name, const char *pass);

/** borra un usuario */
enum users_status users_remove(const char *name);

/** cambia la contraseña de un usuario existente */
enum users_status users_set_password(const char *name, const char *pass);

/** true si existe el usuario y la contraseña coincide */
bool users_authenticate(const char *name, const char *pass);

/** cantidad de usuarios registrados */
size_t users_count(void);

/** invoca `cb' por cada usuario (para listarlos); no expone contraseñas */
void users_foreach(void (*cb)(const char *name, void *data), void *data);

#endif
