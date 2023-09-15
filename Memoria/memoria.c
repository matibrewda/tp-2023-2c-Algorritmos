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
	int resultado_kernel = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, conexion_con_kernel);
	log_info(logger, "Se recibio la operacion %d desde %s", resultado_kernel, NOMBRE_MODULO_KERNEL);
	log_info(logger, "Mando señal a %s", NOMBRE_MODULO_KERNEL);
	enviar_operacion_sin_paquete(logger, conexion_con_kernel, MENSAJE_DE_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);

	int resultado_cpu = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, conexion_con_cpu);
	log_info(logger, "Se recibio la operacion %d desde %s", resultado_cpu, NOMBRE_MODULO_CPU);
	log_info(logger, "Mando señal a %s", NOMBRE_MODULO_CPU);
	enviar_operacion_sin_paquete(logger, conexion_con_cpu, MENSAJE_DE_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	int resultado_filesystem = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem);
	log_info(logger, "Se recibio la operacion %d desde %s", resultado_filesystem, NOMBRE_MODULO_FILESYSTEM);
	log_info(logger, "Mando señal a %s", NOMBRE_MODULO_FILESYSTEM);
	enviar_operacion_sin_paquete(logger, conexion_con_filesystem, MENSAJE_DE_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	int resultado_handshake_cpu = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, conexion_con_cpu);
	log_info(logger, "Se recibio la operacion %d desde %s", resultado_handshake_cpu, NOMBRE_MODULO_CPU);
	if (resultado_handshake_cpu == HANDSHAKE_CPU_MEMORIA)
	{
		realizar_handshake_cpu(logger, configuracion_memoria, conexion_con_cpu);
	}

	int recibir_iniciar_operacion_kernel = esperar_operacion(logger,NOMBRE_MODULO_MEMORIA,NOMBRE_MODULO_KERNEL,conexion_con_kernel);
	log_info(logger, "Se recibio la operacion %d desde %s", recibir_iniciar_operacion_kernel, NOMBRE_MODULO_KERNEL);
	if (recibir_iniciar_operacion_kernel == INICIAR_PROCESO)
	{
		iniciar_proceso_memoria();//todo
	}

	// Finalizacion
	terminar_memoria(logger, argumentos_memoria, configuracion_memoria, socket_kernel, conexion_con_kernel, socket_cpu, conexion_con_cpu, socket_filesystem, conexion_con_filesystem);
	return EXIT_SUCCESS;
}

void realizar_handshake_cpu(t_log *logger, t_config_memoria *configuracion_memoria, int conexion_con_cpu) {
	log_debug(logger, "Comenzando la creacion de paquete para enviar handshake al cpu!");
	t_paquete *paquete = crear_paquete(logger, HANDSHAKE_CPU_MEMORIA);

  	agregar_int_a_paquete(logger, paquete, configuracion_memoria->tam_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, HANDSHAKE_CPU_MEMORIA);
    log_debug(logger, "Exito en la creacion de paquete para enviar handshake al cpu!");

	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	log_debug(logger, "Exito en el envio de paquete para handshake al cpu!");
	destruir_paquete(logger, paquete);
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