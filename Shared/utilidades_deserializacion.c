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

int leer_info_inicial_de_memoria_para_cpu(t_log *logger, int conexion_con_memoria)
{
	op_code codigo_operacion = DEVOLVER_INFO_DE_MEMORIA_INICIAL_PARA_CPU;
	log_debug(logger, "Comenzando la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &tamanio_buffer, conexion_con_memoria, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_contexto_de_ejecucion *contexto_de_ejecucion = malloc(sizeof(t_contexto_de_ejecucion));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	int tamanio_memoria;
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, &buffer_con_offset, &tamanio_memoria, codigo_operacion);

	free(buffer);

	log_debug(logger, "Exito en la lectura del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	return tamanio_memoria;
}