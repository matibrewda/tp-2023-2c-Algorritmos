#ifndef FAT_H_
#define FAT_H_

#include "commons/config.h"
#include "commons/log.h"
#include "string.h"
#include "bloque.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//ubicar Struct y HEADERS de funciones de utilidad

typedef struct {
    int32_t block_value;  // Valor de la entrada de la FAT
} FATEntry;

void modificarFATenArchivoFAT(const char* pathFAT, uint32_t numeroBloque, FATEntry *nuevaEntrada);
int iniciarFAT(t_log *logger, char *fat_path, uint32_t cant_bloques_total, uint32_t cant_bloques_swap, uint32_t tamanio_bloque);
FATEntry* abrirFAT(char *fat_path, uint32_t cant_bloques_total, uint32_t cant_bloques_swap);
void cerrarFAT(FATEntry *arreglo);
int buscarBloqueLibre(FATEntry *fat, size_t total_blocks);
void asignarBloques(char* pathFAT, char* pathBLOQUES, char* pathFCB, BLOQUE *bloques[],FATEntry fat[],size_t cantBloquesTotales,size_t cantBLoquesSWAP);
void eliminarBloques(char* pathFAT, char* pathFCB, FATEntry *fat[],size_t cantBloquesTotales,size_t cantBLoquesSWAP);
#endif /* FAT_H_ */

