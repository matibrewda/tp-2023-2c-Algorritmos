#include "Headers/argumentos_servidor.h"

t_argumentos_servidor* leer_argumentos(t_log* logger, int cantidad_argumentos_recibidos, char **argumentos)
{
	log_debug(logger, "Comenzando la lectura de argumentos");

	if (!cantidad_de_argumentos_es_valida(logger, cantidad_argumentos_recibidos, CANTIDAD_ARGUMENTOS_ESPERADOS))
	{
		return NULL;
	}

	log_trace(logger, "La cantidad de argumentos recibida es la esperada: %d", CANTIDAD_ARGUMENTOS_ESPERADOS);

	t_argumentos_servidor* argumentos_servidor = malloc(sizeof (t_argumentos_servidor));
	
	argumentos_servidor -> ruta_archivo_configuracion = leer_argumento_string(logger, argumentos, INDICE_ARGUMENTO_RUTA_ARCHIVO_CONFIGURACION, NOMBRE_ARGUMENTO_RUTA_ARCHIVO_CONFIGURACION);

	log_debug(logger, "Exito en la lectura de argumentos");

	return argumentos_servidor;
}

void destruir_argumentos(t_argumentos_servidor* argumentos_servidor)
{
    if (argumentos_servidor == NULL)
	{
		return;
	}
	
	free(argumentos_servidor -> ruta_archivo_configuracion);

	free(argumentos_servidor);
}