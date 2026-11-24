#ifndef CONFIG_H
#define CONFIG_H

/**
 * config.h - configuración del servidor modificable en tiempo de ejecución.
 *
 * Se inicializa desde los argumentos de línea de comandos y management puede
 * cambiarla sin reiniciar. Los cambios aplican a las conexiones nuevas.
 */
#include <stdbool.h>
#include <stddef.h>

#include "args.h"

#define CONFIG_DEFAULT_IO_BUFFER_SIZE 4096
#define CONFIG_MIN_IO_BUFFER_SIZE     512
#define CONFIG_MAX_IO_BUFFER_SIZE     (1 << 20)

struct server_config {
    /** tamaño de cada buffer de I/O por conexión, en bytes */
    size_t io_buffer_size;
    /** disectores de credenciales (POP3) habilitados; se apaga con -N */
    bool   disectors_enabled;
};

/** carga los valores iniciales desde los argumentos */
void config_init(const struct socks5args *args);

/** configuración vigente (solo lectura) */
const struct server_config *config_get(void);

/** false si `size' está fuera de [CONFIG_MIN_IO_BUFFER_SIZE, CONFIG_MAX_IO_BUFFER_SIZE] */
bool config_set_io_buffer_size(size_t size);

void config_set_disectors_enabled(bool enabled);

#endif
