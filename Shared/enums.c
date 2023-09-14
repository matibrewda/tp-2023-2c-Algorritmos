#include "Headers/enums.h"

const char* nombre_opcode(op_code opcode)
{
    switch (opcode)
    {
        case MENSAJE_DE_KERNEL: return "'MENSAJE DE KERNEL'";
        case EJECUTAR_PROCESO: return "'EJECUTAR_PROCESO'";
        case INTERRUMPIR_PROCESO: return "'INTERRUMPIR_PROCESO'";
        default: return "NOMBRE DE OPCODE DESCONOCIDO";
    }
}