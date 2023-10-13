#include "kernel.h"

// Variables globales
t_log *logger = NULL;
t_argumentos_kernel *argumentos_kernel = NULL;
t_config_kernel *configuracion_kernel = NULL;
t_list *pcbs;

bool planificacion_detenida = false;
int proximo_pid = 0;
bool hay_un_proceso_ejecutando = false;

// Conexiones
int conexion_con_cpu_dispatch = -1;
int conexion_con_cpu_interrupt = -1;
int conexion_con_memoria = -1;
int conexion_con_filesystem = -1;

// Hilos
pthread_t hilo_planificador_largo_plazo;
pthread_t hilo_planificador_corto_plazo;
pthread_t hilo_dispatcher;
pthread_t hilo_escucha_cpu;

// Semaforos
sem_t semaforo_grado_max_multiprogramacion;
sem_t semaforo_hay_algun_proceso_en_cola_new;
sem_t semaforo_hay_algun_proceso_en_cola_ready;

pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_conexion_cpu_dispatch;
pthread_mutex_t mutex_conexion_cpu_interrupt;
pthread_mutex_t mutex_conexion_memoria;
pthread_mutex_t mutex_conexion_filesystem;

// Colas de planificacion
t_queue *cola_new = NULL;
t_queue *cola_ready = NULL;
t_pcb *pcb_ejecutando = NULL;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	setbuf(stdout, NULL); // Why? Era algo de consola esto.

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_KERNEL, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_kernel();
		return EXIT_FAILURE;
	}

	argumentos_kernel = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_kernel == NULL)
	{
		terminar_kernel();
		return EXIT_FAILURE;
	}

	configuracion_kernel = leer_configuracion(logger, argumentos_kernel->ruta_archivo_configuracion);
	if (configuracion_kernel == NULL)
	{
		terminar_kernel();
		return EXIT_FAILURE;
	}

	conexion_con_cpu_dispatch = crear_socket_cliente(logger, configuracion_kernel->ip_cpu, configuracion_kernel->puerto_cpu_dispatch, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	if (conexion_con_cpu_dispatch == -1)
	{
		terminar_kernel();
		return EXIT_FAILURE;
	}

	conexion_con_cpu_interrupt = crear_socket_cliente(logger, configuracion_kernel->ip_cpu, configuracion_kernel->puerto_cpu_interrupt, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);
	if (conexion_con_cpu_interrupt == -1)
	{
		terminar_kernel();
		return EXIT_FAILURE;
	}

	conexion_con_memoria = crear_socket_cliente(logger, configuracion_kernel->ip_cpu, configuracion_kernel->puerto_memoria, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
	if (conexion_con_memoria == -1)
	{
		terminar_kernel();
		return EXIT_FAILURE;
	}

	conexion_con_filesystem = crear_socket_cliente(logger, configuracion_kernel->ip_cpu, configuracion_kernel->puerto_filesystem, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);
	if (conexion_con_filesystem == -1)
	{
		terminar_kernel();
		return EXIT_FAILURE;
	}

	// Semaforos
	sem_init(&semaforo_grado_max_multiprogramacion, false, configuracion_kernel->grado_multiprogramacion_inicial);
	sem_init(&semaforo_hay_algun_proceso_en_cola_new, false, 0);
	sem_init(&semaforo_hay_algun_proceso_en_cola_ready, false, 0);

	// Colas de planificacion
	cola_new = queue_create();
	cola_ready = queue_create();

	// Listas
	pcbs = list_create();

	// Hilos
	pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);

	if (strcmp(configuracion_kernel->algoritmo_planificacion, ALGORITMO_PLANIFICACION_FIFO) == 0)
	{
		pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo_fifo, NULL);
	}
	else if (strcmp(configuracion_kernel->algoritmo_planificacion, ALGORITMO_PLANIFICACION_ROUND_ROBIN) == 0)
	{
		pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo_round_robin, NULL);
	}
	else
	{
		pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo_prioridades, NULL);
	}

	// Hilo principal
	consola();

	// Finalizacion
	terminar_kernel();
	return EXIT_SUCCESS;
}

void terminar_kernel()
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_KERNEL);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_kernel);
	destruir_configuracion(configuracion_kernel);

	if (conexion_con_cpu_dispatch != -1)
	{
		close(conexion_con_cpu_dispatch);
	}

	if (conexion_con_cpu_interrupt != -1)
	{
		close(conexion_con_cpu_interrupt);
	}

	if (conexion_con_memoria != -1)
	{
		close(conexion_con_memoria);
	}

	if (conexion_con_filesystem != -1)
	{
		close(conexion_con_filesystem);
	}

	// sem_destroys
	// thread destroy?
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* PLANIFICADORES *////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////

void *planificador_largo_plazo()
{
	while (true)
	{
		sem_wait(&semaforo_hay_algun_proceso_en_cola_new);
		sem_wait(&semaforo_grado_max_multiprogramacion);

		t_pcb *pcb = queue_pop_thread_safe(cola_new, &mutex_cola_new);
		transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_READY);
	}
}

void *planificador_corto_plazo_fifo()
{
	while (true)
	{
		sem_wait(&semaforo_hay_algun_proceso_en_cola_ready);

		t_pcb *pcb = queue_pop_thread_safe(cola_ready, &mutex_cola_ready);
		transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXECUTING);

		op_code codigo_operacion_recibido;
		t_contexto_de_ejecucion *contexto_de_ejecucion = recibir_paquete_de_cpu_dispatch(&codigo_operacion_recibido);
		actualizar_pcb(pcb, contexto_de_ejecucion);

		if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION)
		{
			transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
		}
	}
}

void *planificador_corto_plazo_round_robin()
{
	while (true)
	{
	}
}

void *planificador_corto_plazo_prioridades()
{
	while (true)
	{
	}
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* TRANSICIONES *//////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////

void transicionar_proceso(t_pcb *pcb, char nuevo_estado_proceso)
{
	if (pcb->estado == CODIGO_ESTADO_PROCESO_DESCONOCIDO)
	{
		if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_NEW)
		{
			transicionar_proceso_a_new(pcb);
		}
	}
	else if (pcb->estado == CODIGO_ESTADO_PROCESO_NEW)
	{
		if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_READY)
		{
			transicionar_proceso_de_new_a_ready(pcb);
		}
		else if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_EXIT)
		{
			transicionar_proceso_de_new_a_exit(pcb);
		}
	}
	else if (pcb->estado == CODIGO_ESTADO_PROCESO_READY)
	{
		if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_EXECUTING)
		{
			transicionar_proceso_de_ready_a_executing(pcb);
		}
		else if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_EXIT)
		{
			transicionar_proceso_de_ready_a_exit(pcb);
		}
	}
	else if (pcb->estado == CODIGO_ESTADO_PROCESO_EXECUTING)
	{
		if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_READY)
		{
			transicionar_proceso_de_executing_a_ready(pcb);
		}
		else if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_BLOCKED)
		{
			transicionar_proceso_de_executing_a_bloqueado(pcb);
		}
		else if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_EXIT)
		{
			transicionar_proceso_de_executing_a_exit(pcb);
		}
	}
	else if (pcb->estado == CODIGO_ESTADO_PROCESO_BLOCKED)
	{
		if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_READY)
		{
			transicionar_proceso_de_bloqueado_a_ready(pcb);
		}
		else if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_EXIT)
		{
			transicionar_proceso_de_bloqueado_a_exit(pcb);
		}
	}
}

void transicionar_proceso_a_new(t_pcb *pcb)
{
	bool estructuras_inicializadas_correctamente = iniciar_estructuras_de_proceso_en_memoria(pcb);

	if (!estructuras_inicializadas_correctamente)
	{
		return;
	}

	log_info(logger, "Se crea el proceso %d en NEW", pcb->pid);
	pcb->estado = CODIGO_ESTADO_PROCESO_NEW;
	queue_push_thread_safe(cola_new, pcb, &mutex_cola_new);
	sem_post(&semaforo_hay_algun_proceso_en_cola_new);
}

void transicionar_proceso_de_new_a_ready(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_READY));
	loguear_cola_pcbs(cola_ready, "ready");
	pcb->estado = CODIGO_ESTADO_PROCESO_READY;
	queue_push_thread_safe(cola_ready, pcb, &mutex_cola_ready);
	sem_post(&semaforo_hay_algun_proceso_en_cola_ready);
}

void transicionar_proceso_de_new_a_exit(t_pcb *pcb)
{
}

void transicionar_proceso_de_ready_a_executing(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXECUTING));
	pcb_ejecutando = pcb;
	pcb->estado = CODIGO_ESTADO_PROCESO_EXECUTING;
	ejecutar_proceso_en_cpu(pcb);
}

void transicionar_proceso_de_ready_a_exit(t_pcb *pcb)
{
}

void transicionar_proceso_de_executing_a_ready(t_pcb *pcb)
{
}

void transicionar_proceso_de_executing_a_bloqueado(t_pcb *pcb)
{
}

void transicionar_proceso_de_executing_a_exit(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXIT));
	pcb_ejecutando = NULL;
	pcb->estado = CODIGO_ESTADO_PROCESO_EXIT;
	destruir_estructuras_de_proceso_en_memoria(pcb);
	sem_post(&semaforo_grado_max_multiprogramacion);
}

void transicionar_proceso_de_bloqueado_a_ready(t_pcb *pcb)
{
}

void transicionar_proceso_de_bloqueado_a_exit(t_pcb *pcb)
{
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* CONSOLA *///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////

void consola()
{
	bool finalizar = false;

	while (!finalizar)
	{
		char *valor_ingresado_por_teclado = NULL;

		do
		{
			valor_ingresado_por_teclado = readline("KERNEL> ");
		} while (valor_ingresado_por_teclado == NULL);

		log_trace(logger, "Valor ingresado por consola: %s", valor_ingresado_por_teclado);

		char *saveptr = valor_ingresado_por_teclado;
		char *funcion_seleccionada = strtok_r(saveptr, " ", &saveptr);

		if (strcmp(funcion_seleccionada, INICIAR_PROCESO) == 0)
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);

			char *path;
			char *size_str;
			int size;
			char *prioridad_str;
			int prioridad;

			log_trace(logger, "Leyendo argumentos de funcion %s", funcion_seleccionada);

			log_trace(logger, "Intentando leer argumento 'path' de funcion %s", funcion_seleccionada);
			if ((path = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'path' para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'path' para la funcion %s.", funcion_seleccionada);
				continue;
			}
			else
			{
				log_trace(logger, "Se leyo el argumento 'path' de funcion %s y es: '%s'", funcion_seleccionada, path);
			}

			log_trace(logger, "Intentando leer argumento 'size' de funcion %s", funcion_seleccionada);
			if ((size_str = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'size' para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'size' para la funcion %s.", funcion_seleccionada);
				continue;
			}
			else
			{
				log_trace(logger, "Se leyo el argumento 'size' de funcion %s y es: '%s'", funcion_seleccionada, size_str);
				log_trace(logger, "Intentando convertir el argumento 'size' = '%s' de funcion %s a entero", size_str, funcion_seleccionada);

				if (size = atoi(size_str))
				{
					log_trace(logger, "Se pudo convertir el argumento 'size' de funcion %s al entero %d", funcion_seleccionada, size);
				}
				else
				{
					log_trace(logger, "No se pudo convertir el argumento 'size' = '%s' de funcion %s a entero", size_str, funcion_seleccionada);
					printf("No se pudo convertir el argumento 'size' = '%s' de funcion %s a entero\n", size_str, funcion_seleccionada);
					continue;
				}
			}

			log_trace(logger, "Intentando leer argumento 'prioridad' de funcion %s", funcion_seleccionada);
			if ((prioridad_str = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'prioridad' para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'prioridad' para la funcion %s.", funcion_seleccionada);
				continue;
			}
			else
			{
				log_trace(logger, "Se leyo el argumento 'prioridad' de funcion %s y es: '%s'", funcion_seleccionada, prioridad_str);
				log_trace(logger, "Intentando convertir el argumento 'prioridad' = '%s' de funcion %s a entero", prioridad_str, funcion_seleccionada);

				if (prioridad = atoi(prioridad_str))
				{
					log_trace(logger, "Se pudo convertir el argumento 'prioridad' de funcion %s al entero %d", funcion_seleccionada, prioridad);
				}
				else
				{
					log_trace(logger, "No se pudo convertir el argumento 'prioridad' = '%s' de funcion %s a entero", prioridad_str, funcion_seleccionada);
					printf("No se pudo convertir el argumento 'prioridad' = '%s' de funcion %s a entero\n", prioridad_str, funcion_seleccionada);
					continue;
				}
			}

			iniciar_proceso(path, size, prioridad);
		}
		else if (strcmp(funcion_seleccionada, FINALIZAR_PROCESO) == 0)
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);

			char *pid_str;
			int pid;

			log_trace(logger, "Leyendo argumentos de funcion %s", funcion_seleccionada);

			log_trace(logger, "Intentando leer argumento 'pid' de funcion %s", funcion_seleccionada);
			if ((pid_str = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'pid' para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'pid' para la funcion %s.", funcion_seleccionada);
				continue;
			}
			else
			{
				log_trace(logger, "Se leyo el argumento 'pid' de funcion %s y es: '%s'", funcion_seleccionada, pid_str);
				log_trace(logger, "Intentando convertir el argumento 'pid' = '%s' de funcion %s a entero", pid_str, funcion_seleccionada);

				if (pid = atoi(pid_str))
				{
					log_trace(logger, "Se pudo convertir el argumento 'pid' de funcion %s al entero %d", funcion_seleccionada, pid);
				}
				else
				{
					log_trace(logger, "No se pudo convertir el argumento 'pid' = '%s' de funcion %s a entero", pid_str, funcion_seleccionada);
					printf("No se pudo convertir el argumento 'pid' = '%s' de funcion %s a entero\n", pid_str, funcion_seleccionada);
					continue;
				}
			}

			finalizar_proceso(pid);
		}
		else if (strcmp(funcion_seleccionada, DETENER_PLANIFICACION) == 0)
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);
			detener_planificacion();
		}
		else if (strcmp(funcion_seleccionada, INICIAR_PLANIFICACION) == 0)
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);
			iniciar_planificacion();
		}
		else if (strcmp(funcion_seleccionada, MULTIPROGRAMACION) == 0)
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);

			char *nuevo_grado_multiprogramacion_str;
			int nuevo_grado_multiprogramacion;

			log_trace(logger, "Leyendo argumentos de funcion %s", funcion_seleccionada);

			log_trace(logger, "Intentando leer argumento 'nuevo_grado_multiprogramacion' de funcion %s", funcion_seleccionada);
			if ((nuevo_grado_multiprogramacion_str = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'nuevo_grado_multiprogramacion para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'nuevo_grado_multiprogramacion' para la funcion %s.", funcion_seleccionada);
				continue;
			}
			else
			{
				log_trace(logger, "Se leyo el argumento 'nuevo_grado_multiprogramacion' de funcion %s y es: '%s'", funcion_seleccionada, nuevo_grado_multiprogramacion_str);
				log_trace(logger, "Intentando convertir el argumento 'nuevo_grado_multiprogramacion' = '%s' de funcion %s a entero", nuevo_grado_multiprogramacion_str, funcion_seleccionada);

				if (nuevo_grado_multiprogramacion = atoi(nuevo_grado_multiprogramacion_str))
				{
					log_trace(logger, "Se pudo convertir el argumento 'nuevo_grado_multiprogramacion' de funcion %s al entero %d", funcion_seleccionada, nuevo_grado_multiprogramacion);
				}
				else
				{
					log_trace(logger, "No se pudo convertir el argumento 'nuevo_grado_multiprogramacion' = '%s' de funcion %s a entero", nuevo_grado_multiprogramacion_str, funcion_seleccionada);
					printf("No se pudo convertir el argumento 'nuevo_grado_multiprogramacion' = '%s' de funcion %s a entero\n", nuevo_grado_multiprogramacion_str, funcion_seleccionada);
					continue;
				}
			}

			modificar_grado_max_multiprogramacion(nuevo_grado_multiprogramacion);
		}
		else if (strcmp(funcion_seleccionada, PROCESO_ESTADO) == 0)
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);
			listar_procesos();
		}
		else
		{
			log_error(logger, "'%s' no coincide con ninguna funcion conocida.", funcion_seleccionada);
			printf("'%s' no coincide con ninguna funcion conocida.\n", funcion_seleccionada);
		}
	}
}

void iniciar_proceso(char *path, int size, int prioridad)
{
	t_pcb *pcb = crear_pcb(path, size, prioridad);
	transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_NEW);
}

void finalizar_proceso(int pid)
{
	// buscar proceso en colas y llamar a transicionar a exit
}

void iniciar_planificacion()
{
	if (!planificacion_detenida)
	{
		printf("ERROR: La planificacion ya esta iniciada.\n");
		return;
	}

	log_info(logger, "INICIO DE PLANIFICACIÓN");
}

void detener_planificacion()
{
	if (planificacion_detenida)
	{
		printf("ERROR: La planificacion ya se encuentra detenida.\n");
		return;
	}

	log_info(logger, "PAUSA DE PLANIFICACIÓN");
}

void modificar_grado_max_multiprogramacion(int grado_multiprogramacion)
{
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* CPU *///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
void enviar_paquete_solicitud_ejecutar_proceso(t_pcb *pcb_proceso_a_ejecutar)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = malloc(sizeof(t_contexto_de_ejecucion));
	contexto_de_ejecucion->pid = pcb_proceso_a_ejecutar->pid;
	contexto_de_ejecucion->program_counter = pcb_proceso_a_ejecutar->program_counter;
	contexto_de_ejecucion->registro_ax = pcb_proceso_a_ejecutar->registro_ax;
	contexto_de_ejecucion->registro_bx = pcb_proceso_a_ejecutar->registro_bx;
	contexto_de_ejecucion->registro_cx = pcb_proceso_a_ejecutar->registro_cx;
	contexto_de_ejecucion->registro_dx = pcb_proceso_a_ejecutar->registro_dx;

	t_paquete *paquete_solicitud_ejecutar_proceso = crear_paquete_solicitud_ejecutar_proceso(logger, contexto_de_ejecucion);
	enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_solicitud_ejecutar_proceso, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);

	free(contexto_de_ejecucion);
}

void enviar_paquete_solicitud_interrumpir_ejecucion()
{
	t_paquete *paquete_solicitud_interrumpir_ejecucion = crear_paquete_solicitud_interrumpir_proceso(logger);
	enviar_paquete(logger, conexion_con_cpu_interrupt, paquete_solicitud_interrumpir_ejecucion, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);
}

void enviar_paquete_respuesta_devolver_proceso_por_ser_interrumpido()
{
	t_paquete *paquete_respuesta_devolver_proceso_por_ser_interrumpido = crear_paquete_respuesta_devolver_proceso_por_ser_interrumpido(logger);
	enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_ser_interrumpido, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
}

void enviar_paquete_respuesta_devolver_proceso_por_correcta_finalizacion()
{
	t_paquete *paquete_respuesta_devolver_proceso_por_correcta_finalizacion = crear_paquete_respuesta_devolver_proceso_por_correcta_finalizacion(logger);
	enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_correcta_finalizacion, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
}

t_contexto_de_ejecucion *recibir_paquete_de_cpu_dispatch(op_code *codigo_operacion_recibido)
{
	pthread_mutex_lock(&mutex_conexion_cpu_dispatch);

	*codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, conexion_con_cpu_dispatch);
	t_contexto_de_ejecucion *contexto_de_ejecucion;

	// Recibo de CPU
	if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO || *codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION)
	{
		contexto_de_ejecucion = leer_paquete_contexto_de_ejecucion(logger, conexion_con_cpu_dispatch, *codigo_operacion_recibido, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
	}

	// Envio OK a CPU
	if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO)
	{
		enviar_paquete_respuesta_devolver_proceso_por_ser_interrumpido();
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION)
	{
		enviar_paquete_respuesta_devolver_proceso_por_correcta_finalizacion();
	}

	pthread_mutex_unlock(&mutex_conexion_cpu_dispatch);

	return contexto_de_ejecucion;
}

bool recibir_operacion_de_cpu_dispatch(op_code codigo_operacion_esperado)
{
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, conexion_con_cpu_dispatch);
	return codigo_operacion_recibido == codigo_operacion_esperado;
}

bool recibir_operacion_de_cpu_interrupt(op_code codigo_operacion_esperado)
{
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, conexion_con_cpu_interrupt);
	return codigo_operacion_recibido == codigo_operacion_esperado;
}

void ejecutar_proceso_en_cpu(t_pcb *pcb_proceso_a_ejecutar)
{
	pthread_mutex_lock(&mutex_conexion_cpu_dispatch);
	enviar_paquete_solicitud_ejecutar_proceso(pcb_proceso_a_ejecutar);
	recibir_operacion_de_cpu_dispatch(RESPUESTA_EJECUTAR_PROCESO);
	pthread_mutex_unlock(&mutex_conexion_cpu_dispatch);
}

void interrumpir_proceso_en_cpu()
{
	pthread_mutex_lock(&mutex_conexion_cpu_interrupt);
	enviar_paquete_solicitud_interrumpir_ejecucion();
	recibir_operacion_de_cpu_interrupt(RESPUESTA_INTERRUMPIR_PROCESO);
	pthread_mutex_unlock(&mutex_conexion_cpu_interrupt);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* MEMORIA *///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
void enviar_paquete_iniciar_estructuras_de_proceso_en_memoria(t_pcb *pcb)
{
	t_proceso_memoria *proceso_memoria = malloc(sizeof(t_proceso_memoria));
	proceso_memoria->path = pcb->path;
	proceso_memoria->size = pcb->size;
	proceso_memoria->prioridad = pcb->prioridad;
	proceso_memoria->pid = pcb->pid;

	t_paquete *paquete_solicitud_iniciar_proceso_en_memoria = crear_paquete_solicitud_iniciar_proceso_en_memoria(logger, proceso_memoria);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_iniciar_proceso_en_memoria, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);

	free(proceso_memoria);
}

void enviar_paquete_destruir_estructuras_de_proceso_en_memoria(t_pcb *pcb)
{
	t_proceso_memoria *proceso_memoria = malloc(sizeof(t_proceso_memoria));
	proceso_memoria->path = pcb->path;
	proceso_memoria->size = pcb->size;
	proceso_memoria->prioridad = pcb->prioridad;
	proceso_memoria->pid = pcb->pid;

	t_paquete *paquete_solicitud_finalizar_proceso_en_memoria = crear_paquete_solicitud_finalizar_proceso_en_memoria(logger, proceso_memoria);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_finalizar_proceso_en_memoria, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);

	free(proceso_memoria);
}

bool recibir_operacion_de_memoria(op_code codigo_operacion_esperado)
{
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
	return codigo_operacion_recibido == codigo_operacion_esperado;
}

bool iniciar_estructuras_de_proceso_en_memoria(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_conexion_memoria);

	enviar_paquete_iniciar_estructuras_de_proceso_en_memoria(pcb);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
	bool resultado_iniciar_proceso_en_memoria = leer_paquete_respuesta_iniciar_proceso_en_memoria(logger, conexion_con_memoria);

	pthread_mutex_unlock(&mutex_conexion_memoria);

	return resultado_iniciar_proceso_en_memoria;
}

void destruir_estructuras_de_proceso_en_memoria(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_conexion_memoria);
	enviar_paquete_destruir_estructuras_de_proceso_en_memoria(pcb);
	recibir_operacion_de_memoria(RESPUESTA_FINALIZAR_PROCESO_MEMORIA);
	pthread_mutex_unlock(&mutex_conexion_memoria);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* UTILIDADES *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////

const char *nombre_estado_proceso(char codigo_estado_proceso)
{
	switch (codigo_estado_proceso)
	{
	case CODIGO_ESTADO_PROCESO_READY:
		return NOMBRE_ESTADO_PROCESO_READY;
	case CODIGO_ESTADO_PROCESO_NEW:
		return NOMBRE_ESTADO_PROCESO_NEW;
	case CODIGO_ESTADO_PROCESO_EXECUTING:
		return NOMBRE_ESTADO_PROCESO_EXECUTING;
	case CODIGO_ESTADO_PROCESO_BLOCKED:
		return NOMBRE_ESTADO_PROCESO_BLOCKED;
	case CODIGO_ESTADO_PROCESO_EXIT:
		return NOMBRE_ESTADO_PROCESO_EXIT;
	default:
		return "?";
	}
}

int obtener_nuevo_pid()
{
	++proximo_pid;
	return proximo_pid;
}

void loguear_cola_pcbs(t_queue *cola, const char *nombre_cola)
{
	// TO DO: a funcion thread safe
	pthread_mutex_lock(&mutex_cola_ready);

	t_list_iterator *iterador = list_iterator_create(cola->elements);

	char *pids = malloc(sizeof(char));
	strcpy(pids, "");

	while (list_iterator_has_next(iterador))
	{
		t_pcb *pcb = list_iterator_next(iterador);
		int cantidad_digitos_pid = floor(log10(abs(pcb->pid))) + 1;
		char pid_string[cantidad_digitos_pid + 1];
		sprintf(pid_string, "%d,", pcb->pid);
		int tamanio_anterior = strlen(pids);
		int tamanio_a_aumentar = strlen(pid_string);
		pids = realloc(pids, (tamanio_anterior + tamanio_a_aumentar) * sizeof(char));
		strcpy(pids + (tamanio_anterior) * sizeof(char), pid_string);
	}

	int tamanio_pids = strlen(pids);
	if (tamanio_pids > 0)
	{
		pids[tamanio_pids - 1] = '\0';
	}

	list_iterator_destroy(iterador);

	log_info(logger, "Cola %s %s: [%s]", nombre_cola, configuracion_kernel->algoritmo_planificacion, pids);
	free(pids);

	pthread_mutex_unlock(&mutex_cola_ready);
}

void imprimir_proceso_en_consola(t_pcb *pcb)
{
	printf("Proceso PID: %d - Estado %s\n", pcb->pid, nombre_estado_proceso(pcb->estado));
}

void listar_procesos()
{
	queue_iterate_thread_safe(cola_new, (void (*)(void *)) & imprimir_proceso_en_consola, &mutex_cola_new);
	queue_iterate_thread_safe(cola_ready, (void (*)(void *)) & imprimir_proceso_en_consola, &mutex_cola_ready);

	if (pcb_ejecutando != NULL)
	{
		imprimir_proceso_en_consola(pcb_ejecutando);
	}

	// TO DO: Listo procesos BLOQUEADOS
}

t_pcb *crear_pcb(char *path, int size, int prioridad)
{
	t_pcb *pcb = malloc(sizeof(t_pcb));

	pcb->pid = obtener_nuevo_pid();
	pcb->estado = CODIGO_ESTADO_PROCESO_DESCONOCIDO;
	pcb->program_counter = 1;
	pcb->registro_ax = 0;
	pcb->registro_bx = 0;
	pcb->registro_cx = 0;
	pcb->registro_dx = 0;
	pcb->prioridad = prioridad;
	pcb->path = path;
	pcb->size = size;

	return pcb;
}

void actualizar_pcb(t_pcb *pcb, t_contexto_de_ejecucion *contexto_de_ejecucion)
{
	pcb->program_counter = contexto_de_ejecucion->program_counter;
	pcb->registro_ax = contexto_de_ejecucion->registro_ax;
	pcb->registro_bx = contexto_de_ejecucion->registro_bx;
	pcb->registro_cx = contexto_de_ejecucion->registro_cx;
	pcb->registro_dx = contexto_de_ejecucion->registro_dx;
}