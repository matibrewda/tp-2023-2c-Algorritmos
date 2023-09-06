#include "memoria.h"

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	t_log *logger = NULL;
	t_argumentos_memoria *argumentos_memoria = NULL;
	t_config_memoria *configuracion_memoria = NULL;
	int socket_kernel = -1;
	int socket_cpu = -1;
	int socket_filesystem = -1;
	int conexion_con_kernel = -1;
	int conexion_con_cpu = -1;
	int conexion_con_filesystem = -1;

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_MEMORIA, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_MEMORIA);

	argumentos_memoria = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_memoria == NULL)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	configuracion_memoria = leer_configuracion(logger, argumentos_memoria->ruta_archivo_configuracion);
	if (configuracion_memoria == NULL)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	socket_kernel = crear_socket_servidor(logger, configuracion_memoria->puerto_escucha_kernel, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
	if (socket_kernel == -1)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	socket_cpu = crear_socket_servidor(logger, configuracion_memoria->puerto_escucha_cpu, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	if (socket_cpu == -1)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	socket_filesystem = crear_socket_servidor(logger, configuracion_memoria->puerto_escucha_filesystem, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	if (socket_kernel == -1)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	conexion_con_filesystem = esperar_conexion_de_cliente(logger, socket_filesystem, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	if (conexion_con_filesystem == -1)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	conexion_con_cpu = esperar_conexion_de_cliente(logger, socket_cpu, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	if (conexion_con_cpu == -1)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	conexion_con_kernel = esperar_conexion_de_cliente(logger, socket_kernel, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
	if (conexion_con_kernel == -1)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	// Logica principal
	esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, conexion_con_kernel);

	// Finalizacion
	terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
	return EXIT_SUCCESS;
}

void terminar_memoria(t_log *logger, t_argumentos_memoria *argumentos_memoria, t_config_memoria *configuracion_memoria, int socket_kernel, int conexion_con_kernel, int socket_cpu, int conexion_con_cpu, int socket_filesystem, int conexion_con_filesystem)
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_MEMORIA);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_memoria);
	destruir_configuracion(configuracion_memoria);

	if (socket_kernel != -1)
	{
		close(socket_kernel);
	}

	if (conexion_con_kernel != -1)
	{
		close(conexion_con_kernel);
	}

	if (socket_cpu != -1)
	{
		close(socket_cpu);
	}

	if (conexion_con_cpu != -1)
	{
		close(conexion_con_cpu);
	}

	if (socket_filesystem != -1)
	{
		close(socket_filesystem);
	}

	if (conexion_con_filesystem != -1)
	{
		close(conexion_con_filesystem);
	}
}