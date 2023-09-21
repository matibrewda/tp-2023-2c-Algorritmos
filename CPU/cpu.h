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
#define LOG_LEVEL LOG_LEVEL_INFO

void terminar_cpu();
void *interrupt();
void *dispatch();

uint32_t obtener_valor_registro(char *nombre_registro);
void escribir_valor_a_registro(char *nombre_registro, uint32_t valor);

t_contexto_de_ejecucion *crear_objeto_contexto_de_ejecucion();
void devolver_contexto_por_ser_interrumpido();
void devolver_contexto_por_correcta_finalizacion();

// Ciclo de ejecucion
void ciclo_de_ejecucion();
t_instruccion *fetch();
int decode(char* nombre_instruccion);
void execute(int opcode, char* parametro_1_instruccion, char* parametro_2_instruccion);
void check_interrupt();

// Instrucciones
void ejecutar_instruccion_set(char *nombre_registro, uint32_t valor);
void ejecutar_instruccion_sum(char *nombre_registro_destino, char *nombre_registro_origen);
void ejecutar_instruccion_sub(char *nombre_registro_destino, char *nombre_registro_origen);
void ejecutar_instruccion_jnz(char *nombre_registro, uint32_t nuevo_program_counter);
void ejecutar_instruccion_sleep(int tiempo);
void ejecutar_instruccion_wait(char* nombre_recurso);
void ejecutar_instruccion_signal(char* nombre_recurso);
void ejecutar_instruccion_mov_in(char* nombre_registro, char* direccion_logica);
void ejecutar_instruccion_mov_out(char* direccion_logica, char* nombre_registro);
void ejecutar_instruccion_fopen(char* nombre_archivo, char* modo_apertura);
void ejecutar_instruccion_fclose(char* nombre_archivo);
void ejecutar_instruccion_fseek(char* nombre_archivo, char* posicion);
void ejecutar_instruccion_fread(char* nombre_archivo, char* direccion_logica);
void ejecutar_instruccion_fwrite(char* nombre_archivo, char* direccion_logica);
void ejecutar_instruccion_ftruncate(char* nombre_archivo, char* tamanio);
void ejecutar_instruccion_exit();

#endif /* CPU_H_ */
