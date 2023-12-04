#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

// Para incluir la funcion asprintf()
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>

#include "fat.h"
#include "fcb.h"
#include "bloque.h"

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

// Comunicaciones
void *comunicacion_kernel();
void *comunicacion_memoria();

// Inicializacion
void inicializar_archivo_de_bloques();
void inicializar_fat();

// Manejo de archivos de usuario
int abrir_archivo_fs(char *nombre_archivo);
void crear_archivo_fs(char *nombre_archivo);

// Swap
void *leer_bloque_swap(int numero_de_bloque);
void escribir_bloque_swap(int numero_de_bloque, void *bloque);
t_list *buscar_bloques_libres_en_swap(int cantidad_de_bloques);
void liberar_bloques_en_swap(t_list *numeros_de_bloques_a_liberar);
t_list *reservar_bloques_en_swap(int cantidad_de_bloques);

// ?
t_list *archivos;
void truncar_archivo(char *path, int nuevo_tamano);
uint32_t buscar_bloque_fat(int nro_bloque, char *nombre_archivo);
FCB *buscar_archivo(char *nombre_archivo);
void leer_bloque(uint32_t bloqueFAT);
void escribir_bloque(uint32_t bloqueFAT, char *informacion);
void reducir_tamano_archivo(FCB *fcb, int nuevo_tamano);
void ampliar_tamano_archivo(FCB *fcb, int nuevo_tamano);

// Utilidades
void dar_full_permisos_a_archivo(char *path_archivo);

// Terminar
void terminar_filesystem();

#endif /* FILESYSTEM_H_ */
