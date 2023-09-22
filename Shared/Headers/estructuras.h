#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdint.h>

typedef struct
{
    int pid;
    int prioridad;
    char estado;

    // Registros del CPU
    int program_counter;
    uint32_t registro_ax;
    uint32_t registro_bx;
    uint32_t registro_cx;
    uint32_t registro_dx;

    // tabla de archivos abiertos
    
} t_pcb;

typedef struct
{
    char* nombre_instruccion;
	char* parametro_1_instruccion;
	char* parametro_2_instruccion;
} t_instruccion;

typedef struct
{
    int pid;
    int program_counter;
    uint32_t registro_ax;
    uint32_t registro_bx;
    uint32_t registro_cx;
    uint32_t registro_dx;
} t_contexto_de_ejecucion;

#endif /* ESTRUCTURAS_H_ */