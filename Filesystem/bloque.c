#include "bloque.h"

BLOQUE* asignarTamanioBloque(uint32_t tamanioBloqueEnArchivoConfig);

/* int crearArchivoDeBloques(char* blocksFilePath, uint32_t cantDeBloquesTotales, uint32_t tamanioBloque) {
    FILE* archivoDeBloques = fopen(blocksFilePath, "wb");

    if (archivoDeBloques == NULL) {
        perror("Error al abrir el archivo de bloques");
        return 1;
    }

    for (uint32_t ai = 0; i < cantDeBloquesTotales; i++) {
        BLOQUE* bloque = asignarTamanioBloque(tamanioBloque);
        fwrite(bloque, sizeof(BLOQUE), 1, archivoDeBloques);
        //fwrite(bloque, sizeof(BLOQUE), 1, archivoDeBloques);
        free(bloque->valorDeBloque);
        free(bloque);
    }

    fclose(archivoDeBloques);
    return 0;
} */

int crearArchivoDeBloques1(char* blocksFilePath, uint32_t cantDeBloquesTotales, uint32_t tamanioBloque) {
    FILE* archivoDeBloques = fopen(blocksFilePath, "wb");

    if (archivoDeBloques == NULL) {
        perror("Error al abrir el archivo de bloques");
        return 1;
    }

    for (uint32_t i = 0; i < cantDeBloquesTotales; i++) {
        BLOQUE* bloque = asignarTamanioBloque(tamanioBloque);
        bloque->valorDeBloque[i] = 'N';

        if (bloque == NULL) {
            // Manejar el error aquí, liberar memoria y cerrar el archivo si es necesario
            fclose(archivoDeBloques);
            return 1;
        }

        fwrite(bloque->valorDeBloque, tamanioBloque * sizeof(uint8_t), 1, archivoDeBloques);

        free(bloque->valorDeBloque);
        free(bloque);
    }

    fclose(archivoDeBloques);
    return 0;
}

int crearArchivoDeBloques(char* blocksFilePath, uint32_t cantDeBloquesTotales, uint32_t tamanioBloque) {
    
    FILE* archivoDeBloques = fopen(blocksFilePath, "rb+");
    if (archivoDeBloques == NULL)
    {

    FILE* archivoDeBloques = fopen(blocksFilePath, "wb+");

    if (archivoDeBloques == NULL)
        { 
            printf("FS FAT: No se pudo crear el archivo fat.bin");
            return 1;
        } else {
    BLOQUE* bloque = asignarTamanioBloque(tamanioBloque);

    if (bloque == NULL) {
        // Manejar el error aquí, cerrar el archivo si es necesario
        fclose(archivoDeBloques);
        return 1;
    }

    // Rellenar el bloque con el carácter 'N'
    memset(bloque->valorDeBloque, 'N', tamanioBloque);

    for (uint32_t i = 0; i < cantDeBloquesTotales; i++) {
        // Escribir el bloque en el archivo
        fwrite(bloque->valorDeBloque, tamanioBloque * sizeof(uint8_t), 1, archivoDeBloques);
    }

    free(bloque->valorDeBloque);
    free(bloque);

    fclose(archivoDeBloques);
    return 0;}
    } else printf("EL archivo BLOQUES ya existe");
}


/* 
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
} */

BLOQUE* asignarTamanioBloque(uint32_t tamanioBloqueEnArchivoConfig) {
    BLOQUE* bloque = (BLOQUE*)malloc(sizeof(BLOQUE));
    if (bloque == NULL) {
        perror("Error al asignar bloque");
        return NULL;
    }

    bloque->valorDeBloque = (int8_t*)calloc(tamanioBloqueEnArchivoConfig, sizeof(int8_t));
    if (bloque->valorDeBloque == NULL) {
        free(bloque);
        perror("Error al asignar datos de bloque");
        return NULL;
    }

    return bloque;
}

void liberarESpacioBloqueCreado(BLOQUE* bloque) {
    if (bloque != NULL) {
        free(bloque->valorDeBloque);
        free(bloque);
    }
}



/* void modificarBLOQUEenArchivoBLOQUE(char* pathBLOQUES,uint32_t numeroBloque, BLOQUE* nuevaEntrada){
    
    // Abrir el archivo en modo lectura y escritura binaria
    FILE *archivoBLOQUE = fopen(pathBLOQUES, "r+b");

    if (archivoBLOQUE == NULL) {
        perror("Error al abrir el archivo FAT");
    }

    // Mover el puntero de archivo a la posición de la entrada deseada
    fseek(archivoBLOQUE, (numeroBloque) * sizeof(BLOQUE), SEEK_SET);

    // Escribir la nueva entrada en el archivo
    fwrite(nuevaEntrada, sizeof(BLOQUE), 1, archivoBLOQUE);

    // Cerrar el archivo
    fclose(archivoBLOQUE);

}
 */
int modificarBLOQUEenArchivoBLOQUE(char* pathBLOQUES, uint32_t numeroBloque, BLOQUE* nuevaEntrada, u_int32_t tamanioBloque) {
    // Abrir el archivo en modo lectura y escritura
    FILE* archivoBLOQUE = fopen(pathBLOQUES, "r+");

    if (archivoBLOQUE == NULL) {
        perror("Error al abrir el archivo de bloques");
        return 1; // Indicador de error
    }

    // Mover el puntero de archivo a la posición de la entrada deseada
    fseek(archivoBLOQUE, numeroBloque * tamanioBloque, SEEK_SET);

    BLOQUE* bloqueExistente = asignarTamanioBloque(tamanioBloque);
    fread(bloqueExistente->valorDeBloque, tamanioBloque, 1, archivoBLOQUE);

    memcpy(bloqueExistente->valorDeBloque, nuevaEntrada->valorDeBloque, tamanioBloque);

    fseek(archivoBLOQUE, numeroBloque * tamanioBloque, SEEK_SET);

    // Escribir la nueva entrada en el archivo
    fwrite(bloqueExistente->valorDeBloque, tamanioBloque, 1, archivoBLOQUE);

    // Cerrar el archivo
    fclose(archivoBLOQUE);

    return 0; // Operación exitosa
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
        fread(bloques[i]->valorDeBloque, tamanioBloque * sizeof(u_int8_t), 1, archivoBLOQUE);
    }

    // Cerrar el archivo
    fclose(archivoBLOQUE);

    return bloques;
}


void modificarBloque (char* path,int32_t tamanioBloque){
    FILE* archivoBloques = fopen(path,"r+b");

    fwrite("UUUU",sizeof(u_int8_t)*tamanioBloque,1,archivoBloques);

}

