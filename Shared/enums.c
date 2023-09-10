#include "Headers/enums.h"

const char* nombre_opcode(op_code opcode)
{
    switch (opcode)
    {
        case MENSAJE_DE_KERNEL: return "'MENSAJE DE KERNEL'";
        default: return "NOMBRE DE OPCODE DESCONOCIDO";
    }
}