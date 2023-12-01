#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

typedef struct
{
    int pid;
    int prioridad;
    char estado;
    char *path;
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
    t_list *recursos_asignados;

} t_pcb;

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
    char *nombre_archivo;
    char *modo_apertura;
    int posicion;
    int direccion_fisica;
    int tamanio;
} t_operacion_filesystem;

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

typedef struct
{
    char *informacion;
} t_pedido_leer_archivo;

typedef struct
{
    int direccion_fisica;
} t_pedido_escribir_archivo;

typedef struct
{
    uint32_t valor_leido;
} t_valor_leido_en_memoria;

typedef struct
{
    /* PEDIIIILLOOOOOOO */
    int pid;
    int numero_de_pagina;
} t_pedido_pagina_en_memoria;

typedef struct
{
    /* ESCRIBIIIILOOOOOOO */
    uint32_t valor_a_escribir;
    int direccion_fisica;
} t_pedido_escribir_valor_en_memoria;

typedef struct
{
    char *nombre;
    int instancias_iniciales;
    int instancias_disponibles;
    t_queue *pcbs_bloqueados;
    t_list *pcbs_asignados;
    pthread_mutex_t mutex_pcbs_bloqueados;
    pthread_mutex_t mutex_pcbs_asignados;
} t_recurso;

typedef struct
{
    int pid;
    bool finalizado;
    int *recursos_asignados;
    int *solicitudes_actuales;
} t_pcb_analisis_deadlock;

typedef struct
{
    t_pcb *pcb;
    int tiempo_sleep;
} t_bloqueo_sleep;

typedef struct
{
    t_list *contenido;
    int largo; // TODO ver si es necesario
}t_contenido_pagina;


#endif /* ESTRUCTURAS_H_ */