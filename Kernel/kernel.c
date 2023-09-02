#include "kernel.h"

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	setbuf(stdout, NULL); // Why?

	t_log* logger = NULL;
	t_argumentos_kernel* argumentos_kernel = NULL;
	t_config_kernel* configuracion_kernel = NULL;


	int conexion_con_servidor = -1;
	t_paquete* paquete_para_servidor = NULL;

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_KERNEL, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_KERNEL);

	argumentos_kernel = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_kernel == NULL)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}

	configuracion_kernel = leer_configuracion(logger, argumentos_kernel -> ruta_archivo_configuracion);
	if (configuracion_kernel == NULL)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}

	// log_debug(logger, "Intentando conectar %s con %s ...", NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_SERVIDOR);

	// conexion_con_servidor = crear_socket_kernel(logger, configuracion_kernel -> ip_servidor, configuracion_kernel -> puerto_servidor, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_SERVIDOR);
	// if (conexion_con_servidor == -1)
	// {
	// 	terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
	// 	return EXIT_FAILURE;
	// }
	
	// // Logica principal
	// paquete_para_servidor = crear_paquete_para_servidor(logger);
	// log_info(logger, "Enviando señal para hacer quack a %s", NOMBRE_MODULO_SERVIDOR);
	// enviar_paquete(logger, conexion_con_servidor, paquete_para_servidor, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_SERVIDOR);
	// log_info(logger, "Exito en el envio de señal para hacer quack a %s", NOMBRE_MODULO_SERVIDOR);

	// int resultado_servidor = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_SERVIDOR, conexion_con_servidor);

	// log_info(logger, "Se recibio la operacion %d desde %s", resultado_servidor, NOMBRE_MODULO_SERVIDOR);

	terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
	
	return EXIT_SUCCESS;
}

void terminar_kernel(t_log* logger, t_argumentos_kernel* argumentos_kernel, t_config_kernel* configuracion_kernel, int conexion_con_servidor, t_paquete* paquete_para_servidor)
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_KERNEL);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_kernel);
	destruir_configuracion(configuracion_kernel);

	if (conexion_con_servidor != -1)
	{
    	close(conexion_con_servidor);
	}

	//destruir_paquete(logger, paquete_para_servidor);
}