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

    /** pedidos que terminaron con REP distinto de SOCKS5_REPLY_SUCCEEDED */
    uint64_t failed_requests;
    /** intentos de autenticación fallidos */
    uint64_t auth_failures;
};

/** inicializa los contadores en 0 y registra `start_time' */
void metrics_init(void);

void metrics_connection_opened(void);
void metrics_connection_closed(void);

void metrics_add_bytes_client_to_origin(size_t n);
void metrics_add_bytes_origin_to_client(size_t n);

void metrics_request_failed(void);
void metrics_auth_failed(void);

/** copia el estado actual en `out' */
void metrics_get(struct metrics_snapshot *out);

#endif
