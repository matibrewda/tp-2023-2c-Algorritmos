#include "fat.h"

// Implementacion de funciones

int iniciarFAT(t_log *logger, char *fat_path, char *blocks_file_path, uint32_t cant_bloques_total, uint32_t cant_bloques_swap)
{
    size_t total_blocks = cant_bloques_total - cant_bloques_swap;

    // Abro archivo BLOCKS FILE para sincronizar la info de mi FAT en el.
    log_info(logger, "HOLA 1");
    FILE *blocks_file = fopen(blocks_file_path, "rb+");
    size_t posicionSiguienteParaEscritura = cant_bloques_swap * sizeof(u_int32_t);
    fseek(blocks_file, posicionSiguienteParaEscritura, SEEK_SET);
    log_info(logger, "HOLA 2");

    // Abro archivo FAT para escribir en el.
    FILE *fat_file = fopen(fat_path, "r+");
    if (fat_file == NULL)
    {
        // No se pudo abrir el archivo, entonces lo creamos y lo inicializamos
        fat_file = fopen(fat_path, "w+");
        if (fat_file == NULL)
        {
            log_debug(logger, "FS FAT: No se pudo crear el archivo fat.bin");
            return 1;
        }

        // solicito memoria para el arreglo de bloques FAT

        FATEntry *arreglo = (FATEntry *)malloc(total_blocks * sizeof(FATEntry));

        if (arreglo == NULL)
        {
            // Error: no se pudo asignar memoria
        }
        else
        {
            // Inicializar el arreglo con 0 en todos sus bloques.
            for (size_t i = 0; i < total_blocks; i++)
            {
                arreglo[i].block_value = 0;
            }

            arreglo[0].block_value = UINT32_MAX;
            arreglo[total_blocks-1].block_value = UINT32_MAX;

            // EScribimos el contenido del array en LOS archivos

            for (size_t j = 0; j < total_blocks; j++)
            {

                fwrite(&arreglo[j], sizeof(u_int32_t), 1, fat_file);
                fwrite(&arreglo[j], sizeof(u_int32_t), 1, blocks_file);
            }

            log_debug(logger, "FS FAT: El archivo FAT se ha creado y inicializado con %ld bloques, todos ellos con 0.\n", total_blocks);
            log_debug(logger, "FS BLOCKS FILE: El archivo FAT fue sincronizado en el BLOCKS FILE.\n");
            free(arreglo);
        }
    }
    else
    {
        log_debug(logger, "FS FAT: El archivo FAT ya existe.\n");
        log_debug(logger, "FS BLOCKS FILE: No se realiza sincronizacion con BLOCKS FILE.\n");
    }

    fclose(fat_file);
    fclose(blocks_file);

    return 0;
}
