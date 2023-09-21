#include "Headers/utilidades_serializacion.h"

// Kernel _> CPU
t_paquete *crear_paquete_ejecutar_proceso(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
    return crear_paquete_contexto_de_ejecucion(logger, EJECUTAR_PROCESO, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, contexto_de_ejecucion);
}

t_paquete *crear_paquete_devuelvo_proceso_por_ser_interrumpido(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
    return crear_paquete_contexto_de_ejecucion(logger, DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, contexto_de_ejecucion);
}

t_paquete *crear_paquete_devuelvo_proceso_por_correcta_finalizacion(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
    return crear_paquete_contexto_de_ejecucion(logger, DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, contexto_de_ejecucion);
}

t_paquete *crear_paquete_interrumpir_ejecucion(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, INTERRUMPIR_PROCESO, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
}

t_paquete *crear_paquete_handshake_memoria(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, HANDSHAKE_CPU_MEMORIA, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
}

t_paquete *crear_paquete_enviar_instruccion_a_cpu(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, ENVIAR_INSTRUCCION_MEMORIA_A_CPU, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
}

t_paquete *crear_paquete_contexto_de_ejecucion(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, contexto_de_ejecucion->program_counter, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_ax, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_bx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_cx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_dx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);

    log_debug(logger, "Extio en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    return paquete;
}

t_paquete *crear_paquete_con_opcode_y_sin_contenido(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'VACIO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    log_debug(logger, "Extio en la creacion del paquete de codigo de operacion %s y contenido 'VACIO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    return paquete;
}