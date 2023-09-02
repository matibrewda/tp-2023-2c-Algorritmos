#include "kernel.h"

pthread_t hilo_consola;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	setbuf(stdout, NULL); // Why?

	t_log* logger = NULL;
	t_argumentos_kernel* argumentos_kernel = NULL;
	t_config_kernel* configuracion_kernel = NULL;
	int conexion_con_servidor = -1;
	t_paquete* paquete_para_servidor = NULL;

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_KERNEL, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_KERNEL);

	argumentos_kernel = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_kernel == NULL)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}

	configuracion_kernel = leer_configuracion(logger, argumentos_kernel -> ruta_archivo_configuracion);
	if (configuracion_kernel == NULL)
	{
		terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
		return EXIT_FAILURE;
	}

	// log_debug(logger, "Intentando conectar %s con %s ...", NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_SERVIDOR);

	// conexion_con_servidor = crear_socket_kernel(logger, configuracion_kernel -> ip_servidor, configuracion_kernel -> puerto_servidor, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_SERVIDOR);
	// if (conexion_con_servidor == -1)
	// {
	// 	terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
	// 	return EXIT_FAILURE;
	// }

	crear_hilo_consola(logger, configuracion_kernel);
	
	// // Logica principal
	// paquete_para_servidor = crear_paquete_para_servidor(logger);
	// log_info(logger, "Enviando señal para hacer quack a %s", NOMBRE_MODULO_SERVIDOR);
	// enviar_paquete(logger, conexion_con_servidor, paquete_para_servidor, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_SERVIDOR);
	// log_info(logger, "Exito en el envio de señal para hacer quack a %s", NOMBRE_MODULO_SERVIDOR);

	// int resultado_servidor = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_SERVIDOR, conexion_con_servidor);

	// log_info(logger, "Se recibio la operacion %d desde %s", resultado_servidor, NOMBRE_MODULO_SERVIDOR);

	while(true);

	terminar_kernel(logger, argumentos_kernel, configuracion_kernel, conexion_con_servidor, paquete_para_servidor);
	
	return EXIT_SUCCESS;
}

void terminar_kernel(t_log* logger, t_argumentos_kernel* argumentos_kernel, t_config_kernel* configuracion_kernel, int conexion_con_servidor, t_paquete* paquete_para_servidor)
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_KERNEL);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_kernel);
	destruir_configuracion(configuracion_kernel);

	if (conexion_con_servidor != -1)
	{
    	close(conexion_con_servidor);
	}

	//destruir_paquete(logger, paquete_para_servidor);
}

void crear_hilo_consola(t_log* logger, t_config_kernel* configuracion_kernel)
{
	t_argumentos_hilo_consola* argumentos_hilo_consola = malloc(sizeof(t_argumentos_hilo_consola));
	
	argumentos_hilo_consola -> logger = logger;
	argumentos_hilo_consola -> configuracion_kernel = configuracion_kernel;

    pthread_create(&hilo_consola, NULL , (void*) consola, (void*) argumentos_hilo_consola);
    pthread_detach(hilo_consola);
}

void consola(void* argumentos)
{
	t_argumentos_hilo_consola* argumentos_hilo_consola = (t_argumentos_hilo_consola*) argumentos;
	
	t_log* logger = argumentos_hilo_consola -> logger;
	t_config_kernel* configuracion_kernel = argumentos_hilo_consola -> configuracion_kernel;

	while (true)
	{
		char* valor_ingresado_por_teclado = NULL;

		do
		{
			valor_ingresado_por_teclado = readline("KERNEL> ");
		} 
		while (valor_ingresado_por_teclado == NULL);

		log_trace(logger, "Valor ingresado por consola: %s", valor_ingresado_por_teclado);

		char* saveptr = valor_ingresado_por_teclado;
		char* funcion_seleccionada = strtok_r(saveptr, " ", &saveptr);

		if (strcmp(funcion_seleccionada, INICIAR_PROCESO) == 0) 
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);

			char* path;
			char* size_str;
			int size;
			char* prioridad_str;
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
		}
		else if (strcmp(funcion_seleccionada, FINALIZAR_PROCESO) == 0) 
		{
			log_trace(logger, "Se recibio la funcion %s por consola", funcion_seleccionada);

			char* pid_str;
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

			char* nuevo_grado_multiprogramacion_str;
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