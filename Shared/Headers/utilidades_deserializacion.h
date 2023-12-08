#ifndef UTILIDADES_DESERIALIZACION_H_
#define UTILIDADES_DESERIALIZACION_H_

#include <commons/log.h>
#include <commons/collections/list.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "enums.h"
#include "estructuras.h"
#include "constantes.h"
#include "utilidades_conexion.h"

// Kernel recibe de Memoria
bool leer_paquete_respuesta_iniciar_proceso_en_memoria(t_log *logger, int conexion_con_memoria);
bool leer_paquete_respuesta_cargar_pagina_en_memoria(t_log *logger, int conexion_con_memoria);

// Kernel recibe de CPU
t_contexto_de_ejecucion* leer_paquete_solicitud_devolver_proceso_por_ser_interrumpido(t_log *logger, int conexion_con_cpu_dispatch, int* motivo_interrupcion);
t_contexto_de_ejecucion* leer_paquete_solicitud_devolver_proceso_por_sleep(t_log *logger, int conexion_con_cpu_dispatch, int* tiempo_sleep);
t_contexto_de_ejecucion* leer_paquete_solicitud_devolver_proceso_por_wait(t_log *logger, int conexion_con_cpu_dispatch, char** nombre_recurso);
t_contexto_de_ejecucion* leer_paquete_solicitud_devolver_proceso_por_signal(t_log *logger, int conexion_con_cpu_dispatch, char** nombre_recurso);
t_contexto_de_ejecucion* leer_paquete_solicitud_devolver_proceso_por_error(t_log *logger, int conexion_con_cpu_dispatch, int* codigo_error);
t_contexto_de_ejecucion* leer_paquete_solicitud_devolver_proceso_por_pagefault(t_log *logger, int conexion_con_cpu_dispatch, int* numero_pagina);
t_contexto_de_ejecucion *leer_paquete_solicitud_devolver_proceso_por_operacion_filesystem(t_log *logger, int conexion_con_cpu_dispatch, char **nombre_archivo, int* modo_apertura, int *posicion_puntero_archivo, int *direccion_fisica, int *nuevo_tamanio_archivo, int* fs_opcode);

// Kernel recibe de Filesystem
void leer_paquete_respuesta_abrir_archivo_fs(t_log *logger, int conexion_con_filesystem, int* existe, int* tamanio_archivo);

// CPU recibe de Kernel
t_contexto_de_ejecucion *leer_paquete_solicitud_ejecutar_proceso(t_log *logger, int conexion_con_kernel_dispatch);
int leer_paquete_solicitud_interrumpir_proceso(t_log *logger, int conexion_con_kernel_interrupt);

// Memoria recibe de Kernel
t_proceso_memoria *leer_paquete_solicitud_iniciar_proceso_en_memoria(t_log *logger, int conexion_con_kernel);
t_proceso_memoria *leer_paquete_solicitud_finalizar_proceso_en_memoria(t_log *logger, int conexion_con_kernel);

// CPU recibe de Memoria
t_info_memoria* leer_paquete_respuesta_pedir_info_de_memoria_inicial_para_cpu(t_log *logger, int conexion_con_memoria);
char* leer_paquete_respuesta_pedir_instruccion_a_memoria(t_log *logger, int conexion_con_memoria);
int leer_paquete_respuesta_pedir_numero_de_marco_a_memoria(t_log *logger, int conexion_con_memoria);
uint32_t leer_paquete_respuesta_leer_valor_en_memoria(t_log *logger, int conexion_con_memoria);

// Memoria recibe de CPU
t_pedido_instruccion *leer_paquete_solicitud_pedir_instruccion_a_memoria(t_log *logger, int conexion_con_cpu);
t_pedido_leer_valor_de_memoria *leer_paquete_solicitud_leer_valor_en_memoria(t_log *logger, int conexion_con_cpu);
t_pedido_escribir_valor_en_memoria *leer_paquete_solicitud_escribir_valor_en_memoria(t_log *logger, int conexion_con_cpu);

// Memoria recibe de Filesystem
void *leer_paquete_respuesta_leer_pagina_en_swap(t_log *logger, int conexion_con_filsystem, int *numero_pagina, int *pid);
t_list *leer_paquete_respuesta_pedir_bloques_a_filesystem(t_log *logger, int conexion_con_filesystem, int *pid);
void leer_paquete_solicitud_escribir_bloque_en_memoria(t_log *logger, int conexion_con_filesystem, int* direccion_fisica, void** contenido_bloque);
void leer_paquete_solicitud_leer_marco_de_memoria(t_log *logger, int conexion_con_filesystem, int* direccion_fisica, char** nombre_archivo, int* puntero_archivo);

// Filesystem recibe de Memoria
void leer_paquete_solicitud_pedir_bloques_a_fs(t_log *logger, int conexion_con_memoria, int* cantidad_de_bloques, int* pid);
t_list* leer_paquete_solicitud_liberar_bloques_de_fs(t_log *logger, int conexion_con_memoria);
void leer_paquete_respuesta_leer_marco_de_memoria(t_log *logger, int conexion_con_memoria, void **contenido_marco, char **nombre_archivo, int *puntero_archivo);
void leer_paquete_solicitud_leer_pagina_swap(t_log *logger, int conexion_con_memoria, int* posicion_swap, int* numero_pagina, int* pid);
void leer_paquete_solicitud_escribir_pagina_en_swap(t_log *logger, int conexion_con_memoria, void** contenido_marco, int* posicion_swap);

// Filesystem recibe de Kernel
char* leer_paquete_solicitud_abrir_archivo_fs(t_log *logger, int conexion_con_kernel);
char* leer_paquete_solicitud_crear_archivo_fs(t_log *logger, int conexion_con_kernel);
void leer_paquete_solicitud_truncar_archivo_fs(t_log *logger, int conexion_con_kernel, char** nombre_archivo, int* nuevo_tamanio_archivo);
void leer_paquete_solicitud_leer_archivo_fs(t_log *logger, int conexion_con_kernel, char **nombre_archivo, int *puntero_archivo_a_leer, int* direccion_fisica_a_escribir);
void leer_paquete_solicitud_escribir_archivo_fs(t_log *logger, int conexion_con_kernel, char **nombre_archivo, int *puntero_archivo_a_escribir, int* direccion_fisica_a_leer);

// Comunes
t_pedido_pagina_en_memoria *leer_paquete_solicitud_pedido_pagina_en_memoria(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino);
t_proceso_memoria *leer_paquete_proceso_memoria(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino);
t_contexto_de_ejecucion *leer_paquete_contexto_de_ejecucion(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino);
t_contexto_de_ejecucion *leer_contexto_de_ejecucion_de_paquete(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, void ** buffer_con_offset);

#endif /* UTILIDADES_DESERIALIZACION_H_ */