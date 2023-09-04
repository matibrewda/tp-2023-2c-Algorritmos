#include "Headers/configuracion_memoria.h"

t_config_memoria *leer_configuracion(t_log *logger, char *ruta_archivo_configuracion)
{
	log_debug(logger, "Comenzando la lectura de archivo de configuracion");

	t_config *configuracion = leer_archivo_configuracion(logger, ruta_archivo_configuracion);

	if (configuracion == NULL)
	{
		return NULL;
	}

	char *claves[] = {
		CLAVE_CONFIGURACION_PUERTO_ESCUCHA,
		CLAVE_CONFIGURACION_IP_FILESYSTEM,
		CLAVE_CONFIGURACION_PUERTO_FILESYSTEM,
		CLAVE_CONFIGURACION_TAM_MEMORIA,
		CLAVE_CONFIGURACION_TAM_PAGINA,
		CLAVE_CONFIGURACION_PATH_INSTRUCCIONES,
		CLAVE_CONFIGURACION_RETARDO_RESPUESTA,
		CLAVE_CONFIGURACION_ALGORITMO_REEMPLAZO,
		NULL};

	if (!existen_claves_en_configuracion(logger, configuracion, claves))
	{
		return NULL;
	}

	t_config_memoria *configuracion_memoria = malloc(sizeof(t_config_memoria));
	configuracion_memoria->puerto_escucha = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_ESCUCHA);
	configuracion_memoria->ip_filesystem = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_IP_FILESYSTEM);
	configuracion_memoria->puerto_filesystem = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_FILESYSTEM);
	configuracion_memoria->tam_memoria = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_TAM_MEMORIA);
	configuracion_memoria->tam_pagina = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_TAM_PAGINA);
	configuracion_memoria->path_instrucciones = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PATH_INSTRUCCIONES);
	configuracion_memoria->retardo_respuesta = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_RETARDO_RESPUESTA);
	configuracion_memoria->algoritmo_reemplazo = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_ALGORITMO_REEMPLAZO);

	log_debug(logger, "Exito en la lectura de archivo de configuracion");

	config_destroy(configuracion);
	return configuracion_memoria;
}

void destruir_configuracion(t_config_memoria *configuracion_memoria)
{
	if (configuracion_memoria == NULL)
	{
		return;
	}

	free(configuracion_memoria->ip_filesystem);
	free(configuracion_memoria->path_instrucciones);
	free(configuracion_memoria->algoritmo_reemplazo);

	free(configuracion_memoria);
}