#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

// Para incluir la funcion asprintf()
#define _GNU_SOURCE

#include <stdio.h>
#include "fat.h"
#include "bloque.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdint.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>

#include "Source/Headers/argumentos_filesystem.h"
#include "Source/Headers/configuracion_filesystem.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/utilidades_serializacion.h"
#include "../Shared/Headers/utilidades_deserializacion.h"
#include "../Shared/Headers/enums.h"
#include "../Shared/Headers/estructuras.h"
#include "../Shared/Headers/constantes.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/filesystem.log"
#define LOG_LEVEL LOG_LEVEL_TRACE


void *comunicacion_kernel();
void *comunicacion_memoria();
int32_t crear_archivo (char* path,char* nombreNuevoArchivo);
int32_t truncar_archivo(char* path, uint32_t nuevo_tamano,t_config_filesystem *configuracion_filesystem,FATEntry fat[], BLOQUE *bloques[]);

// Terminar
void terminar_filesystem();


#endif /* FILESYSTEM_H_ */
