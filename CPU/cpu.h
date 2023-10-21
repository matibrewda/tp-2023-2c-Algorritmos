#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
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

#include "Source/Headers/argumentos_cpu.h"
#include "Source/Headers/configuracion_cpu.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/utilidades_serializacion.h"
#include "../Shared/Headers/utilidades_deserializacion.h"
#include "../Shared/Headers/enums.h"
#include "../Shared/Headers/estructuras.h"
#include "../Shared/Headers/constantes.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/cpu.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

void terminar_cpu();

// Comunicacion con Kernel
void *interrupt();
void *dispatch();
void enviar_paquete_respuesta_ejecutar_proceso();
void enviar_paquete_respuesta_interrumpir_ejecucion();
void enviar_paquete_solicitud_devolver_proceso_por_ser_interrumpido();
void enviar_paquete_solicitud_devolver_proceso_por_correcta_finalizacion();
bool recibir_operacion_de_kernel_dispatch(op_code codigo_operacion_esperado);
void devolver_contexto_por_ser_interrumpido();
void devolver_contexto_por_correcta_finalizacion();

// Comunicacion con Memoria
void enviar_paquete_solicitud_pedir_info_de_memoria_inicial();
void enviar_paquete_solicitud_pedir_instruccion_a_memoria();
void solicitar_info_inicial_a_memoria();
char *pedir_instruccion_a_memoria();

// Ciclo de ejecucion
void ciclo_de_ejecucion();
char *fetch();
t_instruccion *decode(char *instruccion_string);
void execute(t_instruccion *instruccion);

// Instrucciones
void ejecutar_instruccion_set(char *nombre_registro, uint32_t valor);
void ejecutar_instruccion_sum(char *nombre_registro_destino, char *nombre_registro_origen);
void ejecutar_instruccion_sub(char *nombre_registro_destino, char *nombre_registro_origen);
void ejecutar_instruccion_jnz(char *nombre_registro, uint32_t nuevo_program_counter);
void ejecutar_instruccion_sleep(int tiempo);
void ejecutar_instruccion_wait(char *nombre_recurso);
void ejecutar_instruccion_signal(char *nombre_recurso);
void ejecutar_instruccion_mov_in(char *nombre_registro, char *direccion_logica);
void ejecutar_instruccion_mov_out(char *direccion_logica, char *nombre_registro);
void ejecutar_instruccion_fopen(char *nombre_archivo, char *modo_apertura);
void ejecutar_instruccion_fclose(char *nombre_archivo);
void ejecutar_instruccion_fseek(char *nombre_archivo, char *posicion);
void ejecutar_instruccion_fread(char *nombre_archivo, char *direccion_logica);
void ejecutar_instruccion_fwrite(char *nombre_archivo, char *direccion_logica);
void ejecutar_instruccion_ftruncate(char *nombre_archivo, char *tamanio);
void ejecutar_instruccion_exit();

// Utilidades
t_contexto_de_ejecucion *crear_objeto_contexto_de_ejecucion(int motivo_interrupcion);
void destruir_instruccion(t_instruccion *instruccion);
uint32_t obtener_valor_registro(char *nombre_registro);
void escribir_valor_a_registro(char *nombre_registro, uint32_t valor);

#endif /* CPU_H_ */
