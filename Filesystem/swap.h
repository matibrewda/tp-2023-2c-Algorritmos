#ifndef SWAP_H_
#define SWAP_H_

#include "commons/config.h"
#include "commons/log.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//ubicar Struct y HEADERS de funciones de utilidad

typedef struct {
    uint32_t block_value;  // Valor de la entrada 
} SWAPEntry;

int iniciarSWAP (t_log* logger,char* blocks_file_path, uint32_t cant_bloques_swap);

#endif /* SWAP_H_ */