#include "Headers/utilidades_deserializacion.h"

// Kernel recibe de Memoria
bool leer_paquete_respuesta_iniciar_proceso_en_memoria(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = RESPUESTA_INICIAR_PROCESO_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	int respuesta_iniciar_proceso_en_memoria;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_con_offset, &(respuesta_iniciar_proceso_en_memoria), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

	return respuesta_iniciar_proceso_en_memoria;
}

// Kernel recibe de Memoria
bool leer_paquete_respuesta_cargar_pagina_en_memoria(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = RESPUESTA_CARGAR_PAGINA_EN_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	int respuesta_cargar_pagina_en_memoria;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_con_offset, &(respuesta_cargar_pagina_en_memoria), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

	return respuesta_cargar_pagina_en_memoria;
}

// Kernel recibe de Filesystem
void leer_paquete_respuesta_abrir_archivo_fs(t_log *logger, int conexion_con_filesystem, int *existe, int *tamanio_archivo)
{
	op_code codigo_operacion = RESPUESTA_ABRIR_ARCHIVO_FS;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'EXISTE? + TAMANIO ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, &tamanio_buffer, conexion_con_filesystem, codigo_operacion);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, existe, codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, tamanio_archivo, codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'EXISTE? + TAMANIO ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
}

// Kernel recibe de CPU
t_contexto_de_ejecucion *leer_paquete_solicitud_devolver_proceso_por_ser_interrumpido(t_log *logger, int conexion_con_cpu_dispatch, int *motivo_interrupcion)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + MOTIVO INTERRUPCION' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &tamanio_buffer, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, &buffer_con_offset);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, motivo_interrupcion, SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	return contexto_de_ejecucion;
}

// Kernel recibe de CPU
t_contexto_de_ejecucion *leer_paquete_solicitud_devolver_proceso_por_sleep(t_log *logger, int conexion_con_cpu_dispatch, int *tiempo_sleep)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + TIEMPO SLEEP' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &tamanio_buffer, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, &buffer_con_offset);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, tiempo_sleep, SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + TIEMPO SLEEP' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	return contexto_de_ejecucion;
}

// Kernel recibe de CPU
t_contexto_de_ejecucion *leer_paquete_solicitud_devolver_proceso_por_wait(t_log *logger, int conexion_con_cpu_dispatch, char **nombre_recurso)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NOMBRE RECURSO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_WAIT), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &tamanio_buffer, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_WAIT);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_WAIT, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, &buffer_con_offset);
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, nombre_recurso, SOLICITUD_DEVOLVER_PROCESO_POR_WAIT);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NOMBRE RECURSO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_WAIT), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	return contexto_de_ejecucion;
}

// Kernel recibe de CPU
t_contexto_de_ejecucion *leer_paquete_solicitud_devolver_proceso_por_signal(t_log *logger, int conexion_con_cpu_dispatch, char **nombre_recurso)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NOMBRE RECURSO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &tamanio_buffer, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, &buffer_con_offset);
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, nombre_recurso, SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NOMBRE RECURSO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	return contexto_de_ejecucion;
}

// Kernel recibe de CPU
t_contexto_de_ejecucion *leer_paquete_solicitud_devolver_proceso_por_error(t_log *logger, int conexion_con_cpu_dispatch, int *codigo_error)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + CODIGO ERROR' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_ERROR), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &tamanio_buffer, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_ERROR);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_ERROR, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, &buffer_con_offset);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, codigo_error, SOLICITUD_DEVOLVER_PROCESO_POR_ERROR);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + CODIGO ERROR' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_ERROR), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	return contexto_de_ejecucion;
}

// Kernel recibe de CPU
t_contexto_de_ejecucion *leer_paquete_solicitud_devolver_proceso_por_pagefault(t_log *logger, int conexion_con_cpu_dispatch, int *numero_pagina)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NUMERO PAGINA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &tamanio_buffer, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, &buffer_con_offset);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, numero_pagina, SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NUMERO PAGINA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	return contexto_de_ejecucion;
}

// Kernel recibe de CPU
t_contexto_de_ejecucion *leer_paquete_solicitud_devolver_proceso_por_operacion_filesystem(t_log *logger, int conexion_con_cpu_dispatch, char **nombre_archivo, int*modo_apertura, int *posicion_puntero_archivo, int *direccion_fisica, int *nuevo_tamanio_archivo, int *fs_opcode)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + OPERACION FILESYSTEM' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &tamanio_buffer, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion_con_cpu_dispatch, SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, &buffer_con_offset);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, fs_opcode, SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM);
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, nombre_archivo, SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, modo_apertura, SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, posicion_puntero_archivo, SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, direccion_fisica, SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, &buffer_con_offset, nuevo_tamanio_archivo, SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION  + OPERACION FILESYSTEM' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM), NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

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
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT, &tamanio_buffer, conexion_con_kernel_interrupt, codigo_operacion);
	void *buffer_con_offset = buffer;

	int motivo_interrupcion;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT, &buffer_con_offset, &(motivo_interrupcion), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);

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
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'INFO_MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_info_memoria *info_memoria = malloc(sizeof(t_info_memoria));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, &(info_memoria->tamanio_memoria), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, &(info_memoria->tamanio_pagina), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'INFO_MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	return info_memoria;
}

// CPU recibe de Memoria
char *leer_paquete_respuesta_pedir_instruccion_a_memoria(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = RESPUESTA_PEDIR_INSTRUCCION_A_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	char *instruccion_string;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, &(instruccion_string), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	return instruccion_string;
}

// CPU recibe de Memoria
int leer_paquete_respuesta_pedir_numero_de_marco_a_memoria(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = RESPUESTA_NUMERO_DE_MARCO_A_CPU;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	int numero_de_marco;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, &(numero_de_marco), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	return numero_de_marco;
}

// CPU recibe de Memoria
uint32_t leer_paquete_respuesta_leer_valor_en_memoria(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = RESPUESTA_LEER_VALOR_EN_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	uint32_t valor;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int32_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, &(valor), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	return valor;
}

// Memoria recibe de CPU
t_pedido_instruccion *leer_paquete_solicitud_pedir_instruccion_a_memoria(t_log *logger, int conexion_con_cpu)
{
	op_code codigo_operacion = SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO INSTRUCCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &tamanio_buffer, conexion_con_cpu, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_pedido_instruccion *pedido_instruccion = malloc(sizeof(t_pedido_instruccion));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_instruccion->pid), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_instruccion->pc), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO INSTRUCCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	return pedido_instruccion;
}

// Memoria recibe de CPU
t_pedido_leer_valor_de_memoria *leer_paquete_solicitud_leer_valor_en_memoria(t_log *logger, int conexion_con_cpu)
{
	op_code codigo_operacion = SOLICITUD_ESCRIBIR_VALOR_EN_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PID + DIRECCION FISICA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &tamanio_buffer, conexion_con_cpu, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_pedido_leer_valor_de_memoria *pedido_leer_valor_de_memoria = malloc(sizeof(t_pedido_leer_valor_de_memoria));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_leer_valor_de_memoria->pid), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_leer_valor_de_memoria->direccion_fisica), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PID + DIRECCION FISICA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	return pedido_leer_valor_de_memoria;
}

// Memoria recibe de CPU
t_pedido_escribir_valor_en_memoria *leer_paquete_solicitud_escribir_valor_en_memoria(t_log *logger, int conexion_con_cpu)
{
	op_code codigo_operacion = SOLICITUD_ESCRIBIR_VALOR_EN_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PID + DIRECCION FISICA + VALOR' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &tamanio_buffer, conexion_con_cpu, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_pedido_escribir_valor_en_memoria *pedido_escribir_valor_en_memoria = malloc(sizeof(t_pedido_escribir_valor_en_memoria));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_escribir_valor_en_memoria->pid), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_escribir_valor_en_memoria->direccion_fisica), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, &buffer_con_offset, &(pedido_escribir_valor_en_memoria->valor_a_escribir), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PID + DIRECCION FISICA + VALOR' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	return pedido_escribir_valor_en_memoria;
}

// Memoria recibe de Filesystem
void *leer_paquete_respuesta_leer_pagina_en_swap(t_log *logger, int conexion_con_filsystem, int *numero_pagina, int *pid)
{
	op_code codigo_operacion = RESPUESTA_LEER_PAGINA_EN_SWAP;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTENIDO DEL BLOQUE' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &tamanio_buffer, conexion_con_filsystem, codigo_operacion);
	void *buffer_con_offset = buffer;

	void *contenido_del_bloque;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_void_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, &(contenido_del_bloque), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, numero_pagina, codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, pid, codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTENIDO DEL BLOQUE' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

	return contenido_del_bloque;
}

// Memoria recibe de Filesystem
t_list *leer_paquete_respuesta_pedir_bloques_a_filesystem(t_log *logger, int conexion_con_filesystem, int *pid)
{
	op_code codigo_operacion = RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'POSICIONES SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &tamanio_buffer, conexion_con_filesystem, RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM);
	void *buffer_con_offset = buffer;

	t_list *posiciones_swap;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	posiciones_swap = leer_lista_de_enteros_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, pid, RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM);
	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'POSICIONES SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	return posiciones_swap;
}

// Memoria recibe de Filesystem
void leer_paquete_solicitud_escribir_bloque_en_memoria(t_log *logger, int conexion_con_filesystem, int *direccion_fisica, void **contenido_bloque)
{
	op_code codigo_operacion = SOLICITUD_ESCRIBIR_BLOQUE_EN_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &tamanio_buffer, conexion_con_filesystem, codigo_operacion);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, direccion_fisica, codigo_operacion);
	leer_void_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, contenido_bloque, codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

// Memoria recibe de Filesystem
void leer_paquete_solicitud_leer_marco_de_memoria(t_log *logger, int conexion_con_filesystem, int *direccion_fisica, char **nombre_archivo, int *puntero_archivo)
{
	op_code codigo_operacion = SOLICITUD_ESCRIBIR_BLOQUE_EN_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &tamanio_buffer, conexion_con_filesystem, codigo_operacion);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, direccion_fisica, codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, puntero_archivo, codigo_operacion);
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &buffer_con_offset, nombre_archivo, codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

// Filesystem recibe de Memoria
void leer_paquete_solicitud_leer_pagina_swap(t_log *logger, int conexion_con_memoria, int *posicion_swap, int *numero_pagina, int *pid)
{
	op_code codigo_operacion = SOLICITUD_LEER_PAGINA_EN_SWAP;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, posicion_swap, codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, pid, codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, numero_pagina, codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

// Filesystem recibe de Memoria
void leer_paquete_solicitud_escribir_pagina_en_swap(t_log *logger, int conexion_con_memoria, void **contenido_marco, int *posicion_swap)
{
	op_code codigo_operacion = SOLICITUD_ESCRIBIR_PAGINA_EN_SWAP;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'POSICION SWAP Y CONTENIDO PAGINA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_void_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, contenido_marco, codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, posicion_swap, codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'POSICION SWAP Y CONTENIDO PAGINA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

// Filesystem recibe de Memoria
void leer_paquete_respuesta_leer_marco_de_memoria(t_log *logger, int conexion_con_memoria, void **contenido_marco, char **nombre_archivo, int *puntero_archivo)
{
	op_code codigo_operacion = RESPUESTA_LEER_MARCO_DE_MEMORIA;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTENIDO MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, puntero_archivo, codigo_operacion);
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, nombre_archivo, codigo_operacion);
	leer_void_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, contenido_marco, codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTENIDO MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

// Filesystem recibe de Memoria
t_list *leer_paquete_solicitud_liberar_bloques_de_fs(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'NUMEROS DE BLOQUES' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_list *bloques_a_liberar;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	bloques_a_liberar = leer_lista_de_enteros_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'NUMEROS DE BLOQUES' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	return bloques_a_liberar;
}

// Filesystem recibe de Memoria
void leer_paquete_solicitud_pedir_bloques_a_fs(t_log *logger, int conexion_con_memoria, int *cantidad_de_bloques, int *pid)
{
	op_code codigo_operacion = SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM;
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CANTIDAD DE BLOQUES' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, pid, codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, cantidad_de_bloques, codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CANTIDAD DE BLOQUES' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

// Comunes
t_pedido_pagina_en_memoria *leer_paquete_solicitud_pedido_pagina_en_memoria(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO PAGINA EN MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &tamanio_buffer, conexion, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_pedido_pagina_en_memoria *pedido_numero_de_marco = malloc(sizeof(t_pedido_pagina_en_memoria));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pedido_numero_de_marco->pid), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pedido_numero_de_marco->numero_de_pagina), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PEDIDO PAGINA EN MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	return pedido_numero_de_marco;
}

// Comunes
t_proceso_memoria *leer_paquete_proceso_memoria(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

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

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	return proceso_memoria;
}

// Filesystem recibe de Kernel
char *leer_paquete_solicitud_abrir_archivo_fs(t_log *logger, int conexion_con_kernel)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_ABRIR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_kernel, SOLICITUD_ABRIR_ARCHIVO_FS);
	void *buffer_con_offset = buffer;

	char *nombre_archivo;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, &nombre_archivo, SOLICITUD_ABRIR_ARCHIVO_FS);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_ABRIR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	return nombre_archivo;
}

// Filesystem recibe de Kernel
char *leer_paquete_solicitud_crear_archivo_fs(t_log *logger, int conexion_con_kernel)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_CREAR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_kernel, SOLICITUD_CREAR_ARCHIVO_FS);
	void *buffer_con_offset = buffer;

	char *nombre_archivo;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, &nombre_archivo, SOLICITUD_CREAR_ARCHIVO_FS);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_CREAR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	return nombre_archivo;
}

// Filesystem recibe de Kernel
void leer_paquete_solicitud_truncar_archivo_fs(t_log *logger, int conexion_con_kernel, char **nombre_archivo, int *nuevo_tamanio_archivo)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + NUEVO TAMANIO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_TRUNCAR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_kernel, SOLICITUD_TRUNCAR_ARCHIVO_FS);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, nombre_archivo, SOLICITUD_TRUNCAR_ARCHIVO_FS);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, nuevo_tamanio_archivo, SOLICITUD_TRUNCAR_ARCHIVO_FS);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + NUEVO TAMANIO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_TRUNCAR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);
}

// Filesystem recibe de Kernel
void leer_paquete_solicitud_leer_archivo_fs(t_log *logger, int conexion_con_kernel, char **nombre_archivo, int *puntero_archivo_a_leer, int *direccion_fisica_a_escribir)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + PUNTERO + DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_LEER_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_kernel, SOLICITUD_LEER_ARCHIVO_FS);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, nombre_archivo, SOLICITUD_LEER_ARCHIVO_FS);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, puntero_archivo_a_leer, SOLICITUD_LEER_ARCHIVO_FS);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, direccion_fisica_a_escribir, SOLICITUD_LEER_ARCHIVO_FS);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + PUNTERO + DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_LEER_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);
}

// Filesystem recibe de Kernel
void leer_paquete_solicitud_escribir_archivo_fs(t_log *logger, int conexion_con_kernel, char **nombre_archivo, int *puntero_archivo_a_escribir, int *direccion_fisica_a_leer)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + PUNTERO + DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_ESCRIBIR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_kernel, SOLICITUD_ESCRIBIR_ARCHIVO_FS);
	void *buffer_con_offset = buffer;

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, nombre_archivo, SOLICITUD_ESCRIBIR_ARCHIVO_FS);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, puntero_archivo_a_escribir, SOLICITUD_ESCRIBIR_ARCHIVO_FS);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, &buffer_con_offset, direccion_fisica_a_leer, SOLICITUD_ESCRIBIR_ARCHIVO_FS);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + PUNTERO + DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_ESCRIBIR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);
}

// Comunes
t_contexto_de_ejecucion *leer_paquete_contexto_de_ejecucion(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
	log_trace(logger, "Comenzando la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &tamanio_buffer, conexion, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = leer_contexto_de_ejecucion_de_paquete(logger, conexion, codigo_operacion, nombre_proceso_origen, nombre_proceso_destino, &buffer_con_offset);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

	return contexto_de_ejecucion;
}

t_contexto_de_ejecucion *leer_contexto_de_ejecucion_de_paquete(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, void **buffer_con_offset)
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