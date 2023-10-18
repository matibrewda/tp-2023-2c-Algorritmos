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

// CPU recibe de Kernel
t_contexto_de_ejecucion *leer_paquete_ejecutar_proceso(t_log *logger, int conexion_con_kernel_dispatch);

// CPU recibe de Memoria
t_info_memoria* leer_info_inicial_de_memoria_para_cpu(t_log *logger, int conexion_con_memoria);
char* leer_instrucion_recibida_desde_memoria(t_log *logger, int conexion_con_memoria);

// Memoria recibe de Kernel
t_proceso_memoria *leer_paquete_iniciar_proceso_en_memoria(t_log *logger, int conexion_con_kernel);
t_proceso_memoria *leer_paquete_finalizar_proceso_en_memoria(t_log *logger, int conexion_con_kernel);

// Memoria recibe de CPU
t_pedido_instruccion *leer_paquete_pedido_instruccion(t_log *logger, int conexion_con_cpu);

//Memoria recibe de FileSystem
t_pedido_leer_archivo *leer_paquete_pedido_leer_archivo(t_log *logger, int conexion_con_filsystem);
t_pedido_escribir_archivo *leer_paquete_pedido_escribir_archivo(t_log *logger, int conexion_con_filsystem);


// Comunes
t_proceso_memoria *leer_paquete_proceso_memoria(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino);
t_contexto_de_ejecucion *leer_paquete_contexto_de_ejecucion(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino);

#endif /* UTILIDADES_DESERIALIZACION_H_ */