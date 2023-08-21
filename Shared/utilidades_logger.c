#include "Headers/utilidades_logger.h"

t_log* crear_logger(char* ruta_archivo_de_logs, char* nombre_modulo, t_log_level log_level)
{
	t_log* logger = log_create(ruta_archivo_de_logs, nombre_modulo, false, log_level);
	
	if (logger == NULL)
	{
		printf("Error al crear logger para %s", nombre_modulo);
		return NULL;
	}

	return logger;
}

void destruir_logger(t_log* logger)
{
	if (logger == NULL)
	{
		return;
	}
	
	log_destroy(logger);
}