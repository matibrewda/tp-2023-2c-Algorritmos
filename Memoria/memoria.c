#include "memoria.h"

// Variables globales
t_log *logger = NULL;
t_argumentos_memoria *argumentos_memoria = NULL;
t_config_memoria *configuracion_memoria = NULL;
t_list *procesos_iniciados = NULL;

// Conexiones
int socket_kernel = -1;
int socket_cpu = -1;
int socket_filesystem = -1;
int conexion_con_kernel = -1;
int conexion_con_cpu = -1;
int conexion_con_filesystem = -1;

// Hilos
pthread_t hilo_atiendo_kernel;
pthread_t hilo_atiendo_cpu;
pthread_t hilo_atiendo_filesystem;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	atexit(terminar_memoria);

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

	// Hilos
	pthread_create(&hilo_atiendo_cpu, NULL, atender_cpu, NULL);
	pthread_create(&hilo_atiendo_kernel, NULL, atender_kernel, NULL);
	pthread_create(&hilo_atiendo_filesystem, NULL, atender_filesystem, NULL);

	// Logica principal
	pthread_join(hilo_atiendo_cpu, NULL);
	pthread_join(hilo_atiendo_kernel, NULL);

	// Finalizacion
	terminar_memoria();
	return EXIT_SUCCESS;
}

void *atender_kernel()
{
	while (true)
	{
		int operacion_recibida_de_kernel = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL, conexion_con_kernel);
		log_debug(logger, "Se recibio la operacion %s desde %s", nombre_opcode(operacion_recibida_de_kernel), NOMBRE_MODULO_KERNEL);

		if (operacion_recibida_de_kernel == SOLICITUD_INICIAR_PROCESO_MEMORIA)
		{
			t_proceso_memoria *proceso_memoria = leer_paquete_solicitud_iniciar_proceso_en_memoria(logger, conexion_con_kernel);
			iniciar_proceso_memoria(proceso_memoria->path, proceso_memoria->size, proceso_memoria->prioridad, proceso_memoria->pid);
			free(proceso_memoria->path);
			free(proceso_memoria);
		}
		else if (operacion_recibida_de_kernel == SOLICITUD_FINALIZAR_PROCESO_MEMORIA)
		{
			t_proceso_memoria *proceso_memoria = leer_paquete_solicitud_finalizar_proceso_en_memoria(logger, conexion_con_kernel);
			finalizar_proceso_en_memoria(proceso_memoria->pid);
			free(proceso_memoria->path);
			free(proceso_memoria);
		}
	}
}

void *atender_cpu()
{
	while (true)
	{
		int operacion_recibida_de_cpu = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU, conexion_con_cpu);
		log_debug(logger, "Se recibio la operacion %s desde %s", nombre_opcode(operacion_recibida_de_cpu), NOMBRE_MODULO_CPU);

		if (operacion_recibida_de_cpu == SOLICITUD_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU)
		{
			enviar_info_de_memoria_inicial_para_cpu();
		}
		else if (operacion_recibida_de_cpu == SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA)
		{
			t_pedido_instruccion *pedido_instruccion = leer_paquete_solicitud_pedir_instruccion_a_memoria(logger, conexion_con_cpu);
			enviar_instruccion_a_cpu(pedido_instruccion->pid, pedido_instruccion->pc);
			free(pedido_instruccion);
		}
	}
}

void *atender_filesystem()
{
	while (true)
	{
		int operacion_recibida_de_filesystem = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem);
		log_debug(logger, "Se recibio la operacion %s desde %s", nombre_opcode(operacion_recibida_de_filesystem), NOMBRE_MODULO_FILESYSTEM);

		if (operacion_recibida_de_filesystem == SOLICITUD_LEER_ARCHIVO_MEMORIA)
		{
			t_pedido_leer_archivo *pedido_leer_archivo = leer_paquete_pedido_leer_archivo(logger,conexion_con_filesystem);
			notificar_lectura_a_filesystem();
			free(pedido_leer_archivo);
		}
		else if (operacion_recibida_de_filesystem == SOLICITUD_ESCRIBIR_ARCHIVO_MEMORIA)
		{
			t_pedido_escribir_archivo *pedido_escribir_archivo = leer_paquete_pedido_escribir_archivo(logger,conexion_con_filesystem);
			notificar_escritura_a_filesystem();
			free(pedido_escribir_archivo);
		}
	}
}

void enviar_info_de_memoria_inicial_para_cpu()
{
	log_debug(logger, "Comenzando la creacion de paquete para enviar informacion inicial de memoria a la CPU");

	t_info_memoria *info_memoria = malloc(sizeof(t_info_memoria));
	info_memoria->tamanio_memoria = configuracion_memoria->tam_memoria;
	info_memoria->tamanio_pagina = configuracion_memoria->tam_pagina;

	t_paquete *paquete = crear_paquete_respuesta_pedir_info_de_memoria_inicial_para_cpu(logger, info_memoria);
	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	free(info_memoria);
}

void iniciar_proceso_memoria(char *path, int size, int prioridad, int pid)
{
	log_info(logger, "El path del archivo con el pseudocodigo para iniciar el proceso es: %s", path);
	log_info(logger, "El tamanio del proceso a iniciar es: %d", size);
	log_info(logger, "La prioridad del proceso a iniciar es: %d", prioridad);
	log_info(logger, "El PID del proceso a iniciar es: %d", pid);

	char *ruta_archivo_completa = NULL;
	int error_archivo = asprintf(&ruta_archivo_completa, "%s/%s", configuracion_memoria->path_instrucciones, path);

	if (error_archivo == -1)
	{
		log_error(logger, "Error al calcular ruta completa para archivo de pseudocodigo (para PID: %d)", pid);
		enviar_paquete_respuesta_iniciar_proceso_en_memoria_a_kernel(false);
		return;
	}

	FILE *archivo = abrir_archivo(logger, ruta_archivo_completa);

	if (archivo == NULL)
	{
		log_error(logger, "No se pudo abrir archivo de pseudocodigo (para PID: %d)", pid);
		enviar_paquete_respuesta_iniciar_proceso_en_memoria_a_kernel(false);
		return;
	}

	t_archivo_proceso *iniciar_proceso = malloc(sizeof(t_archivo_proceso));
	iniciar_proceso->archivo = archivo;
	iniciar_proceso->pid = pid;

	log_trace(logger, "Intento agregar proceso PID: %d a la lista", pid);
	list_add(procesos_iniciados, iniciar_proceso);
	log_trace(logger, "Agregado proceso PID: %d a la lista", pid);

	int cantidad_de_bloques_mock = 1; // TODO MOCK
	t_list *posiciones_swap = pedir_bloques_a_filesystem(cantidad_de_bloques_mock); // TODO pasar cantidad de bloques correspondiente
	enviar_paquete_respuesta_iniciar_proceso_en_memoria_a_kernel(true);
}

void enviar_paquete_respuesta_iniciar_proceso_en_memoria_a_kernel(bool resultado_iniciar_proceso_en_memoria)
{
	t_paquete *paquete = crear_paquete_respuesta_iniciar_proceso_en_memoria(logger, resultado_iniciar_proceso_en_memoria);
	enviar_paquete(logger, conexion_con_kernel, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
}

void enviar_paquete_respuesta_finalizar_proceso_en_memoria_a_kernel()
{
	t_paquete *paquete = crear_paquete_respuesta_finalizar_proceso_en_memoria(logger);
	enviar_paquete(logger, conexion_con_kernel, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
}

void enviar_instruccion_a_cpu(int pid, int pc)
{
	log_info(logger, "El proceso pid=%d pide instruccion en pc=%d", pid, pc);

	t_archivo_proceso *archivo_proceso = buscar_archivo_con_pid(pid);

	if (archivo_proceso == NULL)
	{
		return;
	}

	char *linea_instruccion = buscar_linea(logger, archivo_proceso->archivo, pc);

	log_debug(logger, "Comenzando la creacion de paquete para enviar la instruccion %s al cpu!", linea_instruccion);
	t_paquete *paquete = crear_paquete_respuesta_pedir_instruccion_a_memoria(logger, linea_instruccion);

	// Retardo de respuesta!
	usleep((configuracion_memoria->retardo_respuesta)*1000);

	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	log_debug(logger, "Exito en el envio de paquete para instruccion %s al cpu!", linea_instruccion);
}

void finalizar_proceso_en_memoria(int pid)
{
	// aca hay algun error (falta alguna validacion o algo asi)
	log_info(logger, "El PID del proceso a finalizar es: %d", pid);

	// todo buscar dentro de lista de procesos iniciados y cerrar archivo y hacer un free de la estructura
	t_archivo_proceso *archivo_proceso = buscar_archivo_con_pid(pid);
	if (archivo_proceso == NULL)
	{
		return;
	}
	cerrar_archivo_con_pid(pid);

	pedir_liberacion_de_bloques_a_filesystem();//todo pasar lista de bloques
	enviar_paquete_respuesta_finalizar_proceso_en_memoria_a_kernel();
	free(archivo_proceso);
}

t_archivo_proceso *buscar_archivo_con_pid(int pid)
{
	bool _filtro_archivo_proceso_por_id(t_archivo_proceso * archivo_proceso)
	{
		return archivo_proceso->pid == pid;
	};

	t_archivo_proceso* resultado = list_find(procesos_iniciados, (void *)_filtro_archivo_proceso_por_id);

	if (resultado == NULL)
	{
		log_warning(logger, "No se encontro archivo proceso con PID %d", pid);
	}

	return resultado;
}

void cerrar_archivo_con_pid(int pid)
{
	bool _filtro_archivo_proceso_por_id(t_archivo_proceso *archivo_proceso)
	{
		return archivo_proceso->pid == pid;
	};

	void _finalizar_archivo_proceso(t_archivo_proceso *archivo_proceso)
	{
		cerrar_archivo(logger, archivo_proceso->archivo);
		free(archivo_proceso);
	}

	list_remove_and_destroy_by_condition(procesos_iniciados, (void *)_filtro_archivo_proceso_por_id, (void *)_finalizar_archivo_proceso);
}


void notificar_lectura_a_filesystem() 
{
	log_debug(logger,"Notificando a File System de lectura exitosa en memoria");
	t_paquete *paquete = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_LEER_ARCHIVO_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

void notificar_escritura_a_filesystem() 
{
	log_debug(logger,"Notificando a File System de lectura exitosa en memoria");
	t_paquete *paquete = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_ESCRIBIR_ARCHIVO_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

t_list *pedir_bloques_a_filesystem(int cantidad_de_bloques)
{
	t_list *posiciones_swap = list_create();
	t_paquete *paquete = crear_paquete_pedir_bloques_a_filesystem(logger, cantidad_de_bloques);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	// TODO bloquear hilo esperando el paquete de respuesta?
	return posiciones_swap;
}

void pedir_liberacion_de_bloques_a_filesystem()
{
	t_list *posiciones_swap = list_create(); // TODO MOCK
	t_paquete *paquete = crear_paquete_liberar_bloques_en_filesystem(logger, posiciones_swap);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	// TODO esperar respuesta de liberacion de bloques?
}

void destruir_listas()
{
	list_destroy(procesos_iniciados);
}

void terminar_memoria()
{
	if (logger != NULL)
	{
		log_warning(logger, "Algo salio mal!");
		log_warning(logger, "Finalizando %s", NOMBRE_MODULO_MEMORIA);
	}

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