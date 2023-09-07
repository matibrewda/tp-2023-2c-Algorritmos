#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

typedef struct
{
	int pid;
    int pc;
    int prioridad;
    // registros CPU
    // tabla de archivos abiertos
    char estado;
} t_pcb;

typedef struct
{
    char* nombre_instruccion;
	char* parametro_1_instruccion;
	char* parametro_2_instruccion;
} t_instruccion;

#endif /* ESTRUCTURAS_H_ */