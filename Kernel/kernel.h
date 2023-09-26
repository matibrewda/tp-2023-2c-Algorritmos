#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdint.h>

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
#include "../Shared/Headers/enums.h"
#include "../Shared/Headers/estructuras.h"
#include "../Shared/Headers/constantes.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/kernel.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

// Funciones por consola
#define INICIAR_PROCESO "INICIAR_PROCESO"
#define FINALIZAR_PROCESO "FINALIZAR_PROCESO"
#define DETENER_PLANIFICACION "DETENER_PLANIFICACION"
#define INICIAR_PLANIFICACION "INICIAR_PLANIFICACION"
#define MULTIPROGRAMACION "MULTIPROGRAMACION"
#define PROCESO_ESTADO "PROCESO_ESTADO"

void terminar_kernel();
void consola();
void *planificador_largo_plazo();
void *planificador_corto_plazo();
void *dispatcher();
void *escuchador_cpu();
void crear_proceso(char *path, int size, int prioridad);
void finalizar_proceso(int pid);
void iniciar_planificacion();
void detener_planificacion();
void listar_procesos();
void modificar_grado_max_multiprogramacion(int grado_multiprogramacion);
int obtener_nuevo_pid();
void ejecutar_proceso_en_cpu(t_pcb *pcb_proceso_a_ejecutar);
void interrumpir_proceso_en_cpu();
void terminar_kernel();
void crear_hilo_consola(t_log *logger, t_config_kernel *config_kernel, int conexion_con_memoria);
void consola();
void enviar_inciar_proceso_memoria(t_log *logger, char *path, int size, int prioridad);

void loguear_cola_pcbs(t_queue *cola, const char *nombre_cola);

void fifo();

#endif /* KERNEL_H_ */