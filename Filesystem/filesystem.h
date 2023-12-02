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
#include "commons/config.h"
#include "fcb.h"
#include "fat.h"
#include "swap.h"
#include "bloque.h"
#include "directorio.h"


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


t_list* archivos; 

void *comunicacion_kernel();
void *comunicacion_memoria();
int crear_archivo (char* nombreNuevoArchivo);
void truncar_archivo(char* path, int nuevo_tamano);
uint32_t buscar_bloque_fat(int nro_bloque,char* nombre_archivo);
void solicitar_escribir_memoria(char *informacion);
FCB* buscar_archivo(char* nombre_archivo);
int abrirarchivo(char* nombre_archivo);
void leer_bloque(uint32_t bloqueFAT);
void escribir_bloque(uint32_t bloqueFAT,char* informacion);
void inicializar_archivo_de_bloques();
void inicializar_fat();
void reducir_tamano_archivo(FCB *fcb, int nuevo_tamano);
void ampliar_tamano_archivo(FCB *fcb, int nuevo_tamano);
int crear_archivo (char* nombreNuevoArchivo);
char* concatenarRutas(const char* rutaFat, const char* nombreFCB);





// Terminar
void terminar_filesystem(t_log *logger, t_argumentos_filesystem *argumentos_filesystem, t_config_filesystem *configuracion_filesystem, int socket_kernel, int conexion_con_kernel, int conexion_con_memoria);


#endif /* FILESYSTEM_H_ */
