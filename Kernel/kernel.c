#include "kernel.h"

// Variables globales
t_log *logger = NULL;
t_argumentos_kernel *argumentos_kernel = NULL;
t_config_kernel *configuracion_kernel = NULL;
t_list *pcbs;
t_pcb *pcb_a_ejecutar = NULL;
t_pcb *pcb_ejecutando = NULL;
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
sem_t semaforo_se_creo_un_proceso;
sem_t semaforo_se_agrego_proceso_en_cola_ready;
sem_t semaforo_se_agrego_proceso_para_ejecutar;
sem_t semaforo_cpu;
sem_t semaforo_planificado_largo_plazo;
sem_t semaforo_planificado_corto_plazo;
pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;

// Colas de planificacion
t_queue *cola_new = NULL;
t_queue *cola_ready = NULL;

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
	sem_init(&semaforo_se_creo_un_proceso, false, 0);
	sem_init(&semaforo_se_agrego_proceso_en_cola_ready, false, 0);
	sem_init(&semaforo_se_agrego_proceso_para_ejecutar, false, 0);
	sem_init(&semaforo_cpu, false, 1); // inicialmente la CPU NO esta ocupada

	// Colas de planificacion
	cola_new = queue_create();
	cola_ready = queue_create();

	// Listas
	pcbs = list_create();

	// Hilos
	pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);
	pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo, NULL);
	pthread_create(&hilo_dispatcher, NULL, dispatcher, NULL);
	pthread_create(&hilo_escucha_cpu, NULL, escuchador_cpu, NULL);

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

void *planificador_largo_plazo()
{
	while (true)
	{
		sem_wait(&semaforo_se_creo_un_proceso);

		pthread_mutex_lock(&mutex_cola_new);
		t_pcb *pcb = queue_pop(cola_new);
		pthread_mutex_unlock(&mutex_cola_new);

		pcb->estado = 'R';

		sem_wait(&semaforo_grado_max_multiprogramacion);

		pthread_mutex_lock(&mutex_cola_ready);
		queue_push(cola_ready, pcb);
		pthread_mutex_unlock(&mutex_cola_ready);

		log_info(logger, "PID: %d - Estado Anterior: 'NEW' - Estado Actual: 'READY'", pcb->pid);
		loguear_cola_pcbs(cola_ready, "ready");

		sem_post(&semaforo_se_agrego_proceso_en_cola_ready);
	}
}

void loguear_cola_pcbs(t_queue *cola, const char *nombre_cola)
{
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

int obtener_nuevo_pid()
{
	++proximo_pid;
	return proximo_pid;
}

void crear_proceso(char *path, int size, int prioridad)
{
	t_pcb *pcb = malloc(sizeof(t_pcb));

	pcb->pid = obtener_nuevo_pid();
	pcb->estado = 'N';
	pcb->program_counter = 1;
	pcb->registro_ax = 0;
	pcb->registro_bx = 0;
	pcb->registro_cx = 0;
	pcb->registro_dx = 0;
	pcb->prioridad = prioridad;
	pcb->path = path;
	pcb->size = size;

	pthread_mutex_lock(&mutex_cola_new);
	queue_push(cola_new, pcb);
	pthread_mutex_unlock(&mutex_cola_new);

	iniciar_estructuras_de_proceso_en_memoria(logger, pcb->path, pcb->size, pcb->prioridad, pcb->pid);

	sleep(3);

	sem_post(&semaforo_se_creo_un_proceso);

	log_info(logger, "Se crea el proceso %d en NEW", pcb->pid);
}

void finalizar_proceso(int pid)
{
	// aca me tengo que fijar en que estado estaba el proceso antes!
	log_info(logger, "Finaliza el proceso %d - Motivo: SUCCESS", pid);

	// TO DO: finalizar estructuras de proceso en memoria
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

void listar_procesos()
{
	// Listo procesos en NEW
	pthread_mutex_lock(&mutex_cola_new);
	t_list_iterator *iterador_new = list_iterator_create(cola_new->elements);
	while (list_iterator_has_next(iterador_new))
	{
		t_pcb *pcb_new = list_iterator_next(iterador_new);
		printf("Proceso PID: %d - Estado NEW\n", pcb_new->pid);
	}
	list_iterator_destroy(iterador_new);
	pthread_mutex_unlock(&mutex_cola_new);

	// Listo procesos en READY
	pthread_mutex_lock(&mutex_cola_ready);
	t_list_iterator *iterador_ready = list_iterator_create(cola_ready->elements);
	while (list_iterator_has_next(iterador_ready))
	{
		t_pcb *pcb_ready = list_iterator_next(iterador_ready);
		printf("Proceso PID: %d - Estado READY\n", pcb_ready->pid);
	}
	list_iterator_destroy(iterador_ready);
	pthread_mutex_unlock(&mutex_cola_ready);

	// Listo proceso ejecutando (si es que lo hay)
	if (pcb_ejecutando != NULL)
	{
		printf("Proceso PID: %d - Estado EXEC\n", pcb_ejecutando->pid);
	}

	// Listo procesos BLOQUEADOS
	// TO DO
}

void modificar_grado_max_multiprogramacion(int grado_multiprogramacion)
{
}

void *planificador_corto_plazo()
{
	while (true)
	{
		fifo();
	}
}

void fifo()
{
	// Espero a que la CPU este libre
	sem_wait(&semaforo_cpu);

	// Si la CPU esta libre, saco un proceso de la cola de ready y lo mando a ejecutar
	sem_wait(&semaforo_se_agrego_proceso_en_cola_ready);

	pthread_mutex_lock(&mutex_cola_ready);
	t_pcb *pcb = queue_pop(cola_ready);
	pthread_mutex_unlock(&mutex_cola_ready);

	pcb_a_ejecutar = pcb;
	log_info(logger, "PID: %d - Estado Anterior: 'READY' - Estado Actual: 'EXEC'", pcb->pid);
	sem_post(&semaforo_se_agrego_proceso_para_ejecutar);
}

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

			crear_proceso(path, size, prioridad);
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
		else if (strcmp(funcion_seleccionada, "EXIT") == 0)
		{
			finalizar = true;
		}
		else
		{
			log_error(logger, "'%s' no coincide con ninguna funcion conocida.", funcion_seleccionada);
			printf("'%s' no coincide con ninguna funcion conocida.\n", funcion_seleccionada);
		}
	}
}

void *dispatcher()
{
	while (true)
	{
		log_trace(logger, "Soy el dispatcher");

		sem_wait(&semaforo_se_agrego_proceso_para_ejecutar);
		ejecutar_proceso_en_cpu(pcb_a_ejecutar);
		pcb_ejecutando = pcb_a_ejecutar;
		pcb_a_ejecutar = NULL;
	}
}

void *escuchador_cpu()
{
	while (true)
	{
		// Bloqueado hasta que reciba una operacion desde la conexion dispatch.
		op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, conexion_con_cpu_dispatch);

		if (codigo_operacion_recibido == DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION)
		{
			log_trace(logger, "Se recibio una orden de %s en %s avisando que termino un proceso por correcta finalizacion.", NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
			t_contexto_de_ejecucion *contexto_de_ejecucion = leer_paquete_contexto_de_ejecucion(logger, conexion_con_cpu_dispatch, DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);

			log_info(logger, "PID: %d - Estado Anterior: 'EXEC' - Estado Actual: 'EXIT'", contexto_de_ejecucion->pid);

			finalizar_proceso(contexto_de_ejecucion->pid);
			sem_post(&semaforo_grado_max_multiprogramacion);

			// Aviso que se desocupo la CPU
			sem_post(&semaforo_cpu);
			pcb_ejecutando = NULL;
		}
		else
		{
			log_trace(logger, "Se recibio una orden desconocida de %s en %s.", NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
		}
	}
}

// No bloquea!
void ejecutar_proceso_en_cpu(t_pcb *pcb_proceso_a_ejecutar)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = malloc(sizeof(t_contexto_de_ejecucion));
	contexto_de_ejecucion->pid = pcb_proceso_a_ejecutar->pid;
	contexto_de_ejecucion->program_counter = pcb_proceso_a_ejecutar->program_counter;
	contexto_de_ejecucion->registro_ax = pcb_proceso_a_ejecutar->registro_ax;
	contexto_de_ejecucion->registro_bx = pcb_proceso_a_ejecutar->registro_bx;
	contexto_de_ejecucion->registro_cx = pcb_proceso_a_ejecutar->registro_cx;
	contexto_de_ejecucion->registro_dx = pcb_proceso_a_ejecutar->registro_dx;

	t_paquete *paquete_ejecutar_proceso_en_cpu = crear_paquete_ejecutar_proceso(logger, contexto_de_ejecucion);
	enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_ejecutar_proceso_en_cpu, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);

	free(contexto_de_ejecucion);
}

// No bloquea!
void interrumpir_proceso_en_cpu()
{
	t_paquete *paquete_interrumpir_proceso_en_cpu = crear_paquete_interrumpir_ejecucion(logger);
	enviar_paquete(logger, conexion_con_cpu_interrupt, paquete_interrumpir_proceso_en_cpu, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);
}

void iniciar_estructuras_de_proceso_en_memoria(t_log *logger, char *path, int size, int prioridad, int pid)
{
	log_debug(logger, "Comenzando la creacion de paquete para enviar iniciar proceso a memoria!");
	t_proceso_memoria *proceso_memoria = malloc(sizeof(t_proceso_memoria));
	proceso_memoria->path = path;
	proceso_memoria->size = size;
	proceso_memoria->prioridad = prioridad;
	proceso_memoria->pid = pid;
	t_paquete *paquete = crear_paquete_iniciar_proceso_en_memoria(logger, proceso_memoria);

	enviar_paquete(logger, conexion_con_memoria, paquete, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
	log_debug(logger, "Exito en el envio de paquete para iniciar proceso a memoria!");

	free(proceso_memoria);
}