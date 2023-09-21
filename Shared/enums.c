#include "Headers/enums.h"

const char* nombre_opcode(op_code opcode)
{
    switch (opcode)
    {
        case MENSAJE_DE_KERNEL: return "'MENSAJE DE KERNEL'";
        case EJECUTAR_PROCESO: return "'EJECUTAR_PROCESO'";
        case INTERRUMPIR_PROCESO: return "'INTERRUMPIR_PROCESO'";
        case DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO: return "'DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO'";
        case DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION: return "'DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION'";
        default: return "NOMBRE DE OPCODE DESCONOCIDO";
    }
}