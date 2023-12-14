#include "Headers/utilidades_configuracion.h"

t_config* leer_archivo_configuracion(t_log* logger, char* ruta_archivo_configuracion)
{
	t_config* configuracion;

	configuracion = config_create(ruta_archivo_configuracion);

	if (configuracion == NULL)
	{
		log_error(logger, "Error al leer archivo de configuracion: no se pudo leer el archivo de configuracion con la siguiente ruta: %s", ruta_archivo_configuracion);
		return NULL;
	}

    return configuracion;
}

bool existe_clave_en_configuracion(t_log* logger, t_config* configuracion, char* clave)
{
	if (!config_has_property(configuracion, clave))
	{
		log_error(logger, "Error al leer archivo de configuracion: el archivo de configuracion NO tiene la clave %s", clave);
		config_destroy(configuracion);
		return false;
	}

	return true;
}

bool existen_claves_en_configuracion(t_log* logger, t_config* configuracion, char** claves)
{
    for (int i = 0; claves[i] != NULL; i++) 
	{
        if (!existe_clave_en_configuracion(logger, configuracion, claves[i]))
        {
            return false;
        }
    }

    return true;
}

// Hace un malloc, por lo cual hay que encargarse de hacer un free en algun momento!
char* leer_clave_string(t_log* logger, t_config* configuracion, char* clave)
{
    char* valor = config_get_string_value(configuracion, clave);
    log_trace(logger, "Se leyo desde archivo de configuracion la clave %s con valor tipo string = %s ", clave, valor);
	size_t valor_len = strlen(valor) + 1;
	char* valor_alocado = malloc(valor_len);
	strcpy(valor_alocado, valor);
    return valor_alocado;
}

// No reserva memoria dinamica: NO hace falta un free!
int leer_clave_int(t_log* logger, t_config* configuracion, char* clave)
{
    int valor = config_get_int_value(configuracion, clave);
    log_trace(logger, "Se leyo desde archivo de configuracion la clave %s con valor tipo entero = %d ", clave, valor);
    return valor;
}

// Hace un malloc por el arreglo de strings y otro por cada string leido, por lo cual hay que encargarse de hacer un free en algun momento!
// Guarda en tamanio_arreglo el tamanio del arreglo (para poder iterar)
char** leer_clave_arreglo_de_strings(t_log* logger, t_config* configuracion, char* clave, int* tamanio_arreglo)
{
    char** valores = config_get_array_value(configuracion, clave);
    *tamanio_arreglo = 0;

	log_trace(logger, "Se leyo desde archivo de configuracion la clave %s con los siguientes valores de tipo string:", clave);
    for (int i = 0; valores[i] != NULL; i++) 
	{
        log_trace(logger, "%s", valores[i]);
		*tamanio_arreglo = *tamanio_arreglo + 1;
    }

	char** valores_alocados = malloc(*tamanio_arreglo * sizeof(char*));

	for (int i = 0; i < *tamanio_arreglo; i++) 
	{
		size_t valor_len = strlen(valores[i]) + 1;
		valores_alocados[i] = malloc(valor_len);
		strcpy(valores_alocados[i], valores[i]);
	}

	log_trace(logger, "El tamaño del arreglo de strings para la clave %s fue de %d", clave, *tamanio_arreglo);
	free(valores);

	return valores_alocados;
}

// Hace un malloc, por lo cual hay que encargarse de hacer un free en algun momento!
// Guarda en tamanio_arreglo el tamanio del arreglo (para poder iterar)
int* leer_clave_arreglo_de_enteros(t_log* logger, t_config* configuracion, char* clave, int* tamanio_arreglo)
{
    char** valores = config_get_array_value(configuracion, clave);
    *tamanio_arreglo = 0;

	log_trace(logger, "Se leyo desde archivo de configuracion la clave %s con los siguientes valores de tipo entero:", clave);
    for (int i = 0; valores[i] != NULL; i++) 
	{
        log_trace(logger, "%s", valores[i]);
		*tamanio_arreglo = *tamanio_arreglo + 1;
    }

	int* valores_alocados = malloc(*tamanio_arreglo * sizeof(int));

	for (int i = 0; i < *tamanio_arreglo; i++) 
	{
		valores_alocados[i] = atoi(valores[i]);
	}

	log_trace(logger, "El tamaño del arreglo de enteros para la clave %s fue de %d", clave, *tamanio_arreglo);

	free(valores);

	return valores_alocados;
}