#include "memoria.h"

// Variables globales
t_log *logger = NULL;
t_argumentos_memoria *argumentos_memoria = NULL;
t_config_memoria *configuracion_memoria = NULL;
t_list *procesos_iniciados = NULL;
t_list *tabla_de_paginas = NULL;
t_bitarray *tabla_de_marcos = NULL;
void *memoria_real;
int cantidad_de_frames = -1;

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

// Colas
t_queue *cola_fifo_entradas = NULL;

// Mutex
pthread_mutex_t mutex_cola_fifo_entradas;
pthread_mutex_t mutex_conexion_filesystem;
pthread_mutex_t mutex_entradas_tabla_de_paginas;
pthread_mutex_t mutex_procesos_iniciados;
pthread_mutex_t mutex_memoria_real;
pthread_mutex_t mutex_tabla_de_marcos;

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

	// Mutex
	pthread_mutex_init(&mutex_cola_fifo_entradas, NULL);
	pthread_mutex_init(&mutex_conexion_filesystem, NULL);
	pthread_mutex_init(&mutex_entradas_tabla_de_paginas, NULL);
	pthread_mutex_init(&mutex_procesos_iniciados, NULL);
	pthread_mutex_init(&mutex_memoria_real, NULL);
	pthread_mutex_init(&mutex_tabla_de_marcos, NULL);

	// Colas
	cola_fifo_entradas = queue_create();

	// Listas
	procesos_iniciados = list_create();
	tabla_de_paginas = list_create();

	// Creacion de Estructuras
	memoria_real = malloc(configuracion_memoria->tam_memoria);
	inicializar_lista_de_marcos_bitmap();

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
		else if (operacion_recibida_de_kernel == SOLICITUD_CARGAR_PAGINA_EN_MEMORIA)
		{
			t_pedido_pagina_en_memoria *pedido_cargar_pagina = leer_paquete_solicitud_pedido_pagina_en_memoria(logger, conexion_con_cpu, SOLICITUD_CARGAR_PAGINA_EN_MEMORIA, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
			cargar_pagina_en_memoria(pedido_cargar_pagina->pid, pedido_cargar_pagina->numero_de_pagina);
			free(pedido_cargar_pagina);
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
			t_info_memoria *info_memoria = malloc(sizeof(t_info_memoria));
			info_memoria->tamanio_memoria = configuracion_memoria->tam_memoria;
			info_memoria->tamanio_pagina = configuracion_memoria->tam_pagina;

			t_paquete *paquete = crear_paquete_respuesta_pedir_info_de_memoria_inicial_para_cpu(logger, info_memoria);
			enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);

			free(info_memoria);
		}
		else if (operacion_recibida_de_cpu == SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA)
		{
			t_pedido_instruccion *pedido_instruccion = leer_paquete_solicitud_pedir_instruccion_a_memoria(logger, conexion_con_cpu);
			enviar_instruccion_a_cpu(pedido_instruccion->pid, pedido_instruccion->pc);
			free(pedido_instruccion);
		}
		else if (operacion_recibida_de_cpu == SOLICITUD_PEDIR_NUMERO_DE_MARCO_A_MEMORIA)
		{
			t_pedido_pagina_en_memoria *pedido_numero_de_marco = leer_paquete_solicitud_pedido_pagina_en_memoria(logger, conexion_con_cpu, SOLICITUD_PEDIR_NUMERO_DE_MARCO_A_MEMORIA, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
			enviar_numero_de_marco_a_cpu(pedido_numero_de_marco->pid, pedido_numero_de_marco->numero_de_pagina);
			free(pedido_numero_de_marco);
		}
		else if (operacion_recibida_de_cpu == SOLICITUD_LEER_VALOR_EN_MEMORIA)
		{
			int direccion_fisica = leer_paquete_solicitud_leer_valor_en_memoria(logger, conexion_con_cpu);
			uint32_t valor_leido = leer_valor_en_memoria(direccion_fisica);
			t_valor_leido_en_memoria *valor_leido_en_memoria = malloc(sizeof(t_valor_leido_en_memoria));
			valor_leido_en_memoria->valor_leido = valor_leido;
			t_paquete *paquete = crear_paquete_respuesta_leer_valor_en_memoria(logger, valor_leido_en_memoria);
			enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
			free(valor_leido_en_memoria);
		}
		else if (operacion_recibida_de_cpu == SOLICITUD_ESCRIBIR_VALOR_EN_MEMORIA)
		{
			t_pedido_escribir_valor_en_memoria *pedido_escribir_valor_en_memoria = leer_paquete_solicitud_escribir_valor_en_memoria(logger, conexion_con_cpu);
			escribir_valor_en_memoria(pedido_escribir_valor_en_memoria->direccion_fisica, pedido_escribir_valor_en_memoria->valor_a_escribir);
			t_paquete *paquete = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_ESCRIBIR_VALOR_EN_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
			enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
			free(pedido_escribir_valor_en_memoria);
		}
	}
}

void *atender_filesystem()
{
	while (true)
	{
		int operacion_recibida_de_filesystem = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem);
		log_debug(logger, "Se recibio la operacion %s desde %s", nombre_opcode(operacion_recibida_de_filesystem), NOMBRE_MODULO_FILESYSTEM);

		if (operacion_recibida_de_filesystem == SOLICITUD_LEER_MARCO_DE_MEMORIA)
		{
			char *nombre_archivo_a_escribir;
			int puntero_archivo_a_escribir;
			int direccion_fisica;
			leer_paquete_solicitud_leer_marco_de_memoria(logger, conexion_con_filesystem, &direccion_fisica, &nombre_archivo_a_escribir, &puntero_archivo_a_escribir);
			void* contenido_marco; // TODO: LEER MARCO ENTERO
			t_paquete *paquete = crear_paquete_respuesta_leer_marco_de_memoria(logger, nombre_archivo_a_escribir, puntero_archivo_a_escribir, contenido_marco, configuracion_memoria->tam_pagina);
			enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
		}
		else if (operacion_recibida_de_filesystem == SOLICITUD_ESCRIBIR_BLOQUE_EN_MEMORIA)
		{
			void* contenido_bloque;
			int direccion_fisica;
			leer_paquete_solicitud_escribir_bloque_en_memoria(logger, conexion_con_filesystem, &direccion_fisica, &contenido_bloque);
			// TODO: ESCRIBIR BLOQUE ENTERO
			t_paquete *respuesta_escribir_bloque_en_memoria = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_ESCRIBIR_BLOQUE_EN_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
			enviar_paquete(logger, conexion_con_filesystem, respuesta_escribir_bloque_en_memoria, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
		}
		else if (operacion_recibida_de_filesystem == RESPUESTA_LIBERAR_BLOQUES_EN_FILESYSTEM)
		{
			t_paquete *paquete = crear_paquete_respuesta_finalizar_proceso_en_memoria(logger);
			enviar_paquete(logger, conexion_con_kernel, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
		}
		else if (operacion_recibida_de_filesystem == RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM)
		{
			int pid_reservando;
			t_list *posiciones_swap = leer_paquete_respuesta_pedir_bloques_a_filesystem(logger, conexion_con_filesystem, &pid_reservando);

			if (list_is_empty(posiciones_swap))
			{
				log_error(logger, "No alcanzan los bloques de swap (para PID: %d)", pid_reservando);
				enviar_paquete_respuesta_iniciar_proceso_en_memoria_a_kernel(false);
			}
			else
			{
				crear_entrada_de_tabla_de_paginas_de_proceso(list_size(posiciones_swap), posiciones_swap, pid_reservando);
				log_info(logger, "Creacion - PID: <%d> - Tamanio : <%d>", pid_reservando, list_size(posiciones_swap));
				enviar_paquete_respuesta_iniciar_proceso_en_memoria_a_kernel(true);
			}
		}
	}
}

void escribir_valor_en_memoria(int direccion_fisica, uint32_t valor_a_escribir)
{
	pthread_mutex_lock(&mutex_memoria_real);
	*(uint32_t *)(memoria_real + direccion_fisica) = valor_a_escribir;
	pthread_mutex_unlock(&mutex_memoria_real);

	int numero_de_marco = floor(direccion_fisica / configuracion_memoria->tam_pagina);
	t_entrada_de_tabla_de_pagina *pagina = obtener_entrada_de_tabla_de_pagina_por_marco_presente(numero_de_marco);
	pthread_mutex_lock(&mutex_entradas_tabla_de_paginas);
	pagina->modificado = 1;
	pthread_mutex_unlock(&mutex_entradas_tabla_de_paginas);

	usleep((configuracion_memoria->retardo_respuesta) * 1000);
}

uint32_t leer_valor_en_memoria(int direccion_fisica)
{
	pthread_mutex_lock(&mutex_memoria_real);
	uint32_t valor_leido = *(uint32_t *)(memoria_real + direccion_fisica);
	pthread_mutex_unlock(&mutex_memoria_real);

	usleep((configuracion_memoria->retardo_respuesta) * 1000);
	return valor_leido;
}

void inicializar_lista_de_marcos_bitmap()
{
	cantidad_de_frames = configuracion_memoria->tam_memoria / configuracion_memoria->tam_pagina;
	char *data = (char *)malloc(cantidad_de_frames / 8);
	tabla_de_marcos = bitarray_create_with_mode(data, cantidad_de_frames / 8, MSB_FIRST);
}

void iniciar_proceso_memoria(char *path, int size, int prioridad, int pid)
{
	log_debug(logger, "El path del archivo con el pseudocodigo para iniciar el proceso es: %s", path);
	log_debug(logger, "El tamanio del proceso a iniciar es: %d", size);
	log_debug(logger, "El PID del proceso a iniciar es: %d", pid);

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
	list_add_thread_safe(procesos_iniciados, iniciar_proceso, &mutex_procesos_iniciados);

	int cantidad_de_bloques = size / configuracion_memoria->tam_pagina;
	if (size % configuracion_memoria->tam_pagina != 0)
	{
		cantidad_de_bloques++;
	}

	pthread_mutex_lock(&mutex_conexion_filesystem);
	t_paquete *paquete = crear_paquete_pedir_bloques_a_filesystem(logger, pid, cantidad_de_bloques);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	pthread_mutex_unlock(&mutex_conexion_filesystem);
}

void enviar_paquete_respuesta_iniciar_proceso_en_memoria_a_kernel(bool resultado_iniciar_proceso_en_memoria)
{
	t_paquete *paquete = crear_paquete_respuesta_iniciar_proceso_en_memoria(logger, resultado_iniciar_proceso_en_memoria);
	enviar_paquete(logger, conexion_con_kernel, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
}

void enviar_paquete_respuesta_cargar_pagina_en_memoria_a_kernel(bool resultado_cargar_pagina_en_memoria)
{
	t_paquete *paquete = crear_paquete_respuesta_cargar_pagina_en_memoria(logger, resultado_cargar_pagina_en_memoria);
	enviar_paquete(logger, conexion_con_kernel, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
}

void enviar_instruccion_a_cpu(int pid, int pc)
{
	log_info(logger, "El proceso pid=%d pide instruccion en pc=%d", pid, pc);

	t_archivo_proceso *archivo_proceso = buscar_archivo_con_pid(pid);

	char *linea_instruccion = buscar_linea(logger, archivo_proceso->archivo, pc);

	log_debug(logger, "Comenzando la creacion de paquete para enviar la instruccion %s al cpu!", linea_instruccion);
	t_paquete *paquete = crear_paquete_respuesta_pedir_instruccion_a_memoria(logger, linea_instruccion);

	usleep((configuracion_memoria->retardo_respuesta) * 1000);

	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	log_debug(logger, "Exito en el envio de paquete para instruccion %s al cpu!", linea_instruccion);
}

void finalizar_proceso_en_memoria(int pid)
{
	log_info(logger, "El PID del proceso a finalizar es: %d", pid);
	t_archivo_proceso *archivo_proceso = buscar_archivo_con_pid(pid);
	cerrar_archivo_con_pid(pid);
	int cantidad_de_bloques = cantidad_de_paginas_proceso(pid);
	limpiar_entradas_tabla_de_paginas(pid);
	log_info(logger, "Destruccion - PID: <%d> - Tamanio : <%d>", pid, cantidad_de_bloques);
	free(archivo_proceso);
}

int cantidad_de_paginas_proceso(int pid)
{
	return list_size_thread_safe(obtener_entradas_de_tabla_de_pagina_por_pid(pid), &mutex_entradas_tabla_de_paginas);
}

t_archivo_proceso *buscar_archivo_con_pid(int pid)
{
	bool _filtro_archivo_proceso_por_id(t_archivo_proceso * archivo_proceso)
	{
		return archivo_proceso->pid == pid;
	};

	t_archivo_proceso *resultado = list_find_thread_safe(procesos_iniciados, (void *)_filtro_archivo_proceso_por_id, &mutex_procesos_iniciados);

	if (resultado == NULL)
	{
		log_warning(logger, "No se encontro archivo proceso con PID %d", pid);
	}

	return resultado;
}

void cerrar_archivo_con_pid(int pid)
{
	bool _filtro_archivo_proceso_por_id(t_archivo_proceso * archivo_proceso)
	{
		return archivo_proceso->pid == pid;
	};

	void _finalizar_archivo_proceso(t_archivo_proceso * archivo_proceso)
	{
		cerrar_archivo(logger, archivo_proceso->archivo);
		free(archivo_proceso);
	}

	list_remove_and_destroy_by_condition_thread_safe(procesos_iniciados, (void *)_filtro_archivo_proceso_por_id, (void *)_finalizar_archivo_proceso, &mutex_procesos_iniciados);
}

void limpiar_entradas_tabla_de_paginas(int pid)
{
	t_list *entradas_tabla_de_paginas = obtener_entradas_de_tabla_de_pagina_por_pid(pid);
	t_list *bloques_a_liberar = list_create();
	t_entrada_de_tabla_de_pagina *entrada_tabla_de_paginas;

	for (int i = 0; i < list_size_thread_safe(entradas_tabla_de_paginas, &mutex_entradas_tabla_de_paginas); i++)
	{
		entrada_tabla_de_paginas = list_get_thread_safe(entradas_tabla_de_paginas, i, &mutex_entradas_tabla_de_paginas);

		int *posicion_en_swap = malloc(sizeof(int));
		*posicion_en_swap = entrada_tabla_de_paginas->posicion_en_swap;
		list_add(bloques_a_liberar, posicion_en_swap);

		eliminar_entrada_de_cola(pid, cola_fifo_entradas, &mutex_cola_fifo_entradas);
		if (es_pagina_presente(entrada_tabla_de_paginas))
		{
			borrar_contenido_de_marco_en_memoria_real(entrada_tabla_de_paginas->marco);
		}
		eliminar_entrada_de_tabla_de_paginas(pid);
		free(entrada_tabla_de_paginas);
	}

	t_paquete *paquete = crear_paquete_liberar_bloques_en_filesystem(logger, bloques_a_liberar);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	while (!list_is_empty(bloques_a_liberar))
	{
		int *a_liberar = list_remove(bloques_a_liberar, 0);
		free(a_liberar);
	}
	list_destroy(bloques_a_liberar);

	log_trace(logger, "Se limpian correctamente las entradas de tabla de paginas para el pid %d", pid);
}

void eliminar_entrada_de_tabla_de_paginas(int pid)
{
	// TODO agregar mutex a tabla_de_paginas
	bool _filtro_paginas_de_memoria_pid(t_entrada_de_tabla_de_pagina * pagina_de_memoria)
	{
		return pagina_de_memoria->pid == pid;
	};

	list_remove_by_condition_thread_safe(tabla_de_paginas, (void *)_filtro_paginas_de_memoria_pid, &mutex_entradas_tabla_de_paginas);
}

void eliminar_entrada_de_cola(int pid, t_queue *cola, pthread_mutex_t *mutex)
{
	bool _filtro_paginas_de_memoria_pid(t_entrada_de_tabla_de_pagina * pagina_de_memoria)
	{
		return pagina_de_memoria->pid == pid;
	};

	list_remove_by_condition_thread_safe(cola->elements, (void *)_filtro_paginas_de_memoria_pid, mutex);
}

void pedir_liberacion_de_bloques_a_filesystem(t_list *posiciones_en_swap)
{
	t_paquete *paquete = crear_paquete_liberar_bloques_en_filesystem(logger, posiciones_en_swap);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

// Función para inicializar la tabla de páginas por proceso
void crear_entrada_de_tabla_de_paginas_de_proceso(int cantidad_de_paginas, t_list *posiciones_swap, int pid)
{
	for (int i = 0; i < cantidad_de_paginas; i++)
	{
		t_entrada_de_tabla_de_pagina *entrada_tabla_de_paginas = malloc(sizeof(t_entrada_de_tabla_de_pagina));
		entrada_tabla_de_paginas->numero_de_pagina = i;
		entrada_tabla_de_paginas->presencia = 0;
		entrada_tabla_de_paginas->posicion_en_swap = (int)(*((int *)list_get_thread_safe(posiciones_swap, i, &mutex_entradas_tabla_de_paginas)));
		list_add_thread_safe(tabla_de_paginas, entrada_tabla_de_paginas, &mutex_entradas_tabla_de_paginas);
	}
	log_trace(logger, "Se genera correctamente las %d entradas de tabla de paginas para el pid %d", cantidad_de_paginas, pid);
}

void escribir_pagina_en_swap(t_entrada_de_tabla_de_pagina *victima)
{
	void *contenido_marco = buscar_contenido_marco(victima->marco);
	t_paquete *paquete = crear_paquete_solicitud_escribir_pagina_en_swap(logger, contenido_marco, configuracion_memoria->tam_pagina, victima->posicion_en_swap);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	// TODO esperamos respuesta?
}

int es_pagina_presente(t_entrada_de_tabla_de_pagina *pagina)
{
	return (pagina->presencia == 1) ? 1 : 0;
}

int es_pagina_modificada(t_entrada_de_tabla_de_pagina *pagina)
{
	return (pagina->modificado == 1) ? 1 : 0;
}

t_entrada_de_tabla_de_pagina *encontrar_pagina_victima_fifo()
{
	t_entrada_de_tabla_de_pagina *pagina_victima = queue_pop_thread_safe(cola_fifo_entradas, &mutex_cola_fifo_entradas);
	return pagina_victima;
}

t_entrada_de_tabla_de_pagina *encontrar_pagina_victima_lru()
{
	// TODO implementame PORFI
}

t_entrada_de_tabla_de_pagina *encontrar_pagina_victima()
{
	// TODO no va a cambiar dinamicamente, no hace falta chequearlo siempre que se quiera reemplazar una pagina
	if (configuracion_memoria->algoritmo_reemplazo == "FIFO")
	{
		return encontrar_pagina_victima_fifo();
	}
	else if (configuracion_memoria->algoritmo_reemplazo == "LRU")
	{
		return encontrar_pagina_victima_lru();
	}
	log_debug(logger, "No se encontro un algoritmo de reemplazo correcto en la configuracion de la memoria");
	return NULL;
}

t_list *obtener_entradas_de_tabla_de_pagina_por_pid(int pid)
{
	bool _filtro_paginas_de_memoria_pid(t_entrada_de_tabla_de_pagina * pagina_de_memoria)
	{
		return pagina_de_memoria->pid == pid;
	};

	t_list *entradas_tabla_de_pagina = list_filter(tabla_de_paginas, (void *)_filtro_paginas_de_memoria_pid);

	if (entradas_tabla_de_pagina == NULL)
	{
		log_warning(logger, "No se encontraron entradas de tabla de pagina con el PID %d", pid);
		// TODO ver que hay que retornar en este caso
		return NULL;
	}

	return entradas_tabla_de_pagina;
}

t_entrada_de_tabla_de_pagina *obtener_entrada_de_tabla_de_pagina_por_marco_presente(int marco)
{
	bool _filtro_paginas_de_memoria_por_marco_presente(t_entrada_de_tabla_de_pagina * pagina_de_memoria)
	{
		return pagina_de_memoria->marco == marco && pagina_de_memoria->presencia == 1;
	};

	t_entrada_de_tabla_de_pagina *pagina = list_find_thread_safe(tabla_de_paginas, (void *)_filtro_paginas_de_memoria_por_marco_presente, &mutex_entradas_tabla_de_paginas);

	if (pagina == NULL)
	{
		log_error(logger, "No existe un marco presente en memoria con el numero %d", marco);
		// TODO ver que notificar aca, es Page Fault? Es un error?
		return NULL;
	}

	return pagina;
}

void *obtener_contenido_de_pagina_en_swap(int posicion_en_swap)
{
	pthread_mutex_lock(&mutex_conexion_filesystem);

	t_paquete *paquete = crear_paquete_solicitud_leer_pagina_swap(logger, posicion_en_swap);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	void *contenido_del_bloque = malloc(configuracion_memoria->tam_pagina);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem); // RESPUESTA_CONTENIDO_BLOQUE_EN_FILESYSTEM
	contenido_del_bloque = leer_paquete_respuesta_contenido_bloque(logger, conexion_con_filesystem, configuracion_memoria->tam_pagina);

	pthread_mutex_unlock(&mutex_conexion_filesystem);

	return contenido_del_bloque;
}

void cargar_pagina_en_memoria(int pid, int numero_de_pagina)
{
	t_entrada_de_tabla_de_pagina *pagina = obtener_entrada_de_tabla_de_pagina_por_pid_y_numero(pid, numero_de_pagina);
	// log_info(logger,"Acceso a tabla de paginas PID : <%d> - Pagina: <%d> - Marco: <%d>",pid,pagina->numero_de_pagina,pagina->marco); TODO ver si este log va
	if (pagina == NULL)
	{
		enviar_paquete_respuesta_cargar_pagina_en_memoria_a_kernel(false);
		return;
	}

	void *contenido_en_swap = obtener_contenido_de_pagina_en_swap(pagina->posicion_en_swap);

	int marco_desocupado;
	if (obtener_primer_marco_desocupado(&marco_desocupado))
	{
		// cargar el marco con el contenido en swap y actualizar entrada tabla de pagina
		cargar_datos_de_pagina_en_memoria_real(contenido_en_swap, marco_desocupado);
		// Actualizar entrada tabla de paginas de la nueva pagina
		actualizar_entrada_tabla_de_paginas(pagina, marco_desocupado);
		log_info(logger, "Acceso a tabla de paginas PID : <%d> - Pagina: <%d> - Marco: <%d>", pid, pagina->numero_de_pagina, pagina->marco);
		ocupar_marco(marco_desocupado);
	}
	else
	{
		int marco_libre = reemplazar_pagina(pid, numero_de_pagina);
		cargar_datos_de_pagina_en_memoria_real(contenido_en_swap, marco_libre);
		actualizar_entrada_tabla_de_paginas(pagina, marco_libre);
		log_info(logger, "Acceso a tabla de paginas PID : <%d> - Pagina: <%d> - Marco: <%d>", pid, pagina->numero_de_pagina, pagina->marco);
		ocupar_marco(marco_libre);
	}

	free(contenido_en_swap);
	queue_push_thread_safe(cola_fifo_entradas, pagina, &mutex_cola_fifo_entradas);
	enviar_paquete_respuesta_cargar_pagina_en_memoria_a_kernel(true);
	return;
}

void actualizar_entrada_tabla_de_paginas(t_entrada_de_tabla_de_pagina *pagina, int marco_asignado)
{
	pthread_mutex_lock(&mutex_entradas_tabla_de_paginas);
	pagina->presencia = 1;
	pagina->marco = marco_asignado;
	pthread_mutex_unlock(&mutex_entradas_tabla_de_paginas);

	return;
}

void *buscar_contenido_marco(int numero_de_marco)
{
	void *contenido_marco = malloc(configuracion_memoria->tam_pagina);
	void *fuente = memoria_real + (numero_de_marco * configuracion_memoria->tam_pagina);

	pthread_mutex_lock(&mutex_memoria_real);
	memcpy(contenido_marco, fuente, configuracion_memoria->tam_pagina);
	pthread_mutex_unlock(&mutex_memoria_real);

	return contenido_marco;
}

int reemplazar_pagina(int pid, int numero_de_pagina)
{
	// Seleccionar una página víctima para reemplazo (FIFO o LRU)
	t_entrada_de_tabla_de_pagina *victima = encontrar_pagina_victima();

	// Verifica si la página víctima está modificada y la escribe en el swap si es necesario
	if (es_pagina_presente(victima) == 1 && es_pagina_modificada(victima) == 1)
	{
		escribir_pagina_en_swap(victima);
	}

	borrar_contenido_de_marco_en_memoria_real(victima->marco);

	// Actualizar entrada tabla de paginas de la victima
	pthread_mutex_lock(&mutex_entradas_tabla_de_paginas);
	victima->presencia = 0;
	victima->modificado = 0;
	pthread_mutex_unlock(&mutex_entradas_tabla_de_paginas);

	return victima->marco;
}

void borrar_contenido_de_marco_en_memoria_real(int numero_de_marco)
{
	void *fuente = memoria_real + (numero_de_marco * configuracion_memoria->tam_pagina);

	pthread_mutex_lock(&mutex_memoria_real);
	// Establecer todos los bytes del marco en cero
	memset(fuente, 0, configuracion_memoria->tam_pagina);
	pthread_mutex_unlock(&mutex_memoria_real);

	liberar_marco(numero_de_marco);
}

void cargar_datos_de_pagina_en_memoria_real(void *contenido_pagina, int numero_de_marco)
{
	// Cargar datos de la nueva pagina a reemplazar en memoria real
	void *fuente = memoria_real + (numero_de_marco * configuracion_memoria->tam_pagina);

	pthread_mutex_lock(&mutex_memoria_real);
	// Copiar el contenido de contenido_pagina en fuente
	memcpy(fuente, contenido_pagina, configuracion_memoria->tam_pagina);
	pthread_mutex_unlock(&mutex_memoria_real);
}

t_entrada_de_tabla_de_pagina *obtener_entrada_de_tabla_de_pagina_por_pid_y_numero(int pid, int numero_de_pagina)
{
	bool _filtro_pagina_de_memoria_por_numero_y_pid(t_entrada_de_tabla_de_pagina * pagina_de_memoria)
	{
		return pagina_de_memoria->pid == pid && pagina_de_memoria->numero_de_pagina == numero_de_pagina;
	};

	t_entrada_de_tabla_de_pagina *pagina = list_find_thread_safe(tabla_de_paginas, (void *)_filtro_pagina_de_memoria_por_numero_y_pid, &mutex_entradas_tabla_de_paginas);

	if (pagina == NULL)
	{
		log_error(logger, "No existe una pagina de memoria con el numero %d y el PID %d", numero_de_pagina, pid);
		// TODO ver que notificar aca, es Page Fault? Es un error?
		return NULL;
	}

	return pagina;
}

void enviar_numero_de_marco_a_cpu(int pid, int numero_de_pagina)
{
	t_entrada_de_tabla_de_pagina *pagina = obtener_entrada_de_tabla_de_pagina_por_pid_y_numero(pid, numero_de_pagina);
	int numero_marco = -1;

	if (pagina->presencia == 0)
	{
		log_warning(logger, "La pagina de memoria con el numero %d y el PID %d no se encuentra en memoria, bit de presencia = 0", numero_de_pagina, pid);
	}
	else
	{
		numero_marco = pagina->marco;
	}
	log_info(logger, "Acceso a tabla de paginas PID : <%d> - Pagina: <%d> - Marco: <%d>", pid, pagina->numero_de_pagina, pagina->marco);
	t_paquete *paquete = crear_paquete_respuesta_pedido_numero_de_marco(logger, numero_marco);
	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
}

// Marcos
int obtener_primer_marco_desocupado(int *indice_marco_desocupado)
{
	for (int i = 0; i < cantidad_de_frames; ++i)
	{
		if (!bitarray_test_bit_thread_safe(tabla_de_marcos, i, &mutex_tabla_de_marcos))
		{
			if (indice_marco_desocupado != NULL)
			{
				*indice_marco_desocupado = i;
			}
			return 1;
		}
	}
	return 0;
}

void ocupar_marco(int numero_de_marco)
{
	bitarray_set_bit_thread_safe(tabla_de_marcos, numero_de_marco, &mutex_tabla_de_marcos);
}

void liberar_marco(int numero_de_marco)
{
	bitarray_clean_bit_thread_safe(tabla_de_marcos, numero_de_marco, &mutex_tabla_de_marcos);
}

void destruir_listas()
{
	list_destroy(procesos_iniciados);
	list_destroy(tabla_de_paginas);
	bitarray_destroy(tabla_de_marcos);
	log_trace(logger, "Se destruyen todas las listas de manera correcta");
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
	destruir_listas();
}