#ifndef ESTRUCTURAS_MEMORIA_H_
#define ESTRUCTURAS_MEMORIA_H_

typedef struct
{
    void *inicio;
    int tam_memoria;
} t_espacio_usuario_memoria;

typedef struct
{
    int tam_pagina;
} t_tabla_paginas_memoria;

typedef struct
{
    FILE *archivo;
} t_instrucciones_archivo_pseudocodigo;

#endif /* ESTRUCTURAS_MEMORIA_H_ */