#include "Headers/argumentos_kernel.h"

t_argumentos_kernel *leer_argumentos(t_log *logger, int cantidad_argumentos_recibidos, char **argumentos)
{
	log_trace(logger, "Comenzando la lectura de argumentos");

	if (!cantidad_de_argumentos_es_valida(logger, cantidad_argumentos_recibidos, CANTIDAD_ARGUMENTOS_ESPERADOS))
	{
		return NULL;
	}

	log_trace(logger, "La cantidad de argumentos recibida es la esperada: %d", CANTIDAD_ARGUMENTOS_ESPERADOS);

	t_argumentos_kernel *argumentos_kernel = malloc(sizeof(t_argumentos_kernel));

	argumentos_kernel->ruta_archivo_configuracion = leer_argumento_string(logger, argumentos, INDICE_ARGUMENTO_RUTA_ARCHIVO_CONFIGURACION, NOMBRE_ARGUMENTO_RUTA_ARCHIVO_CONFIGURACION);

	log_trace(logger, "Exito en la lectura de argumentos");

	return argumentos_kernel;
}

void destruir_argumentos(t_argumentos_kernel *argumentos_kernel)
{
	if (argumentos_kernel == NULL)
	{
		return;
	}

	free(argumentos_kernel->ruta_archivo_configuracion);

	free(argumentos_kernel);
}