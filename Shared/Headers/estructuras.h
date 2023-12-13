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
    int id_hilo_quantum;
    int motivo_finalizacion;
    char *ultimo_recurso_pedido;
    bool quantum_finalizado;

    // Registros del CPU
    int program_counter;
    uint32_t registro_ax;
    uint32_t registro_bx;
    uint32_t registro_cx;
    uint32_t registro_dx;

    t_list* tabla_archivos;
    pthread_mutex_t mutex_tabla_archivos;
} t_pcb;

typedef struct
{
    char* nombre_archivo;
    int puntero;
    int modo_apertura;
} t_archivo_abierto_proceso;

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
    fs_op_code fs_opcode;
    char *nombre_archivo;
    int modo_apertura;
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
    int pid;
    int numero_de_pagina;
} t_pedido_pagina_en_memoria;

typedef struct
{
    int pid;
    uint32_t valor_a_escribir;
    int direccion_fisica;
} t_pedido_escribir_valor_en_memoria;

typedef struct 
{
    int pid;
    int direccion_fisica;
}t_pedido_leer_valor_de_memoria;

typedef struct
{
    char *nombre; // Nombre de recurso o nombre de archivo
    int instancias_iniciales;
    int instancias_disponibles;
    t_queue *pcbs_bloqueados;
    t_list *pcbs_asignados;
    
    bool es_archivo;
    int tamanio_archivo;
    t_pcb* pcb_lock_escritura;
    t_list* pcbs_lock_lectura;
    t_queue* pcbs_bloqueados_por_archivo;

    pthread_mutex_t mutex_recurso;
} t_recurso;

typedef struct
{
    t_pcb* pcb;
    int lock;
} t_pcb_bloqueado_archivo;

typedef struct
{
    int pid;
    bool finalizado;
    int *recursos_asignados;
    int *solicitudes_actuales;
    char *ultimo_recurso_pedido;
} t_pcb_analisis_deadlock;

typedef struct
{
    t_pcb *pcb;
    int tiempo_sleep;
} t_bloqueo_sleep;

typedef struct
{
    t_pcb *pcb;
    char* nombre_archivo;
    int codigo_operacion_archivo;
    int modo_apertura;
    int puntero;
    int nuevo_tamanio;
    int direccion_fisica;
} t_operacion_archivo;

typedef struct
{
   char *path;
   int size;
   int prioridad;
} t_iniciar_proceso;

typedef struct
{
   int pid;
} t_finalizar_proceso;

typedef struct
{
    t_pcb *pcb;
    int numero_pagina;
} t_bloqueo_page_fault;

typedef struct
{
    int pid_a_interrumpir;
    int id_hilo_quantum;
} t_arg_hilo_quantum;

typedef struct
{
    char *nombre_archivo;
    uint32_t tamanio_archivo;
    uint32_t bloque_inicial;
} FCB;

#endif /* ESTRUCTURAS_H_ */