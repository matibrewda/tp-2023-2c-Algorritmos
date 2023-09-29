#include "Headers/utilidades_serializacion.h"

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

t_paquete *crear_paquete_solicitar_instruccion_a_memoria(t_log *logger, t_pedido_instruccion* pedido_instruccion)
{
    op_code codigo_operacion = SOLICITAR_INSTRUCCION_A_MEMORIA;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, pedido_instruccion->pid, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pedido_instruccion->pc, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, codigo_operacion);

    log_debug(logger, "Extio en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    return paquete;
}

t_paquete *crear_paquete_solicitar_info_de_memoria_inicial_para_cpu(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, SOLICITAR_INFO_DE_MEMORIA_INICIAL_PARA_CPU, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
}

t_paquete *crear_paquete_enviar_instruccion_a_cpu(t_log *logger, char* linea_instruccion)
{
    op_code codigo_operacion = ENVIAR_INSTRUCCION_MEMORIA_A_CPU;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, linea_instruccion, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_debug(logger, "Extio en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    return paquete;
}

t_paquete *crear_paquete_enviar_info_inicial_de_memoria_a_cpu(t_log *logger, t_info_memoria* info_memoria)
{
    op_code codigo_operacion = ENVIAR_INFO_DE_MEMORIA_INICIAL_PARA_CPU;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'INFO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, info_memoria->tamanio_memoria, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, info_memoria->tamanio_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_debug(logger, "Extio en la creacion del paquete de codigo de operacion %s y contenido 'INFO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    return paquete;
}

t_paquete *crear_paquete_estado_iniciar_proceso(t_log *logger, int estado_iniciar_proceso_memoria)
{
    op_code codigo_operacion = ESTADO_INICIAR_PROCESO_MEMORIA;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'ESTADO INICIAR PROCESO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    agregar_int_a_paquete(logger, paquete, estado_iniciar_proceso_memoria, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'ESTADO INICIAR PROCESO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

    return paquete;
}

t_paquete *crear_paquete_iniciar_proceso_en_memoria(t_log *logger, t_proceso_memoria *proceso_memoria)
{
    return crear_paquete_proceso_memoria(logger, INICIAR_PROCESO_MEMORIA, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, proceso_memoria);
}

t_paquete *crear_paquete_finalizar_proceso_en_memoria(t_log *logger, t_proceso_memoria *proceso_memoria)
{
    return crear_paquete_proceso_memoria(logger, INICIAR_PROCESO_MEMORIA, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, proceso_memoria);
}

t_paquete *crear_paquete_proceso_memoria(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_proceso_memoria *proceso_memoria)
{
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, proceso_memoria->path, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, proceso_memoria->size, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, proceso_memoria->prioridad, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, proceso_memoria->pid, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);

    log_debug(logger, "Extio en la creacion del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    return paquete;
}

t_paquete *crear_paquete_contexto_de_ejecucion(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, contexto_de_ejecucion->pid, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
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