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

// Comunicacion con Kernel
void *interrupt();
void *dispatch();

void devolver_contexto_por_ser_interrumpido();
void devolver_contexto_por_correcta_finalizacion();
void devolver_contexto_por_sleep(int segundos_sleep);
void devolver_contexto_por_wait(char *nombre_recurso);
void devolver_contexto_por_signal(char *nombre_recurso);
void devolver_contexto_por_error(int codigo_error);
void devolver_contexto_por_page_fault(int numero_de_pagina);
void devolver_contexto_por_operacion_filesystem(fs_op_code fs_opcode, char *nombre_archivo, int modo_apertura, int posicion, int direccion_fisica, int tamanio);

// Comunicacion con Memoria
void pedir_info_inicial_a_memoria();
int pedir_numero_de_marco_a_memoria(int numero_de_pagina);

// Ciclo de ejecucion
void ciclo_de_ejecucion();

// MMU
int mmu(int direccion_logica);

// Utilidades
t_contexto_de_ejecucion *crear_objeto_contexto_de_ejecucion();
t_operacion_filesystem *crear_objeto_operacion_filesystem(fs_op_code fs_opcode, char *nombre_archivo, int modo_apertura, int posicion, int direccion_fisica, int tamanio);
uint32_t leer_valor_de_registro(char *nombre_registro);
void escribir_valor_a_registro(char *nombre_registro, uint32_t valor);

#endif /* CPU_H_ */
