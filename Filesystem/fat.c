#include "fat.h"

// Implementacion de funciones

int iniciarFAT (t_log *logger,char* fat_path, uint32_t cant_bloques_total, uint32_t cant_bloques_swap)
{


 size_t total_blocks = cant_bloques_total - cant_bloques_swap;

    FILE* fat_file = fopen(fat_path, "r+");
    if (fat_file == NULL) {
        // No se pudo abrir el archivo, entonces lo creamos y lo inicializamos
        fat_file = fopen(fat_path, "w+");
        if (fat_file == NULL) {
            log_debug(logger,"FS FAT: No se pudo crear el archivo fat.bin");
            return 1;
        }

        // Inicializamos todos los bloques de 4 bytes con 0
        uint32_t zero = 0;
        for (size_t i = 0; i < total_blocks; i++) {
            fwrite(&zero, sizeof(zero), 1, fat_file);
        }
        log_debug(logger,"FS FAT: El archivo FAT se ha creado y inicializado con %ld bloques, todos ellos con 0.\n",total_blocks);
    } else {
        log_debug(logger,"FS FAT: El archivo FAT ya existe.\n");
    }

    fclose(fat_file);
    return 0;
}

