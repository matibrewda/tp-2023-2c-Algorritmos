#include "Headers/configuracion_cpu.h"

t_config_cpu *leer_configuracion(t_log *logger, char *ruta_archivo_configuracion)
{
	log_trace(logger, "Comenzando la lectura de archivo de configuracion");

	t_config *configuracion = leer_archivo_configuracion(logger, ruta_archivo_configuracion);

	if (configuracion == NULL)
	{
		return NULL;
	}

	char *claves[] = {
		CLAVE_CONFIGURACION_IP_MEMORIA,
		CLAVE_CONFIGURACION_PUERTO_MEMORIA,
		CLAVE_CONFIGURACION_PUERTO_ESCUCHA_DISPATCH,
		CLAVE_CONFIGURACION_PUERTO_ESCUCHA_INTERRUPT,
		NULL};

	if (!existen_claves_en_configuracion(logger, configuracion, claves))
	{
		return NULL;
	}

	t_config_cpu *configuracion_cpu = malloc(sizeof(t_config_cpu));
	configuracion_cpu->ip_memoria = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_IP_MEMORIA);
	configuracion_cpu->puerto_memoria = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_MEMORIA);
	configuracion_cpu->puerto_escucha_dispatch = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_ESCUCHA_DISPATCH);
	configuracion_cpu->puerto_escucha_interrupt = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_ESCUCHA_INTERRUPT);

	log_trace(logger, "Exito en la lectura de archivo de configuracion");

	config_destroy(configuracion);
	return configuracion_cpu;
}

void destruir_configuracion(t_config_cpu *configuracion_cpu)
{
	if (configuracion_cpu == NULL)
	{
		return;
	}

	free(configuracion_cpu->ip_memoria);
	free(configuracion_cpu->puerto_memoria);
	free(configuracion_cpu->puerto_escucha_dispatch);
	free(configuracion_cpu->puerto_escucha_interrupt);

	free(configuracion_cpu);
}