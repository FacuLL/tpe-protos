#ifndef METRICS_H
#define METRICS_H

/**
 * metrics.h - métricas de operación del servidor (volátiles).
 *
 * El proxy las actualiza y management las expone. Todas las funciones se
 * invocan desde el hilo del selector (nunca desde el hilo de resolución DNS).
 */
#include <stddef.h>
#include <stdint.h>
#include <time.h>

/** cantidad de códigos REP de RFC 1928 (0x00 a 0x08) */
#define METRICS_REPLY_CODES 9

struct metrics_snapshot {
    /** momento en que arrancó el servidor */
    time_t   start_time;

    /** conexiones SOCKS aceptadas desde el arranque */
    uint64_t historic_connections;
    /** conexiones SOCKS abiertas ahora */
    uint64_t current_connections;
    /** máximo de conexiones simultáneas alcanzado */
    uint64_t max_concurrent_connections;

    /** bytes cliente -> origen */
    uint64_t bytes_client_to_origin;
    /** bytes origen -> cliente */
    uint64_t bytes_origin_to_client;

    /**
     * respuestas enviadas, indexadas por código REP (enum socks5_reply).
     * replies[0] son los éxitos; la suma de replies[1..8] son los fallos.
     */
    uint64_t replies[METRICS_REPLY_CODES];
    /** intentos de autenticación fallidos */
    uint64_t auth_failures;

    /** líneas del registro de acceso descartadas por tener la cola llena */
    uint64_t log_lines_dropped;
};

/** inicializa los contadores en 0 y registra `start_time' */
void metrics_init(void);

void metrics_connection_opened(void);
void metrics_connection_closed(void);

void metrics_add_bytes_client_to_origin(size_t n);
void metrics_add_bytes_origin_to_client(size_t n);

/** cuenta una respuesta al pedido SOCKS con código `reply'; ignora códigos fuera de rango */
void metrics_request_replied(uint8_t reply);
void metrics_auth_failed(void);
void metrics_log_line_dropped(void);

/** copia el estado actual en `out' */
void metrics_get(struct metrics_snapshot *out);

#endif
