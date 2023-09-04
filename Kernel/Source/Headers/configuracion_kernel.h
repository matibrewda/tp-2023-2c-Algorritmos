#ifndef CONFIGURACION_KERNEL_H_
#define CONFIGURACION_KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <commons/log.h>
#include <commons/config.h>

#include "../../../Shared/Headers/utilidades_configuracion.h"

#define CLAVE_CONFIGURACION_IP_MEMORIA "IP_MEMORIA"
#define CLAVE_CONFIGURACION_PUERTO_MEMORIA "PUERTO_MEMORIA"
#define CLAVE_CONFIGURACION_IP_FILESYSTEM "IP_FILESYSTEM"
#define CLAVE_CONFIGURACION_PUERTO_FILESYSTEM "PUERTO_FILESYSTEM"
#define CLAVE_CONFIGURACION_IP_CPU "IP_CPU"
#define CLAVE_CONFIGURACION_PUERTO_CPU_DISPATCH "PUERTO_CPU_DISPATCH"
#define CLAVE_CONFIGURACION_PUERTO_CPU_INTERRUPT "PUERTO_CPU_INTERRUPT"
#define CLAVE_CONFIGURACION_ALGORITMO_PLANIFICACION "ALGORITMO_PLANIFICACION"
#define CLAVE_CONFIGURACION_QUANTUM "QUANTUM"
#define CLAVE_CONFIGURACION_RECURSOS "RECURSOS"
#define CLAVE_CONFIGURACION_INSTANCIAS_RECURSOS "INSTANCIAS_RECURSOS"
#define CLAVE_CONFIGURACION_GRADO_MULTIPROGRAMACION_INI "GRADO_MULTIPROGRAMACION_INI"

typedef struct
{
	char *ip_memoria;
	char *puerto_memoria;
	char *ip_filesystem;
	char *puerto_filesystem;
	char *ip_cpu;
	char *puerto_cpu_dispatch;
	char *puerto_cpu_interrupt;
	char *algoritmo_planificacion;
	int quantum;
	char **recursos;
	int *instancias_recursos;
	int cantidad_de_recursos;
	int grado_multiprogramacion_inicial;
} t_config_kernel;

t_config_kernel *leer_configuracion(t_log *logger, char *ruta_archivo_configuracion);
void destruir_configuracion(t_config_kernel *config_cliente);

#endif /* CONFIGURACION_KERNEL_H_ */
