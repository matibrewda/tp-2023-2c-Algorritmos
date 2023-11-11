#ifndef FAT_H_
#define FAT_H_

#include "commons/config.h"
#include "commons/log.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//ubicar Struct y HEADERS de funciones de utilidad

typedef struct {
    uint32_t block_value;  // Valor de la entrada de la FAT
} FATEntry;

int iniciarFAT(t_log *logger, char *fat_path, uint32_t cant_bloques_total, uint32_t cant_bloques_swap, uint32_t tamanio_bloque);
#endif /* FAT_H_ */