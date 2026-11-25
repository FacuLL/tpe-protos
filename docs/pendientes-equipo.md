# Pendientes para coordinar entre módulos

## Toba (management)

- **Autenticación de administrador.** El protocolo de management debe pedir
  usuario y clave de administrador, separados de los usuarios del proxy.
  Escuchar en `127.0.0.1` no alcanza:
  - con `-L 0.0.0.0` management queda expuesto a toda la red;
  - un usuario del proxy puede hacer `CONNECT 127.0.0.1:8080` y llegar a
    management a través del propio proxy (la conexión llega desde loopback).
- **Métricas:** `metrics_get()` devuelve una copia. Cambió el struct:
  - `failed_requests` ahora es `replies[METRICS_REPLY_CODES]` (contador por
    código REP; `replies[0]` son éxitos, la suma de `replies[1..8]` los fallos);
  - nuevo campo `log_lines_dropped`.
  - Valores derivados (bytes totales, tasa de error, uptime) se calculan al
    mostrar, no se guardan.
- **Config:** `config_set_io_buffer_size()` devuelve `false` si el valor está
  fuera de `[512, 1 MiB]`; responder error al cliente en ese caso. Los cambios
  aplican solo a conexiones nuevas.

## Facu (proxy)

- **Rechazar CONNECT a management.** Si el destino del request es la dirección
  y puerto de management, responder REP `0x02` (connection not allowed) sin
  conectar.
- **Métricas:** `metrics_request_failed()` se reemplazó por
  `metrics_request_replied(uint8_t reply)`: llamarla siempre que se envía el
  reply del request, con el código REP (éxito o error).
- **Config:** leer `config_get()->io_buffer_size` al crear cada conexión, no
  una sola vez al arrancar.
