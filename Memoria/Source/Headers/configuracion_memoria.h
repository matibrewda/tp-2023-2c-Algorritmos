#ifndef CONFIGURACION_MEMORIA_H_
#define CONFIGURACION_MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <commons/log.h>
#include <commons/config.h>

#include "../../../Shared/Headers/utilidades_configuracion.h"

#define CLAVE_CONFIGURACION_PUERTO_ESCUCHA "PUERTO_ESCUCHA"
#define CLAVE_CONFIGURACION_IP_FILESYSTEM "IP_FILESYSTEM"
#define CLAVE_CONFIGURACION_PUERTO_FILESYSTEM "PUERTO_FILESYSTEM"
#define CLAVE_CONFIGURACION_TAM_MEMORIA "TAM_MEMORIA"
#define CLAVE_CONFIGURACION_TAM_PAGINA "TAM_PAGINA"
#define CLAVE_CONFIGURACION_PATH_INSTRUCCIONES "PATH_INSTRUCCIONES"
#define CLAVE_CONFIGURACION_RETARDO_RESPUESTA "RETARDO_RESPUESTA"
#define CLAVE_CONFIGURACION_ALGORITMO_REEMPLAZO "ALGORITMO_REEMPLAZO"

typedef struct
{
	int puerto_escucha;
	char *ip_filesystem;
	int puerto_filesystem;
	int tam_memoria;
	int tam_pagina;
	char *path_instrucciones;
	int retardo_respuesta;
	char *algoritmo_reemplazo;
} t_config_memoria;

t_config_memoria *leer_configuracion(t_log *logger, char *ruta_archivo_configuracion);
void destruir_configuracion(t_config_memoria *config_memoria);

#endif /* CONFIGURACION_MEMORIA_H_ */
