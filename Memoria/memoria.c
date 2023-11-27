#include "memoria.h"

// Variables globales
t_log *logger = NULL;
t_argumentos_memoria *argumentos_memoria = NULL;
t_config_memoria *configuracion_memoria = NULL;
t_list *procesos_iniciados = NULL;
t_list *tabla_de_paginas = NULL;
t_list *tabla_de_marcos = NULL; // TODO implementar una lista propia de tamaño fijo? t_list no tiene tamaño fijo, es enlazada
void *memoria_real;

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
	tabla_de_paginas = list_create();
	tabla_de_marcos = list_create();

	// Creacion de Estructuras
	inicializar_espacio_contiguo_de_memoria();

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
			enviar_info_de_memoria_inicial_para_cpu();
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
			t_pedido_leer_archivo *pedido_leer_archivo = leer_paquete_pedido_leer_archivo(logger, conexion_con_filesystem);
			notificar_lectura_a_filesystem();
			free(pedido_leer_archivo);
		}
		else if (operacion_recibida_de_filesystem == SOLICITUD_ESCRIBIR_ARCHIVO_MEMORIA)
		{
			t_pedido_escribir_archivo *pedido_escribir_archivo = leer_paquete_pedido_escribir_archivo(logger, conexion_con_filesystem);
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

void inicializar_espacio_contiguo_de_memoria()
{
	memoria_real = malloc(configuracion_memoria->tam_memoria);
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

	int cantidad_de_bloques = size / configuracion_memoria->tam_pagina;
	if (size % configuracion_memoria->tam_pagina != 0)
	{
		cantidad_de_bloques++;
	}

	t_list *posiciones_swap = pedir_bloques_a_filesystem(cantidad_de_bloques);
	crear_entrada_de_tabla_de_paginas_de_proceso(cantidad_de_bloques, posiciones_swap, pid);
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
	usleep((configuracion_memoria->retardo_respuesta) * 1000);

	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	log_debug(logger, "Exito en el envio de paquete para instruccion %s al cpu!", linea_instruccion);
}

void finalizar_proceso_en_memoria(int pid)
{
	// aca hay algun error (falta alguna validacion o algo asi)
	log_info(logger, "El PID del proceso a finalizar es: %d", pid);

	t_archivo_proceso *archivo_proceso = buscar_archivo_con_pid(pid);
	if (archivo_proceso == NULL)
	{
		return;
	}
	cerrar_archivo_con_pid(pid);

	limpiar_entradas_tabla_de_paginas(pid);
	enviar_paquete_respuesta_finalizar_proceso_en_memoria_a_kernel();
	free(archivo_proceso);
}

t_archivo_proceso *buscar_archivo_con_pid(int pid)
{
	bool _filtro_archivo_proceso_por_id(t_archivo_proceso * archivo_proceso)
	{
		return archivo_proceso->pid == pid;
	};

	t_archivo_proceso *resultado = list_find(procesos_iniciados, (void *)_filtro_archivo_proceso_por_id);

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

	list_remove_and_destroy_by_condition(procesos_iniciados, (void *)_filtro_archivo_proceso_por_id, (void *)_finalizar_archivo_proceso);
}

void notificar_lectura_a_filesystem()
{
	log_debug(logger, "Notificando a File System de lectura exitosa en memoria");
	t_paquete *paquete = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_LEER_ARCHIVO_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

void notificar_page_fault_a_cpu()
{
	log_debug(logger, "Notificando Page Fault a CPU");
	t_paquete *paquete = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_PAGE_FAULT_A_CPU, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
}

void notificar_escritura_a_filesystem()
{
	log_debug(logger, "Notificando a File System de lectura exitosa en memoria");
	t_paquete *paquete = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_ESCRIBIR_ARCHIVO_MEMORIA, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
}

t_list *pedir_bloques_a_filesystem(int cantidad_de_bloques)
{
	t_paquete *paquete = crear_paquete_pedir_bloques_a_filesystem(logger, cantidad_de_bloques);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	// TODO bloquear hilo esperando el paquete de respuesta?
	return recibir_paquete_pedir_bloques_a_filesystem();
}

t_list *recibir_paquete_pedir_bloques_a_filesystem()
{
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem);
	t_list *posiciones_swap;
	if (codigo_operacion_recibido == RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM)
	{
		int tamanio_buffer;
		void *buffer = recibir_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, &tamanio_buffer, conexion_con_filesystem, RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM);
		void *buffer_con_offset = buffer;

		posiciones_swap = leer_lista_de_enteros_desde_buffer_de_paquete(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, buffer_con_offset, RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM);
	}

	return posiciones_swap;
}

void limpiar_entradas_tabla_de_paginas(int pid)
{
	t_list *entradas_tabla_de_paginas = obtener_entradas_de_tabla_de_pagina_por_pid(pid);
	for (int i = 0; i < list_size(entradas_tabla_de_paginas); i++)
	{
		t_entrada_de_tabla_de_pagina *entrada_tabla_de_paginas = list_get(entradas_tabla_de_paginas, i); // TODO ver si es puntero
		if (es_pagina_presente(entrada_tabla_de_paginas))
		{
			// TODO vaciar contenido del marco
		}
		pedir_liberacion_de_bloques_a_filesystem(entrada_tabla_de_paginas->posicion_en_swap);
		list_remove_element(tabla_de_paginas, entrada_tabla_de_paginas);
		free(entrada_tabla_de_paginas);
	}
	log_trace(logger, "Se limpian correctamente las entradas de tabla de paginas para el pid %d", pid);
}

void pedir_liberacion_de_bloques_a_filesystem(int posicion_de_swap)
{
	t_paquete *paquete = crear_paquete_liberar_bloque_en_filesystem(logger, posicion_de_swap);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
	// TODO esperar respuesta de liberacion de bloques?
}

// Función para inicializar la tabla de páginas por proceso
void crear_entrada_de_tabla_de_paginas_de_proceso(int cantidad_de_paginas, t_list *posiciones_swap, int pid)
{
	for (int i = 0; i < cantidad_de_paginas; i++)
	{
		t_entrada_de_tabla_de_pagina *entrada_tabla_de_paginas = malloc(sizeof(t_entrada_de_tabla_de_pagina));
		entrada_tabla_de_paginas->numero_de_pagina = i;
		entrada_tabla_de_paginas->presencia = 0;
		entrada_tabla_de_paginas->posicion_en_swap = (int)(*((int*)list_get(posiciones_swap, i)));
		list_add(tabla_de_paginas, entrada_tabla_de_paginas);
	}
	log_trace(logger, "Se genera correctamente las %d entradas de tabla de paginas para el pid %d", cantidad_de_paginas, pid);
}

void escribir_pagina_en_swap()
{
	// TODO IMPLEMENTAME PAA
}

int es_pagina_presente(t_entrada_de_tabla_de_pagina *pagina)
{
	if (pagina->presencia == 1)
	{
		return 1;
	}
	return 0;
}

int es_pagina_modificada(t_entrada_de_tabla_de_pagina *pagina)
{
	if (pagina->modificado == 1)
	{
		return 1;
	}
	return 0;
}

t_entrada_de_tabla_de_pagina *encontrar_pagina_victima_fifo()
{
	// TODO implementame PORFI
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

t_contenido_pagina *obtener_contenido_de_pagina_en_swap(int posicion_en_swap)
{
	// TODO ver que retorna esta funcion
	t_paquete *paquete = crear_paquete_solicitud_contenido_de_bloque(logger, posicion_en_swap);
	enviar_paquete(logger, conexion_con_filesystem, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);

	// TODO hace falta sincronizar
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem);
	t_contenido_pagina *contenido_del_bloque;
	if (codigo_operacion_recibido == RESPUESTA_CONTENIDO_BLOQUE_EN_FILESYSTEM)
	{
		contenido_del_bloque = leer_paquete_respuesta_contenido_bloque(logger, conexion_con_filesystem);
	}

	return contenido_del_bloque;
}

void cargar_pagina_en_memoria(int pid, int numero_de_pagina)
{
	t_entrada_de_tabla_de_pagina *pagina = obtener_entradas_de_tabla_de_pagina_por_pid_y_numero(pid, numero_de_pagina);
	if (es_pagina_presente(pagina))
	{
		// Retornar el contenido del marco en el que esta esa pagina
		t_contenido_pagina *contenido_pagina = buscar_contenido_marco(pagina->marco);
		t_paquete *paquete = crear_paquete_respuesta_cargar_pagina_en_memoria(logger, contenido_pagina);
		enviar_paquete(logger, conexion_con_kernel, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
		return;
	}
	
	t_contenido_pagina *contenido_en_swap = obtener_contenido_de_pagina_en_swap(pagina->posicion_en_swap);

	/*
	TODO Listas de marcos? Posible solucion
	if (marco_vacio)
	{
		cargar el marco con el contenido en swap y actualizar entrada tabla de pagina
		return;
	}
	*/

	reemplazar_pagina(pid, numero_de_pagina);
	cargar_datos_de_pagina_en_memoria_real(pagina);

	// Actualizar entrada tabla de paginas de la nueva pagina
	pagina->presencia = 1;
	// pagina->marco = ?;

	t_paquete *paquete = crear_paquete_respuesta_cargar_pagina_en_memoria(logger, NULL); // TODOO mandar contenido pagina
	enviar_paquete(logger, conexion_con_kernel, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_KERNEL);
}

t_contenido_pagina *buscar_contenido_marco(int numero_de_marco)
{
	/* TODO implementar
	Va a hacer un memcpy(ver q params se le pasa) desde el numero de marco por el tamaño de pagina y eso me va a traer la primer
	posicion del marco
	*/
	return NULL;
}

bool existe_un_marco_vacio()
{
	//
}

void reemplazar_pagina(int pid, int numero_de_pagina)
{
	// Seleccionar una página víctima para reemplazo (FIFO o LRU)
	t_entrada_de_tabla_de_pagina *victima = encontrar_pagina_victima();

	// Verifica si la página víctima está modificada y la escribe en el swap si es necesario
	if (es_pagina_presente(victima) == 1 && es_pagina_modificada(victima) == 1)
	{
		escribir_pagina_en_swap(pid, victima);
	}

	// TODO numero de marco
	borrar_contenido_de_marco_en_memoria_real(victima->marco);

	// Actualizar entrada tabla de paginas de la victima
	victima->presencia = 0;
}

void borrar_contenido_de_marco_en_memoria_real(int numero_de_marco)
{
	// TODO todavia no sabemos lo que devuelve, es lo que ocupa un marco de bytes
	// obtener_contenido_de_marco_en_memoria_real
	// TODO implementar el borrado de datos de la victima en memoria real
}

void cargar_datos_de_pagina_en_memoria_real(t_entrada_de_tabla_de_pagina *pagina)
{
	// TODO implementar
	// Cargar datos de la nueva pagina a reemplazar en memoria real
}

t_entrada_de_tabla_de_pagina *obtener_entradas_de_tabla_de_pagina_por_pid_y_numero(int pid, int numero_de_pagina)
{
	bool _filtro_pagina_de_memoria_por_numero_y_pid(t_entrada_de_tabla_de_pagina * pagina_de_memoria)
	{
		return pagina_de_memoria->pid == pid && pagina_de_memoria->numero_de_pagina == numero_de_pagina;
	};

	t_entrada_de_tabla_de_pagina *pagina = list_find(tabla_de_paginas, (void *)_filtro_pagina_de_memoria_por_numero_y_pid);

	if (pagina == NULL)
	{
		log_error(logger, "No existe una pagina de memoria con el numero %d y el PID %d", numero_de_pagina, pid);
		// TODO ver que notificar aca, es Page Fault? Es un error de pagina INVALIDA?
		return;
	}

	return pagina;
}

void enviar_numero_de_marco_a_cpu(int pid, int numero_de_pagina)
{
	t_entrada_de_tabla_de_pagina *pagina = obtener_entradas_de_tabla_de_pagina_por_pid_y_numero(pid, numero_de_pagina);

	if (pagina->presencia == 0)
	{
		log_warning(logger, "La pagina de memoria con el numero %d y el PID %d no se encuentra en memoria, bit de presencia = 0", numero_de_pagina, pid);
		notificar_page_fault_a_cpu();
		return;
	}

	t_paquete *paquete = crear_paquete_respuesta_pedido_numero_de_marco(logger, pagina->marco);
	enviar_paquete(logger, conexion_con_cpu, paquete, NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_CPU);
}

void destruir_listas()
{
	list_destroy(procesos_iniciados);
	list_destroy(tabla_de_paginas);
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