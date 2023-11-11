#include "directorio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <commons/config.h>
#include <commons/log.h>
#include <limits.h>  

// Función para inicializar el array de directorios
DirectorioArray inicializarDirectorioArray(t_log* logger) {

    DirectorioArray array;
    array.directorios = NULL;
    array.cantidadDirectorios = 0;
    return array;
}

// Función para liberar la memoria utilizada por el array de directorios
void liberarDirectorioArray(DirectorioArray* array) {
    free(array->directorios);
    array->directorios = NULL;
    array->cantidadDirectorios = 0;
}

// Función para agregar una entrada al array de directorios desde un archivo .fcb

// Función para agregar una entrada al array de directorios desde un archivo .fcb
void agregarEntradaDesdeFCB(DirectorioArray* array, char* rutaArchivoFCB) {
    t_config* config = config_create(rutaArchivoFCB);
    if (config == NULL) {
        perror("Error al abrir el archivo .fcb");
        return;
    }

    // Crear una nueva entrada de directorio
    EntradaDirectorio nuevaEntrada;
    memset(&nuevaEntrada, 0, sizeof(EntradaDirectorio)); // Inicializar la estructura

    // Leer el nombre del archivo y el número de bloque inicial desde el archivo .fcb
    strncpy(nuevaEntrada.nombreArchivo, config_get_string_value(config, "NOMBRE_ARCHIVO"), sizeof(nuevaEntrada.nombreArchivo) - 1);
    nuevaEntrada.numBloqueInicial = config_get_int_value(config, "BLOQUE_INICIAL");

    // Incrementar el tamaño del array de directorios
    array->directorios = realloc(array->directorios, (array->cantidadDirectorios + 1) * sizeof(EntradaDirectorio));
    if (array->directorios == NULL) {
        config_destroy(config);
        return;
    }

    // Agregar la nueva entrada al array de directorios
    array->directorios[array->cantidadDirectorios] = nuevaEntrada;
    array->cantidadDirectorios++;

    // Liberar la memoria utilizada por el archivo de configuración
    config_destroy(config);
}

void procesarArchivosEnDirectorio(DirectorioArray* directorio,char* rutaDirectorio) {
    DIR* dir = opendir(rutaDirectorio);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return;
    }

    struct dirent* entrada;
    while ((entrada = readdir(dir)) != NULL) {
        if (entrada->d_name[0] != '.') {
            // Construir la ruta completa del archivo
            char rutaArchivo[PATH_MAX];  // Usa un tamaño seguro para almacenar la ruta completa
            size_t longitudDirectorio = strlen(rutaDirectorio);
            size_t longitudNombreArchivo = strlen(entrada->d_name);

            // Asegurarse de que hay suficiente espacio en rutaArchivo
            if (longitudNombreArchivo + longitudDirectorio + 2 <= sizeof(rutaArchivo)) {
                snprintf(rutaArchivo, sizeof(rutaArchivo), "%s/%s", rutaDirectorio, entrada->d_name);

                if (strstr(entrada->d_name, ".fcb") != NULL) {
                    agregarEntradaDesdeFCB(directorio, rutaArchivo);
                }
            } else {
                fprintf(stderr, "Nombre de archivo demasiado largo: %s/%s\n", rutaDirectorio, entrada->d_name);
            }
        }
    }

    closedir(dir);
}
