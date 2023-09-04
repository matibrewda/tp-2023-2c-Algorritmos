#include "filesystem.h"

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	t_log *logger = NULL;
	t_argumentos_filesystem *argumentos_filesystem = NULL;
	t_config_filesystem *configuracion_filesystem = NULL;

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_FILESYSTEM, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_FILESYSTEM);

	argumentos_filesystem = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_filesystem == NULL)
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem);
		return EXIT_FAILURE;
	}

	configuracion_filesystem = leer_configuracion(logger, argumentos_filesystem->ruta_archivo_configuracion);
	if (configuracion_filesystem == NULL)
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem);
		return EXIT_FAILURE;
	}

	// socket_filesystem = crear_socket_filesystem(logger, configuracion_filesystem -> puerto_escucha_a_cliente, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_CLIENTE);
	// if (socket_filesystem == -1)
	// {
	// 	log_error(logger, "No se pudo inicializar correctamente a %s como filesystem.", NOMBRE_MODULO_FILESYSTEM);
	// 	terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_filesystem);
	// 	return EXIT_FAILURE;
	// }

	// bool se_conecto_cliente = false;

	// int conexion_con_cliente = esperar_conexion_de_cliente(logger, configuracion_filesystem->puerto_escucha_a_cliente, socket_filesystem, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_CLIENTE);

	// log_info(logger, "Se conecto cliente!.");

	// int codigo_operacion = esperar_operacion(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_CLIENTE, conexion_con_cliente);

	// int tamanio_buffer;
	// void *buffer_de_paquete = recibir_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_CLIENTE, &tamanio_buffer, conexion_con_cliente, codigo_operacion);
	// void *buffer_de_paquete_con_offset = buffer_de_paquete;

	// int *entero1 = malloc(sizeof(int));
	// int *entero2 = malloc(sizeof(int));
	// int *entero3 = malloc(sizeof(int));

	// leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero1, codigo_operacion);
	// leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero2, codigo_operacion);
	// leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero3, codigo_operacion);

	// log_info(logger, "Tengo mi entero1: %d", *entero1);
	// log_info(logger, "Tengo mi entero2: %d", *entero2);
	// log_info(logger, "Tengo mi entero3: %d", *entero3);

	// log_info(logger, "QUACK!");

	// log_info(logger, "Enviando señal de que termine de hacer quack a %s", NOMBRE_MODULO_CLIENTE);
	// enviar_operacion_sin_paquete(logger, conexion_con_cliente, TERMINE_EL_QUACK, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_FILESYSTEM);
	// log_info(logger, "Exito en el envio de señal de que termine de hacer quack a %s", NOMBRE_MODULO_CLIENTE);

	terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem);

	return EXIT_SUCCESS;
}

void terminar_filesystem(t_log *logger, t_argumentos_filesystem *argumentos_filesystem, t_config_filesystem *configuracion_filesystem)
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_FILESYSTEM);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_filesystem);
	destruir_configuracion(configuracion_filesystem);
}