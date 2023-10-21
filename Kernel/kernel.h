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
#include "../Shared/Headers/utilidades_thread_safe.h"
#include "../Shared/Headers/enums.h"
#include "../Shared/Headers/estructuras.h"
#include "../Shared/Headers/constantes.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/kernel.log"
#define LOG_LEVEL LOG_LEVEL_INFO

// Funciones por consola
#define INICIAR_PROCESO "INICIAR_PROCESO"
#define FINALIZAR_PROCESO "FINALIZAR_PROCESO"
#define DETENER_PLANIFICACION "DETENER_PLANIFICACION"
#define INICIAR_PLANIFICACION "INICIAR_PLANIFICACION"
#define MULTIPROGRAMACION "MULTIPROGRAMACION"
#define PROCESO_ESTADO "PROCESO_ESTADO"

void terminar_kernel();

// Planificadores
void *planificador_largo_plazo();
void *planificador_corto_plazo();
void *contador_quantum(void *id_hilo_quantum);

// Transicion de estado de procesos
void transicionar_proceso(t_pcb *pcb, char nuevo_estado_proceso);
void transicionar_proceso_a_new(t_pcb *pcb);
void transicionar_proceso_de_new_a_ready(t_pcb *pcb);
void transicionar_proceso_de_new_a_exit(t_pcb *pcb);
void transicionar_proceso_de_ready_a_executing(t_pcb *pcb);
void transicionar_proceso_de_ready_a_exit(t_pcb *pcb);
void transicionar_proceso_de_executing_a_exit(t_pcb *pcb);
void transicionar_proceso_de_executing_a_ready(t_pcb *pcb);
void transicionar_proceso_de_executing_a_bloqueado(t_pcb *pcb);
void transicionar_proceso_de_bloqueado_a_ready(t_pcb *pcb);
void transicionar_proceso_de_bloqueado_a_exit(t_pcb *pcb);

// Bloqueos
void crear_hilo_sleep(int pid, int tiempo_sleep);
void* bloqueo_sleep();

// Comunicacion con CPU
void enviar_paquete_solicitud_ejecutar_proceso(t_pcb *pcb_proceso_a_ejecutar);
void enviar_paquete_solicitud_interrumpir_ejecucion(int motivo_interrupcion);
void enviar_paquete_respuesta_devolver_proceso_por_ser_interrumpido();
void enviar_paquete_respuesta_devolver_proceso_por_correcta_finalizacion();
void enviar_paquete_respuesta_devolver_proceso_por_sleep();
t_contexto_de_ejecucion *recibir_paquete_de_cpu_dispatch(op_code *codigo_operacion_recibido, int* tiempo_sleep, int* motivo_interrupcion);
bool recibir_operacion_de_cpu_dispatch(op_code codigo_operacion_esperado);
bool recibir_operacion_de_cpu_interrupt(op_code codigo_operacion_esperado);
void ejecutar_proceso_en_cpu(t_pcb *pcb_proceso_a_ejecutar);
void interrumpir_proceso_en_cpu(int motivo_interrupcion);

// Comunicacion con Memoria
void enviar_paquete_iniciar_estructuras_de_proceso_en_memoria(t_pcb *pcb);
void enviar_paquete_destruir_estructuras_de_proceso_en_memoria(t_pcb *pcb);
bool recibir_operacion_de_memoria(op_code codigo_operacion_esperado);
bool iniciar_estructuras_de_proceso_en_memoria(t_pcb *pcb);
void destruir_estructuras_de_proceso_en_memoria(t_pcb *pcb);

// Consola
void consola();
void iniciar_proceso(char *path, int size, int prioridad);
void finalizar_proceso(int pid);
void iniciar_planificacion();
void detener_planificacion();
void modificar_grado_max_multiprogramacion(int grado_multiprogramacion);

// Utilidades
const char *nombre_estado_proceso(char codigo_estado_proceso);
int obtener_nuevo_pid();
void agregar_pid_a_aux_pids_cola(t_pcb *pcb);
void loguear_cola(t_queue *cola, const char *nombre_cola, pthread_mutex_t *mutex_cola);
void imprimir_proceso_en_consola(t_pcb *pcb);
void listar_procesos();
t_pcb *crear_pcb(char *path, int size, int prioridad);
void actualizar_pcb(t_pcb *pcb, t_contexto_de_ejecucion *contexto_de_ejecucion);
t_pcb *buscar_pcb_con_pid(int pid);
t_pcb *buscar_pcb_con_pid_en_cola(int pid, t_queue *cola, pthread_mutex_t *mutex);
void eliminar_pcb_de_cola(int pid, t_queue *cola, pthread_mutex_t *mutex);
void push_cola_ready(t_pcb* pcb);
void crear_recursos();
t_recurso* crear_recurso(char* nombre, int instancias);
bool recurso_existe(char* nombre);

#endif /* KERNEL_H_ */