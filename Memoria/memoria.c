#include "memoria.h"

t_log *logger = NULL;
t_argumentos_memoria *argumentos_memoria = NULL;
t_config_memoria *configuracion_memoria = NULL;
int socket_kernel = -1;
int socket_cpu = -1;
int socket_filesystem = -1;
int conexion_con_kernel = -1;
int conexion_con_cpu = -1;
int conexion_con_filesystem = -1;

t_list *procesos_iniciados = NULL;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_MEMORIA, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_memoria();
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_MEMORIA);

	argumentos_memoria = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_memoria == NULL)
	{
		terminar_memoria();
		return EXIT_FAILURE;
	}

	configuracion_memoria = leer_configuracion(logger, argumentos_memoria->ruta_archivo_configuracion);
	if (configuracion_memoria == NULL)
	{
		terminar_memoria();
		return EXIT_FAILURE;
	}

	socket_kernel = crear_socket_servidor(logger, configuracion_memoria->puerto_escucha_kernel, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
	if (socket_kernel == -1)
	{
		terminar_memoria();
		return EXIT_FAILURE;
	}

	socket_cpu = crear_socket_servidor(logger, configuracion_memoria->puerto_escucha_cpu, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	if (socket_cpu == -1)
	{
		terminar_memoria();
		return EXIT_FAILURE;
	}

	socket_filesystem = crear_socket_servidor(logger, configuracion_memoria->puerto_escucha_filesystem, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	if (socket_kernel == -1)
	{
		terminar_memoria();
		return EXIT_FAILURE;
	}

	conexion_con_filesystem = esperar_conexion_de_cliente(logger, socket_filesystem, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	if (conexion_con_filesystem == -1)
	{
		terminar_memoria();
		return EXIT_FAILURE;
	}

	conexion_con_cpu = esperar_conexion_de_cliente(logger, socket_cpu, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	if (conexion_con_cpu == -1)
	{
		terminar_memoria();
		return EXIT_FAILURE;
	}

	conexion_con_kernel = esperar_conexion_de_cliente(logger, socket_kernel, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
	if (conexion_con_kernel == -1)
	{
		terminar_memoria();
		return EXIT_FAILURE;
	}

	// Listas
	procesos_iniciados = list_create();

	// Logica principal
	esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, conexion_con_kernel);

	int resultado_handshake_cpu = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, conexion_con_cpu);
	log_info(logger, "Se recibio la operacion %d desde %s", resultado_handshake_cpu, NOMBRE_MODULO_CPU);
	if (resultado_handshake_cpu == HANDSHAKE_CPU_MEMORIA)
	{
		realizar_handshake_cpu();
	}

	int recibir_iniciar_operacion_kernel = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, conexion_con_kernel);
	log_info(logger, "Se recibio la operacion %d desde %s", recibir_iniciar_operacion_kernel, NOMBRE_MODULO_KERNEL);
	if (recibir_iniciar_operacion_kernel == INICIAR_PROCESO_MEMORIA)
	{
		iniciar_proceso_memoria();
	}

	// Finalizacion
	terminar_memoria();
	return EXIT_SUCCESS;
}

void realizar_handshake_cpu()
{
	log_debug(logger, "Comenzando la creacion de paquete para enviar handshake al cpu!");
	t_paquete *paquete = crear_paquete(logger, HANDSHAKE_CPU_MEMORIA);

	agregar_int_a_paquete(logger, paquete, configuracion_memoria->tam_pagina, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, HANDSHAKE_CPU_MEMORIA);
	log_debug(logger, "Exito en la creacion de paquete para enviar handshake al cpu!");

	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	log_debug(logger, "Exito en el envio de paquete para handshake al cpu!");
	destruir_paquete(logger, paquete);
}

void iniciar_proceso_memoria()
{
	int tamanio_buffer;
	void *buffer_de_paquete = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_kernel, INICIAR_PROCESO_MEMORIA);

	void *buffer_de_paquete_con_offset = buffer_de_paquete;

	char *path;
	int size;
	int prioridad;
	int pid;

	leer_string_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_de_paquete_con_offset, &path, INICIAR_PROCESO_MEMORIA);
	log_info(logger, "El path del archivo con el pseudocodigo para iniciar el proceso es: %d", *path);

	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_de_paquete_con_offset, &size, INICIAR_PROCESO_MEMORIA);
	log_info(logger, "El tamanio del proceso a iniciar es: %d", size);

	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_de_paquete_con_offset, &prioridad, INICIAR_PROCESO_MEMORIA);
	log_info(logger, "La prioridad del proceso a iniciar es: %d", prioridad);

	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_de_paquete_con_offset, &pid, INICIAR_PROCESO_MEMORIA);
	log_info(logger, "El PID del proceso a iniciar es: %d", pid);

	FILE *archivo = abrir_archivo(logger, path);
	free(path);

	t_archivo_proceso *iniciar_proceso = malloc(sizeof(t_archivo_proceso));
	iniciar_proceso->archivo = archivo;
	iniciar_proceso->pid = pid;

	list_add(procesos_iniciados, iniciar_proceso);

	// todo ver si es necesario avisarle al kernel que el proceso se inicio correctamente
}

void enviar_instruccion_a_cpu()
{
	int tamanio_buffer;
	void *buffer_de_paquete = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_kernel, ENVIAR_INSTRUCCION_MEMORIA_A_CPU);

	void *buffer_de_paquete_con_offset = buffer_de_paquete;

	int pid;
	int pc;

	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_de_paquete_con_offset, &pid, ENVIAR_INSTRUCCION_MEMORIA_A_CPU);
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_de_paquete_con_offset, &pc, ENVIAR_INSTRUCCION_MEMORIA_A_CPU);

	log_info(logger, "El proceso que se va a ejecutar tiene el pid %d y se pidio la instruccion del pc %d", pid, pc);

	t_archivo_proceso *archivo_proceso = buscar_archivo_con_pid(pid);
	if (archivo_proceso == NULL) {
		return;
	}

	char *proxima_instruccion = buscar_linea(logger, archivo_proceso->archivo, pc);

	log_debug(logger, "Comenzando la creacion de paquete para enviar la instruccion %s al cpu!", proxima_instruccion);
	t_paquete *paquete = crear_paquete_enviar_instruccion_a_cpu(logger);

	agregar_string_a_paquete(logger, paquete, proxima_instruccion, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, ENVIAR_INSTRUCCION_MEMORIA_A_CPU);
	log_debug(logger, "Exito en la creacion de paquete para enviar instruccion %s al cpu!", proxima_instruccion);

	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	log_debug(logger, "Exito en el envio de paquete para instruccion %s al cpu!", proxima_instruccion);
	destruir_paquete(logger, paquete);
}

void finalizar_proceso_en_memoria() {
	int tamanio_buffer;
	void *buffer_de_paquete = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &tamanio_buffer, conexion_con_kernel, FINALIZAR_PROCESO_MEMORIA);

	void *buffer_de_paquete_con_offset = buffer_de_paquete;

	int *pid = malloc(sizeof(int));
	leer_int_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, &buffer_de_paquete_con_offset, pid, FINALIZAR_PROCESO_MEMORIA);
	log_info(logger, "El PID del proceso a finalizar es: %d", *pid);

	// todo buscar dentro de lista de procesos iniciados y cerrar archivo y hacer un free de la estructura
	t_archivo_proceso *archivo_proceso = buscar_archivo_con_pid(pid);
	if (archivo_proceso == NULL) {
		return;
	}
	cerrar_archivo(logger, archivo_proceso->archivo);
	list_remove_element(procesos_iniciados, archivo_proceso);
	free(archivo_proceso);
}

// TODO revisar find en commons
t_archivo_proceso *buscar_archivo_con_pid(int *pid)
{
	if (list_is_empty(procesos_iniciados))
	{
		log_error(logger, "No hay archivos de procesos iniciados en la lista");
		return;
	}

	t_list_iterator *iterador = list_iterator_create(procesos_iniciados);

	while (list_iterator_has_next(iterador))
	{
		t_archivo_proceso *archivo_proceso = list_iterator_next(iterador);
		if (archivo_proceso->pid == *pid)
		{
			log_debug(logger, "Se encuentra la estructura de pseudocodigo para el pid %d", *pid);
			list_iterator_destroy(iterador);
			return archivo_proceso;
		}
		
	}

	list_iterator_destroy(iterador);
	log_error(logger, "No se encuentran estructuras de pseudocodigo para el pid %d", *pid);
	return NULL;
}

void destruir_listas() {
	list_destroy(procesos_iniciados);
}

void terminar_memoria()
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_MEMORIA);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_memoria);
	destruir_configuracion(configuracion_memoria);
	destruir_listas();
	// todo revisar si es necesario aca finalizar los procesos iniciados

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