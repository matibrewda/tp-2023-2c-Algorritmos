#include "Headers/utilidades_serializacion.h"

// Kernel a CPU
t_paquete *crear_paquete_solicitud_ejecutar_proceso(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
    return crear_paquete_contexto_de_ejecucion(logger, SOLICITUD_EJECUTAR_PROCESO, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, contexto_de_ejecucion);
}

// Kernel a CPU
t_paquete *crear_paquete_solicitud_interrumpir_proceso(t_log *logger, int motivo_interrupcion)
{
    op_code codigo_operacion = SOLICITUD_INTERRUMPIR_PROCESO;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, motivo_interrupcion, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);

    return paquete;
}

// Kernel a CPU
t_paquete *crear_paquete_respuesta_devolver_proceso_por_ser_interrumpido(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
}

// Kernel a CPU
t_paquete *crear_paquete_respuesta_devolver_proceso_por_correcta_finalizacion(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
}

// Kernel a CPU
t_paquete *crear_paquete_respuesta_devolver_proceso_por_sleep(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_DEVOLVER_PROCESO_POR_SLEEP, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
}

// Kernel a Memoria
t_paquete *crear_paquete_solicitud_iniciar_proceso_en_memoria(t_log *logger, t_proceso_memoria *proceso_memoria)
{
    return crear_paquete_proceso_memoria(logger, SOLICITUD_INICIAR_PROCESO_MEMORIA, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, proceso_memoria);
}

// Kernel a Memoria
t_paquete *crear_paquete_solicitud_finalizar_proceso_en_memoria(t_log *logger, t_proceso_memoria *proceso_memoria)
{
    return crear_paquete_proceso_memoria(logger, SOLICITUD_FINALIZAR_PROCESO_MEMORIA, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, proceso_memoria);
}

// CPU a Kernel
t_paquete *crear_paquete_respuesta_ejecutar_proceso(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_EJECUTAR_PROCESO, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
}

// CPU a Kernel
t_paquete *crear_paquete_respuesta_interrumpir_proceso(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_INTERRUMPIR_PROCESO, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
}

// CPU a Kernel
t_paquete *crear_paquete_solicitud_devolver_proceso_por_ser_interrumpido(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, int motivo_interrupcion)
{
    op_code codigo_operacion = SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + MOTIVO INTERRUPCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);
    agregar_int_a_paquete(logger, paquete, motivo_interrupcion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + MOTIVO INTERRUPCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// CPU a Kernel
t_paquete *crear_paquete_solicitud_devolver_proceso_por_correcta_finalizacion(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
    return crear_paquete_contexto_de_ejecucion(logger, SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, contexto_de_ejecucion);
}

// CPU a Kernel
t_paquete *crear_paquete_solicitud_devolver_proceso_por_sleep(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, int tiempo_sleep)
{
    op_code codigo_operacion = SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + TIEMPO SLEEP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);
    agregar_int_a_paquete(logger, paquete, tiempo_sleep, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + TIEMPO SLEEP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// CPU a Memoria
t_paquete *crear_paquete_solicitud_pedir_instruccion_a_memoria(t_log *logger, t_pedido_instruccion *pedido_instruccion)
{
    op_code codigo_operacion = SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, pedido_instruccion->pid, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pedido_instruccion->pc, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    return paquete;
}

// CPU a Memoria
t_paquete *crear_paquete_solicitud_pedir_info_de_memoria_inicial_para_cpu(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, SOLICITUD_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
}

// Memoria a Kernel
t_paquete *crear_paquete_respuesta_iniciar_proceso_en_memoria(t_log *logger, bool resultado_iniciar_proceso)
{
    op_code codigo_operacion = RESPUESTA_INICIAR_PROCESO_MEMORIA;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, resultado_iniciar_proceso, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// Memoria a Kernel
t_paquete *crear_paquete_respuesta_finalizar_proceso_en_memoria(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_FINALIZAR_PROCESO_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
}

// Memoria a CPU
t_paquete *crear_paquete_respuesta_pedir_instruccion_a_memoria(t_log *logger, char *linea_instruccion)
{
    op_code codigo_operacion = RESPUESTA_PEDIR_INSTRUCCION_A_MEMORIA;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, linea_instruccion, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    return paquete;
}

// Memoria a CPU
t_paquete *crear_paquete_respuesta_pedir_info_de_memoria_inicial_para_cpu(t_log *logger, t_info_memoria *info_memoria)
{
    op_code codigo_operacion = RESPUESTA_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'INFO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, info_memoria->tamanio_memoria, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, info_memoria->tamanio_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'INFO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    return paquete;
}

// Comunes
t_paquete *crear_paquete_proceso_memoria(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_proceso_memoria *proceso_memoria)
{
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, proceso_memoria->path, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, proceso_memoria->size, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, proceso_memoria->prioridad, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, proceso_memoria->pid, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    return paquete;
}

// Comunes
t_paquete *crear_paquete_contexto_de_ejecucion(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, nombre_proceso_origen, nombre_proceso_destino);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    return paquete;
}

// Comunes
t_paquete *crear_paquete_con_opcode_y_sin_contenido(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'VACIO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'VACIO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    return paquete;
}

t_paquete *crear_paquete_pedir_bloques_a_filesystem(t_log *logger, int cantidad_de_bloques)
{
    op_code codigo_operacion = SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CANTIDAD DE BLOQUES' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, cantidad_de_bloques, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CANTIDAD DE BLOQUES' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

t_paquete *crear_paquete_liberar_bloque_en_filesystem(t_log *logger, int posicion_swap)
{
    op_code codigo_operacion = SOLICITUD_LIBERAR_BLOQUES_EN_FILESYSTEM;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

	// RESPETAR EL ORDEN -> SERIALIZACION!
    log_debug(logger, "Agrego la posicion en swap %d a paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", posicion_swap, nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
    agregar_int_a_paquete(logger, paquete, posicion_swap, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

t_paquete *crear_paquete_respuesta_pedido_numero_de_marco(t_log *logger, int numero_de_marco)
{
    op_code codigo_operacion = RESPUESTA_NUMERO_DE_MARCO_A_CPU;
    log_debug(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'NUMERO DE MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

	// RESPETAR EL ORDEN -> SERIALIZACION!
    log_debug(logger, "Agrego el numero de marco %d a paquete de codigo de operacion %s y contenido 'NUMERO DE MARCO' (Origen: %s - Destino %s).", numero_de_marco, nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
    agregar_int_a_paquete(logger, paquete, numero_de_marco, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_debug(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'NUMERO DE MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    return paquete;
}

void agregar_contexto_de_ejecucion_a_paquete(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, t_paquete* paquete, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, contexto_de_ejecucion->pid, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, contexto_de_ejecucion->program_counter, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_ax, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_bx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_cx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_dx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
}