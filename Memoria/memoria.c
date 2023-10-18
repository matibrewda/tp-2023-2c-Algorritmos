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

		if (operacion_recibida_de_kernel == INICIAR_PROCESO_MEMORIA)
		{
			t_proceso_memoria *proceso_memoria = leer_paquete_iniciar_proceso_en_memoria(logger, conexion_con_kernel);
			int estado_iniciar_proceso = iniciar_proceso_memoria(proceso_memoria->path, proceso_memoria->size, proceso_memoria->prioridad, proceso_memoria->pid);
			free(proceso_memoria->path);
			free(proceso_memoria);
			t_paquete *paquete = crear_paquete_estado_iniciar_proceso(logger, estado_iniciar_proceso);
			enviar_paquete(logger, conexion_con_kernel, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
		}
		else if (operacion_recibida_de_kernel == FINALIZAR_PROCESO_MEMORIA)
		{
			t_proceso_memoria *proceso_memoria = leer_paquete_finalizar_proceso_en_memoria(logger, conexion_con_kernel);
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

		if (operacion_recibida_de_cpu == SOLICITAR_INFO_DE_MEMORIA_INICIAL_PARA_CPU)
		{
			enviar_info_de_memoria_inicial_para_cpu();
		}
		else if (operacion_recibida_de_cpu == SOLICITAR_INSTRUCCION_A_MEMORIA)
		{
			t_pedido_instruccion *pedido_instruccion = leer_paquete_pedido_instruccion(logger, conexion_con_cpu);
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

		if (operacion_recibida_de_filesystem == LEER_ARCHIVO_MEMORIA)
		{
			t_pedido_leer_archivo *pedido_leer_archivo = leer_paquete_pedido_leer_archivo(logger,conexion_con_filesystem);
			notificar_lectura_a_filesystem();//devuelve un paquete con el mismo opcode y sin info por ahora
			free(pedido_leer_archivo);
		}
		else if (operacion_recibida_de_filesystem == ESCRIBIR_ARCHIVO_MEMORIA)
		{
			t_pedido_escribir_archivo *pedido_escribir_archivo = leer_paquete_pedido_escribir_archivo(logger,conexion_con_filesystem);
			notificar_escritura_a_filesystem();//devuelve un paquete con el mismo opcode y sin info por ahora
			free(pedido_escribir_archivo);
		}
	}
}

void enviar_info_de_memoria_inicial_para_cpu()
{
	log_debug(logger, "Comenzando la creacion de paquete para enviar informacion inicial de memoria a la CPU");

	t_info_memoria* info_memoria = malloc(sizeof(t_info_memoria));
	info_memoria->tamanio_memoria = configuracion_memoria->tam_memoria;
	info_memoria->tamanio_pagina = configuracion_memoria->tam_pagina;

	t_paquete *paquete = crear_paquete_enviar_info_inicial_de_memoria_a_cpu(logger, info_memoria);
	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

	free(info_memoria);
}

int iniciar_proceso_memoria(char *path, int size, int prioridad, int pid)
{
	log_info(logger, "El path del archivo con el pseudocodigo para iniciar el proceso es: %s", path);
	log_info(logger, "El tamanio del proceso a iniciar es: %d", size);
	log_info(logger, "La prioridad del proceso a iniciar es: %d", prioridad);
	log_info(logger, "El PID del proceso a iniciar es: %d", pid);

	FILE *archivo = abrir_archivo(logger, strcat(strcat(configuracion_memoria->path_instrucciones, "/"),path));
	if (archivo == NULL) {
		return 0;
	}

	t_archivo_proceso *iniciar_proceso = malloc(sizeof(t_archivo_proceso));
	iniciar_proceso->archivo = archivo;
	iniciar_proceso->pid = pid;

	list_add(procesos_iniciados, iniciar_proceso);

	//pedir_bloques_a_fylesystem(cantidad_de_bloques);//todo
	return 1;
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
	t_paquete *paquete = crear_paquete_enviar_instruccion_a_cpu(logger, linea_instruccion);

	// Retardo de respuesta!
	sleep(configuracion_memoria->retardo_respuesta);

	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	log_debug(logger, "Exito en el envio de paquete para instruccion %s al cpu!", linea_instruccion);
}

void finalizar_proceso_en_memoria(int pid)
{
	log_info(logger, "El PID del proceso a finalizar es: %d", pid);

	// todo buscar dentro de lista de procesos iniciados y cerrar archivo y hacer un free de la estructura
	t_archivo_proceso *archivo_proceso = buscar_archivo_con_pid(pid);
	if (archivo_proceso == NULL)
	{
		return;
	}
	cerrar_archivo(logger, archivo_proceso->archivo);
	//list_remove_element(procesos_iniciados, archivo_proceso); // NO COMPILA!

	//pedir_liberacion_de_bloques_a_filesystem(bloques);//todo
	free(archivo_proceso);
}

// TODO revisar find en commons
t_archivo_proceso *buscar_archivo_con_pid(int pid)
{
	if (list_is_empty(procesos_iniciados))
	{
		log_error(logger, "No hay archivos de procesos iniciados en la lista");
		return NULL;
	}

	t_list_iterator *iterador = list_iterator_create(procesos_iniciados);

	while (list_iterator_has_next(iterador))
	{
		t_archivo_proceso *archivo_proceso = list_iterator_next(iterador);
		if (archivo_proceso->pid == pid)
		{
			log_debug(logger, "Se encuentra la estructura de pseudocodigo para el pid %d", pid);
			list_iterator_destroy(iterador);
			return archivo_proceso;
		}
	}

	list_iterator_destroy(iterador);
	log_error(logger, "No se encuentran estructuras de pseudocodigo para el pid %d", pid);
	return NULL;
}


void notificar_lectura_a_filesystem() 
{
	log_debug(logger,"Notificando a File System de lectura exitosa en memoria");
	t_paquete *paquete = crear_paquete_con_opcode_y_sin_contenido(logger,LEER_ARCHIVO_MEMORIA,NOMBRE_MODULO_MEMORIA,NOMBRE_MODULO_FILESYSTEM);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

void notificar_escritura_a_filesystem() 
{
	log_debug(logger,"Notificando a File System de lectura exitosa en memoria");
	t_paquete *paquete = crear_paquete_con_opcode_y_sin_contenido(logger,ESCRIBIR_ARCHIVO_MEMORIA,NOMBRE_MODULO_MEMORIA,NOMBRE_MODULO_FILESYSTEM);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

void pedir_bloques_a_fylesystem(int cantidad_de_bloques)
{
	//todo
}

void pedir_liberacion_de_bloques_a_filesystem(int bloques)
{
	//todo
}


void destruir_listas()
{
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