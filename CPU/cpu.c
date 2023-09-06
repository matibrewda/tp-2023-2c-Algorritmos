#include "cpu.h"

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	t_log *logger = NULL;
	t_argumentos_cpu *argumentos_cpu = NULL;
	t_config_cpu *configuracion_cpu = NULL;
	int socket_kernel_dispatch = -1;
	int socket_kernel_interrupt = -1;
	int conexion_con_kernel_dispatch = -1;
	int conexion_con_kernel_interrupt = -1;
	int conexion_con_memoria = -1;

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_CPU, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_kernel_dispatch, conexion_con_kernel_dispatch, socket_kernel_interrupt, conexion_con_kernel_interrupt, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_CPU);

	argumentos_cpu = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_cpu == NULL)
	{
		terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_kernel_dispatch, conexion_con_kernel_dispatch, socket_kernel_interrupt, conexion_con_kernel_interrupt, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	configuracion_cpu = leer_configuracion(logger, argumentos_cpu->ruta_archivo_configuracion);
	if (configuracion_cpu == NULL)
	{
		terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_kernel_dispatch, conexion_con_kernel_dispatch, socket_kernel_interrupt, conexion_con_kernel_interrupt, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	conexion_con_memoria = crear_socket_cliente(logger, configuracion_cpu->ip_memoria, configuracion_cpu->puerto_memoria, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
	if (conexion_con_memoria == -1)
	{
		terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_kernel_dispatch, conexion_con_kernel_dispatch, socket_kernel_interrupt, conexion_con_kernel_interrupt, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	socket_kernel_dispatch = crear_socket_servidor(logger, configuracion_cpu->puerto_escucha_dispatch, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
	if (socket_kernel_dispatch == -1)
	{
		terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_kernel_dispatch, conexion_con_kernel_dispatch, socket_kernel_interrupt, conexion_con_kernel_interrupt, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	socket_kernel_interrupt = crear_socket_servidor(logger, configuracion_cpu->puerto_escucha_interrupt, NOMBRE_MODULO_CPU_INTERRUPT, NOMBRE_MODULO_KERNEL);
	if (socket_kernel_interrupt == -1)
	{
		terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_kernel_dispatch, conexion_con_kernel_dispatch, socket_kernel_interrupt, conexion_con_kernel_interrupt, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	conexion_con_kernel_dispatch = esperar_conexion_de_cliente(logger, socket_kernel_dispatch, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
	if (conexion_con_kernel_dispatch == -1)
	{
		terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_kernel_dispatch, conexion_con_kernel_dispatch, socket_kernel_interrupt, conexion_con_kernel_interrupt, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	conexion_con_kernel_interrupt = esperar_conexion_de_cliente(logger, socket_kernel_interrupt, NOMBRE_MODULO_CPU_INTERRUPT, NOMBRE_MODULO_KERNEL);
	if (conexion_con_kernel_interrupt == -1)
	{
		terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_kernel_dispatch, conexion_con_kernel_dispatch, socket_kernel_interrupt, conexion_con_kernel_interrupt, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	// Logica principal
	

	// Finalizacion
	terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_kernel_dispatch, conexion_con_kernel_dispatch, socket_kernel_interrupt, conexion_con_kernel_interrupt, conexion_con_memoria);
	return EXIT_SUCCESS;
}

void terminar_cpu(t_log *logger, t_argumentos_cpu *argumentos_cpu, t_config_cpu *configuracion_cpu, int socket_kernel_dispatch, int conexion_con_kernel_dispatch, int socket_kernel_interrupt, int conexion_con_kernel_interrupt, int conexion_con_memoria)
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_CPU);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_cpu);
	destruir_configuracion(configuracion_cpu);

	if (socket_kernel_dispatch != -1)
	{
		close(socket_kernel_dispatch);
	}

	if (conexion_con_kernel_dispatch != -1)
	{
		close(conexion_con_kernel_dispatch);
	}

	if (socket_kernel_interrupt != -1)
	{
		close(socket_kernel_interrupt);
	}

	if (conexion_con_kernel_interrupt != -1)
	{
		close(conexion_con_kernel_interrupt);
	}

	if (conexion_con_memoria != -1)
	{
		close(conexion_con_memoria);
	}
}