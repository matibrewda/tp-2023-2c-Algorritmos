#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

#include <readline/readline.h>

#include "Source/Headers/argumentos_kernel.h"
#include "Source/Headers/configuracion_kernel.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_argumentos.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/utilidades_serializacion.h"
#include "../Shared/Headers/utilidades_deserializacion.h"
#include "../Shared/Headers/nombres_modulos.h"
#include "../Shared/Headers/estructuras.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/kernel.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

// Funciones por consola
#define INICIAR_PROCESO "INICIAR_PROCESO"
#define FINALIZAR_PROCESO "FINALIZAR_PROCESO"
#define DETENER_PLANIFICACION "DETENER_PLANIFICACION"
#define INICIAR_PLANIFICACION "INICIAR_PLANIFICACION"
#define MULTIPROGRAMACION "MULTIPROGRAMACION"
#define PROCESO_ESTADO "PROCESO_ESTADO"

typedef struct
{
    t_log *logger;
    t_config_kernel *configuracion_kernel;
    int conexion_con_memoria;
} t_argumentos_hilo_consola;

void terminar_kernel(t_log *logger, t_argumentos_kernel *argumentos_kernel, t_config_kernel *configuracion_kernel, int conexion_con_cpu_dispatch, int conexion_con_cpu_interrupt, int conexion_con_memoria, int conexion_con_filesystem);
void crear_hilo_consola(t_log *logger, t_config_kernel *config_kernel,int conexion_con_memoria);
void consola(void *argumentos);
void enviar_inciar_proceso_memoria(t_log *logger,char *path,int size,int prioridad,int conexion_con_memoria);

#endif /* KERNEL_H_ */
