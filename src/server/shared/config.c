/**
 * config.c - configuración del servidor modificable en tiempo de ejecución.
 *
 * Igual que metrics.c: el estado es `static' y solo se modifica a través de
 * las funciones de config.h, que validan los valores antes de aceptarlos.
 */
#include "config.h"

static struct server_config cfg;

void
config_init(const struct socks5args *args) {
    cfg.io_buffer_size    = CONFIG_DEFAULT_IO_BUFFER_SIZE;
    cfg.disectors_enabled = args->disectors_enabled;
}

const struct server_config *
config_get(void) {
    return &cfg;
}

bool
config_set_io_buffer_size(const size_t size) {
    if (size < CONFIG_MIN_IO_BUFFER_SIZE || size > CONFIG_MAX_IO_BUFFER_SIZE) {
        return false;
    }
    cfg.io_buffer_size = size;
    return true;
}

void
config_set_disectors_enabled(const bool enabled) {
    cfg.disectors_enabled = enabled;
}
