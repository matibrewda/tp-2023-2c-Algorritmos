#include "Headers/configuracion_servidor.h"

t_config_servidor* leer_configuracion(t_log* logger, char* ruta_archivo_configuracion)
{
	log_debug(logger, "Comenzando la lectura de archivo de configuracion");

	t_config* configuracion = leer_archivo_configuracion(logger, ruta_archivo_configuracion);
	
	if (configuracion == NULL)
	{
		return NULL;
	}

	char* claves[] = { 
		CLAVE_CONFIGURACION_IP_SERVIDOR, 
		CLAVE_CONFIGURACION_PUERTO_ESCUCHA_A_CLIENTE,
		NULL };

	if (!existen_claves_en_configuracion(logger, configuracion, claves))
	{
		return NULL;
	}

	t_config_servidor* configuracion_servidor = malloc(sizeof (t_config_servidor));
	configuracion_servidor -> ip_servidor = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_IP_SERVIDOR);
	configuracion_servidor -> puerto_escucha_a_cliente = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_ESCUCHA_A_CLIENTE);

	log_debug(logger, "Exito en la lectura de archivo de configuracion");

	config_destroy(configuracion);
	return configuracion_servidor;
}

void destruir_configuracion(t_config_servidor* configuracion_servidor)
{
	if (configuracion_servidor == NULL)
	{
		return;
	}
	
	free(configuracion_servidor -> ip_servidor);
	free(configuracion_servidor -> puerto_escucha_a_cliente);

	free(configuracion_servidor);
}