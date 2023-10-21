#include "cpu.h"

// Variables globales
t_log *logger = NULL;
t_argumentos_cpu *argumentos_cpu = NULL;
t_config_cpu *configuracion_cpu = NULL;
bool ocurrio_interrupcion = false;
bool se_ejecuto_instruccion_bloqueante = false;
bool se_leyo_instruccion_exit = false;
int pid_ejecutando = 0;
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
sem_t semaforo_espero_ejecutar_proceso;
sem_t semaforo_devuelvo_proceso;

// Registros
uint32_t program_counter = 0;
uint32_t registro_ax = 0;
uint32_t registro_bx = 0;
uint32_t registro_cx = 0;
uint32_t registro_dx = 0;

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

	solicitar_info_inicial_a_memoria();

	// Semaforos
	sem_init(&semaforo_ejecutar_ciclo_de_instruccion, false, 0); // Inicialmente la CPU NO ejecuta (IDLE)
	sem_init(&semaforo_devuelvo_proceso, false, 0);
	sem_init(&semaforo_espero_ejecutar_proceso, false, 1);

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

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* KERNEL *////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
void *interrupt()
{
	while (true)
	{
		// Bloqueado hasta que reciba una operacion desde la conexion interrupt.
		op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU_INTERRUPT, NOMBRE_MODULO_KERNEL, conexion_con_kernel_interrupt);

		if (codigo_operacion_recibido == SOLICITUD_INTERRUMPIR_PROCESO)
		{
			log_trace(logger, "Se recibio una orden de %s para interrumpir el proceso en %s.", NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);
			motivo_interrupcion = leer_paquete_solicitud_interrumpir_proceso(logger, conexion_con_kernel_interrupt);
			ocurrio_interrupcion = true;
			enviar_paquete_respuesta_interrumpir_ejecucion();
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
		sem_wait(&semaforo_espero_ejecutar_proceso);

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

			ocurrio_interrupcion = false;
			se_ejecuto_instruccion_bloqueante = false;
			se_leyo_instruccion_exit = false;

			// Aviso que hay que ejecutar el ciclo de instruccion
			sem_post(&semaforo_ejecutar_ciclo_de_instruccion);

			enviar_paquete_respuesta_ejecutar_proceso();
		}
		else
		{
			log_trace(logger, "Se recibio una orden desconocida de %s en %s.", NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
		}

		sem_post(&semaforo_devuelvo_proceso);
	}
}

void enviar_paquete_respuesta_ejecutar_proceso()
{
	t_paquete *paquete_respuesta_ejecutar_proceso = crear_paquete_respuesta_ejecutar_proceso(logger);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_respuesta_ejecutar_proceso, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
}

void enviar_paquete_respuesta_interrumpir_ejecucion()
{
	t_paquete *paquete_respuesta_interrumpir_proceso = crear_paquete_respuesta_interrumpir_proceso(logger);
	enviar_paquete(logger, conexion_con_kernel_interrupt, paquete_respuesta_interrumpir_proceso, NOMBRE_MODULO_CPU_INTERRUPT, NOMBRE_MODULO_KERNEL);
}

void enviar_paquete_solicitud_devolver_proceso_por_ser_interrumpido()
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_paquete *paquete_devuelvo_proceso_por_ser_interrumpido = crear_paquete_solicitud_devolver_proceso_por_ser_interrumpido(logger, contexto_de_ejecucion, motivo_interrupcion);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_ser_interrumpido, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

void enviar_paquete_solicitud_devolver_proceso_por_correcta_finalizacion()
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion();
	t_paquete *paquete_devuelvo_proceso_por_correcta_finalizacion = crear_paquete_solicitud_devolver_proceso_por_correcta_finalizacion(logger, contexto_de_ejecucion);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_correcta_finalizacion, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

void enviar_paquete_solicitud_devolver_proceso_por_sleep(int segundos_sleep)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = crear_objeto_contexto_de_ejecucion(motivo_interrupcion, segundos_sleep);
	t_paquete *paquete_devuelvo_proceso_por_sleep = crear_paquete_solicitud_devolver_proceso_por_sleep(logger, contexto_de_ejecucion, segundos_sleep);
	enviar_paquete(logger, conexion_con_kernel_dispatch, paquete_devuelvo_proceso_por_sleep, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	free(contexto_de_ejecucion);
}

bool recibir_operacion_de_kernel_dispatch(op_code codigo_operacion_esperado)
{
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, conexion_con_kernel_dispatch);
	return codigo_operacion_recibido == codigo_operacion_esperado;
}

void devolver_contexto_por_ser_interrumpido()
{
	sem_wait(&semaforo_devuelvo_proceso);
	enviar_paquete_solicitud_devolver_proceso_por_ser_interrumpido();
	recibir_operacion_de_kernel_dispatch(RESPUESTA_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO);
	sem_post(&semaforo_espero_ejecutar_proceso);
}

void devolver_contexto_por_correcta_finalizacion()
{
	sem_wait(&semaforo_devuelvo_proceso);
	enviar_paquete_solicitud_devolver_proceso_por_correcta_finalizacion();
	recibir_operacion_de_kernel_dispatch(RESPUESTA_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION);
	sem_post(&semaforo_espero_ejecutar_proceso);
}

void devolver_contexto_por_sleep(int segundos_sleep)
{
	sem_wait(&semaforo_devuelvo_proceso);
	enviar_paquete_solicitud_devolver_proceso_por_sleep(segundos_sleep);
	recibir_operacion_de_kernel_dispatch(RESPUESTA_DEVOLVER_PROCESO_POR_SLEEP);
	sem_post(&semaforo_espero_ejecutar_proceso);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* MEMORIA *///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
void enviar_paquete_solicitud_pedir_info_de_memoria_inicial()
{
	t_paquete *paquete_solicitar_info_de_memoria_inicial_para_cpu = crear_paquete_solicitud_pedir_info_de_memoria_inicial_para_cpu(logger);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitar_info_de_memoria_inicial_para_cpu, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
}

void enviar_paquete_solicitud_pedir_instruccion_a_memoria()
{
	t_pedido_instruccion *pedido_instruccion = malloc(sizeof(t_pedido_instruccion));
	pedido_instruccion->pc = program_counter;
	pedido_instruccion->pid = pid_ejecutando;
	t_paquete *paquete_solicitud_pedir_instruccion_a_memoria = crear_paquete_solicitud_pedir_instruccion_a_memoria(logger, pedido_instruccion);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_pedir_instruccion_a_memoria, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA);
}

void solicitar_info_inicial_a_memoria()
{
	enviar_paquete_solicitud_pedir_info_de_memoria_inicial();

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
	t_info_memoria *info_memoria = leer_paquete_respuesta_pedir_info_de_memoria_inicial_para_cpu(logger, conexion_con_memoria);
	tamanio_pagina = info_memoria->tamanio_pagina;
	tamanio_memoria = info_memoria->tamanio_memoria;
	free(info_memoria);
}

char *pedir_instruccion_a_memoria()
{
	enviar_paquete_solicitud_pedir_instruccion_a_memoria();

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
	char *instruccion_string = leer_paquete_respuesta_pedir_instruccion_a_memoria(logger, conexion_con_memoria);

	return instruccion_string;
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* CICLO DE EJECUCION *////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////

void ciclo_de_ejecucion()
{
	char *instruccion_string;
	t_instruccion *instruccion;
	int opcode;

	while (true)
	{
		// Espero a que haya algo para ejecutar
		sem_wait(&semaforo_ejecutar_ciclo_de_instruccion); 

		instruccion_string = fetch();
		instruccion = decode(instruccion_string);
		execute(instruccion);

		if (instruccion->opcode != JNZ_OPCODE_INSTRUCCION)
		{
			program_counter++;
		}

		if (ocurrio_interrupcion)
		{
			devolver_contexto_por_ser_interrumpido();
		}

		if (!se_ejecuto_instruccion_bloqueante && !ocurrio_interrupcion && !se_leyo_instruccion_exit)
		{
			// Sigo ejecutando
			sem_post(&semaforo_ejecutar_ciclo_de_instruccion);
		}

		destruir_instruccion(instruccion);
	}
}

char *fetch()
{
	log_info(logger, "PID: %d - FETCH - Program Counter: %d", pid_ejecutando, program_counter);
	return pedir_instruccion_a_memoria();
}

t_instruccion *decode(char *instruccion_string)
{
	char *saveptr = instruccion_string;
	char *nombre_instruccion = strtok_r(saveptr, " ", &saveptr);
	t_instruccion *instruccion = malloc(sizeof(t_instruccion));
	instruccion->parametro_1 = NULL;
	instruccion->parametro_2 = NULL;
	bool tengo_que_leer_parametro_1 = false;
	bool tengo_que_leer_parametro_2 = false;

	if (strcmp(nombre_instruccion, SET_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = SET_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, SUM_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = SUM_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, SUB_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = SUB_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, JNZ_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = JNZ_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, SLEEP_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = SLEEP_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = false;
	}
	else if (strcmp(nombre_instruccion, WAIT_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = WAIT_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = false;
	}
	else if (strcmp(nombre_instruccion, SIGNAL_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = SIGNAL_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = false;
	}
	else if (strcmp(nombre_instruccion, MOV_IN_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = MOV_IN_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, MOV_OUT_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = MOV_OUT_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, FOPEN_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = FOPEN_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, FCLOSE_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = FCLOSE_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = false;
	}
	else if (strcmp(nombre_instruccion, FSEEK_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = FSEEK_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, FREAD_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = FREAD_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, FWRITE_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = FWRITE_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, FTRUNCATE_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = FTRUNCATE_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = true;
		tengo_que_leer_parametro_2 = true;
	}
	else if (strcmp(nombre_instruccion, EXIT_NOMBRE_INSTRUCCION) == 0)
	{
		instruccion->opcode = EXIT_OPCODE_INSTRUCCION;
		tengo_que_leer_parametro_1 = false;
		tengo_que_leer_parametro_2 = false;
	}
	else
	{
		log_error(logger, "Instrucción '%s' a decodificar no reconocida.", nombre_instruccion);
		free(instruccion_string);
		destruir_instruccion(instruccion);
		return NULL;
	}

	if (tengo_que_leer_parametro_1)
	{
		char *parametro_1 = strtok_r(saveptr, " ", &saveptr);

		if (parametro_1 == NULL)
		{
			log_error(logger, "Parametro 1 para instrucción '%s' no reconocido.", nombre_instruccion);
			free(instruccion_string);
			destruir_instruccion(instruccion);
			return NULL;
		}

		instruccion->parametro_1 = malloc(strlen(parametro_1));
		strcpy(instruccion->parametro_1, parametro_1);
	}

	if (tengo_que_leer_parametro_2)
	{
		char *parametro_2 = strtok_r(saveptr, " ", &saveptr);

		if (parametro_2 == NULL)
		{
			log_error(logger, "Parametro 2 para instrucción '%s' no reconocido.", nombre_instruccion);
			free(instruccion_string);
			destruir_instruccion(instruccion);
			return NULL;
		}

		instruccion->parametro_2 = malloc(strlen(parametro_2));
		strcpy(instruccion->parametro_2, parametro_2);
	}

	free(instruccion_string);
	return instruccion;
}

void execute(t_instruccion *instruccion)
{
	if (instruccion->opcode == SET_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_set(instruccion->parametro_1, atoi(instruccion->parametro_2));
		return;
	}

	if (instruccion->opcode == SUM_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_sum(instruccion->parametro_1, instruccion->parametro_2);
		return;
	}

	if (instruccion->opcode == SUB_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_sub(instruccion->parametro_1, instruccion->parametro_2);
		return;
	}

	if (instruccion->opcode == JNZ_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_jnz(instruccion->parametro_1, atoi(instruccion->parametro_2));
		return;
	}

	if (instruccion->opcode == SLEEP_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_sleep(atoi(instruccion->parametro_1));
		return;
	}

	if (instruccion->opcode == WAIT_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_wait(instruccion->parametro_1);
		return;
	}

	if (instruccion->opcode == SIGNAL_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_signal(instruccion->parametro_1);
		return;
	}

	if (instruccion->opcode == MOV_IN_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_mov_in(instruccion->parametro_1, instruccion->parametro_2);
		return;
	}

	if (instruccion->opcode == MOV_OUT_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_mov_out(instruccion->parametro_1, instruccion->parametro_2);
		return;
	}

	if (instruccion->opcode == FOPEN_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_fopen(instruccion->parametro_1, instruccion->parametro_2);
		return;
	}

	if (instruccion->opcode == FCLOSE_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_fclose(instruccion->parametro_1);
		return;
	}

	if (instruccion->opcode == FSEEK_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_fseek(instruccion->parametro_1, instruccion->parametro_2);
		return;
	}

	if (instruccion->opcode == FREAD_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_fread(instruccion->parametro_1, instruccion->parametro_2);
		return;
	}

	if (instruccion->opcode == FWRITE_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_fwrite(instruccion->parametro_1, instruccion->parametro_2);
		return;
	}

	if (instruccion->opcode == FTRUNCATE_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_ftruncate(instruccion->parametro_1, instruccion->parametro_2);
		return;
	}

	if (instruccion->opcode == EXIT_OPCODE_INSTRUCCION)
	{
		ejecutar_instruccion_exit();
		return;
	}

	log_error(logger, "Instrucción a ejecutar no reconocida.");
	return;
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* INSTRUCCIONES */////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////

void ejecutar_instruccion_set(char *nombre_registro, uint32_t valor)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %d", pid_ejecutando, SET_NOMBRE_INSTRUCCION, nombre_registro, valor);

	escribir_valor_a_registro(nombre_registro, valor);

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %d", pid_ejecutando, SET_NOMBRE_INSTRUCCION, nombre_registro, valor);
}

void ejecutar_instruccion_sum(char *nombre_registro_destino, char *nombre_registro_origen)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, SUM_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);

	escribir_valor_a_registro(nombre_registro_destino, obtener_valor_registro(nombre_registro_destino) + obtener_valor_registro(nombre_registro_origen));

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %s", pid_ejecutando, SUM_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);
}

void ejecutar_instruccion_sub(char *nombre_registro_destino, char *nombre_registro_origen)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, SUB_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);

	escribir_valor_a_registro(nombre_registro_destino, obtener_valor_registro(nombre_registro_destino) - obtener_valor_registro(nombre_registro_origen));

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %s", pid_ejecutando, SUB_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);
}

void ejecutar_instruccion_jnz(char *nombre_registro, uint32_t nuevo_program_counter)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %d", pid_ejecutando, JNZ_NOMBRE_INSTRUCCION, nombre_registro, nuevo_program_counter);

	if (obtener_valor_registro(nombre_registro) == 0)
	{
		escribir_valor_a_registro(PC_NOMBRE_REGISTRO, nuevo_program_counter);
	}

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %d", pid_ejecutando, JNZ_NOMBRE_INSTRUCCION, nombre_registro, nuevo_program_counter);
}

void ejecutar_instruccion_sleep(int tiempo)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %d", pid_ejecutando, SLEEP_NOMBRE_INSTRUCCION, tiempo);

	se_ejecuto_instruccion_bloqueante = true;
	program_counter++;
	devolver_contexto_por_sleep(tiempo);

	log_trace(logger, "PID: %d - Ejecutada: %s - %d", pid_ejecutando, SLEEP_NOMBRE_INSTRUCCION, tiempo);
}

void ejecutar_instruccion_wait(char *nombre_recurso)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s", pid_ejecutando, WAIT_NOMBRE_INSTRUCCION, nombre_recurso);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s", pid_ejecutando, WAIT_NOMBRE_INSTRUCCION, nombre_recurso);
}

void ejecutar_instruccion_signal(char *nombre_recurso)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s", pid_ejecutando, SIGNAL_NOMBRE_INSTRUCCION, nombre_recurso);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s", pid_ejecutando, SIGNAL_NOMBRE_INSTRUCCION, nombre_recurso);
}

void ejecutar_instruccion_mov_in(char *nombre_registro, char *direccion_logica)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, MOV_IN_NOMBRE_INSTRUCCION, nombre_registro, direccion_logica);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %s", pid_ejecutando, MOV_OUT_NOMBRE_INSTRUCCION, nombre_registro, direccion_logica);
}

void ejecutar_instruccion_mov_out(char *direccion_logica, char *nombre_registro)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, MOV_OUT_NOMBRE_INSTRUCCION, direccion_logica, nombre_registro);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %s", pid_ejecutando, MOV_OUT_NOMBRE_INSTRUCCION, direccion_logica, nombre_registro);
}

void ejecutar_instruccion_fopen(char *nombre_archivo, char *modo_apertura)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, FOPEN_NOMBRE_INSTRUCCION, nombre_archivo, modo_apertura);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %s", pid_ejecutando, FOPEN_NOMBRE_INSTRUCCION, nombre_archivo, modo_apertura);
}

void ejecutar_instruccion_fclose(char *nombre_archivo)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s", pid_ejecutando, FCLOSE_NOMBRE_INSTRUCCION, nombre_archivo);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s", pid_ejecutando, FCLOSE_NOMBRE_INSTRUCCION, nombre_archivo);
}

void ejecutar_instruccion_fseek(char *nombre_archivo, char *posicion)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, FSEEK_NOMBRE_INSTRUCCION, nombre_archivo, posicion);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %s", pid_ejecutando, FSEEK_NOMBRE_INSTRUCCION, nombre_archivo, posicion);
}

void ejecutar_instruccion_fread(char *nombre_archivo, char *direccion_logica)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, FREAD_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %s", pid_ejecutando, FREAD_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);
}

void ejecutar_instruccion_fwrite(char *nombre_archivo, char *direccion_logica)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, FWRITE_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %s", pid_ejecutando, FWRITE_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);
}

void ejecutar_instruccion_ftruncate(char *nombre_archivo, char *tamanio)
{
	log_info(logger, "PID: %d - Ejecutando: %s - %s - %s", pid_ejecutando, FTRUNCATE_NOMBRE_INSTRUCCION, nombre_archivo, tamanio);

	// TO DO

	log_trace(logger, "PID: %d - Ejecutada: %s - %s - %s", pid_ejecutando, FTRUNCATE_NOMBRE_INSTRUCCION, nombre_archivo, tamanio);
}

void ejecutar_instruccion_exit()
{
	log_info(logger, "PID: %d - Ejecutando: %s", pid_ejecutando, EXIT_NOMBRE_INSTRUCCION);

	se_leyo_instruccion_exit = true;
	devolver_contexto_por_correcta_finalizacion();

	log_trace(logger, "PID: %d - Ejecutada: %s", pid_ejecutando, EXIT_NOMBRE_INSTRUCCION);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* UTILIDADES *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
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

void destruir_instruccion(t_instruccion *instruccion)
{
	if (instruccion == NULL)
	{
		return;
	}

	if (instruccion->parametro_1 != NULL)
	{
		free(instruccion->parametro_1);
	}

	if (instruccion->parametro_2 != NULL)
	{
		free(instruccion->parametro_2);
	}

	free(instruccion);
}

uint32_t obtener_valor_registro(char *nombre_registro)
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
		log_info(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, BX_NOMBRE_REGISTRO) == 0)
	{
		registro_bx = valor;
		log_info(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, CX_NOMBRE_REGISTRO) == 0)
	{
		registro_cx = valor;
		log_info(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, DX_NOMBRE_REGISTRO) == 0)
	{
		registro_dx = valor;
		log_info(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, PC_NOMBRE_REGISTRO) == 0)
	{
		program_counter = valor;
		log_info(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else
	{
		log_error(logger, "No se asigno el valor %d al registro '%s' porque es un registro no conocido.", valor, nombre_registro);
	}
}