#ifndef DIRECTORIO_H_
#define DIRECTORIO_H_

#include "stdint.h"
#include "commons/config.h"
#include "stdlib.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//ENTRADA DE CADA ARCHIVO.
typedef struct {
    char nombreArchivo[256]; 
    uint32_t numBloqueInicial;
} EntradaDirectorio;

// Definición de la estructura de  array de directorios
typedef struct {
    EntradaDirectorio* directorios;
    size_t cantidadDirectorios;
} DirectorioArray;

void agregarEntradaDesdeFCB(DirectorioArray* array, char* rutaArchivoFCB);

void procesarArchivosEnDirectorio(DirectorioArray* directorio, char* rutaDirectorio);

void liberarDirectorioArray(DirectorioArray* array);
DirectorioArray inicializarDirectorioArray();


#endif /* DIRECTORIO_H_ */