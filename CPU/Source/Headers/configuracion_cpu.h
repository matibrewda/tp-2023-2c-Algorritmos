#ifndef CONFIGURACION_CPU_H_
#define CONFIGURACION_CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <commons/log.h>
#include <commons/config.h>

#include "../../../Shared/Headers/utilidades_configuracion.h"

#define CLAVE_CONFIGURACION_IP_MEMORIA "IP_MEMORIA"
#define CLAVE_CONFIGURACION_PUERTO_MEMORIA "PUERTO_MEMORIA"
#define CLAVE_CONFIGURACION_PUERTO_ESCUCHA_DISPATCH "PUERTO_ESCUCHA_DISPATCH"
#define CLAVE_CONFIGURACION_PUERTO_ESCUCHA_INTERRUPT "PUERTO_ESCUCHA_INTERRUPT"

typedef struct
{
	char *ip_memoria;
	char *puerto_memoria;
	char *puerto_escucha_dispatch;
	char *puerto_escucha_interrupt;
} t_config_cpu;

t_config_cpu *leer_configuracion(t_log *logger, char *ruta_archivo_configuracion);
void destruir_configuracion(t_config_cpu *config_cpu);

#endif /* CONFIGURACION_CPU_H_ */
