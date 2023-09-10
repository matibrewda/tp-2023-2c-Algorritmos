#include "Headers/utilidades_serializacion.h"

t_paquete *crear_paquete_pcb(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_pcb *pcb)
{
    log_debug(logger, "Comenzando la creacion del paquete PCB de origen %s, destino %s, y codigo de operacion %s.", nombre_proceso_origen, nombre_proceso_destino, nombre_opcode(codigo_operacion));

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, pcb->pid, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pcb->prioridad, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_caracter_a_paquete(logger, paquete, pcb->estado, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pcb->pc, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, pcb->registro_ax, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, pcb->registro_bx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, pcb->registro_cx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, pcb->registro_dx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete PCB de origen %s, destino %s, y codigo de operacion %s.", nombre_proceso_origen, nombre_proceso_destino, nombre_opcode(codigo_operacion));

    return paquete;
}