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


