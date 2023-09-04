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

	// socket_cpu = crear_socket_cpu(logger, configuracion_cpu -> puerto_escucha_a_cliente, NOMBRE_MODULO_CPU, NOMBRE_MODULO_CLIENTE);
	// if (socket_cpu == -1)
	// {
	// 	log_error(logger, "No se pudo inicializar correctamente a %s como cpu.", NOMBRE_MODULO_CPU);
	// 	terminar_cpu(logger, argumentos_cpu, configuracion_cpu, socket_cpu);
	// 	return EXIT_FAILURE;
	// }

	// bool se_conecto_cliente = false;

	// int conexion_con_cliente = esperar_conexion_de_cliente(logger, configuracion_cpu->puerto_escucha_a_cliente, socket_cpu, NOMBRE_MODULO_CPU, NOMBRE_MODULO_CLIENTE);

	// log_info(logger, "Se conecto cliente!.");

	// int codigo_operacion = esperar_operacion(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_CLIENTE, conexion_con_cliente);

	// int tamanio_buffer;
	// void *buffer_de_paquete = recibir_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_CLIENTE, &tamanio_buffer, conexion_con_cliente, codigo_operacion);
	// void *buffer_de_paquete_con_offset = buffer_de_paquete;

	// int *entero1 = malloc(sizeof(int));
	// int *entero2 = malloc(sizeof(int));
	// int *entero3 = malloc(sizeof(int));

	// leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero1, codigo_operacion);
	// leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero2, codigo_operacion);
	// leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero3, codigo_operacion);

	// log_info(logger, "Tengo mi entero1: %d", *entero1);
	// log_info(logger, "Tengo mi entero2: %d", *entero2);
	// log_info(logger, "Tengo mi entero3: %d", *entero3);

	// log_info(logger, "QUACK!");

	// log_info(logger, "Enviando señal de que termine de hacer quack a %s", NOMBRE_MODULO_CLIENTE);
	// enviar_operacion_sin_paquete(logger, conexion_con_cliente, TERMINE_EL_QUACK, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_CPU);
	// log_info(logger, "Exito en el envio de señal de que termine de hacer quack a %s", NOMBRE_MODULO_CLIENTE);

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