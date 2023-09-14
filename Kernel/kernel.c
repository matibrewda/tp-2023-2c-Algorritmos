#include "kernel.h"

pthread_t hilo_planificador_largo_plazo;
pthread_t hilo_planificador_corto_plazo;
t_log *logger = NULL;
t_argumentos_kernel *argumentos_kernel = NULL;
t_config_kernel *configuracion_kernel = NULL;
t_colas_planificacion *colas_planificacion = NULL;
int conexion_con_cpu_dispatch = -1;
int conexion_con_cpu_interrupt = -1;
int conexion_con_memoria = -1;
int conexion_con_filesystem = -1;
sem_t semaforo_grado_max_multiprogramacion;
sem_t semaforo_cantidad_procesos_en_new;
t_list *pcbs;
bool planificacion_detenida = false;
int proximo_pid = 0;

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

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_KERNEL);

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
	sem_init(&semaforo_cantidad_procesos_en_new, false, 0);

	// Colas de planificacion
	colas_planificacion = malloc(sizeof(t_colas_planificacion));

	colas_planificacion->cola_new = queue_create();
	colas_planificacion->cola_ready = queue_create();
	colas_planificacion->cola_executing = queue_create();

	// Listas
	pcbs = list_create();

	// Hilos
	pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);
	pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo, NULL);

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
	log_trace(logger, "Planificador de largo plazo inicializado con exito.");

	while (true)
	{
		log_trace(logger, "Planificador de largo plazo: esperando algun proceso en NEW...");
		sem_wait(&semaforo_cantidad_procesos_en_new);
		log_trace(logger, "Planificador de largo plazo: esperando grado maximo de multiprogramacion..");
		sem_wait(&semaforo_grado_max_multiprogramacion);

		if (!queue_is_empty(colas_planificacion->cola_new))
		{
			t_pcb *pcb = queue_pop(colas_planificacion->cola_new);
			pcb->estado = 'R';
			queue_push(colas_planificacion->cola_ready, pcb);
		}
	}
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
	pcb->program_counter = 3;
	pcb->registro_ax = 4;
	pcb->registro_bx = 5;
	pcb->registro_cx = 6;
	pcb->registro_dx = 7;

	// list_add(pcbs, pcb);
	// queue_push(colas_planificacion->cola_new, pcb);
	// sem_post(&semaforo_cantidad_procesos_en_new);

	ejecutar_proceso_en_cpu(pcb);

	log_info(logger, "Se crea el proceso %d en NEW", pcb->pid);
}

void finalizar_proceso(int pid)
{
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
	interrumpir_proceso_en_cpu();

	if (planificacion_detenida)
	{
		printf("ERROR: La planificacion ya se encuentra detenida.\n");
		return;
	}

	log_info(logger, "PAUSA DE PLANIFICACIÓN");
}

void listar_procesos()
{
	if (list_is_empty(pcbs))
	{
		printf("No hay procesos que listar.\n");
		return;
	}

	t_list_iterator *iterador = list_iterator_create(pcbs);

	while (list_iterator_has_next(iterador))
	{
		t_pcb *pcb = list_iterator_next(iterador);
		printf("Proceso PID: %d esta en estado %c\n", pcb->pid, pcb->estado);
	}

	list_iterator_destroy(iterador);
}

void modificar_grado_max_multiprogramacion(int grado_multiprogramacion)
{
}

void *planificador_corto_plazo()
{
	log_trace(logger, "Soy el planificador de corto plazo");

	while (true)
	{
		esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
	}
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

// No bloquea!
void ejecutar_proceso_en_cpu(t_pcb *pcb_proceso_a_ejecutar)
{
	t_contexto_de_ejecucion *contexto_de_ejecucion = malloc(sizeof(t_contexto_de_ejecucion));
	contexto_de_ejecucion->program_counter=pcb_proceso_a_ejecutar->program_counter;
	contexto_de_ejecucion->registro_ax=pcb_proceso_a_ejecutar->registro_ax;
	contexto_de_ejecucion->registro_bx=pcb_proceso_a_ejecutar->registro_bx;
	contexto_de_ejecucion->registro_cx=pcb_proceso_a_ejecutar->registro_cx;
	contexto_de_ejecucion->registro_dx=pcb_proceso_a_ejecutar->registro_dx;

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