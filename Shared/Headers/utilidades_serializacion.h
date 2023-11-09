#ifndef UTILIDADES_SERIALIZACION_H_
#define UTILIDADES_SERIALIZACION_H_

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

// Kernel a CPU
t_paquete *crear_paquete_solicitud_ejecutar_proceso(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion);
t_paquete *crear_paquete_solicitud_interrumpir_proceso(t_log *logger, int motivo_interrupcion);
t_paquete *crear_paquete_respuesta_devolver_proceso_por_ser_interrumpido(t_log *logger);
t_paquete *crear_paquete_respuesta_devolver_proceso_por_correcta_finalizacion(t_log *logger);
t_paquete *crear_paquete_respuesta_devolver_proceso_por_sleep(t_log *logger);
t_paquete *crear_paquete_respuesta_devolver_proceso_por_wait(t_log *logger);
t_paquete *crear_paquete_respuesta_devolver_proceso_por_signal(t_log *logger);

// Kernel a Memoria
t_paquete *crear_paquete_solicitud_iniciar_proceso_en_memoria(t_log *logger, t_proceso_memoria *proceso_memoria);
t_paquete *crear_paquete_solicitud_finalizar_proceso_en_memoria(t_log *logger, t_proceso_memoria *proceso_memoria);

// CPU a Kernel
t_paquete *crear_paquete_respuesta_ejecutar_proceso(t_log *logger);
t_paquete *crear_paquete_respuesta_interrumpir_proceso(t_log *logger);
t_paquete *crear_paquete_solicitud_devolver_proceso_por_ser_interrumpido(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, int motivo_interrupcion);
t_paquete *crear_paquete_solicitud_devolver_proceso_por_correcta_finalizacion(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion);
t_paquete *crear_paquete_solicitud_devolver_proceso_por_sleep(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, int tiempo_sleep);
t_paquete *crear_paquete_solicitud_devolver_proceso_por_wait(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, char* nombre_recurso);
t_paquete *crear_paquete_solicitud_devolver_proceso_por_signal(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, char* nombre_recurso);

// CPU a Memoria
t_paquete *crear_paquete_solicitud_pedir_instruccion_a_memoria(t_log *logger, t_pedido_instruccion *pedido_instruccion);
t_paquete *crear_paquete_solicitud_pedir_info_de_memoria_inicial_para_cpu(t_log *logger);

// Memoria a Kernel
t_paquete *crear_paquete_respuesta_iniciar_proceso_en_memoria(t_log *logger, bool resultado_iniciar_proceso);
t_paquete *crear_paquete_respuesta_finalizar_proceso_en_memoria(t_log *logger);

// Memoria a CPU
t_paquete *crear_paquete_respuesta_pedir_instruccion_a_memoria(t_log *logger, char *linea_instruccion);
t_paquete *crear_paquete_respuesta_pedir_info_de_memoria_inicial_para_cpu(t_log *logger, t_info_memoria *info_memoria);

// Memoria a Filesystem
t_paquete *crear_paquete_pedir_bloques_a_filesystem(t_log *logger, int cantidad_de_bloques);
t_paquete *crear_paquete_liberar_bloques_en_filesystem(t_log *logger, t_list * posiciones_swap);

// Comunes
t_paquete *crear_paquete_proceso_memoria(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_proceso_memoria *proceso_memoria);
t_paquete *crear_paquete_contexto_de_ejecucion(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_contexto_de_ejecucion *contexto_de_ejecucion);
t_paquete *crear_paquete_con_opcode_y_sin_contenido(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino);
void agregar_contexto_de_ejecucion_a_paquete(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, t_paquete* paquete, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino);

#endif /* UTILIDADES_SERIALIZACION_H_ */