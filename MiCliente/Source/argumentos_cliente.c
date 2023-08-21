#include "Headers/argumentos_cliente.h"

t_argumentos_cliente* leer_argumentos(t_log* logger, int cantidad_argumentos_recibidos, char **argumentos)
{
	log_debug(logger, "Comenzando la lectura de argumentos");

	if (!cantidad_de_argumentos_es_valida(logger, cantidad_argumentos_recibidos, CANTIDAD_ARGUMENTOS_ESPERADOS))
	{
		return NULL;
	}

	log_trace(logger, "La cantidad de argumentos recibida es la esperada: %d", CANTIDAD_ARGUMENTOS_ESPERADOS);

	t_argumentos_cliente* argumentos_cliente = malloc(sizeof (t_argumentos_cliente));
	
	argumentos_cliente -> ruta_archivo_configuracion = leer_argumento_string(logger, argumentos, INDICE_ARGUMENTO_RUTA_ARCHIVO_CONFIGURACION, NOMBRE_ARGUMENTO_RUTA_ARCHIVO_CONFIGURACION);

	log_debug(logger, "Exito en la lectura de argumentos");

	return argumentos_cliente;
}

void destruir_argumentos(t_argumentos_cliente* argumentos_cliente)
{
    if (argumentos_cliente == NULL)
	{
		return;
	}
	
	free(argumentos_cliente -> ruta_archivo_configuracion);

	free(argumentos_cliente);
}