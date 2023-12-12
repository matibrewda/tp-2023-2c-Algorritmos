#include "Headers/configuracion_kernel.h"

t_config_kernel *leer_configuracion(t_log *logger, char *ruta_archivo_configuracion)
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
		CLAVE_CONFIGURACION_IP_FILESYSTEM,
		CLAVE_CONFIGURACION_PUERTO_FILESYSTEM,
		CLAVE_CONFIGURACION_IP_CPU,
		CLAVE_CONFIGURACION_PUERTO_CPU_DISPATCH,
		CLAVE_CONFIGURACION_PUERTO_CPU_INTERRUPT,
		CLAVE_CONFIGURACION_ALGORITMO_PLANIFICACION,
		CLAVE_CONFIGURACION_QUANTUM,
		CLAVE_CONFIGURACION_RECURSOS,
		CLAVE_CONFIGURACION_INSTANCIAS_RECURSOS,
		CLAVE_CONFIGURACION_GRADO_MULTIPROGRAMACION_INI,
		NULL};

	if (!existen_claves_en_configuracion(logger, configuracion, claves))
	{
		return NULL;
	}

	t_config_kernel *configuracion_kernel = malloc(sizeof(t_config_kernel));
	configuracion_kernel->ip_memoria = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_IP_MEMORIA);
	configuracion_kernel->puerto_memoria = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_MEMORIA);
	configuracion_kernel->ip_filesystem = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_IP_FILESYSTEM);
	configuracion_kernel->puerto_filesystem = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_FILESYSTEM);
	configuracion_kernel->ip_cpu = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_IP_CPU);
	configuracion_kernel->puerto_cpu_dispatch = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_CPU_DISPATCH);
	configuracion_kernel->puerto_cpu_interrupt = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_CPU_INTERRUPT);
	configuracion_kernel->algoritmo_planificacion = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_ALGORITMO_PLANIFICACION);
	configuracion_kernel->quantum = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_QUANTUM);
	configuracion_kernel->recursos = leer_clave_arreglo_de_strings(logger, configuracion, CLAVE_CONFIGURACION_RECURSOS, &(configuracion_kernel->cantidad_de_recursos));
	configuracion_kernel->instancias_recursos = leer_clave_arreglo_de_enteros(logger, configuracion, CLAVE_CONFIGURACION_INSTANCIAS_RECURSOS, &(configuracion_kernel->cantidad_de_recursos));
	configuracion_kernel->grado_multiprogramacion_inicial = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_GRADO_MULTIPROGRAMACION_INI);

	log_trace(logger, "Exito en la lectura de archivo de configuracion");

	config_destroy(configuracion);

	return configuracion_kernel;
}

void destruir_configuracion(t_config_kernel *configuracion_kernel)
{
	if (configuracion_kernel == NULL)
	{
		return;
	}

	free(configuracion_kernel->ip_memoria);
	free(configuracion_kernel->puerto_memoria);
	free(configuracion_kernel->ip_filesystem);
	free(configuracion_kernel->puerto_filesystem);
	free(configuracion_kernel->ip_cpu);
	free(configuracion_kernel->puerto_cpu_dispatch);
	free(configuracion_kernel->puerto_cpu_interrupt);
	free(configuracion_kernel->algoritmo_planificacion);
	free(configuracion_kernel->instancias_recursos);

	for (int i = 0; i < configuracion_kernel->cantidad_de_recursos; i++)
	{
		free(configuracion_kernel->recursos[i]);
	}
	free(configuracion_kernel->recursos);

	free(configuracion_kernel);
}