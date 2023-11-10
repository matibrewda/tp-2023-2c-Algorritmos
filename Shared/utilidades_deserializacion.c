#include "Headers/utilidades_deserializacion.h"

// Kernel recibe de Memoria
bool leer_paquete_respuesta_iniciar_proceso_en_memoria(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = RESPUESTA_INICIAR_PROCESO_MEMORIA;
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	int respuesta_iniciar_proceso_en_memoria;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_con_offset, &(respuesta_iniciar_proceso_en_memoria), codigo_operacion);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

	return respuesta_iniciar_proceso_en_memoria;
}

// Kernel recibe de CPU
t_contexto_de_ejecucion* leer_paquete_solicitud_devolver_proceso_por_ser_interrumpido(t_log *logger, int conexion_con_cpu_dispatch, int* motivo_interrupcion)
{
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + MOTIVO INTERRUPCION' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &tamanio_buffer, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, &buffer_con_offset);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, motivo_interrupcion, SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	return contexto_de_ejecucion;
}

// Kernel recibe de CPU
t_contexto_de_ejecucion* leer_paquete_solicitud_devolver_proceso_por_sleep(t_log *logger, int conexion_con_cpu_dispatch, int* tiempo_sleep)
{
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + TIEMPO SLEEP' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &tamanio_buffer, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, &buffer_con_offset);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, tiempo_sleep, SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + TIEMPO SLEEP' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	return contexto_de_ejecucion;
}

// CPU recibe de Kernel
t_contexto_de_ejecucion *leer_paquete_solicitud_ejecutar_proceso(t_log *logger, int conexion_con_kernel_dispatch)
{
	return leer_paquete_contexto_de_ejecucion(logger, conexion_con_kernel_dispatch, SOLICITUD_EJECUTAR_PROCESO, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
}

// CPU recibe de Kernel
int leer_paquete_solicitud_interrumpir_proceso(t_log *logger, int conexion_con_kernel_interrupt)
{
	op_code codigo_operacion = SOLICITUD_INTERRUMPIR_PROCESO;
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT, &tamanio_buffer, conexion_con_kernel_interrupt, codigo_operacion);
	void *buffer_con_offset = buffer;

	int motivo_interrupcion;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT, &buffer_con_offset, &(motivo_interrupcion), codigo_operacion);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);

	return motivo_interrupcion;
}

// Memoria recibe de Kernel
t_proceso_memoria *leer_paquete_solicitud_iniciar_proceso_en_memoria(t_log *logger, int conexion_con_kernel)
{
	return leer_paquete_proceso_memoria(logger, conexion_con_kernel, SOLICITUD_INICIAR_PROCESO_MEMORIA, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
}

// Memoria recibe de Kernel
t_proceso_memoria *leer_paquete_solicitud_finalizar_proceso_en_memoria(t_log *logger, int conexion_con_kernel)
{
	return leer_paquete_proceso_memoria(logger, conexion_con_kernel, SOLICITUD_FINALIZAR_PROCESO_MEMORIA, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
}

// CPU recibe de Memoria
t_info_memoria *leer_paquete_respuesta_pedir_info_de_memoria_inicial_para_cpu(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = RESPUESTA_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU;
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

// CPU recibe de Memoria
char *leer_paquete_respuesta_pedir_instruccion_a_memoria(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = RESPUESTA_PEDIR_INSTRUCCION_A_MEMORIA;
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

// Memoria recibe de CPU
t_pedido_instruccion *leer_paquete_solicitud_pedir_instruccion_a_memoria(t_log *logger, int conexion_con_cpu)
{
	op_code codigo_operacion = SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA;
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

t_pedido_numero_de_marco *leer_paquete_solicitud_pedir_numero_de_marco_a_memoria(t_log *logger, int conexion_con_cpu)
{
	op_code codigo_operacion = SOLICITUD_PEDIR_NUMERO_DE_MARCO_A_MEMORIA;
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO NUMERO DE MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &tamanio_buffer, conexion_con_cpu, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_pedido_numero_de_marco *pedido_numero_de_marco = malloc(sizeof(t_pedido_numero_de_marco));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_numero_de_marco->pid), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_numero_de_marco->numero_de_pagina), codigo_operacion);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO NUMERO DE MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	return pedido_numero_de_marco;
}

// Memoria recibe de Filesystem
t_pedido_leer_archivo *leer_paquete_pedido_leer_archivo(t_log *logger, int conexion_con_filsystem)
{
	op_code codigo_operacion = SOLICITUD_LEER_ARCHIVO_MEMORIA;
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO LEER ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &tamanio_buffer, conexion_con_filsystem, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_pedido_leer_archivo *pedido_leer_archivo = malloc(sizeof(t_pedido_leer_archivo));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_string_desde_buffer_de_paquete(logger,NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM,&buffer_con_offset, &(pedido_leer_archivo->informacion), codigo_operacion);
	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO LEER ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

	return pedido_leer_archivo;
}

// Memoria recibe de Filesystem
t_pedido_escribir_archivo *leer_paquete_pedido_escribir_archivo(t_log *logger, int conexion_con_filsystem)
{
	op_code codigo_operacion = SOLICITUD_ESCRIBIR_ARCHIVO_MEMORIA;
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO ESCRIBIR ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &tamanio_buffer, conexion_con_filsystem, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_pedido_escribir_archivo *pedido_escribir_archivo = malloc(sizeof(t_pedido_escribir_archivo));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger,NOMBRE_MODULO_MEMORIA,NOMBRE_MODULO_FILESYSTEM,&buffer_con_offset, &(pedido_escribir_archivo->direccion_fisica), codigo_operacion);
	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO ESCRIBIR ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

	return pedido_escribir_archivo;
}

// Comunes
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

// Comunes
t_contexto_de_ejecucion *leer_paquete_contexto_de_ejecucion(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &tamanio_buffer, conexion, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion, codigo_operacion, nombre_proceso_origen, nombre_proceso_destino, &buffer_con_offset);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	return contexto_de_ejecucion;
}

t_contexto_de_ejecucion *leer_contexto_de_ejecucion_de_paquete(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, void ** buffer_con_offset)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = malloc(sizeof(t_contexto_de_ejecucion));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, buffer_con_offset, &(contexto_de_ejecucion->pid), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, buffer_con_offset, &(contexto_de_ejecucion->program_counter), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, buffer_con_offset, &(contexto_de_ejecucion->registro_ax), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, buffer_con_offset, &(contexto_de_ejecucion->registro_bx), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, buffer_con_offset, &(contexto_de_ejecucion->registro_cx), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, buffer_con_offset, &(contexto_de_ejecucion->registro_dx), codigo_operacion);

	return contexto_de_ejecucion;
}