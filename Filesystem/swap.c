#include "swap.h"

// Implementacion de funciones

int iniciarSWAP (t_log *logger,char* blocks_file_path, uint32_t cant_bloques_swap)
{


 size_t total_blocks = cant_bloques_swap;

    FILE* blocks_file = fopen(blocks_file_path, "rb+");
    if (blocks_file == NULL) {
        // No se pudo abrir el archivo, entonces lo creamos y lo inicializamos
        blocks_file = fopen(blocks_file_path, "wb+");
        if (blocks_file == NULL) {
            log_debug(logger,"FS BLOCKS FILE: No se pudo crear el archivo BLOCKSFILE.bin");
            return 1;
        }

        // solicito memoria para el arreglo de bloques FAT

        SWAPEntry *arregloSwap = (SWAPEntry *)malloc(total_blocks * sizeof(SWAPEntry));

        if (arregloSwap == NULL) {
    // Error: no se pudo asignar memoria
        } else {
    // Inicializar el arreglo con 0 en todos sus bloques.
        for (size_t i = 0; i < total_blocks; i++) {
        arregloSwap[i].block_value = 0;
    }

        // EScribimos el contenido del array en el archivo
        
        for (size_t j = 0; j < total_blocks; j++) {

            fwrite(&arregloSwap[j], sizeof(u_int32_t), 1, blocks_file);
        }

        log_debug(logger,"FS BLOCKS FILE: El BLOCKS FILE se ha creado y inicializado con %ld BLOQUES SWAP, todos ellos con 0.\n",total_blocks);
        free(arregloSwap);

    } } else {
        log_debug(logger,"FS BLOCKS FILE: El archivo BLOCKS FILE ya existe.\n");
    }

    fclose(blocks_file);
    return 0;
}

