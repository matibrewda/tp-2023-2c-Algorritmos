#include "servidor.h"

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	t_log *logger = NULL;
	t_argumentos_servidor *argumentos_servidor = NULL;
	t_config_servidor *configuracion_servidor = NULL;
	int socket_servidor = -1;

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_SERVIDOR, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_servidor(logger, argumentos_servidor, configuracion_servidor, socket_servidor);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_SERVIDOR);

	argumentos_servidor = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_servidor == NULL)
	{
		terminar_servidor(logger, argumentos_servidor, configuracion_servidor, socket_servidor);
		return EXIT_FAILURE;
	}

	configuracion_servidor = leer_configuracion(logger, argumentos_servidor->ruta_archivo_configuracion);
	if (configuracion_servidor == NULL)
	{
		terminar_servidor(logger, argumentos_servidor, configuracion_servidor, socket_servidor);
		return EXIT_FAILURE;
	}

	socket_servidor = crear_socket_servidor(logger, configuracion_servidor->puerto_escucha_a_cliente, NOMBRE_MODULO_SERVIDOR, NOMBRE_MODULO_CLIENTE);
	if (socket_servidor == -1)
	{
		log_error(logger, "No se pudo inicializar correctamente a %s como servidor.", NOMBRE_MODULO_SERVIDOR);
		terminar_servidor(logger, argumentos_servidor, configuracion_servidor, socket_servidor);
		return EXIT_FAILURE;
	}

	bool se_conecto_cliente = false;

	int conexion_con_cliente = esperar_conexion_de_cliente(logger, configuracion_servidor->puerto_escucha_a_cliente, socket_servidor, NOMBRE_MODULO_SERVIDOR, NOMBRE_MODULO_CLIENTE);

	log_info(logger, "Se conecto cliente!.");

	int codigo_operacion = esperar_operacion(logger, NOMBRE_MODULO_SERVIDOR, NOMBRE_MODULO_CLIENTE, conexion_con_cliente);

	int tamanio_buffer;
	void *buffer_de_paquete = recibir_paquete(logger, NOMBRE_MODULO_SERVIDOR, NOMBRE_MODULO_CLIENTE, &tamanio_buffer, conexion_con_cliente, codigo_operacion);
	void *buffer_de_paquete_con_offset = buffer_de_paquete;

	int *entero1 = malloc(sizeof(int));
	int *entero2 = malloc(sizeof(int));
	int *entero3 = malloc(sizeof(int));

	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_SERVIDOR, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero1, codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_SERVIDOR, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero2, codigo_operacion);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_SERVIDOR, NOMBRE_MODULO_CLIENTE, &buffer_de_paquete_con_offset, entero3, codigo_operacion);

	log_info(logger, "Tengo mi entero1: %d", *entero1);
	log_info(logger, "Tengo mi entero2: %d", *entero2);
	log_info(logger, "Tengo mi entero3: %d", *entero3);

	log_info(logger, "QUACK!");

	log_info(logger, "Enviando señal de que termine de hacer quack a %s", NOMBRE_MODULO_CLIENTE);
	enviar_operacion_sin_paquete(logger, conexion_con_cliente, TERMINE_EL_QUACK, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_SERVIDOR);
	log_info(logger, "Exito en el envio de señal de que termine de hacer quack a %s", NOMBRE_MODULO_CLIENTE);

	terminar_servidor(logger, argumentos_servidor, configuracion_servidor, socket_servidor);

	return EXIT_SUCCESS;
}

void terminar_servidor(t_log *logger, t_argumentos_servidor *argumentos_servidor, t_config_servidor *configuracion_servidor, int socket_servidor)
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando Servidor");
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_servidor);
	destruir_configuracion(configuracion_servidor);
}