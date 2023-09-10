#include "cpu.h"

pthread_t hilo_interrupt;
pthread_t hilo_dispatch;
t_log *logger = NULL;
t_argumentos_cpu *argumentos_cpu = NULL;
t_config_cpu *configuracion_cpu = NULL;
int socket_kernel_dispatch = -1;
int socket_kernel_interrupt = -1;
int conexion_con_kernel_dispatch = -1;
int conexion_con_kernel_interrupt = -1;
int conexion_con_memoria = -1;

t_pcb *proceso_ejecutando;

sem_t semaforo_ciclo_de_ejecucion;

// Registros
uint32_t program_counter;
uint32_t registro_ax;
uint32_t registro_bx;
uint32_t registro_cx;
uint32_t registro_dx;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_CPU, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_cpu();
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_CPU);

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

	// Semaforos
	sem_init(&semaforo_ciclo_de_ejecucion, false, 0);

	// Hilos
	pthread_create(&hilo_interrupt, NULL, interrupt, NULL);
	pthread_create(&hilo_dispatch, NULL, dispatch, NULL);

	// Hilo principal
	ciclo_de_ejecucion();

	// Finalizacion
	terminar_cpu();
	return EXIT_SUCCESS;
}

// Hilo Interrupt
void *interrupt()
{
}

// Hilo Dispatch
void *dispatch()
{
	log_trace(logger, "SOY DISPATCH");
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL, conexion_con_kernel_dispatch);
	log_trace(logger, "BBBBBB");
	if (codigo_operacion_recibido == MENSAJE_DE_KERNEL)
	{
		log_trace(logger, "RECIBI MENSAJE DE KERNEL.");

		t_pcb *pcb = leer_paquete_pcb(logger, conexion_con_kernel_dispatch, codigo_operacion_recibido, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);

		log_trace(logger, "ESTOY EN LA CPU Y EL PCB TIENE PID %d.", pcb->pid);
	}
	else
	{
		log_error(logger, "NO SE QUE RECIBI.");
	}
}

// Hilo Ciclo de Ejecucion (Hilo Principal)
void ciclo_de_ejecucion()
{
	t_instruccion *instruccion;
	int opcode;

	while (true)
	{
		// sem_wait(&semaforo_ciclo_de_ejecucion);
		// instruccion = fetch();
		// opcode = decode(instruccion->nombre_instruccion);
		// execute(opcode, instruccion->parametro_1_instruccion, instruccion->parametro_2_instruccion);
		// check_interrupt();
	}
}

t_instruccion *fetch()
{
}

int decode(char *nombre_instruccion)
{
	if (strcmp(nombre_instruccion, SET_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return SET_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, SUM_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return SUM_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, SUB_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return SUB_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, JNZ_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return JNZ_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, SLEEP_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return SLEEP_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, WAIT_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return WAIT_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, SIGNAL_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return SIGNAL_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, MOV_IN_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return MOV_IN_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, MOV_OUT_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return MOV_OUT_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, FOPEN_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return FOPEN_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, FCLOSE_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return FCLOSE_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, FSEEK_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return FSEEK_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, FREAD_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return FREAD_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, FWRITE_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return FWRITE_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, FTRUNCATE_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return FTRUNCATE_OPCODE_INSTRUCCION;
	}

	if (strcmp(nombre_instruccion, EXIT_NOMBRE_INSTRUCCION) == 0)
	{
		log_trace(logger, "Instrucción '%s' decodificada.", nombre_instruccion);
		return EXIT_OPCODE_INSTRUCCION;
	}

	log_error(logger, "Instrucción '%s' a decodificar no reconocida.", nombre_instruccion);
	return -1;
}

void execute(int opcode, char *parametro_1_instruccion, char *parametro_2_instruccion)
{
}

void check_interrupt()
{
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
		log_trace(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, BX_NOMBRE_REGISTRO) == 0)
	{
		registro_bx = valor;
		log_trace(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, CX_NOMBRE_REGISTRO) == 0)
	{
		registro_cx = valor;
		log_trace(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, DX_NOMBRE_REGISTRO) == 0)
	{
		registro_dx = valor;
		log_trace(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}
	else if (strcmp(nombre_registro, PC_NOMBRE_REGISTRO) == 0)
	{
		program_counter = valor;
		log_trace(logger, "Se le asigno al registro '%s' el valor %d.", nombre_registro, valor);
	}

	log_error(logger, "No se asigno el valor %d al registro '%s' porque es un registro no conocido.", valor, nombre_registro);
}

void ejecutar_instruccion_set(char *nombre_registro, uint32_t valor)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %d", SET_NOMBRE_INSTRUCCION, nombre_registro, valor);

	escribir_valor_a_registro(nombre_registro, valor);

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %d", SET_NOMBRE_INSTRUCCION, nombre_registro, valor);
}

void ejecutar_instruccion_sum(char *nombre_registro_destino, char *nombre_registro_origen)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %s", SUM_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);

	escribir_valor_a_registro(nombre_registro_destino, obtener_valor_registro(nombre_registro_destino) + obtener_valor_registro(nombre_registro_origen));

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %s", SUM_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);
}

void ejecutar_instruccion_sub(char *nombre_registro_destino, char *nombre_registro_origen)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %s", SUB_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);

	escribir_valor_a_registro(nombre_registro_destino, obtener_valor_registro(nombre_registro_destino) - obtener_valor_registro(nombre_registro_origen));

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %s", SUB_NOMBRE_INSTRUCCION, nombre_registro_destino, nombre_registro_origen);
}

void ejecutar_instruccion_jnz(char *nombre_registro, uint32_t nuevo_program_counter)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %d", JNZ_NOMBRE_INSTRUCCION, nombre_registro, nuevo_program_counter);

	if (obtener_valor_registro(PC_NOMBRE_REGISTRO) == 0)
	{
		escribir_valor_a_registro(PC_NOMBRE_REGISTRO, nuevo_program_counter);
	}

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %d", JNZ_NOMBRE_INSTRUCCION, nombre_registro, nuevo_program_counter);
}

void ejecutar_instruccion_sleep(int tiempo)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %d", SLEEP_NOMBRE_INSTRUCCION, tiempo);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %d", SLEEP_NOMBRE_INSTRUCCION, tiempo);
}

void ejecutar_instruccion_wait(char *nombre_recurso)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s", WAIT_NOMBRE_INSTRUCCION, nombre_recurso);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s", WAIT_NOMBRE_INSTRUCCION, nombre_recurso);
}

void ejecutar_instruccion_signal(char *nombre_recurso)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s", SIGNAL_NOMBRE_INSTRUCCION, nombre_recurso);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s", SIGNAL_NOMBRE_INSTRUCCION, nombre_recurso);
}

void ejecutar_instruccion_mov_in(char *nombre_registro, char *direccion_logica)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %s", MOV_IN_NOMBRE_INSTRUCCION, nombre_registro, direccion_logica);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %s", MOV_OUT_NOMBRE_INSTRUCCION, nombre_registro, direccion_logica);
}

void ejecutar_instruccion_mov_out(char *direccion_logica, char *nombre_registro)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %s", MOV_OUT_NOMBRE_INSTRUCCION, direccion_logica, nombre_registro);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %s", MOV_OUT_NOMBRE_INSTRUCCION, direccion_logica, nombre_registro);
}

void ejecutar_instruccion_fopen(char *nombre_archivo, char *modo_apertura)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %s", FOPEN_NOMBRE_INSTRUCCION, nombre_archivo, modo_apertura);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %s", FOPEN_NOMBRE_INSTRUCCION, nombre_archivo, modo_apertura);
}

void ejecutar_instruccion_fclose(char *nombre_archivo)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s", FCLOSE_NOMBRE_INSTRUCCION, nombre_archivo);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s", FCLOSE_NOMBRE_INSTRUCCION, nombre_archivo);
}

void ejecutar_instruccion_fseek(char *nombre_archivo, char *posicion)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %s", FSEEK_NOMBRE_INSTRUCCION, nombre_archivo, posicion);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %s", FSEEK_NOMBRE_INSTRUCCION, nombre_archivo, posicion);
}

void ejecutar_instruccion_fread(char *nombre_archivo, char *direccion_logica)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %s", FREAD_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %s", FREAD_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);
}

void ejecutar_instruccion_fwrite(char *nombre_archivo, char *direccion_logica)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %s", FWRITE_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %s", FWRITE_NOMBRE_INSTRUCCION, nombre_archivo, direccion_logica);
}

void ejecutar_instruccion_ftruncate(char *nombre_archivo, char *tamanio)
{
	log_info(logger, "PID: <PID> - Ejecutando: %s - %s - %s", FTRUNCATE_NOMBRE_INSTRUCCION, nombre_archivo, tamanio);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s - %s - %s", FTRUNCATE_NOMBRE_INSTRUCCION, nombre_archivo, tamanio);
}

void ejecutar_instruccion_exit()
{
	log_info(logger, "PID: <PID> - Ejecutando: %s", EXIT_NOMBRE_INSTRUCCION);

	// TO DO

	log_trace(logger, "PID: <PID> - Ejecutada: %s", EXIT_NOMBRE_INSTRUCCION);
}

void terminar_cpu()
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_CPU);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_cpu);
	destruir_configuracion(configuracion_cpu);

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