#include "cliente.h"

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	setbuf(stdout, NULL); // Why?

	t_log* logger = NULL;
	t_argumentos_cliente* argumentos_cliente = NULL;
	t_config_cliente* configuracion_cliente = NULL;
	int conexion_con_servidor = -1;
	t_paquete* paquete_para_servidor = NULL;

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_CLIENTE, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_cliente(logger, argumentos_cliente, configuracion_cliente, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando Cliente");

	argumentos_cliente = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_cliente == NULL)
	{
		terminar_cliente(logger, argumentos_cliente, configuracion_cliente, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}

	configuracion_cliente = leer_configuracion(logger, argumentos_cliente -> ruta_archivo_configuracion);
	if (configuracion_cliente == NULL)
	{
		terminar_cliente(logger, argumentos_cliente, configuracion_cliente, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}

	conexion_con_servidor = conectar_cliente_con_servidor(logger, configuracion_cliente -> ip_servidor, configuracion_cliente -> puerto_servidor);
	if (conexion_con_servidor == -1)
	{
		terminar_cliente(logger, argumentos_cliente, configuracion_cliente, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}
	
	// Logica principal
	paquete_para_servidor = crear_paquete_para_servidor(logger);
	log_info(logger, "Enviando señal para hacer quack a %s", NOMBRE_MODULO_SERVIDOR);
	enviar_paquete(logger, conexion_con_servidor, paquete_para_servidor, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_SERVIDOR);
	log_info(logger, "Exito en el envio de señal para hacer quack a %s", NOMBRE_MODULO_SERVIDOR);

	int resultado_servidor = esperar_operacion(logger, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_SERVIDOR, conexion_con_servidor);

	log_info(logger, "Se recibio la operacion %d desde %s", resultado_servidor, NOMBRE_MODULO_SERVIDOR);

	terminar_cliente(logger, argumentos_cliente, configuracion_cliente, conexion_con_servidor, paquete_para_servidor);
	
	return EXIT_SUCCESS;
}

void terminar_cliente(t_log* logger, t_argumentos_cliente* argumentos_cliente, t_config_cliente* configuracion_cliente, int conexion_con_servidor, t_paquete* paquete_para_servidor)
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando Cliente");
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_cliente);
	destruir_configuracion(configuracion_cliente);
	destruir_conexion_con_servidor(conexion_con_servidor);
	destruir_paquete(logger, paquete_para_servidor);
}