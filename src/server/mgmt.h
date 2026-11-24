#ifndef MGMT_H
#define MGMT_H

#include "selector.h"

/**
 * mgmt - protocolo propio de monitoreo y configuración
 * (métricas, usuarios, configuración en tiempo de ejecución).
 */

/** handler de lectura del socket pasivo de management: acepta una conexión nueva */
void mgmt_passive_accept(struct selector_key *key);

#endif
