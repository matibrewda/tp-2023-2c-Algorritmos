#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdint.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

#include "../../Shared/Headers/utilidades_logger.h"

typedef struct
{
	char* nombre;
	int instancias_iniciales;
    int instancias_disponibles;
    t_list* pcbs_asignados;
	t_queue* pcbs_bloqueados;
} t_rec;

typedef struct
{
	int pid;
} t_mipcb;

typedef struct
{
	int pid;
	bool finalizado;
	int* recursos_asignados;
	int* solicitudes_actuales;
} t_pcb_analisis_deadlock;

void crear_recursos();
t_rec *crear_recurso(char *nombre, int instancias);
int* obtener_vector_recursos_disponibles();
int* obtener_vector_recursos_totales();
t_rec *buscar_recurso_por_nombre(char *nombre_recurso);
bool hay_deadlock();
t_list *obtener_procesos_analisis_deadlock();
void loguear_vector(int* vector, int tamanio, char* nombre, int pid);

#endif /* MAIN_H_ */