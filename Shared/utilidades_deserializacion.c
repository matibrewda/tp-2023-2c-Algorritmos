#include "Headers/utilidades_deserializacion.h"

t_contexto_de_ejecucion *leer_paquete_ejecutar_proceso(t_log *logger, int conexion_con_kernel_dispatch)
{
	return leer_paquete_contexto_de_ejecucion(logger, conexion_con_kernel_dispatch, EJECUTAR_PROCESO, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
}

t_contexto_de_ejecucion *leer_paquete_contexto_de_ejecucion(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &tamanio_buffer, conexion, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = malloc(sizeof(t_contexto_de_ejecucion));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(contexto_de_ejecucion->pid), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(contexto_de_ejecucion->program_counter), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(contexto_de_ejecucion->registro_ax), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(contexto_de_ejecucion->registro_bx), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(contexto_de_ejecucion->registro_cx), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(contexto_de_ejecucion->registro_dx), codigo_operacion);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	return contexto_de_ejecucion;
}

t_proceso_memoria *leer_paquete_proceso_memoria(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &tamanio_buffer, conexion, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_proceso_memoria *proceso_memoria = malloc(sizeof(t_proceso_memoria));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_con_offset, &(proceso_memoria->path), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_con_offset, &(proceso_memoria->size), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_con_offset, &(proceso_memoria->prioridad), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_con_offset, &(proceso_memoria->pid), codigo_operacion);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	return proceso_memoria;
}

t_proceso_memoria *leer_paquete_iniciar_proceso_en_memoria(t_log *logger, int conexion_con_kernel)
{
	return leer_paquete_proceso_memoria(logger, conexion_con_kernel, INICIAR_PROCESO_MEMORIA, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
}

t_proceso_memoria *leer_paquete_finalizar_proceso_en_memoria(t_log *logger, int conexion_con_kernel)
{
	return leer_paquete_proceso_memoria(logger, conexion_con_kernel, FINALIZAR_PROCESO_MEMORIA, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
}

char* leer_instrucion_recibida_desde_memoria(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = ENVIAR_INSTRUCCION_MEMORIA_A_CPU;
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	char *instruccion_string;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, &(instruccion_string), codigo_operacion);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	return instruccion_string;
}

t_pedido_instruccion *leer_paquete_pedido_instruccion(t_log *logger, int conexion_con_cpu)
{
	op_code codigo_operacion = SOLICITAR_INSTRUCCION_A_MEMORIA;
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO INSTRUCCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &tamanio_buffer, conexion_con_cpu, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_pedido_instruccion *pedido_instruccion = malloc(sizeof(t_pedido_instruccion));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_instruccion->pid), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_instruccion->pc), codigo_operacion);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO INSTRUCCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	return pedido_instruccion;
}

t_info_memoria* leer_info_inicial_de_memoria_para_cpu(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = ENVIAR_INFO_DE_MEMORIA_INICIAL_PARA_CPU;
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'INFO_MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_info_memoria *info_memoria = malloc(sizeof(t_info_memoria));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, &(info_memoria->tamanio_memoria), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, &(info_memoria->tamanio_pagina), codigo_operacion);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'INFO_MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	return info_memoria;
}