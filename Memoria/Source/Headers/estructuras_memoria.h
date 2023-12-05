#ifndef ESTRUCTURAS_MEMORIA_H_
#define ESTRUCTURAS_MEMORIA_H_

typedef struct
{
    void *inicio;
    int tam_memoria;
} t_espacio_usuario_memoria;

typedef struct
{
    int numero_de_pagina;
    int pid;
    int marco;
    int presencia;
    int modificado;
    int posicion_en_swap;
    time_t timestamp;
} t_entrada_de_tabla_de_pagina;

typedef struct
{
    int numero_de_marco;
    int ocupado; // Ocupado = 1; Desocupado = 0
} t_entrada_de_tabla_de_marco;


typedef struct
{
    FILE *archivo;
    int pid;
} t_archivo_proceso;

#endif /* ESTRUCTURAS_MEMORIA_H_ */