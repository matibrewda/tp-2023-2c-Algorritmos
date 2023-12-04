#ifndef FCB_H_
#define FCB_H_

#include "stdint.h"
#include "commons/config.h"
#include "stdlib.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define la estructura FCB
	typedef struct {
    char* nombre_archivo;  // Nombre del archivo (puntero a char)
    uint32_t tamanio_archivo;  // Tamaño del archivo en bytes
    uint32_t bloque_inicial;   // Número de bloque inicial
} FCB;

uint32_t verificarSiExisteFCBdeArchivo(char* rutaCompleta);
FCB* iniciar_fcb(char* nombre_archivo, uint32_t tamanio_archivo, uint32_t bloque_inicial);
FCB* abrir_fcb(char* ruta_archivo);
void guardar_fcb_en_archivo(FCB* fcb, char* ruta_archivo);

#endif /* FCB_H_ */
