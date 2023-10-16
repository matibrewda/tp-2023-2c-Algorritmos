#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdint.h>

typedef struct
{
    int pid;
    int prioridad;
    char estado;
    char* path;
    int size;
    bool quantum_finalizado;
    int id_hilo_quantum;

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
    int opcode;
	char* parametro_1;
	char* parametro_2;
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

typedef struct
{
    char *path;
	int size;
	int prioridad;
	int pid;
} t_proceso_memoria;

typedef struct
{
	int tamanio_memoria;
	int tamanio_pagina;
} t_info_memoria;

typedef struct
{
	int pid;
	int pc;
} t_pedido_instruccion;

#endif /* ESTRUCTURAS_H_ */