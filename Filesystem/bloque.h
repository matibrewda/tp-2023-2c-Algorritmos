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
    int8_t* valorDeBloque;  // Puntero a datos de tamaño variable
} BLOQUE;

int crearArchivoDeBloques(char* blocksFilePath, uint32_t cantDeBloquesTotales, uint32_t tamanioBloque);
BLOQUE* asignarTamanioBloque(uint32_t tamanioBloqueEnArchivoConfig);
BLOQUE** leerBloquesDesdeArchivo(char* pathBLOQUES, uint32_t cantDeBloquesTotales, uint32_t tamanioBloque);
int modificarBLOQUEenArchivoBLOQUE(char* pathBLOQUES, uint32_t numeroBloque, BLOQUE* nuevaEntrada, u_int32_t tamanioBloque);
void modificarBloque (char* path);
#endif /* BLOQUE_H_ */