#include "kernel.h"

pthread_t hilo_consola;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	setbuf(stdout, NULL); // Why? Era algo de consola esto.

	t_log *logger = NULL;
	t_argumentos_kernel *argumentos_kernel = NULL;
	t_config_kernel *configuracion_kernel = NULL;
	int conexion_con_cpu_dispatch = -1;
	int conexion_con_cpu_interrupt = -1;
	int conexion_con_memoria = -1;
	int conexion_con_filesystem = -1;

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_KERNEL, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_cpu_dispatch, conexion_con_cpu_interrupt, conexion_con_memoria, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_KERNEL);

	argumentos_kernel = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_kernel == NULL)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_cpu_dispatch, conexion_con_cpu_interrupt, conexion_con_memoria, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	configuracion_kernel = leer_configuracion(logger, argumentos_kernel->ruta_archivo_configuracion);
	if (configuracion_kernel == NULL)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_cpu_dispatch, conexion_con_cpu_interrupt, conexion_con_memoria, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	conexion_con_cpu_dispatch = crear_socket_cliente(logger, configuracion_kernel->ip_cpu, configuracion_kernel->puerto_cpu_dispatch, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	if (conexion_con_cpu_dispatch == -1)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_cpu_dispatch, conexion_con_cpu_interrupt, conexion_con_memoria, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	conexion_con_cpu_interrupt = crear_socket_cliente(logger, configuracion_kernel->ip_cpu, configuracion_kernel->puerto_cpu_interrupt, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);
	if (conexion_con_cpu_interrupt == -1)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_cpu_dispatch, conexion_con_cpu_interrupt, conexion_con_memoria, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	conexion_con_memoria = crear_socket_cliente(logger, configuracion_kernel->ip_cpu, configuracion_kernel->puerto_memoria, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
	if (conexion_con_memoria == -1)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_cpu_dispatch, conexion_con_cpu_interrupt, conexion_con_memoria, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	conexion_con_filesystem = crear_socket_cliente(logger, configuracion_kernel->ip_cpu, configuracion_kernel->puerto_filesystem, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);
	if (conexion_con_filesystem == -1)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_cpu_dispatch, conexion_con_cpu_interrupt, conexion_con_memoria, conexion_con_filesystem);
		return EXIT_FAILURE;
	}

	crear_hilo_consola(logger, configuracion_kernel,conexion_con_memoria);

	// Logica principal
	log_info(logger, "Mando señal a %s", NOMBRE_MODULO_CPU_DISPATCH);
	enviar_operacion_sin_paquete(logger, conexion_con_cpu_dispatch, MENSAJE_DE_KERNEL, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	int resultado_cpu_dispatch = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, conexion_con_cpu_dispatch);
	log_info(logger, "Se recibio la operacion %d desde %s", resultado_cpu_dispatch, NOMBRE_MODULO_CPU_DISPATCH);

	log_info(logger, "Mando señal a %s", NOMBRE_MODULO_CPU_INTERRUPT);
	enviar_operacion_sin_paquete(logger, conexion_con_cpu_interrupt, MENSAJE_DE_KERNEL, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);
	int resultado_cpu_interrupt = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT, conexion_con_cpu_interrupt);
	log_info(logger, "Se recibio la operacion %d desde %s", resultado_cpu_interrupt, NOMBRE_MODULO_CPU_INTERRUPT);

	log_info(logger, "Mando señal a %s", NOMBRE_MODULO_FILESYSTEM);
	enviar_operacion_sin_paquete(logger, conexion_con_filesystem, MENSAJE_DE_KERNEL, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);
	int resultado_filesystem = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem);
	log_info(logger, "Se recibio la operacion %d desde %s", resultado_filesystem, NOMBRE_MODULO_FILESYSTEM);

	log_info(logger, "Mando señal a %s", NOMBRE_MODULO_MEMORIA);
	enviar_operacion_sin_paquete(logger, conexion_con_memoria, MENSAJE_DE_KERNEL, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
	int resultado_memoria = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
	log_info(logger, "Se recibio la operacion %d desde %s", resultado_memoria, NOMBRE_MODULO_MEMORIA);

	// Finalizacion
	terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_cpu_dispatch, conexion_con_cpu_interrupt, conexion_con_memoria, conexion_con_filesystem);
	return EXIT_SUCCESS;
}

void terminar_kernel(t_log *logger, t_argumentos_kernel *argumentos_kernel, t_config_kernel *configuracion_kernel, int conexion_con_cpu_dispatch, int conexion_con_cpu_interrupt, int conexion_con_memoria, int conexion_con_filesystem)
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
}

void crear_hilo_consola(t_log *logger, t_config_kernel *configuracion_kernel,int *conexion_con_memoria)
{
	t_argumentos_hilo_consola *argumentos_hilo_consola = malloc(sizeof(t_argumentos_hilo_consola));

	argumentos_hilo_consola->logger = logger;
	argumentos_hilo_consola->configuracion_kernel = configuracion_kernel;

	pthread_create(&hilo_consola, NULL, (void *)consola, (void *)argumentos_hilo_consola);
	pthread_detach(hilo_consola);
}

void consola(void *argumentos)
{
	t_argumentos_hilo_consola *argumentos_hilo_consola = (t_argumentos_hilo_consola *)argumentos;

	t_log *logger = argumentos_hilo_consola->logger;
	t_config_kernel *configuracion_kernel = argumentos_hilo_consola->configuracion_kernel;
	int conexion_con_memoria = argumentos_hilo_consola->conexion_con_memoria;

	while (true)
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

			// LOGICA DE INICIAR_PROCESO...
			enviar_inciar_proceso_memoria(logger,path,size,prioridad,conexion_con_memoria);
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

			// LOGICA DE FINALIZAR_PROCESO...
		}
		else if (strcmp(funcion_seleccionada, DETENER_PLANIFICACION) == 0)
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);

			// LOGICA DE DETENER_PLANIFICACION...
		}
		else if (strcmp(funcion_seleccionada, INICIAR_PLANIFICACION) == 0)
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);

			// LOGICA DE INICIAR_PLANIFICACION...
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

			// LOGICA DE MULTIPROGRAMACION...
		}
		else if (strcmp(funcion_seleccionada, PROCESO_ESTADO) == 0)
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);

			// LOGICA DE PROCESO_ESTADO...
		}
		else
		{
			log_error(logger, "'%s' no coincide con ninguna funcion conocida.", funcion_seleccionada);
			printf("'%s' no coincide con ninguna funcion conocida.\n", funcion_seleccionada);
		}
	}
}

void enviar_inciar_proceso_memoria(t_log *logger,char *path,int size,int prioridad,int conexion_con_memoria){
	log_debug(logger, "Comenzando la creacion de paquete para enviar iniciar proceso a memoria!");
	t_paquete *paquete = crear_paquete(logger, INICIAR_PROCESO);

	agregar_string_a_paquete(logger,paquete,path,NOMBRE_MODULO_KERNEL,NOMBRE_MODULO_MEMORIA,INICIAR_PROCESO);
	agregar_int_a_paquete(logger,paquete,size,NOMBRE_MODULO_KERNEL,NOMBRE_MODULO_MEMORIA,INICIAR_PROCESO);
	agregar_int_a_paquete(logger,paquete,prioridad,NOMBRE_MODULO_KERNEL,NOMBRE_MODULO_MEMORIA,INICIAR_PROCESO);
	log_debug(logger, "Exito en la creacion de paquete para enviar iniciar proceso a memoria!");

	enviar_paquete(logger, conexion_con_memoria, paquete,NOMBRE_MODULO_KERNEL,NOMBRE_MODULO_MEMORIA);
	log_debug(logger, "Exito en el envio de paquete para iniciar proceso a memoria!");
	destruir_paquete(logger, paquete);

}