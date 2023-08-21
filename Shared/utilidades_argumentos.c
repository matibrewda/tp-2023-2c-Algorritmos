#include "Headers/utilidades_argumentos.h"

bool cantidad_de_argumentos_es_valida(t_log* logger, int cantidad_argumentos_recibidos, int cantidad_argumentos_esperados)
{
    // El +1 y el -1 es porque argumentos[0] siempre contiene el nombre del ejecutable
	if (cantidad_argumentos_recibidos != cantidad_argumentos_esperados + 1)
	{
		log_error(logger, "Error al leer argumentos: se recibieron %d parametros cuando se esperaban %d.", cantidad_argumentos_recibidos - 1, cantidad_argumentos_esperados);
		return false;
	}

    return true;
}

char* leer_argumento_string(t_log* logger, char** argumentos, int indice, char* nombre_argumento)
{
    char* valor = argumentos[indice];
	log_trace(logger, "Se leyo el argumento %s con valor %s", nombre_argumento, valor);
	size_t valor_len = strlen(valor) + 1;
	char* valor_alocado = malloc(valor_len);
	strcpy(valor_alocado, valor);
    return valor_alocado;
}