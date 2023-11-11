#ifndef BLOQUE_H_
#define BLOQUE_H_

#include "commons/config.h"
#include "commons/log.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//ubicar Struct y HEADERS de funciones de utilidad



 typedef struct {
    uint8_t* valorDeBloque;  // Puntero a datos de tamaño variable
} BLOQUE;

int crearArchivoDeBloques(char* blocksFilePath, uint32_t cantDeBloquesTotales, uint32_t tamanioBloque);

#endif /* BLOQUE_H_ */