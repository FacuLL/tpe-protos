/**
 * metrics.c - métricas de operación del servidor (volátiles).
 *
 * Todo el estado vive en una variable `static' de este archivo: solo se
 * modifica a través de las funciones de metrics.h.
 *
 * No hay mutex porque todas las llamadas ocurren en el hilo del selector
 * (ver metrics.h).
 */
#include <string.h>

#include "metrics.h"

static struct metrics_snapshot m;

void
metrics_init(void) {
    memset(&m, 0, sizeof(m));
    m.start_time = time(NULL);
}

void
metrics_connection_opened(void) {
    m.historic_connections++;
    m.current_connections++;
    if (m.current_connections > m.max_concurrent_connections) {
        m.max_concurrent_connections = m.current_connections;
    }
}

void
metrics_connection_closed(void) {
    // un close de más no debe hacer que el contador sin signo dé la vuelta
    if (m.current_connections > 0) {
        m.current_connections--;
    }
}

void
metrics_add_bytes_client_to_origin(const size_t n) {
    m.bytes_client_to_origin += n;
}

void
metrics_add_bytes_origin_to_client(const size_t n) {
    m.bytes_origin_to_client += n;
}

void
metrics_request_replied(const uint8_t reply) {
    // un código fuera de rango escribiría fuera del arreglo
    if (reply < METRICS_REPLY_CODES) {
        m.replies[reply]++;
    }
}

void
metrics_auth_failed(void) {
    m.auth_failures++;
}

void
metrics_log_line_dropped(void) {
    m.log_lines_dropped++;
}

void
metrics_get(struct metrics_snapshot *out) {
    *out = m;
}
