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
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, motivo_interrupcion, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);

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

// Kernel a CPU
t_paquete *crear_paquete_respuesta_devolver_proceso_por_wait(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_DEVOLVER_PROCESO_POR_WAIT, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
}

// Kernel a CPU
t_paquete *crear_paquete_respuesta_devolver_proceso_por_signal(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_DEVOLVER_PROCESO_POR_SIGNAL, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
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

// Kernel a Memoria
t_paquete *crear_paquete_solicitud_cargar_pagina_en_memoria(t_log *logger, t_pedido_pagina_en_memoria *pedido_pagina_en_memoria)
{
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'PID + NRO PAGINA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_CARGAR_PAGINA_EN_MEMORIA), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, SOLICITUD_CARGAR_PAGINA_EN_MEMORIA);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, pedido_pagina_en_memoria->pid, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, SOLICITUD_CARGAR_PAGINA_EN_MEMORIA);
    agregar_int_a_paquete(logger, paquete, pedido_pagina_en_memoria->numero_de_pagina, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, SOLICITUD_CARGAR_PAGINA_EN_MEMORIA);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_CARGAR_PAGINA_EN_MEMORIA), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);

    return paquete;
}

// Kernel a Filesystem
t_paquete *crear_paquete_solicitud_abrir_archivo_fs(t_log *logger, char *nombre_archivo)
{
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_ABRIR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, SOLICITUD_ABRIR_ARCHIVO_FS);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, nombre_archivo, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_ABRIR_ARCHIVO_FS);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_ABRIR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Kernel a Filesystem
t_paquete *crear_paquete_solicitud_crear_archivo_fs(t_log *logger, char *nombre_archivo)
{
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_CREAR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, SOLICITUD_CREAR_ARCHIVO_FS);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, nombre_archivo, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_CREAR_ARCHIVO_FS);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_CREAR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Kernel a Filesystem
t_paquete *crear_paquete_solicitud_truncar_archivo_fs(t_log *logger, char *nombre_archivo, int nuevo_tamanio_archivo)
{
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + NUEVO TAMANIO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_TRUNCAR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, SOLICITUD_TRUNCAR_ARCHIVO_FS);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, nombre_archivo, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_TRUNCAR_ARCHIVO_FS);
    agregar_int_a_paquete(logger, paquete, nuevo_tamanio_archivo, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_TRUNCAR_ARCHIVO_FS);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + NUEVO TAMANIO' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_TRUNCAR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Kernel a Filesystem
t_paquete *crear_paquete_solicitud_leer_archivo_fs(t_log *logger, char *nombre_archivo, int puntero_archivo, int direccion_fisica)
{
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + PUNTERO + DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_LEER_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, SOLICITUD_LEER_ARCHIVO_FS);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, nombre_archivo, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_LEER_ARCHIVO_FS);
    agregar_int_a_paquete(logger, paquete, puntero_archivo, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_LEER_ARCHIVO_FS);
    agregar_int_a_paquete(logger, paquete, direccion_fisica, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_LEER_ARCHIVO_FS);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + PUNTERO + DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_LEER_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Kernel a Filesystem
t_paquete *crear_paquete_solicitud_escribir_archivo_fs(t_log *logger, char *nombre_archivo, int puntero_archivo, int direccion_fisica)
{
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + PUNTERO + DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_ESCRIBIR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, SOLICITUD_ESCRIBIR_ARCHIVO_FS);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, nombre_archivo, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_ESCRIBIR_ARCHIVO_FS);
    agregar_int_a_paquete(logger, paquete, puntero_archivo, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_ESCRIBIR_ARCHIVO_FS);
    agregar_int_a_paquete(logger, paquete, direccion_fisica, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, SOLICITUD_ESCRIBIR_ARCHIVO_FS);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'NOMBRE ARCHIVO + PUNTERO + DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(SOLICITUD_ESCRIBIR_ARCHIVO_FS), NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
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
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + MOTIVO INTERRUPCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);
    agregar_int_a_paquete(logger, paquete, motivo_interrupcion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + MOTIVO INTERRUPCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

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
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + TIEMPO SLEEP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);
    agregar_int_a_paquete(logger, paquete, tiempo_sleep, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + TIEMPO SLEEP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// CPU a Kernel
t_paquete *crear_paquete_solicitud_devolver_proceso_por_wait(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, char *nombre_recurso)
{
    op_code codigo_operacion = SOLICITUD_DEVOLVER_PROCESO_POR_WAIT;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NOMBRE RECURSO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);
    agregar_string_a_paquete(logger, paquete, nombre_recurso, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NOMBRE RECURSO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// CPU a Kernel
t_paquete *crear_paquete_solicitud_devolver_proceso_por_signal(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, char *nombre_recurso)
{
    op_code codigo_operacion = SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NOMBRE RECURSO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);
    agregar_string_a_paquete(logger, paquete, nombre_recurso, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NOMBRE RECURSO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// CPU a Kernel
t_paquete *crear_paquete_solicitud_devolver_proceso_por_error(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, int codigo_error)
{
    op_code codigo_operacion = SOLICITUD_DEVOLVER_PROCESO_POR_ERROR;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + CODIGO ERROR' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);
    agregar_int_a_paquete(logger, paquete, codigo_error, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + CODIGO ERROR' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// CPU a Kernel
t_paquete *crear_paquete_solicitud_devolver_proceso_por_pagefault(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, int numero_pagina)
{
    op_code codigo_operacion = SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NUMERO PAGINA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);
    agregar_int_a_paquete(logger, paquete, numero_pagina, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + NUMERO PAGINA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// CPU a Kernel
t_paquete *crear_paquete_solicitud_devolver_proceso_por_operacion_filesystem(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, t_operacion_filesystem *operacion_filesystem)
{
    op_code codigo_operacion = SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + OPERACION FILESYSTEM' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);
    agregar_int_a_paquete(logger, paquete, operacion_filesystem->fs_opcode, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);
    agregar_string_a_paquete(logger, paquete, operacion_filesystem->nombre_archivo, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, operacion_filesystem->modo_apertura, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, operacion_filesystem->posicion, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, operacion_filesystem->direccion_fisica, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, operacion_filesystem->tamanio, NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION + OPERACION FILESYSTEM' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// CPU a Memoria
t_paquete *crear_paquete_solicitud_pedir_instruccion_a_memoria(t_log *logger, t_pedido_instruccion *pedido_instruccion)
{
    op_code codigo_operacion = SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, pedido_instruccion->pid, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pedido_instruccion->pc, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

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
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, resultado_iniciar_proceso, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// Memoria a Kernel
t_paquete *crear_paquete_respuesta_finalizar_proceso_en_memoria(t_log *logger)
{
    return crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_FINALIZAR_PROCESO_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
}

// Memoria a Kernel
t_paquete *crear_paquete_respuesta_cargar_pagina_en_memoria(t_log *logger, bool resultado_cargar_pagina)
{
    op_code codigo_operacion = RESPUESTA_CARGAR_PAGINA_EN_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, resultado_cargar_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    return paquete;
}

// Memoria a CPU
t_paquete *crear_paquete_respuesta_pedir_instruccion_a_memoria(t_log *logger, char *linea_instruccion)
{
    op_code codigo_operacion = RESPUESTA_PEDIR_INSTRUCCION_A_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, linea_instruccion, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    return paquete;
}

// Memoria a CPU
t_paquete *crear_paquete_respuesta_pedir_info_de_memoria_inicial_para_cpu(t_log *logger, t_info_memoria *info_memoria)
{
    op_code codigo_operacion = RESPUESTA_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'INFO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, info_memoria->tamanio_memoria, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, info_memoria->tamanio_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'INFO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    return paquete;
}

// Filesystem a Kernel
t_paquete *crear_paquete_respuesta_abrir_archivo_fs(t_log *logger, bool existe_archivo, int tamanio_archivo)
{
    op_code codigo_operacion = RESPUESTA_ABRIR_ARCHIVO_FS;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'INFO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, existe_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, tamanio_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'INFO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);

    return paquete;
}

// Comunes
t_paquete *crear_paquete_proceso_memoria(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_proceso_memoria *proceso_memoria)
{
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_string_a_paquete(logger, paquete, proceso_memoria->path, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, proceso_memoria->size, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, proceso_memoria->prioridad, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, proceso_memoria->pid, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'PROCESO MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    return paquete;
}

// Comunes
t_paquete *crear_paquete_contexto_de_ejecucion(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_contexto_de_ejecucion_a_paquete(logger, contexto_de_ejecucion, paquete, codigo_operacion, nombre_proceso_origen, nombre_proceso_destino);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTEXTO DE EJECUCION' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    return paquete;
}

// Comunes
t_paquete *crear_paquete_con_opcode_y_sin_contenido(t_log *logger, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'VACIO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'VACIO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_proceso_origen, nombre_proceso_destino);

    return paquete;
}

t_paquete *crear_paquete_pedir_bloques_a_filesystem(t_log *logger, int pid, int cantidad_de_bloques)
{
    op_code codigo_operacion = SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CANTIDAD DE BLOQUES' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, pid, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, cantidad_de_bloques, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CANTIDAD DE BLOQUES' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Filesystem a memoria
t_paquete *crear_paquete_respuesta_leer_pagina_swap(t_log *logger, void* contenido_pagina, int tamanio_pagina, int numero_pagina, int pid)
{
    op_code codigo_operacion = RESPUESTA_LEER_PAGINA_EN_SWAP;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTENIDO PAGINA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_void_a_paquete(logger, paquete, contenido_pagina, tamanio_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, numero_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pid, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTENIDO PAGINA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Filesystem a memoria
t_paquete *crear_paquete_respuesta_pedir_bloques_a_filesystem(t_log *logger, t_list *lista_bloques_reservados, int pid)
{
    op_code codigo_operacion = RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'LISTA BLOQUES + PID' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_lista_de_enteros_a_paquete(logger, lista_bloques_reservados, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pid, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'LISTA BLOQUES + PID' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Filesystem a memoria
t_paquete *crear_paquete_solicitud_escribir_bloque_en_memoria(t_log *logger, int direccion_fisica, void *contenido_bloque, int tamanio_bloque)
{
    op_code codigo_operacion = SOLICITUD_ESCRIBIR_BLOQUE_EN_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'DIR FISICA + CONTENIDO BLOQUE' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, direccion_fisica, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_void_a_paquete(logger, paquete, contenido_bloque, tamanio_bloque, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'DIR FISICA + CONTENIDO BLOQUE' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

    return paquete;
}

// Filesystem a memoria
t_paquete *crear_paquete_solicitud_leer_marco_de_memoria(t_log *logger, int direccion_fisica, char* nombre_archivo, int puntero_archivo)
{
    op_code codigo_operacion = SOLICITUD_LEER_MARCO_DE_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, direccion_fisica, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, puntero_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_string_a_paquete(logger, paquete, nombre_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'DIR FISICA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);

    return paquete;
}

// Memoria a Filesystem
t_paquete *crear_paquete_liberar_bloques_en_filesystem(t_log *logger, t_list *bloques_swap)
{
    op_code codigo_operacion = SOLICITUD_LIBERAR_BLOQUES_EN_FILESYSTEM;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'LISTA BLOQUES SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_lista_de_enteros_a_paquete(logger, bloques_swap, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Memoria a Filesystem
t_paquete *crear_paquete_respuesta_leer_marco_de_memoria(t_log *logger, char* nombre_archivo_a_escribir, int puntero_archivo_a_escribir, void* contenido_marco, int tamanio_marco)
{
    op_code codigo_operacion = RESPUESTA_LEER_MARCO_DE_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'CONTENIDO_MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, puntero_archivo_a_escribir, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);
    agregar_string_a_paquete(logger, paquete, nombre_archivo_a_escribir, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);
    agregar_void_a_paquete(logger, paquete, contenido_marco, tamanio_marco, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'CONTENIDO_MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Memoria a Filesystem
t_paquete *crear_paquete_solicitud_leer_pagina_swap(t_log *logger, int posicion_swap, int pid, int numero_de_pagina)
{
    op_code codigo_operacion = SOLICITUD_LEER_PAGINA_EN_SWAP;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    log_trace(logger, "Agrego la posicion en swap %d a paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", posicion_swap, nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
    agregar_int_a_paquete(logger, paquete, posicion_swap, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pid, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, numero_de_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Memoria a Filesystem
t_paquete *crear_paquete_solicitud_escribir_pagina_en_swap(t_log *logger, void *contenido_marco, int tamanio_bloque, int posicion_swap)
{
    op_code codigo_operacion = SOLICITUD_ESCRIBIR_PAGINA_EN_SWAP;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'MARCO Y NUMERO DE BLOQUE' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    log_trace(logger, "Agrego el contenido del marco con tamaño %d a paquete de codigo de operacion %s y contenido 'MARCO Y NUMERO DE BLOQUE' (Origen: %s - Destino %s).", tamanio_bloque, nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
    agregar_void_a_paquete(logger, paquete, contenido_marco, tamanio_bloque, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_trace(logger, "Agrego la posicion en swap %d a paquete de codigo de operacion %s y contenido 'POSICION SWAP' (Origen: %s - Destino %s).", posicion_swap, nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
    agregar_int_a_paquete(logger, paquete, posicion_swap, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'MARCO Y NUMERO DE BLOQUE' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

    return paquete;
}

// Memoria a CPU
t_paquete *crear_paquete_respuesta_pedido_numero_de_marco(t_log *logger, int numero_de_marco)
{
    op_code codigo_operacion = RESPUESTA_NUMERO_DE_MARCO_A_CPU;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'NUMERO DE MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    log_trace(logger, "Agrego el numero de marco %d a paquete de codigo de operacion %s y contenido 'NUMERO DE MARCO' (Origen: %s - Destino %s).", numero_de_marco, nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
    agregar_int_a_paquete(logger, paquete, numero_de_marco, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'NUMERO DE MARCO' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

    return paquete;
}

// CPU a Memoria
t_paquete *crear_paquete_solicitud_pedido_numero_de_marco(t_log *logger, t_pedido_pagina_en_memoria *pedido_pagina_en_memoria)
{
    op_code codigo_operacion = SOLICITUD_PEDIR_NUMERO_DE_MARCO_A_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'NUMERO DE PAGINA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    log_trace(logger, "Agrego el numero de pagina %d y PID %d a paquete de codigo de operacion %s y contenido 'NUMERO DE PAGINA' (Origen: %s - Destino %s).", pedido_pagina_en_memoria->numero_de_pagina, pedido_pagina_en_memoria->pid, nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
    agregar_int_a_paquete(logger, paquete, pedido_pagina_en_memoria->pid, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pedido_pagina_en_memoria->numero_de_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'NUMERO DE PAGINA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    return paquete;
}

// CPU a Memoria
t_paquete *crear_paquete_solicitud_leer_valor_en_memoria(t_log *logger, t_pedido_leer_valor_de_memoria *pedido_leer_valor_de_memoria)
{
    op_code codigo_operacion = SOLICITUD_LEER_VALOR_EN_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'PID + DIRECCION FISICA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger,paquete, pedido_leer_valor_de_memoria->pid, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pedido_leer_valor_de_memoria->direccion_fisica, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'PID + DIRECCION FISICA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    return paquete;
}

// CPU a Memoria
t_paquete *crear_paquete_solicitud_escribir_valor_en_memoria(t_log *logger, t_pedido_escribir_valor_en_memoria *pedido_escribir_valor_en_memoria, char *nombre_modulo_origen)
{
    op_code codigo_operacion = SOLICITUD_ESCRIBIR_VALOR_EN_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'PID + DIRECCION FISICA + VALOR' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_modulo_origen, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger,paquete,pedido_escribir_valor_en_memoria->pid, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, pedido_escribir_valor_en_memoria->direccion_fisica, nombre_modulo_origen, NOMBRE_MODULO_MEMORIA, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, pedido_escribir_valor_en_memoria->valor_a_escribir, nombre_modulo_origen, NOMBRE_MODULO_MEMORIA, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'PID + DIRECCION FISICA + VALOR' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), nombre_modulo_origen, NOMBRE_MODULO_MEMORIA);

    return paquete;
}

// Memoria a CPU
t_paquete *crear_paquete_respuesta_leer_valor_en_memoria(t_log *logger, t_valor_leido_en_memoria *valor_leido_en_memoria)
{
    op_code codigo_operacion = RESPUESTA_LEER_VALOR_EN_MEMORIA;
    log_trace(logger, "Comenzando la creacion del paquete de codigo de operacion %s y contenido 'VALOR LEIDO EN MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    t_paquete *paquete = crear_paquete(logger, codigo_operacion);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int32_a_paquete(logger, paquete, valor_leido_en_memoria->valor_leido, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, codigo_operacion);

    log_trace(logger, "Exito en la creacion del paquete de codigo de operacion %s y contenido 'VALOR LEIDO EN MEMORIA' (Origen: %s - Destino %s).", nombre_opcode(codigo_operacion), NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

    return paquete;
}

void agregar_contexto_de_ejecucion_a_paquete(t_log *logger, t_contexto_de_ejecucion *contexto_de_ejecucion, t_paquete *paquete, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino)
{
    // RESPETAR EL ORDEN -> SERIALIZACION!
    agregar_int_a_paquete(logger, paquete, contexto_de_ejecucion->pid, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int_a_paquete(logger, paquete, contexto_de_ejecucion->program_counter, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_ax, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_bx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_cx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
    agregar_int32_a_paquete(logger, paquete, contexto_de_ejecucion->registro_dx, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
}