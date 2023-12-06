#ifndef UTILIDADES_CONEXION_H_
#define UTILIDADES_CONEXION_H_

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

typedef struct
{
	int size;
	void *stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer *buffer;
} t_paquete;

// Cliente
int crear_socket_cliente(t_log *logger, char *ip_proceso_servidor, char *puerto_proceso_servidor, const char *nombre_modulo_cliente, const char *nombre_modulo_servidor);

// Servidor
int crear_socket_servidor(t_log *logger, char *puerto_proceso_servidor, const char *nombre_modulo_servidor, const char *nombre_cliente_a_escuchar);
int esperar_conexion_de_cliente(t_log *logger, int socket_servidor, const char *nombre_modulo_servidor, const char *nombre_cliente_a_escuchar);

// Utilidades para enviar
t_paquete *crear_paquete(t_log *logger, op_code codigo_operacion);
bool enviar_paquete(t_log *logger, int conexion, t_paquete *paquete, const char *nombre_proceso_origen, const char *nombre_proceso_destino);
void agregar_caracter_a_paquete(t_log *logger, t_paquete *paquete, char caracter, const char *nombre_proceso_origen, const char *nombre_proceso_destino, op_code codigo_operacion);
void agregar_int_a_paquete(t_log *logger, t_paquete *paquete, int entero, const char *nombre_proceso_origen, const char *nombre_proceso_destino, op_code codigo_operacion);
void agregar_int32_a_paquete(t_log *logger, t_paquete *paquete, uint32_t entero, const char *nombre_proceso_origen, const char *nombre_proceso_destino, op_code codigo_operacion);
void agregar_string_a_paquete(t_log *logger, t_paquete *paquete, char *string, const char *nombre_proceso_origen, const char *nombre_proceso_destino, op_code codigo_operacion);
void agregar_lista_de_enteros_a_paquete(t_log *logger, t_list *lista, t_paquete *paquete, const char *nombre_proceso_origen, const char *nombre_proceso_destino, op_code codigo_operacion);
void agregar_void_a_paquete(t_log* logger, t_paquete* paquete, void* data, int data_length, const char* nombre_proceso_origen, const char* nombre_proceso_destino, op_code codigo_operacion);

// Utilidades para recibir
op_code esperar_operacion(t_log *logger, const char *nombre_proceso_que_espera, const char *nombre_proceso_que_manda, int conexion);
void *recibir_paquete(t_log *logger, const char *nombre_proceso_que_espera, const char *nombre_proceso_que_manda, int *tamanio_buffer, int conexion, op_code codigo_operacion);
t_list *leer_lista_de_enteros_desde_buffer_de_paquete(t_log *logger, const char *nombre_proceso_que_lee, const char *nombre_proceso_mando, void **buffer_de_paquete_con_offset, op_code codigo_operacion);
void leer_caracter_desde_buffer_de_paquete(t_log *logger, const char *nombre_proceso_que_lee, const char *nombre_proceso_mando, void **buffer_de_paquete_con_offset, char *caracter, op_code codigo_operacion);
void leer_int_desde_buffer_de_paquete(t_log *logger, const char *nombre_proceso_que_lee, const char *nombre_proceso_mando, void **buffer_de_paquete_con_offset, int *entero, op_code codigo_operacion);
void leer_int32_desde_buffer_de_paquete(t_log *logger, const char *nombre_proceso_que_lee, const char *nombre_proceso_mando, void **buffer_de_paquete_con_offset, uint32_t *entero, op_code codigo_operacion);
void leer_string_desde_buffer_de_paquete(t_log *logger, const char *nombre_proceso_que_lee, const char *nombre_proceso_mando, void **buffer_de_paquete_con_offset, char **string, op_code codigo_operacion);
void leer_void_desde_buffer_de_paquete(t_log* logger, const char* nombre_proceso_que_lee, const char* nombre_proceso_mando, void** buffer_de_paquete_con_offset, void** data, op_code codigo_operacion);

#endif /* UTILIDADES_CONEXION_H_ */