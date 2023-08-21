#include "Headers/configuracion_cliente.h"

t_config_cliente* leer_configuracion(t_log* logger, char* ruta_archivo_configuracion)
{
	log_debug(logger, "Comenzando la lectura de archivo de configuracion");

	t_config* configuracion = leer_archivo_configuracion(logger, ruta_archivo_configuracion);
	
	if (configuracion == NULL)
	{
		return NULL;
	}

	char* claves[] = { 
		CLAVE_CONFIGURACION_IP_SERVIDOR, 
		CLAVE_CONFIGURACION_PUERTO_SERVIDOR,
		NULL };

	if (!existen_claves_en_configuracion(logger, configuracion, claves))
	{
		return NULL;
	}

	t_config_cliente* configuracion_cliente = malloc(sizeof (t_config_cliente));
	configuracion_cliente -> ip_servidor = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_IP_SERVIDOR);
	configuracion_cliente -> puerto_servidor = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_SERVIDOR);

	log_debug(logger, "Exito en la lectura de archivo de configuracion");

	config_destroy(configuracion);
	return configuracion_cliente;
}

void destruir_configuracion(t_config_cliente* configuracion_cliente)
{
	if (configuracion_cliente == NULL)
	{
		return;
	}
	
	free(configuracion_cliente -> ip_servidor);
	free(configuracion_cliente -> puerto_servidor);

	free(configuracion_cliente);
}