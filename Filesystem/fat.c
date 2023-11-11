#include "fat.h"

// Implementacion de funciones

int iniciarFAT(t_log *logger, char *fat_path, uint32_t cant_bloques_total, uint32_t cant_bloques_swap,uint32_t tamanio_bloque)
{
    size_t total_blocks = cant_bloques_total - cant_bloques_swap;

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

            // EScribimos el contenido del array en LOS archivos

            for (size_t j = 0; j < total_blocks; j++)
            {

                fwrite(&arreglo[j], sizeof(u_int32_t), 1, fat_file);
            }

            log_debug(logger, "FS FAT: La tabla FAT se ha creado y inicializado con %ld entradas (bloques), todos ellos con direccion 0.\n", total_blocks);
            free(arreglo);

        
        }
    }
    else
    {
        log_debug(logger, "FS FAT: La tabla FAT ya existe.\n");
    }

    fclose(fat_file);

    return 0;
}
