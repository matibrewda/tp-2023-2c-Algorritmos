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
#include <readline/history.h>

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
void transicionar_proceso_de_executing_a_ready(t_pcb *pcb);
void transicionar_proceso_de_executing_a_bloqueado(t_pcb *pcb);
void transicionar_proceso_de_executing_a_executing(t_pcb *pcb);
void transicionar_proceso_de_executing_a_exit(t_pcb *pcb);
void transicionar_proceso_de_bloqueado_a_ready(t_pcb *pcb);
void transicionar_proceso_de_bloqueado_a_exit(t_pcb *pcb);

// Bloqueos
void crear_hilo_sleep(t_pcb *pcb, int tiempo_sleep);
void *bloqueo_sleep();
void crear_hilo_operacion_archivo(t_pcb *pcb, int operacion_archivo, char *nombre_archivo, int modo_apertura, int puntero, int direccion_fisica, int nuevo_tamanio);
void *operacion_archivo_h(void *argumentos);
void crear_hilo_page_fault(t_pcb *pcb, int numero_pagina);
void *page_fault();

// Comunicacion con CPU
t_contexto_de_ejecucion *recibir_paquete_de_cpu_dispatch(op_code *codigo_operacion_recibido, int *tiempo_sleep, int *motivo_interrupcion, char **nombre_recurso, int *codigo_error, int *numero_pagina, char **nombre_archivo, int *modo_apertura, int *posicion_puntero_archivo, int *direccion_fisica, int *nuevo_tamanio_archivo, int *fs_opcode);
void ejecutar_proceso_en_cpu(t_pcb *pcb_proceso_a_ejecutar);
void interrumpir_proceso_en_cpu(int motivo_interrupcion);

// Comunicacion con Memoria
bool iniciar_estructuras_de_proceso_en_memoria(t_pcb *pcb);
void destruir_estructuras_de_proceso_en_memoria(t_pcb *pcb);
bool cargar_pagina_en_memoria(int pid, int numero_pagina);

// Consola
void consola();
void iniciar_proceso(char *path, int size, int prioridad);
void *hilo_iniciar_proceso(void *argumentos);
void finalizar_proceso(int pid);
void *hilo_finalizar_proceso(void *argumentos);
void iniciar_planificacion();
void detener_planificacion();
void modificar_grado_max_multiprogramacion(int grado_multiprogramacion);
void listar_procesos();

// Utilidades
const char *nombre_estado_proceso(char codigo_estado_proceso);
void actualizar_pcb(t_pcb *pcb, t_contexto_de_ejecucion *contexto_de_ejecucion);
t_pcb *buscar_pcb_con_pid(int pid);
t_pcb *buscar_pcb_con_pid_en_cola(int pid, t_queue *cola, pthread_mutex_t *mutex);
void eliminar_pcb_de_cola(int pid, t_queue *cola, pthread_mutex_t *mutex);
void eliminar_pcb_de_lista(int pid, t_list *lista, pthread_mutex_t *mutex);
void destruir_pcb(t_pcb *pcb);
void push_cola_ready(t_pcb *pcb);
void log_fin_de_proceso(t_pcb *pcb);
char *crear_string_dinamico();
char *agregar_string_a_string_dinamico(char *string_dinamico, char *string_a_agregar);
char *agregar_entero_a_string_dinamico(char *string_dinamico, int entero);

// Comunicacion con Filesystem
void abrir_archivo_fs(char *nombre_archivo, int *existe, int *tamanio_archivo);
void crear_archivo_fs(char *nombre_archivo);
void truncar_archivo_fs(char *nombre_archivo, int nuevo_tamanio_archivo);
void leer_archivo_fs(char *nombre_archivo, int puntero_archivo, int direccion_fisica);
void escribir_archivo_fs(char *nombre_archivo, int puntero_archivo, int direccion_fisica);

// Recursos
t_recurso *crear_recurso(char *nombre, int instancias);
t_recurso *crear_recurso_archivo(char *nombre_archivo, int lock_actual, int tamanio, t_pcb *pcb);
t_recurso *buscar_recurso_por_nombre(char *nombre_recurso);

bool recurso_esta_asignado_a_pcb(t_recurso *recurso, int pid);
bool desasignar_recurso_a_pcb(t_recurso *recurso, int pid);
void desasignar_todos_los_recursos_a_pcb(int pid);

// Deadlock
t_list *obtener_procesos_analisis_deadlock();
int *obtener_vector_recursos_disponibles();
bool hay_deadlock();

#endif /* KERNEL_H_ */
