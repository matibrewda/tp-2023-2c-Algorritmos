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
    t_queue *cola_new;
    t_queue *cola_ready;
    t_queue *cola_executing;
} t_colas_planificacion;

void terminar_kernel(t_log *logger, t_argumentos_kernel *argumentos_kernel, t_config_kernel *configuracion_kernel, int conexion_con_cpu_dispatch, int conexion_con_cpu_interrupt, int conexion_con_memoria, int conexion_con_filesystem);
void *consola();
void *planificador_largo_plazo();
void *planificador_corto_plazo();
void crear_proceso(char* path, int size, int prioridad);
void finalizar_proceso(int pid);
void iniciar_planificacion();
void detener_planificacion();
void listar_procesos();
void modificar_grado_max_multiprogramacion(int grado_multiprogramacion);
int obtener_nuevo_pid();

#endif /* KERNEL_H_ */
