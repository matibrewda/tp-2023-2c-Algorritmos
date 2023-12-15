#include "cpu.h"

// Variables globales
t_log *logger = NULL;
t_argumentos_cpu *argumentos_cpu = NULL;
t_config_cpu *configuracion_cpu = NULL;
bool dejar_de_ejecutar = false;
bool ocurrio_interrupcion = false;
int pid_ejecutando = -1;
int tamanio_pagina = -1;
int tamanio_memoria = -1;

// Conexiones
int socket_kernel_dispatch = -1;
int socket_kernel_interrupt = -1;
int conexion_con_kernel_dispatch = -1;
int conexion_con_kernel_interrupt = -1;
int conexion_con_memoria = -1;
int motivo_interrupcion = -1;

// Hilos
pthread_t hilo_interrupt;
pthread_t hilo_dispatch;

// Semaforos
sem_t semaforo_ejecutar_ciclo_de_instruccion;
pthread_mutex_t mutex_ocurrio_interrupcion;

// Registros
uint32_t program_counter = -1;
uint32_t registro_ax = -1;
uint32_t registro_bx = -1;
uint32_t registro_cx = -1;
uint32_t registro_dx = -1;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	atexit(terminar_cpu);

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_CPU, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_cpu();
		return EXIT_FAILURE;
	}

	argumentos_cpu = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_cpu == NULL)
	{
		terminar_cpu();
		return EXIT_FAILURE;
	}

	configuracion_cpu = leer_configuracion(logger, argumentos_cpu->ruta_archivo_configuracion);
	if (configuracion_cpu == NULL)
	{
		terminar_cpu();
		return EXIT_FAILURE;
	}

	conexion_con_memoria = crear_socket_cliente(logger, configuracion_cpu->ip_memoria, configuracion_cpu->puerto_memoria, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
	if (conexion_con_memoria == -1)
	{
		terminar_cpu();
		return EXIT_FAILURE;
	}

	socket_kernel_dispatch = crear_socket_servidor(logger, configuracion_cpu->puerto_escucha_dispatch, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
	if (socket_kernel_dispatch == -1)
	{
		terminar_cpu();
		return EXIT_FAILURE;
	}

	socket_kernel_interrupt = crear_socket_servidor(logger, configuracion_cpu->puerto_escucha_interrupt, NOMBRE_MODULO_CPU_INTERRUPT, NOMBRE_MODULO_KERNEL);
	if (socket_kernel_interrupt == -1)
	{
		terminar_cpu();
		return EXIT_FAILURE;
	}

	conexion_con_kernel_dispatch = esperar_conexion_de_cliente(logger, socket_kernel_dispatch, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
	if (conexion_con_kernel_dispatch == -1)
	{
		terminar_cpu();
		return EXIT_FAILURE;
	}

	conexion_con_kernel_interrupt = esperar_conexion_de_cliente(logger, socket_kernel_interrupt, NOMBRE_MODULO_CPU_INTERRUPT, NOMBRE_MODULO_KERNEL);
	if (conexion_con_kernel_interrupt == -1)
	{
		terminar_cpu();
		return EXIT_FAILURE;
	}

	pedir_info_inicial_a_memoria();

	// Semaforos
	sem_init(&semaforo_ejecutar_ciclo_de_instruccion, false, 0); // Inicialmente la CPU NO ejecuta (IDLE)
	pthread_mutex_init(&mutex_ocurrio_interrupcion, NULL);

	// Hilos
	pthread_create(&hilo_interrupt, NULL, interrupt, NULL);
	pthread_create(&hilo_dispatch, NULL, dispatch, NULL);

	// Hilo principal
	ciclo_de_ejecucion();

	// Finalizacion
	terminar_cpu();
	return EXIT_SUCCESS;
}

void terminar_cpu()
{
	if (logger != NULL)
	{
		log_warning(logger, "Algo salio mal!");
		log_warning(logger, "Finalizando %s", NOMBRE_MODULO_CPU);
	}

	if (socket_kernel_dispatch != -1)
	{
		close(socket_kernel_dispatch);
	}

	if (conexion_con_kernel_dispatch != -1)
	{
		close(conexion_con_kernel_dispatch);
	}

	if (socket_kernel_interrupt != -1)
	{
		close(socket_kernel_interrupt);
	}

	if (conexion_con_kernel_interrupt != -1)
	{
		close(conexion_con_kernel_interrupt);
	}

	if (conexion_con_memoria != -1)
	{
		close(conexion_con_memoria);
	}
}

void *interrupt()
{
	while (true)
	{
		// Bloqueado hasta que reciba una operacion desde la conexion interrupt.
		op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU_INTERRUPT, NOMBRE_MODULO_KERNEL, conexion_con_kernel_interrupt);
		if (codigo_operacion_recibido == SOLICITUD_INTERRUMPIR_PROCESO)
		{
			log_debug(logger, "RECIBI INTERRUPT");
			motivo_interrupcion = leer_paquete_solicitud_interrumpir_proceso(logger, conexion_con_kernel_interrupt);

			pthread_mutex_lock(&mutex_ocurrio_interrupcion);
			ocurrio_interrupcion = true;
			pthread_mutex_unlock(&mutex_ocurrio_interrupcion);
		}
		else
		{
			log_trace(logger, "Se recibio una orden desconocida de %s en %s.", NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);
		}
	}
}

void *dispatch()
{
	while (true)
	{
		// Bloqueado hasta que reciba una operacion desde la conexion dispatch.
		op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, conexion_con_kernel_dispatch);

		if (codigo_operacion_recibido == SOLICITUD_EJECUTAR_PROCESO)
		{
			log_trace(logger, "Se recibio una orden de %s para ejecutar un proceso en %s.", NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
			t_contexto_de_ejecucion *contexto_de_ejecucion = leer_paquete_solicitud_ejecutar_proceso(logger, conexion_con_kernel_dispatch);

			// Restauro el contexto de ejecucion
			pid_ejecutando = contexto_de_ejecucion->pid;
			program_counter = contexto_de_ejecucion->program_counter;
			registro_ax = contexto_de_ejecucion->registro_ax;
			registro_bx = contexto_de_ejecucion->registro_bx;
			registro_cx = contexto_de_ejecucion->registro_cx;
			registro_dx = contexto_de_ejecucion->registro_dx;

			free(contexto_de_ejecucion);

			// Aviso que hay que ejecutar el ciclo de instruccion
			sem_post(&semaforo_ejecutar_ciclo_de_instruccion);
		}
		else
		{
			log_trace(logger, "Se recibio una orden desconocida de %s en %s.", NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
		}
	}
}

void devolver_contexto_por_ser_interrumpido()
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_paquete *paquete_devuelvo_proceso_por_ser_interrumpido = crear_paquete_solicitud_devolver_proceso_por_ser_interrumpido(logger, contexto_de_ejecucion, motivo_interrupcion);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_ser_interrumpido, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

void devolver_contexto_por_correcta_finalizacion()
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_paquete *paquete_devuelvo_proceso_por_correcta_finalizacion = crear_paquete_solicitud_devolver_proceso_por_correcta_finalizacion(logger, contexto_de_ejecucion);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_correcta_finalizacion, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

void devolver_contexto_por_sleep(int segundos_sleep)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_paquete *paquete_devuelvo_proceso_por_sleep = crear_paquete_solicitud_devolver_proceso_por_sleep(logger, contexto_de_ejecucion, segundos_sleep);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_sleep, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

void devolver_contexto_por_wait(char *nombre_recurso)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_paquete *paquete_devuelvo_proceso_por_wait = crear_paquete_solicitud_devolver_proceso_por_wait(logger, contexto_de_ejecucion, nombre_recurso);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_wait, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

void devolver_contexto_por_signal(char *nombre_recurso)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_paquete *paquete_devuelvo_proceso_por_signal = crear_paquete_solicitud_devolver_proceso_por_signal(logger, contexto_de_ejecucion, nombre_recurso);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_signal, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

void devolver_contexto_por_error(int codigo_error)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_paquete *paquete_devuelvo_proceso_por_error = crear_paquete_solicitud_devolver_proceso_por_error(logger, contexto_de_ejecucion, codigo_error);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_error, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

void devolver_contexto_por_page_fault(int numero_de_pagina)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_paquete *paquete_devuelvo_proceso_por_pagefault = crear_paquete_solicitud_devolver_proceso_por_pagefault(logger, contexto_de_ejecucion, numero_de_pagina);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_pagefault, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

void devolver_contexto_por_operacion_filesystem(fs_op_code fs_opcode, char *nombre_archivo, int modo_apertura, int posicion, int direccion_fisica, int tamanio)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_operacion_filesystem *operacion_filesystem = crear_objeto_operacion_filesystem(fs_opcode, nombre_archivo, modo_apertura, posicion, direccion_fisica, tamanio);
	t_paquete *paquete_devuelvo_proceso_por_signal = crear_paquete_solicitud_devolver_proceso_por_operacion_filesystem(logger, contexto_de_ejecucion, operacion_filesystem);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_signal, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
	free(operacion_filesystem);
}

void pedir_info_inicial_a_memoria()
{
	// Enviar
	t_paquete *paquete_solicitar_info_de_memoria_inicial_para_cpu = crear_paquete_solicitud_pedir_info_de_memoria_inicial_para_cpu(logger);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitar_info_de_memoria_inicial_para_cpu, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
	t_info_memoria *info_memoria = leer_paquete_respuesta_pedir_info_de_memoria_inicial_para_cpu(logger, conexion_con_memoria);
	tamanio_pagina = info_memoria->tamanio_pagina;
	tamanio_memoria = info_memoria->tamanio_memoria;
	free(info_memoria);
}

int pedir_numero_de_marco_a_memoria(int numero_de_pagina)
{
	// Enviar
	t_pedido_pagina_en_memoria *pedido_numero_de_marco = malloc(sizeof(t_pedido_pagina_en_memoria));
	pedido_numero_de_marco->numero_de_pagina = numero_de_pagina;
	pedido_numero_de_marco->pid = pid_ejecutando;

	t_paquete *paquete_solicitud_pedido_numero_de_marco = crear_paquete_solicitud_pedido_numero_de_marco(logger, pedido_numero_de_marco);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_pedido_numero_de_marco, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	free(pedido_numero_de_marco);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
	int numero_de_marco = leer_paquete_respuesta_pedir_numero_de_marco_a_memoria(logger, conexion_con_memoria);

	return numero_de_marco;
}

void escribir_valor_en_memoria(int pid, int direccion_fisica, u_int32_t valor_a_escribir)
{
	// Enviar
	t_pedido_escribir_valor_en_memoria *pedido_escribir_valor_en_memoria = malloc(sizeof(t_pedido_escribir_valor_en_memoria));
	pedido_escribir_valor_en_memoria->pid = pid;
	pedido_escribir_valor_en_memoria->valor_a_escribir = valor_a_escribir;
	pedido_escribir_valor_en_memoria->direccion_fisica = direccion_fisica;

	t_paquete *paquete_solicitud_escribir_valor_en_memoria = crear_paquete_solicitud_escribir_valor_en_memoria(logger, pedido_escribir_valor_en_memoria, NOMBRE_MODULO_CPU);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_escribir_valor_en_memoria, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, conexion_con_memoria); // RESPUESTA_ESCRIBIR_VALOR_EN_MEMORIA
}

u_int32_t leer_valor_de_memoria(int pid, int direccion_fisica)
{
	// Enviar
	t_pedido_leer_valor_de_memoria *pedido_leer_valor_de_memoria = malloc(sizeof(t_pedido_leer_valor_de_memoria));
	pedido_leer_valor_de_memoria->pid = pid;
	pedido_leer_valor_de_memoria->direccion_fisica = direccion_fisica;

	t_paquete *paquete_solicitud_leer_valor_en_memoria = crear_paquete_solicitud_leer_valor_en_memoria(logger, pedido_leer_valor_de_memoria);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_leer_valor_en_memoria, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);

	free(pedido_leer_valor_de_memoria);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
	u_int32_t valor = leer_paquete_respuesta_leer_valor_en_memoria(logger, conexion_con_memoria);

	return valor;
}

void ciclo_de_ejecucion()
{
	while (true)
	{
		// Espero a que haya algo para ejecutar
		sem_wait(&semaforo_ejecutar_ciclo_de_instruccion);

		dejar_de_ejecutar = false;
		pthread_mutex_lock(&mutex_ocurrio_interrupcion);
		ocurrio_interrupcion = false;
		pthread_mutex_unlock(&mutex_ocurrio_interrupcion);

		// FETCH
		log_info(logger, "Fetch Instruccion: PID: %d - FETCH - Program Counter: %d", pid_ejecutando, program_counter);

		t_pedido_instruccion *pedido_instruccion = malloc(sizeof(t_pedido_instruccion));
		pedido_instruccion->pc = program_counter + 1;
		pedido_instruccion->pid = pid_ejecutando;
		t_paquete *paquete_solicitud_pedir_instruccion_a_memoria = crear_paquete_solicitud_pedir_instruccion_a_memoria(logger, pedido_instruccion);
		enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_pedir_instruccion_a_memoria, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
		free(pedido_instruccion);
		op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
		char *instruccion_con_parametros = leer_paquete_respuesta_pedir_instruccion_a_memoria(logger, conexion_con_memoria);

		// DECODE & EXECUTE
		char *saveptr = instruccion_con_parametros;
		char *nombre_instruccion = strtok_r(saveptr, " ", &saveptr);

		if (strcmp(nombre_instruccion, SET_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_registro = strtok_r(saveptr, " ", &saveptr);
			int valor = atoi(strtok_r(saveptr, " ", &saveptr));
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %d", pid_ejecutando, SET_NOMBRE_INSTRUCCION, nombre_registro, valor);
			escribir_valor_a_registro(nombre_registro, valor);
			program_counter++;
		}
		else if (strcmp(nombre_instruccion, SUM_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_registro_destino = strtok_r(saveptr, " ", &saveptr);
			char *nombre_registro_origen = strtok_r(saveptr, " ", &saveptr);
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, SUM_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);
			escribir_valor_a_registro(nombre_registro_destino, leer_valor_de_registro(nombre_registro_destino) + leer_valor_de_registro(nombre_registro_origen));
			program_counter++;
		}
		else if (strcmp(nombre_instruccion, SUB_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_registro_destino = strtok_r(saveptr, " ", &saveptr);
			char *nombre_registro_origen = strtok_r(saveptr, " ", &saveptr);
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, SUB_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);
			escribir_valor_a_registro(nombre_registro_destino, leer_valor_de_registro(nombre_registro_destino) - leer_valor_de_registro(nombre_registro_origen));
			program_counter++;
		}
		else if (strcmp(nombre_instruccion, JNZ_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_registro = strtok_r(saveptr, " ", &saveptr);
			u_int32_t nuevo_program_counter = atoi(strtok_r(saveptr, " ", &saveptr));
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %d", pid_ejecutando, JNZ_NOMBRE_INSTRUCCION, nombre_registro, nuevo_program_counter);
			if (leer_valor_de_registro(nombre_registro) != 0)
			{
				escribir_valor_a_registro(PC_NOMBRE_REGISTRO, nuevo_program_counter);
			}
			else
			{
				program_counter++;
			}
		}
		else if (strcmp(nombre_instruccion, SLEEP_NOMBRE_INSTRUCCION) == 0)
		{
			int tiempo_sleep = atoi(strtok_r(saveptr, " ", &saveptr));
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %d", pid_ejecutando, SLEEP_NOMBRE_INSTRUCCION, tiempo_sleep);
			program_counter++;
			dejar_de_ejecutar = true;
			devolver_contexto_por_sleep(tiempo_sleep);
		}
		else if (strcmp(nombre_instruccion, WAIT_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_recurso = strtok_r(saveptr, " ", &saveptr);
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s", pid_ejecutando, WAIT_NOMBRE_INSTRUCCION, nombre_recurso);
			program_counter++;
			dejar_de_ejecutar = true;
			devolver_contexto_por_wait(nombre_recurso);
		}
		else if (strcmp(nombre_instruccion, SIGNAL_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_recurso = strtok_r(saveptr, " ", &saveptr);
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s", pid_ejecutando, SIGNAL_NOMBRE_INSTRUCCION, nombre_recurso);
			program_counter++;
			dejar_de_ejecutar = true;
			devolver_contexto_por_signal(nombre_recurso);
		}
		else if (strcmp(nombre_instruccion, MOV_IN_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_registro = strtok_r(saveptr, " ", &saveptr);
			int direccion_logica = atoi(strtok_r(saveptr, " ", &saveptr));
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %d", pid_ejecutando, MOV_IN_NOMBRE_INSTRUCCION, nombre_registro, direccion_logica);
			int direccion_fisica = mmu(direccion_logica);
			if (direccion_fisica != -1)
			{
				u_int32_t valor_leido_en_memoria = leer_valor_de_memoria(pid_ejecutando, direccion_fisica);
				escribir_valor_a_registro(nombre_registro, valor_leido_en_memoria);
				log_info(logger, "Lectura Memoria: PID: %d - Accion: LEER - Direccion Fisica: %d - Valor: %d", pid_ejecutando, direccion_fisica, valor_leido_en_memoria);
				program_counter++;
			}
		}
		else if (strcmp(nombre_instruccion, MOV_OUT_NOMBRE_INSTRUCCION) == 0)
		{
			int direccion_logica = atoi(strtok_r(saveptr, " ", &saveptr));
			char *nombre_registro = strtok_r(saveptr, " ", &saveptr);
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %d - %s", pid_ejecutando, MOV_OUT_NOMBRE_INSTRUCCION, direccion_logica, nombre_registro);
			int direccion_fisica = mmu(direccion_logica);
			if (direccion_fisica != -1)
			{
				u_int32_t valor_a_escribir_en_memoria = leer_valor_de_registro(nombre_registro);
				escribir_valor_en_memoria(pid_ejecutando, direccion_fisica, valor_a_escribir_en_memoria);
				log_info(logger, "Escritura Memoria: PID: %d - Accion: ESCRIBIR - Direccion Fisica: %d - Valor: %d", pid_ejecutando, direccion_fisica, valor_a_escribir_en_memoria);
				program_counter++;
			}
		}
		else if (strcmp(nombre_instruccion, FOPEN_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_archivo = strtok_r(saveptr, " ", &saveptr);
			char *modo_apertura_str = strtok_r(saveptr, " ", &saveptr);
			int modo_apertura;
			if (strcmp("W", modo_apertura_str) == 0)
			{
				modo_apertura = LOCK_ESCRITURA;
			}
			else
			{
				modo_apertura = LOCK_LECTURA;
			}

			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, FOPEN_NOMBRE_INSTRUCCION, nombre_archivo, modo_apertura_str);
			program_counter++;
			dejar_de_ejecutar = true;
			devolver_contexto_por_operacion_filesystem(FOPEN_OPCODE, nombre_archivo, modo_apertura, -1, -1, -1);
		}
		else if (strcmp(nombre_instruccion, FCLOSE_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_archivo = strtok_r(saveptr, " ", &saveptr);
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s", pid_ejecutando, FCLOSE_NOMBRE_INSTRUCCION, nombre_archivo);
			program_counter++;
			dejar_de_ejecutar = true;
			devolver_contexto_por_operacion_filesystem(FCLOSE_OPCODE, nombre_archivo, -1, -1, -1, -1);
		}
		else if (strcmp(nombre_instruccion, FSEEK_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_archivo = strtok_r(saveptr, " ", &saveptr);
			int posicion = atoi(strtok_r(saveptr, " ", &saveptr));
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %d", pid_ejecutando, FSEEK_NOMBRE_INSTRUCCION, nombre_archivo, posicion);
			program_counter++;
			dejar_de_ejecutar = true;
			devolver_contexto_por_operacion_filesystem(FSEEK_OPCODE, nombre_archivo, -1, posicion, -1, -1);
		}
		else if (strcmp(nombre_instruccion, FREAD_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_archivo = strtok_r(saveptr, " ", &saveptr);
			int direccion_logica = atoi(strtok_r(saveptr, " ", &saveptr));
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %d", pid_ejecutando, FREAD_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);
			int direccion_fisica = mmu(direccion_logica);
			if (direccion_fisica != -1)
			{
				program_counter++;
				dejar_de_ejecutar = true;
				devolver_contexto_por_operacion_filesystem(FREAD_OPCODE, nombre_archivo, -1, -1, direccion_fisica, -1);
			}
		}
		else if (strcmp(nombre_instruccion, FWRITE_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_archivo = strtok_r(saveptr, " ", &saveptr);
			int direccion_logica = atoi(strtok_r(saveptr, " ", &saveptr));
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %d", pid_ejecutando, FWRITE_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);
			int direccion_fisica = mmu(direccion_logica);
			if (direccion_fisica != -1)
			{
				program_counter++;
				dejar_de_ejecutar = true;
				devolver_contexto_por_operacion_filesystem(FWRITE_OPCODE, nombre_archivo, -1, -1, direccion_fisica, -1);
			}
		}
		else if (strcmp(nombre_instruccion, FTRUNCATE_NOMBRE_INSTRUCCION) == 0)
		{
			char *nombre_archivo = strtok_r(saveptr, " ", &saveptr);
			int tamanio = atoi(strtok_r(saveptr, " ", &saveptr));
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s - %s - %d", pid_ejecutando, FTRUNCATE_NOMBRE_INSTRUCCION, nombre_archivo, tamanio);
			program_counter++;
			dejar_de_ejecutar = true;
			devolver_contexto_por_operacion_filesystem(FTRUNCATE_OPCODE, nombre_archivo, -1, -1, -1, tamanio);
		}
		else if (strcmp(nombre_instruccion, EXIT_NOMBRE_INSTRUCCION) == 0)
		{
			log_info(logger, "Instruccion Ejecutada: PID: %d - Ejecutando: %s", pid_ejecutando, EXIT_NOMBRE_INSTRUCCION);
			dejar_de_ejecutar = true;
			devolver_contexto_por_correcta_finalizacion();
		}

		free(instruccion_con_parametros);

		// CHECK INTERRUPT
		pthread_mutex_lock(&mutex_ocurrio_interrupcion);
		if (ocurrio_interrupcion && !dejar_de_ejecutar)
		{
			pthread_mutex_unlock(&mutex_ocurrio_interrupcion);
			log_debug(logger, "DEVUELVO INTERRUPT");
			devolver_contexto_por_ser_interrumpido();
			dejar_de_ejecutar = true;
		}
		else
		{
			pthread_mutex_unlock(&mutex_ocurrio_interrupcion);
		}

		if (!dejar_de_ejecutar)
		{

			// Sigo ejecutando
			sem_post(&semaforo_ejecutar_ciclo_de_instruccion);
		}
		else
		{
			log_debug(logger, "FRENO");
		}
	}
}

int mmu(int direccion_logica)
{
	int numero_de_pagina = floor(direccion_logica / tamanio_pagina);
	int numero_de_marco = pedir_numero_de_marco_a_memoria(numero_de_pagina);
	bool page_fault = numero_de_marco == -1;

	if (page_fault)
	{
		log_info(logger, "Page Fault: Page Fault PID: %d - Pagina: %d", pid_ejecutando, numero_de_pagina);
		dejar_de_ejecutar = true;
		devolver_contexto_por_page_fault(numero_de_pagina);
		return -1;
	}

	log_info(logger, "Obtener Marco: PID: %d - OBTENER MARCO - Pagina: %d - Marco: %d", pid_ejecutando, numero_de_pagina, numero_de_marco);
	int desplazamiento = direccion_logica - numero_de_pagina * tamanio_pagina;
	int direccion_fisica = numero_de_marco * tamanio_pagina + desplazamiento;

	return direccion_fisica;
}

t_contexto_de_ejecucion *crear_objeto_contexto_de_ejecucion()
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = malloc(sizeof(t_contexto_de_ejecucion));

	contexto_de_ejecucion->pid = pid_ejecutando;
	contexto_de_ejecucion->program_counter = program_counter;
	contexto_de_ejecucion->registro_ax = registro_ax;
	contexto_de_ejecucion->registro_bx = registro_bx;
	contexto_de_ejecucion->registro_cx = registro_cx;
	contexto_de_ejecucion->registro_dx = registro_dx;

	return contexto_de_ejecucion;
}

t_operacion_filesystem *crear_objeto_operacion_filesystem(fs_op_code fs_opcode, char *nombre_archivo, int modo_apertura, int posicion, int direccion_fisica, int tamanio)
{
	t_operacion_filesystem *operacion_filesystem = malloc(sizeof(t_operacion_filesystem));

	operacion_filesystem->fs_opcode = fs_opcode;
	operacion_filesystem->nombre_archivo = nombre_archivo;
	operacion_filesystem->modo_apertura = modo_apertura;
	operacion_filesystem->posicion = posicion;
	operacion_filesystem->direccion_fisica = direccion_fisica;
	operacion_filesystem->tamanio = tamanio;

	return operacion_filesystem;
}

uint32_t leer_valor_de_registro(char *nombre_registro)
{
	if (strcmp(nombre_registro, AX_NOMBRE_REGISTRO) == 0)
	{
		log_trace(logger, "Se leyo desde el registro '%s' el valor %d.", nombre_registro, registro_ax);
		return registro_ax;
	}
	else if (strcmp(nombre_registro, BX_NOMBRE_REGISTRO) == 0)
	{
		log_trace(logger, "Se leyo desde el registro '%s' el valor %d.", nombre_registro, registro_bx);
		return registro_bx;
	}
	else if (strcmp(nombre_registro, CX_NOMBRE_REGISTRO) == 0)
	{
		log_trace(logger, "Se leyo desde el registro '%s' el valor %d.", nombre_registro, registro_cx);
		return registro_cx;
	}
	else if (strcmp(nombre_registro, DX_NOMBRE_REGISTRO) == 0)
	{
		log_trace(logger, "Se leyo desde el registro '%s' el valor %d.", nombre_registro, registro_dx);
		return registro_dx;
	}
	else if (strcmp(nombre_registro, PC_NOMBRE_REGISTRO) == 0)
	{
		log_trace(logger, "Se leyo desde el registro '%s' el valor %d.", nombre_registro, registro_dx);
		return program_counter;
	}

	log_error(logger, "No se leyo nada del registro '%s' porque es un registro no conocido.", nombre_registro);
	return -1;
}

void escribir_valor_a_registro(char *nombre_registro, uint32_t valor)
{
	if (strcmp(nombre_registro, AX_NOMBRE_REGISTRO) == 0)
	{
		registro_ax = valor;
		log_debug(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, BX_NOMBRE_REGISTRO) == 0)
	{
		registro_bx = valor;
		log_debug(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, CX_NOMBRE_REGISTRO) == 0)
	{
		registro_cx = valor;
		log_debug(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, DX_NOMBRE_REGISTRO) == 0)
	{
		registro_dx = valor;
		log_debug(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, PC_NOMBRE_REGISTRO) == 0)
	{
		program_counter = valor;
		log_debug(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else
	{
		log_error(logger, "No se asigno el valor %d al registro '%s' porque es un registro no conocido.", valor, nombre_registro);
	}
}