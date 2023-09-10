#include "Headers/utilidades_deserializacion.h"

t_pcb *leer_paquete_pcb(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
	log_trace(logger, "Comenzando la lectura del paquete PCB de origen %s, destino %s, y codigo de operacion %s.", nombre_proceso_origen, nombre_proceso_destino, nombre_opcode(codigo_operacion));

	int tamanio_buffer;
	void *buffer = recibir_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &tamanio_buffer, conexion, codigo_operacion);
	void *buffer_con_offset = buffer;

	t_pcb *pcb = malloc(sizeof(t_pcb));

	// RESPETAR EL ORDEN -> DESERIALIZACION!
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pcb->pid), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pcb->prioridad), codigo_operacion);
	leer_caracter_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pcb->estado), codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pcb->pc), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pcb->registro_ax), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pcb->registro_bx), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pcb->registro_cx), codigo_operacion);
	leer_int32_desde_buffer_de_paquete(logger, nombre_proceso_destino, nombre_proceso_origen, &buffer_con_offset, &(pcb->registro_dx), codigo_operacion);

	free(buffer);

	log_trace(logger, "Exito en la lectura del paquete PCB de origen %s, destino %s, y codigo de operacion %s.", nombre_proceso_origen, nombre_proceso_destino, nombre_opcode(codigo_operacion));

	return pcb;
}