#include "bloque.h"

BLOQUE* asignarTamanioBloque(uint32_t tamanioBloqueEnArchivoConfig);

int crearArchivoDeBloques(char* blocksFilePath, uint32_t cantDeBloquesTotales, uint32_t tamanioBloque) {
    FILE* archivoDeBloques = fopen(blocksFilePath, "wb");

    if (archivoDeBloques == NULL) {
        perror("Error al abrir el archivo de bloques");
        return 1;
    }

    for (uint32_t i = 0; i < cantDeBloquesTotales; i++) {
        BLOQUE* bloque = asignarTamanioBloque(tamanioBloque);
        fwrite(bloque->valorDeBloque, tamanioBloque, 1, archivoDeBloques);
        //fwrite(bloque, sizeof(BLOQUE), 1, archivoDeBloques);
        free(bloque->valorDeBloque);
        free(bloque);
    }

    fclose(archivoDeBloques);
    return 0;
}

BLOQUE* asignarTamanioBloque(uint32_t tamanioBloqueEnArchivoConfig) {
    BLOQUE* bloque = malloc(sizeof(BLOQUE));

    if (bloque == NULL) {
        return NULL;
    }

    bloque->valorDeBloque = malloc(tamanioBloqueEnArchivoConfig);
    if (bloque->valorDeBloque == NULL) {
        free(bloque);
        return NULL;
    }

    return bloque;
}

void modificarBLOQUEenArchivoBLOQUE(char* pathBLOQUES,uint32_t numeroBloque, BLOQUE* nuevaEntrada){
    
    // Abrir el archivo en modo lectura y escritura binaria
    FILE *archivoBLOQUE = fopen(pathBLOQUES, "r+b");

    if (archivoBLOQUE == NULL) {
        perror("Error al abrir el archivo FAT");
    }

    // Mover el puntero de archivo a la posición de la entrada deseada
    fseek(archivoBLOQUE, (numeroBloque) * sizeof(BLOQUE), SEEK_SET);

    // Escribir la nueva entrada en el archivo
    fwrite(&nuevaEntrada, sizeof(BLOQUE), 1, archivoBLOQUE);

    // Cerrar el archivo
    fclose(archivoBLOQUE);

}


BLOQUE** leerBloquesDesdeArchivo(char* pathBLOQUES, uint32_t cantDeBloquesTotales, uint32_t tamanioBloque) {
    // Abrir el archivo en modo lectura binaria
    FILE* archivoBLOQUE = fopen(pathBLOQUES, "rb");

    if (archivoBLOQUE == NULL) {
        perror("Error al abrir el archivo de bloques");
        return NULL;
    }

    // Crear un array de punteros a estructuras BLOQUE
    BLOQUE** bloques = malloc(cantDeBloquesTotales * sizeof(BLOQUE*));

    if (bloques == NULL) {
        fclose(archivoBLOQUE);
        return NULL;
    }

    // Leer cada bloque desde el archivo y almacenarlo en el array
    for (uint32_t i = 0; i < cantDeBloquesTotales; i++) {
        // Mover el puntero de archivo a la posición del bloque
        fseek(archivoBLOQUE, i * tamanioBloque, SEEK_SET);

        // Crear una estructura BLOQUE
        bloques[i] = asignarTamanioBloque(tamanioBloque);

        if (bloques[i] == NULL) {
            fclose(archivoBLOQUE);
            // Liberar la memoria asignada hasta ahora en caso de error
            for (uint32_t j = 0; j < i; j++) {
                free(bloques[j]);
            }
            free(bloques);
            return NULL;
        }

        // Leer el bloque desde el archivo
        fread(bloques[i]->valorDeBloque, tamanioBloque, 1, archivoBLOQUE);
    }

    // Cerrar el archivo
    fclose(archivoBLOQUE);

    return bloques;
}


