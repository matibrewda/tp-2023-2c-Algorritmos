#include "memoria.h"

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	t_log *logger = NULL;
	t_argumentos_memoria *argumentos_memoria = NULL;
	t_config_memoria *configuracion_memoria = NULL;

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_MEMORIA, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_MEMORIA);

	argumentos_memoria = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_memoria == NULL)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria);
		return EXIT_FAILURE;
	}

	configuracion_memoria = leer_configuracion(logger, argumentos_memoria->ruta_archivo_configuracion);
	if (configuracion_memoria == NULL)
	{
		terminar_memoria(logger, argumentos_memoria, configuracion_memoria);
		return EXIT_FAILURE;
	}

	// socket_memoria = crear_socket_memoria(logger, configuracion_memoria -> puerto_escucha_a_cliente, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CLIENTE);
	// if (socket_memoria == -1)
	// {
	// 	log_error(logger, "No se pudo inicializar correctamente a %s como memoria.", NOMBRE_MODULO_MEMORIA);
	// 	terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_memoria);
	// 	return EXIT_FAILURE;
	// }

	// bool se_conecto_cliente = false;

	// int conexion_con_cliente = esperar_conexion_de_cliente(logger, configuracion_memoria->puerto_escucha_a_cliente, socket_memoria, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CLIENTE);

	// log_info(logger, "Se conecto cliente!.");

	// int codigo_operacion = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CLIENTE, conexion_con_cliente);

	// int tamanio_buffer;
	// void *buffer_de_paquete = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CLIENTE, &tamanio_buffer, conexion_con_cliente, codigo_operacion);
	// void *buffer_de_paquete_con_offset = buffer_de_paquete;

	// int *entero1 = malloc(sizeof(int));
	// int *entero2 = malloc(sizeof(int));
	// int *entero3 = malloc(sizeof(int));

	// leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero1, codigo_operacion);
	// leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero2, codigo_operacion);
	// leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero3, codigo_operacion);

	// log_info(logger, "Tengo mi entero1: %d", *entero1);
	// log_info(logger, "Tengo mi entero2: %d", *entero2);
	// log_info(logger, "Tengo mi entero3: %d", *entero3);

	// log_info(logger, "QUACK!");

	// log_info(logger, "Enviando señal de que termine de hacer quack a %s", NOMBRE_MODULO_CLIENTE);
	// enviar_operacion_sin_paquete(logger, conexion_con_cliente, TERMINE_EL_QUACK, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_MEMORIA);
	// log_info(logger, "Exito en el envio de señal de que termine de hacer quack a %s", NOMBRE_MODULO_CLIENTE);

	terminar_memoria(logger, argumentos_memoria, configuracion_memoria);

	return EXIT_SUCCESS;
}

void terminar_memoria(t_log *logger, t_argumentos_memoria *argumentos_memoria, t_config_memoria *configuracion_memoria)
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_MEMORIA);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_memoria);
	destruir_configuracion(configuracion_memoria);
}