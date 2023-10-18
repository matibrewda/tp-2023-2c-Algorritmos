#include "Headers/enums.h"

const char* nombre_opcode(op_code opcode)
{
    switch (opcode)
    {
        case EJECUTAR_PROCESO: return "'EJECUTAR_PROCESO'";
        case INTERRUMPIR_PROCESO: return "'INTERRUMPIR_PROCESO'";
        case DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO: return "'DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO'";
        case DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION: return "'DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION'";
        case SOLICITAR_INFO_DE_MEMORIA_INICIAL_PARA_CPU: return "'SOLICITAR_INFO_DE_MEMORIA_INICIAL_PARA_CPU'";
        case ENVIAR_INFO_DE_MEMORIA_INICIAL_PARA_CPU: return "'ENVIAR_INFO_DE_MEMORIA_INICIAL_PARA_CPU'";
        case INICIAR_PROCESO_MEMORIA: return "'INICIAR_PROCESO_MEMORIA'";
        case ESTADO_INICIAR_PROCESO_MEMORIA: return "'ESTADO_INICIAR_PROCESO_MEMORIA'";
        case ENVIAR_INSTRUCCION_MEMORIA_A_CPU: return "'ENVIAR_INSTRUCCION_MEMORIA_A_CPU'";
        case SOLICITAR_INSTRUCCION_A_MEMORIA: return "'SOLICITAR_INSTRUCCION_A_MEMORIA'";
        case FINALIZAR_PROCESO_MEMORIA: return "'FINALIZAR_PROCESO_MEMORIA'";
        case LEER_ARCHIVO_MEMORIA: return "'LEER_ARCHIVO_MEMORIA'";
        case ESCRIBIR_ARCHIVO_MEMORIA: return "'ESCRIBIR_ARCHIVO_MEMORIA'";
        case PEDIR_BLOQUES_A_FILESYSTEM: return "'PEDIR_BLOQUES_A_FILESYSTEM'";
        case LIBERAR_BLOQUES_EN_FILESYSTEM: return "'LIBERAR_BLOQUES_EN_FILESYSTEM'";
        default: return "NOMBRE DE OPCODE DESCONOCIDO";
    }
}