/**
 * users.c - usuarios habilitados para usar el proxy.
 *
 * Un arreglo fijo en memoria: no hay malloc, así que no hay nada que pueda
 * fallar ni perder memoria. Los cambios hechos por management se pierden al
 * reiniciar (los usuarios iniciales vienen de -u).
 *
 * No hay mutex: todo se invoca desde el hilo del selector (ver users.h).
 */
#include <string.h>

#include "users.h"

/** máximo de usuarios registrados a la vez */
#define USERS_CAPACITY 128

struct user_entry {
    char name[USERS_MAX_NAME_LEN + 1];
    char pass[USERS_MAX_PASS_LEN + 1];
};

static struct user_entry users[USERS_CAPACITY];
static size_t            n_users;

/**
 * true si `s' tiene entre 1 y `max' caracteres imprimibles.
 * ':' se rechaza en el nombre porque -u usa el formato nombre:clave.
 */
static bool
valid_field(const char *s, const size_t max, const bool is_name) {
    if (s == NULL) {
        return false;
    }
    size_t len = 0;
    for (; s[len] != '\0'; len++) {
        const unsigned char c = (unsigned char) s[len];
        if (len >= max || c < 0x20 || c == 0x7F || (is_name && c == ':')) {
            return false;
        }
    }
    return len > 0;
}

/** posición del usuario `name', o -1 si no existe */
// ponytail: búsqueda lineal, O(n) con n <= USERS_CAPACITY; tabla hash si crece mucho
static int
find(const char *name) {
    for (size_t i = 0; i < n_users; i++) {
        if (strcmp(users[i].name, name) == 0) {
            return (int) i;
        }
    }
    return -1;
}

bool
users_init(void) {
    n_users = 0;
    return true;
}

void
users_destroy(void) {
    // borramos las contraseñas de la memoria antes de terminar
    memset(users, 0, sizeof(users));
    n_users = 0;
}

enum users_status
users_add(const char *name, const char *pass) {
    if (!valid_field(name, USERS_MAX_NAME_LEN, true)
     || !valid_field(pass, USERS_MAX_PASS_LEN, false)) {
        return USERS_INVALID;
    }
    if (find(name) >= 0) {
        return USERS_EXISTS;
    }
    if (n_users == USERS_CAPACITY) {
        return USERS_FULL;
    }
    // los largos ya se validaron: entran con su '\0'
    strcpy(users[n_users].name, name);
    strcpy(users[n_users].pass, pass);
    n_users++;
    return USERS_OK;
}

enum users_status
users_remove(const char *name) {
    if (name == NULL) {
        return USERS_INVALID;
    }
    const int i = find(name);
    if (i < 0) {
        return USERS_NOT_FOUND;
    }
    // el último ocupa el hueco: O(1), el orden no importa
    n_users--;
    users[i] = users[n_users];
    memset(&users[n_users], 0, sizeof(users[n_users]));
    return USERS_OK;
}

enum users_status
users_set_password(const char *name, const char *pass) {
    if (name == NULL || !valid_field(pass, USERS_MAX_PASS_LEN, false)) {
        return USERS_INVALID;
    }
    const int i = find(name);
    if (i < 0) {
        return USERS_NOT_FOUND;
    }
    strcpy(users[i].pass, pass);
    return USERS_OK;
}

bool
users_authenticate(const char *name, const char *pass) {
    if (name == NULL || pass == NULL) {
        return false;
    }
    const int i = find(name);
    return i >= 0 && strcmp(users[i].pass, pass) == 0;
}

size_t
users_count(void) {
    return n_users;
}

void
users_foreach(void (*cb)(const char *name, void *data), void *data) {
    for (size_t i = 0; i < n_users; i++) {
        cb(users[i].name, data);
    }
}
