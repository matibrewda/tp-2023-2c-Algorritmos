#ifndef CONFIGURACION_MEMORIA_H_
#define CONFIGURACION_MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <commons/log.h>
#include <commons/config.h>

#include "../../../Shared/Headers/utilidades_configuracion.h"

#define CLAVE_CONFIGURACION_PUERTO_ESCUCHA_KERNEL "PUERTO_ESCUCHA_KERNEL"
#define CLAVE_CONFIGURACION_PUERTO_ESCUCHA_FILESYSTEM "PUERTO_ESCUCHA_FILESYSTEM"
#define CLAVE_CONFIGURACION_PUERTO_ESCUCHA_CPU "PUERTO_ESCUCHA_CPU"
#define CLAVE_CONFIGURACION_TAM_MEMORIA "TAM_MEMORIA"
#define CLAVE_CONFIGURACION_TAM_PAGINA "TAM_PAGINA"
#define CLAVE_CONFIGURACION_PATH_INSTRUCCIONES "PATH_INSTRUCCIONES"
#define CLAVE_CONFIGURACION_RETARDO_RESPUESTA "RETARDO_RESPUESTA"
#define CLAVE_CONFIGURACION_ALGORITMO_REEMPLAZO "ALGORITMO_REEMPLAZO"

typedef struct
{
	char *puerto_escucha_kernel;
	char *puerto_escucha_filesystem;
	char *puerto_escucha_cpu;
	int tam_memoria;
	int tam_pagina;
	char *path_instrucciones;
	int retardo_respuesta;
	char *algoritmo_reemplazo;
} t_config_memoria;

t_config_memoria *leer_configuracion(t_log *logger, char *ruta_archivo_configuracion);
void destruir_configuracion(t_config_memoria *config_memoria);

#endif /* CONFIGURACION_MEMORIA_H_ */
