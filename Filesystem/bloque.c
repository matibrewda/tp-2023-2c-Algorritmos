
#include "bloque.h"
#include <stdlib.h> // Para rand() y srand()
#include <time.h>   // Para time()

int crearArchivoDeBloques(char* blocksFilePath, uint32_t cantDeBloquesTotales, uint32_t tamanioBloque) {
    FILE* archivoDeBloques = fopen(blocksFilePath, "wb+");

    if (archivoDeBloques == NULL) {
        printf("FS FAT: No se pudo crear el archivo fat.bin\n");
        return 1;
    }
    else {
        // Crear un bloque para llenar el archivo con datos binarios válidos
        BLOQUE* bloque = asignarTamanioBloque(tamanioBloque);
        for (uint32_t i = 0; i < cantDeBloquesTotales; i++) {
            fwrite(bloque->valorDeBloque, tamanioBloque * sizeof(uint8_t), 1, archivoDeBloques);
        }

        free(bloque->valorDeBloque);
        free(bloque);
    }

    fclose(archivoDeBloques);
    return 0;
}

BLOQUE* asignarTamanioBloque(uint32_t tamanioBloqueEnArchivoConfig) {
    if (tamanioBloqueEnArchivoConfig <= 0) {
        // Manejar error de tamaño no válido
        return NULL;
    }

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

    // Inicializar con el equivalente hexadecimal de 'N' (0x4E)
    memset(bloque->valorDeBloque, 0, tamanioBloqueEnArchivoConfig);

    return bloque;
}

void liberarESpacioBloqueCreado(BLOQUE* bloque) {
    if (bloque != NULL) {
        free(bloque->valorDeBloque);
        free(bloque);
    }
}

int modificarBLOQUEenArchivoBLOQUE(char* pathBLOQUES, uint32_t numeroBloque, BLOQUE* nuevaEntrada, u_int32_t tamanioBloque) {
    // Abrir el archivo en modo lectura y escritura binaria
    FILE* archivoBLOQUE = fopen(pathBLOQUES, "rb+");

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


int obtenerContenidoRandom() {
    srand(time(NULL)); // Inicializar la semilla del generador de números aleatorios

    int valor;
    if (rand() % 2 == 0) {
        // Generar un valor aleatorio entre 0x00 y 0x1F
        valor = rand() % 0x20;
    } else {
        // Generar un valor aleatorio entre 0x7F y 0xFF
        valor = rand() % 0x80 + 0x7F;
    }

    return valor;
}
