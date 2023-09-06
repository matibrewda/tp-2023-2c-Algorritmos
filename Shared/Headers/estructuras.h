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

#endif /* ESTRUCTURAS_H_ */