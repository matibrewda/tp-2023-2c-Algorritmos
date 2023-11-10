#include "Headers/enums.h"

const char* nombre_opcode(op_code opcode)
{
    switch (opcode)
    {
        // Kernel a CPU
        case SOLICITUD_EJECUTAR_PROCESO: return "'SOLICITUD_EJECUTAR_PROCESO'";
        case SOLICITUD_INTERRUMPIR_PROCESO: return "'SOLICITUD_INTERRUMPIR_PROCESO'";
        case RESPUESTA_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO: return "'RESPUESTA_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO'";
        case RESPUESTA_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION: return "'RESPUESTA_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION'";
        case RESPUESTA_DEVOLVER_PROCESO_POR_SLEEP: return "'RESPUESTA_DEVOLVER_PROCESO_POR_SLEEP'";
        
        // Kernel a Memoria
        case SOLICITUD_INICIAR_PROCESO_MEMORIA: return "'SOLICITUD_INICIAR_PROCESO_MEMORIA'";
        case SOLICITUD_FINALIZAR_PROCESO_MEMORIA: return "'SOLICITUD_FINALIZAR_PROCESO_MEMORIA'";
        
        // CPU a Kernel
        case RESPUESTA_EJECUTAR_PROCESO: return "'RESPUESTA_EJECUTAR_PROCESO'";
        case RESPUESTA_INTERRUMPIR_PROCESO: return "'RESPUESTA_INTERRUMPIR_PROCESO'";
        case SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO: return "'SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO'";
        case SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION: return "'SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION'";
        case SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP: return "'SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP'";

        // CPU a Memoria
        case SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA: return "'SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA'";
        case SOLICITUD_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU: return "'SOLICITUD_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU'";

        // Memoria a Kernel
        case RESPUESTA_INICIAR_PROCESO_MEMORIA: return "'RESPUESTA_INICIAR_PROCESO_MEMORIA'";
        case RESPUESTA_FINALIZAR_PROCESO_MEMORIA: return "'RESPUESTA_FINALIZAR_PROCESO_MEMORIA'";
        
        // Memoria a CPU
        case RESPUESTA_PEDIR_INSTRUCCION_A_MEMORIA: return "'RESPUESTA_PEDIR_INSTRUCCION_A_MEMORIA'";
        case RESPUESTA_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU: return "'RESPUESTA_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU'";

        // Memoria a Filesystem
        case SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM: return "'SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM'";
        case SOLICITUD_LIBERAR_BLOQUES_EN_FILESYSTEM: return "'SOLICITUD_LIBERAR_BLOQUES_EN_FILESYSTEM'";
        case RESPUESTA_LEER_ARCHIVO_MEMORIA: return "'RESPUESTA_LEER_ARCHIVO_MEMORIA'";
	    case RESPUESTA_ESCRIBIR_ARCHIVO_MEMORIA: return "'RESPUESTA_ESCRIBIR_ARCHIVO_MEMORIA'";

        // Filesystem a Memoria
        case SOLICITUD_LEER_ARCHIVO_MEMORIA: return "'SOLICITUD_LEER_ARCHIVO_MEMORIA'";
	    case SOLICITUD_ESCRIBIR_ARCHIVO_MEMORIA: return "'SOLICITUD_ESCRIBIR_ARCHIVO_MEMORIA'";
        case RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM: return "'RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM'";
        
        // ERROR
        default: return "NOMBRE DE OPCODE DESCONOCIDO";
    }
}