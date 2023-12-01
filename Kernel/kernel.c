#include "kernel.h"

// Variables globales
t_log *logger = NULL;
t_argumentos_kernel *argumentos_kernel = NULL;
t_config_kernel *configuracion_kernel = NULL;
int proximo_pid = 0;
bool planificacion_detenida = false;
char *aux_pids_cola;
bool planifico_con_round_robin = false;
bool planifico_con_prioridades = false;
int id_hilo_quantum = 0;

// Recursos
t_list *recursos;

// Conexiones
int conexion_con_cpu_dispatch = -1;
int conexion_con_cpu_interrupt = -1;
int conexion_con_memoria = -1;
int conexion_con_filesystem = -1;

// Hilos
pthread_t hilo_planificador_largo_plazo;
pthread_t hilo_planificador_corto_plazo;

// Semaforos
sem_t semaforo_grado_max_multiprogramacion;
sem_t semaforo_hay_algun_proceso_en_cola_new;
sem_t semaforo_hay_algun_proceso_en_cola_ready;
sem_t semaforo_planificador_corto_plazo;
sem_t semaforo_planificador_largo_plazo;

pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_cola_bloqueados_sleep;
pthread_mutex_t mutex_cola_bloqueados_pagefault;
pthread_mutex_t mutex_conexion_cpu_dispatch;
pthread_mutex_t mutex_conexion_cpu_interrupt;
pthread_mutex_t mutex_conexion_memoria;
pthread_mutex_t mutex_conexion_filesystem;
pthread_mutex_t mutex_id_hilo_quantum;

// Colas de planificacion
t_queue *cola_new = NULL;
t_queue *cola_ready = NULL;
t_queue *cola_bloqueados_sleep = NULL;
t_queue *cola_bloqueados_pagefault = NULL;
t_pcb *pcb_ejecutando = NULL;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	setbuf(stdout, NULL); // Why? Era algo de consola esto.
	atexit(terminar_kernel);

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

	conexion_con_memoria = crear_socket_cliente(logger, configuracion_kernel->ip_memoria, configuracion_kernel->puerto_memoria, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
	if (conexion_con_memoria == -1)
	{
		terminar_kernel();
		return EXIT_FAILURE;
	}

	conexion_con_filesystem = crear_socket_cliente(logger, configuracion_kernel->ip_filesystem, configuracion_kernel->puerto_filesystem, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);
	if (conexion_con_filesystem == -1)
	{
		terminar_kernel();
		return EXIT_FAILURE;
	}

	// Recursos
	crear_recursos();

	// Semaforos
	sem_init(&semaforo_grado_max_multiprogramacion, false, configuracion_kernel->grado_multiprogramacion_inicial);
	sem_init(&semaforo_hay_algun_proceso_en_cola_new, false, 0);
	sem_init(&semaforo_hay_algun_proceso_en_cola_ready, false, 0);

	if (planificacion_detenida)
	{
		sem_init(&semaforo_planificador_corto_plazo, false, 0);
		sem_init(&semaforo_planificador_largo_plazo, false, 0);
	}
	else
	{
		sem_init(&semaforo_planificador_corto_plazo, false, 1);
		sem_init(&semaforo_planificador_largo_plazo, false, 1);
	}

	pthread_mutex_init(&mutex_cola_new, NULL);
	pthread_mutex_init(&mutex_cola_ready, NULL);
	pthread_mutex_init(&mutex_cola_bloqueados_sleep, NULL);
	pthread_mutex_init(&mutex_cola_bloqueados_pagefault, NULL);
	pthread_mutex_init(&mutex_conexion_cpu_dispatch, NULL);
	pthread_mutex_init(&mutex_conexion_cpu_interrupt, NULL);
	pthread_mutex_init(&mutex_conexion_memoria, NULL);
	pthread_mutex_init(&mutex_conexion_filesystem, NULL);
	pthread_mutex_init(&mutex_id_hilo_quantum, NULL);

	// Colas de planificacion
	cola_new = queue_create();
	cola_ready = queue_create();
	cola_bloqueados_sleep = queue_create();
	cola_bloqueados_pagefault = queue_create();

	// Hilos
	pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);
	pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo, NULL);

	planifico_con_round_robin = strcmp(configuracion_kernel->algoritmo_planificacion, ALGORITMO_PLANIFICACION_ROUND_ROBIN) == 0;
	planifico_con_prioridades = strcmp(configuracion_kernel->algoritmo_planificacion, ALGORITMO_PLANIFICACION_PRIORIDADES) == 0;

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
		log_warning(logger, "Algo salio mal!");
		log_warning(logger, "Finalizando %s", NOMBRE_MODULO_KERNEL);
	}

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

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* PLANIFICADORES *////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////

void *planificador_largo_plazo()
{
	while (true)
	{
		sem_wait(&semaforo_hay_algun_proceso_en_cola_new);
		sem_wait(&semaforo_grado_max_multiprogramacion);
		sem_wait(&semaforo_planificador_largo_plazo);

		t_pcb *pcb = queue_pop_thread_safe(cola_new, &mutex_cola_new);

		if (pcb != NULL)
		{
			transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_READY);
		}

		if (!planificacion_detenida)
		{
			sem_post(&semaforo_planificador_largo_plazo);
		}
	}
}

void *planificador_corto_plazo()
{
	while (true)
	{
		sem_wait(&semaforo_hay_algun_proceso_en_cola_ready);
		t_pcb *pcb = queue_pop_thread_safe(cola_ready, &mutex_cola_ready);
		bool mantener_proceso_ejecutando = false;

		if (pcb != NULL)
		{
			while (mantener_proceso_ejecutando)
			{
				mantener_proceso_ejecutando = false;
				transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXECUTING);

				op_code codigo_operacion_recibido;
				int tiempo_sleep = -1;
				int motivo_interrupcion = -1;
				int codigo_error = -1;
				int numero_pagina = -1;
				int posicion_puntero_archivo = -1;
				int direccion_fisica = -1;
				int nuevo_tamanio_archivo = -1;
				int fs_opcode = -1;
				char *nombre_recurso = NULL;
				char *nombre_archivo = NULL;
				char *modo_apertura = NULL;

				t_contexto_de_ejecucion *contexto_de_ejecucion = recibir_paquete_de_cpu_dispatch(&codigo_operacion_recibido, &tiempo_sleep, &motivo_interrupcion, &nombre_recurso, &codigo_error, &numero_pagina, &nombre_archivo, &modo_apertura, &posicion_puntero_archivo, &direccion_fisica, &nuevo_tamanio_archivo, &fs_opcode);
				actualizar_pcb(pcb, contexto_de_ejecucion);

				sem_wait(&semaforo_planificador_corto_plazo);

				if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION)
				{
					transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
				}
				else if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_ERROR)
				{
					log_error(logger, "Ocurrio un error de codigo %d en el proceso PID %d.", codigo_error, pcb->pid);
					transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
				}
				else if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO)
				{
					if (motivo_interrupcion == INTERRUPCION_POR_DESALOJO)
					{
						transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_READY);
					}
					else if (motivo_interrupcion == INTERRUPCION_POR_KILL)
					{
						transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
					}
				}
				else if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM)
				{
					if (fs_opcode == FOPEN_OPCODE)
					{
						// TODO
					}
					else if (fs_opcode == FCLOSE_OPCODE)
					{
						// TODO
					}
					else if (fs_opcode == FSEEK_OPCODE)
					{
						// TODO
					}
					else if (fs_opcode == FTRUNCATE_OPCODE)
					{
						// TODO
					}
					else if (fs_opcode == FWRITE_OPCODE)
					{
						// TODO
					}
					else if (fs_opcode == FREAD_OPCODE)
					{
						// TODO
					}
				}
				else if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP)
				{
					transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_BLOCKED);
					queue_push_thread_safe(cola_bloqueados_sleep, pcb, &mutex_cola_bloqueados_sleep);
					crear_hilo_sleep(pcb, tiempo_sleep);
				}
				else if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT)
				{
					transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_BLOCKED);
					queue_push_thread_safe(cola_bloqueados_pagefault, pcb, &mutex_cola_bloqueados_pagefault);
					crear_hilo_page_fault(pcb, numero_pagina);
				}
				else if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_WAIT)
				{
					if (!recurso_existe(nombre_recurso))
					{
						log_warning(logger, "PID: %d hace WAIT del recurso inexistente %s!", pcb->pid, nombre_recurso);
						finalizar_proceso(pcb->pid);
					}
					else
					{
						t_recurso *recurso_para_wait = buscar_recurso_por_nombre(nombre_recurso);
						recurso_para_wait->instancias_disponibles--;
						log_info(logger, "PID: %d - Wait: %s - Instancias: %d", pcb->pid, nombre_recurso, recurso_para_wait->instancias_disponibles);
						list_add_thread_safe(recurso_para_wait->pcbs_asignados, pcb, &recurso_para_wait->mutex_pcbs_asignados);
						if (recurso_para_wait->instancias_disponibles < 0)
						{
							transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_BLOCKED);
							queue_push_thread_safe(recurso_para_wait->pcbs_bloqueados, pcb, &recurso_para_wait->mutex_pcbs_bloqueados);
							log_info(logger, "PID: %d - Bloqueado por: %s", pcb->pid, nombre_recurso);
						}
						else
						{
							mantener_proceso_ejecutando = true;
						}
					}
				}
				else if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL)
				{
					if (!recurso_existe(nombre_recurso))
					{
						log_warning(logger, "PID: %d hace SIGNAL del recurso inexistente %s!", pcb->pid, nombre_recurso);
						finalizar_proceso(pcb->pid);
					}
					else if (!recurso_esta_asignado_a_pcb(nombre_recurso, pcb->pid))
					{
						log_warning(logger, "PID: %d hace SIGNAL del recurso %s que NO tenia asignado previamente!", pcb->pid, nombre_recurso);
						finalizar_proceso(pcb->pid);
					}
					else
					{
						t_recurso *recurso_para_signal = buscar_recurso_por_nombre(nombre_recurso);
						log_info(logger, "PID: %d - Signal: %s - Instancias: %d", pcb->pid, nombre_recurso, recurso_para_signal->instancias_disponibles + 1);
						mantener_proceso_ejecutando = true;
						desasignar_recurso_a_pcb(nombre_recurso, pcb->pid);
					}
				}
			}
		}
		else
		{
			sem_wait(&semaforo_planificador_corto_plazo);
		}

		if (!planificacion_detenida)
		{
			sem_post(&semaforo_planificador_corto_plazo);
		}
	}
}

void *contador_quantum(void *id_hilo_quantum)
{
	int pid_proceso_a_interrumpir = pcb_ejecutando->pid;
	int id_hilo = *((int *)id_hilo_quantum);

	usleep((configuracion_kernel->quantum) * 1000);

	if (pcb_ejecutando != NULL && pcb_ejecutando->pid == pid_proceso_a_interrumpir && pcb_ejecutando->id_hilo_quantum == id_hilo)
	{
		pcb_ejecutando->quantum_finalizado = true;
		if (!queue_is_empty_thread_safe(cola_ready, &mutex_cola_ready))
		{
			interrumpir_proceso_en_cpu(INTERRUPCION_POR_DESALOJO);
			log_info(logger, "PID: %d - Desalojado por fin de Quantum", pid_proceso_a_interrumpir);
		}
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
		else if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_EXECUTING)
		{
			transicionar_proceso_de_executing_a_executing(pcb);
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
		log_error(logger, "Ocurrio un error al inicializar las estructuras en memoria del proceso %d", pcb->pid);
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
	pcb->estado = CODIGO_ESTADO_PROCESO_READY;
	push_cola_ready(pcb);
}

void transicionar_proceso_de_new_a_exit(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXIT));
	log_info(logger, "Finaliza el proceso PID: %d - Motivo SUCCESS", pcb->pid);
	eliminar_pcb_de_cola(pcb->pid, cola_new, &mutex_cola_new);
	free(pcb);
}

void transicionar_proceso_de_ready_a_executing(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXECUTING));
	pcb_ejecutando = pcb;
	pcb->estado = CODIGO_ESTADO_PROCESO_EXECUTING;
	ejecutar_proceso_en_cpu(pcb);

	if (planifico_con_round_robin)
	{
		pthread_mutex_lock(&mutex_id_hilo_quantum);
		id_hilo_quantum++;
		pcb->id_hilo_quantum = id_hilo_quantum;
		pcb->quantum_finalizado = false;
		pthread_t hilo_contador_quantum;
		int *id_hilo_quantum_arg = malloc(sizeof(int));
		*id_hilo_quantum_arg = id_hilo_quantum;
		pthread_create(&hilo_contador_quantum, NULL, contador_quantum, (void *)id_hilo_quantum_arg);
		pthread_mutex_unlock(&mutex_id_hilo_quantum);
	}
}

void transicionar_proceso_de_ready_a_exit(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXIT));
	log_info(logger, "Finaliza el proceso PID: %d - Motivo SUCCESS", pcb->pid);
	desasignar_todos_los_recursos_a_pcb(pcb->pid);
	destruir_estructuras_de_proceso_en_memoria(pcb);
	eliminar_pcb_de_cola(pcb->pid, cola_ready, &mutex_cola_ready);
	free(pcb);
	sem_post(&semaforo_grado_max_multiprogramacion);
}

void transicionar_proceso_de_executing_a_ready(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_READY));
	pcb_ejecutando = NULL;
	pcb->estado = CODIGO_ESTADO_PROCESO_READY;
	push_cola_ready(pcb);
}

void transicionar_proceso_de_executing_a_bloqueado(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_BLOCKED));
	pcb_ejecutando = NULL;
	pcb->estado = CODIGO_ESTADO_PROCESO_BLOCKED;
}

void transicionar_proceso_de_executing_a_executing(t_pcb *pcb)
{
	pcb_ejecutando = pcb;
	ejecutar_proceso_en_cpu(pcb);
}

void transicionar_proceso_de_executing_a_exit(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXIT));
	log_info(logger, "Finaliza el proceso PID: %d - Motivo SUCCESS", pcb->pid);
	pcb_ejecutando = NULL;
	desasignar_todos_los_recursos_a_pcb(pcb->pid);
	destruir_estructuras_de_proceso_en_memoria(pcb);
	free(pcb);
	sem_post(&semaforo_grado_max_multiprogramacion);
}

void transicionar_proceso_de_bloqueado_a_ready(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_READY));
	pcb->estado = CODIGO_ESTADO_PROCESO_READY;
	push_cola_ready(pcb);
}

void transicionar_proceso_de_bloqueado_a_exit(t_pcb *pcb)
{
	log_info(logger, "PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXIT));
	log_info(logger, "Finaliza el proceso PID: %d - Motivo SUCCESS", pcb->pid);
	eliminar_pcb_de_cola(pcb->pid, cola_bloqueados_sleep, &mutex_cola_bloqueados_sleep);
	eliminar_pcb_de_cola(pcb->pid, cola_bloqueados_pagefault, &mutex_cola_bloqueados_pagefault);
	desasignar_todos_los_recursos_a_pcb(pcb->pid);
	destruir_estructuras_de_proceso_en_memoria(pcb);
	free(pcb);
	sem_post(&semaforo_grado_max_multiprogramacion);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* BLOQUEOS *//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
void crear_hilo_sleep(t_pcb *pcb, int tiempo_sleep)
{
	log_info(logger, "PID: %d - Bloqueado por: SLEEP", pcb->pid);
	pthread_t hilo_bloqueo_sleep;
	t_bloqueo_sleep *bloqueo_sleep_parametros = malloc(sizeof(t_bloqueo_sleep));
	bloqueo_sleep_parametros->pcb = pcb;
	bloqueo_sleep_parametros->tiempo_sleep = tiempo_sleep;
	pthread_create(&hilo_bloqueo_sleep, NULL, bloqueo_sleep, (void *)bloqueo_sleep_parametros);
}

void *bloqueo_sleep(void *argumentos)
{
	t_bloqueo_sleep *bloqueo_sleep = (t_bloqueo_sleep *)argumentos;

	int pid_proceso_bloqueado = bloqueo_sleep->pcb->pid;

	usleep((bloqueo_sleep->tiempo_sleep) * 1000);

	t_pcb *pcb = buscar_pcb_con_pid_en_cola(pid_proceso_bloqueado, cola_bloqueados_sleep, &mutex_cola_bloqueados_sleep);

	if (pcb != NULL)
	{
		eliminar_pcb_de_cola(pid_proceso_bloqueado, cola_bloqueados_sleep, &mutex_cola_bloqueados_sleep);
		transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_READY);
	}
}

void crear_hilo_page_fault(t_pcb *pcb, int numero_pagina)
{
	log_info(logger, "Page Fault PID: %d - Pagina: %d", pcb->pid, numero_pagina);
	pthread_t hilo_bloqueo_page_fault;
	t_bloqueo_page_fault *bloqueo_page_fault_parametros = malloc(sizeof(t_bloqueo_page_fault));
	bloqueo_page_fault_parametros->pcb = pcb;
	bloqueo_page_fault_parametros->numero_pagina = numero_pagina;
	pthread_create(&hilo_bloqueo_page_fault, NULL, page_fault, (void *)bloqueo_page_fault_parametros);
}

void *page_fault(void *argumentos)
{
	t_bloqueo_page_fault *bloqueo_page_fault = (t_bloqueo_page_fault *)argumentos;

	int pid_proceso_page_fault = bloqueo_page_fault->pcb->pid;

	bool exito = cargar_pagina_en_memoria(pid_proceso_page_fault, bloqueo_page_fault->numero_pagina);

	t_pcb *pcb = buscar_pcb_con_pid_en_cola(pid_proceso_page_fault, cola_bloqueados_pagefault, &mutex_cola_bloqueados_pagefault);

	if (pcb != NULL)
	{
		eliminar_pcb_de_cola(pid_proceso_page_fault, cola_bloqueados_pagefault, &mutex_cola_bloqueados_pagefault);

		if (exito)
		{
			transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_READY);
		}
		else
		{
			log_error(logger, "Error al cargar pagina %d en memoria para PID %d", bloqueo_page_fault->numero_pagina, pid_proceso_page_fault);
			transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
		}
	}
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

		add_history(valor_ingresado_por_teclado);

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
	t_pcb *pcb = buscar_pcb_con_pid(pid);

	if (pcb != NULL)
	{
		if (pcb->estado == CODIGO_ESTADO_PROCESO_EXECUTING)
		{
			interrumpir_proceso_en_cpu(INTERRUPCION_POR_KILL);
		}
		else
		{
			transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
		}
	}
}

void iniciar_planificacion()
{
	if (planificacion_detenida)
	{
		planificacion_detenida = false;
		sem_post(&semaforo_planificador_corto_plazo);
		sem_post(&semaforo_planificador_largo_plazo);
		log_info(logger, "INICIO DE PLANIFICACIÓN");
	}
}

void detener_planificacion()
{
	if (!planificacion_detenida)
	{
		planificacion_detenida = true;
		sem_wait(&semaforo_planificador_corto_plazo);
		sem_wait(&semaforo_planificador_largo_plazo);
		log_info(logger, "PAUSA DE PLANIFICACIÓN");
	}
}

void modificar_grado_max_multiprogramacion(int grado_multiprogramacion)
{
	// TO DO
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* CPU *///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
t_contexto_de_ejecucion *recibir_paquete_de_cpu_dispatch(op_code *codigo_operacion_recibido, int *tiempo_sleep, int *motivo_interrupcion, char **nombre_recurso, int *codigo_error, int *numero_pagina, char **nombre_archivo, char **modo_apertura, int *posicion_puntero_archivo, int *direccion_fisica, int *nuevo_tamanio_archivo, int *fs_opcode)
{
	pthread_mutex_lock(&mutex_conexion_cpu_dispatch);

	// Esperar operacion
	*codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, conexion_con_cpu_dispatch);
	t_contexto_de_ejecucion *contexto_de_ejecucion;

	// Responder a CPU
	if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION)
	{
		contexto_de_ejecucion = leer_paquete_contexto_de_ejecucion(logger, conexion_con_cpu_dispatch, *codigo_operacion_recibido, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
		t_paquete *paquete_respuesta_devolver_proceso_por_correcta_finalizacion = crear_paquete_respuesta_devolver_proceso_por_correcta_finalizacion(logger);
		enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_correcta_finalizacion, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_ser_interrumpido(logger, conexion_con_cpu_dispatch, motivo_interrupcion);
		t_paquete *paquete_respuesta_devolver_proceso_por_ser_interrumpido = crear_paquete_respuesta_devolver_proceso_por_ser_interrumpido(logger);
		enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_ser_interrumpido, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_sleep(logger, conexion_con_cpu_dispatch, tiempo_sleep);
		t_paquete *paquete_respuesta_devolver_proceso_por_sleep = crear_paquete_respuesta_devolver_proceso_por_sleep(logger);
		enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_sleep, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_WAIT)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_wait(logger, conexion_con_cpu_dispatch, nombre_recurso);
		t_paquete *paquete_respuesta_devolver_proceso_por_wait = crear_paquete_respuesta_devolver_proceso_por_wait(logger);
		enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_wait, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_signal(logger, conexion_con_cpu_dispatch, nombre_recurso);
		t_paquete *paquete_respuesta_devolver_proceso_por_signal = crear_paquete_respuesta_devolver_proceso_por_signal(logger);
		enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_signal, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_ERROR)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_error(logger, conexion_con_cpu_dispatch, codigo_error);
		t_paquete *paquete_respuesta_devolver_proceso_por_error = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_DEVOLVER_PROCESO_POR_ERROR, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
		enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_error, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_pagefault(logger, conexion_con_cpu_dispatch, numero_pagina);
		t_paquete *paquete_respuesta_devolver_proceso_por_pagefault = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_DEVOLVER_PROCESO_POR_PAGEFAULT, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
		enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_pagefault, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_operacion_filesystem(logger, conexion_con_cpu_dispatch, nombre_archivo, modo_apertura, posicion_puntero_archivo, direccion_fisica, nuevo_tamanio_archivo, fs_opcode);
		t_paquete *paquete_respuesta_devolver_proceso_por_operacion_filesystem = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
		enviar_paquete(logger, conexion_con_cpu_dispatch, paquete_respuesta_devolver_proceso_por_operacion_filesystem, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH);
	}

	pthread_mutex_unlock(&mutex_conexion_cpu_dispatch);

	return contexto_de_ejecucion;
}

void ejecutar_proceso_en_cpu(t_pcb *pcb_proceso_a_ejecutar)
{
	pthread_mutex_lock(&mutex_conexion_cpu_dispatch);

	// Enviar
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

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, conexion_con_cpu_dispatch); // RESPUESTA_EJECUTAR_PROCESO

	pthread_mutex_unlock(&mutex_conexion_cpu_dispatch);
}

void interrumpir_proceso_en_cpu(int motivo_interrupcion)
{
	pthread_mutex_lock(&mutex_conexion_cpu_interrupt);

	// Enviar
	t_paquete *paquete_solicitud_interrumpir_ejecucion = crear_paquete_solicitud_interrumpir_proceso(logger, motivo_interrupcion);
	enviar_paquete(logger, conexion_con_cpu_interrupt, paquete_solicitud_interrumpir_ejecucion, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, conexion_con_cpu_interrupt); // RESPUESTA_INTERRUMPIR_PROCESO

	pthread_mutex_unlock(&mutex_conexion_cpu_interrupt);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* MEMORIA *///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
bool iniciar_estructuras_de_proceso_en_memoria(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_conexion_memoria);

	// Enviar
	t_proceso_memoria *proceso_memoria = malloc(sizeof(t_proceso_memoria));
	proceso_memoria->path = pcb->path;
	proceso_memoria->size = pcb->size;
	proceso_memoria->prioridad = pcb->prioridad;
	proceso_memoria->pid = pcb->pid;
	t_paquete *paquete_solicitud_iniciar_proceso_en_memoria = crear_paquete_solicitud_iniciar_proceso_en_memoria(logger, proceso_memoria);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_iniciar_proceso_en_memoria, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
	free(proceso_memoria);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, conexion_con_memoria); // RESPUESTA_INICIAR_PROCESO_MEMORIA
	bool resultado_iniciar_proceso_en_memoria = leer_paquete_respuesta_iniciar_proceso_en_memoria(logger, conexion_con_memoria);

	pthread_mutex_unlock(&mutex_conexion_memoria);

	return resultado_iniciar_proceso_en_memoria;
}

void destruir_estructuras_de_proceso_en_memoria(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_conexion_memoria);

	// Enviar
	t_proceso_memoria *proceso_memoria = malloc(sizeof(t_proceso_memoria));
	proceso_memoria->path = pcb->path;
	proceso_memoria->size = pcb->size;
	proceso_memoria->prioridad = pcb->prioridad;
	proceso_memoria->pid = pcb->pid;
	t_paquete *paquete_solicitud_finalizar_proceso_en_memoria = crear_paquete_solicitud_finalizar_proceso_en_memoria(logger, proceso_memoria);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_finalizar_proceso_en_memoria, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
	free(proceso_memoria);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, conexion_con_memoria); // RESPUESTA_FINALIZAR_PROCESO_MEMORIA

	pthread_mutex_unlock(&mutex_conexion_memoria);
}

bool cargar_pagina_en_memoria(int pid, int numero_pagina)
{
	pthread_mutex_lock(&mutex_conexion_memoria);

	// Enviar
	t_pedido_pagina_en_memoria *pedido_pagina_en_memoria = malloc(sizeof(t_pedido_pagina_en_memoria));
	pedido_pagina_en_memoria->pid = pid;
	pedido_pagina_en_memoria->numero_de_pagina = numero_pagina;
	t_paquete *paquete_solicitud_cargar_pagina_en_memoria = crear_paquete_solicitud_cargar_pagina_en_memoria(logger, pedido_pagina_en_memoria);
	enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_cargar_pagina_en_memoria, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA);
	free(pedido_pagina_en_memoria);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_MEMORIA, conexion_con_memoria); // RESPUESTA_CARGAR_PAGINA_EN_MEMORIA
	bool resultado_cargar_pagina_en_memoria = leer_paquete_respuesta_cargar_pagina_en_memoria(logger, conexion_con_memoria);

	pthread_mutex_unlock(&mutex_conexion_memoria);

	return resultado_cargar_pagina_en_memoria;
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////* FILESYSTEM *//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
void abrir_archivo_fs(char *nombre_archivo, int *existe, int *tamanio_archivo)
{
	pthread_mutex_lock(&mutex_conexion_filesystem);

	// Enviar
	t_paquete *paquete_solicitud_abrir_archivo_fs = crear_paquete_solicitud_abrir_archivo_fs(logger, nombre_archivo);
	enviar_paquete(logger, conexion_con_filesystem, paquete_solicitud_abrir_archivo_fs, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem); // RESPUESTA_ABRIR_ARCHIVO_FS
	leer_paquete_respuesta_abrir_archivo_fs(logger, conexion_con_filesystem, existe, tamanio_archivo);

	pthread_mutex_unlock(&mutex_conexion_filesystem);
}

void crear_archivo_fs(char *nombre_archivo)
{
	pthread_mutex_lock(&mutex_conexion_filesystem);

	// Enviar
	t_paquete *paquete_solicitud_crear_archivo_fs = crear_paquete_solicitud_crear_archivo_fs(logger, nombre_archivo);
	enviar_paquete(logger, conexion_con_filesystem, paquete_solicitud_crear_archivo_fs, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem); // RESPUESTA_CREAR_ARCHIVO_FS

	pthread_mutex_unlock(&mutex_conexion_filesystem);
}

void truncar_archivo_fs(char *nombre_archivo, int nuevo_tamanio_archivo)
{
	pthread_mutex_lock(&mutex_conexion_filesystem);

	// Enviar
	t_paquete *paquete_solicitud_truncar_archivo_fs = crear_paquete_solicitud_truncar_archivo_fs(logger, nombre_archivo, nuevo_tamanio_archivo);
	enviar_paquete(logger, conexion_con_filesystem, paquete_solicitud_truncar_archivo_fs, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem); // RESPUESTA_TRUNCAR_ARCHIVO_FS

	pthread_mutex_unlock(&mutex_conexion_filesystem);
}

void leer_archivo_fs(char *nombre_archivo, int puntero_archivo, int direccion_fisica)
{
	pthread_mutex_lock(&mutex_conexion_filesystem);

	// Enviar
	t_paquete *paquete_solicitud_leer_archivo_fs = crear_paquete_solicitud_leer_archivo_fs(logger, nombre_archivo, puntero_archivo, direccion_fisica);
	enviar_paquete(logger, conexion_con_filesystem, paquete_solicitud_leer_archivo_fs, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem); // RESPUESTA_LEER_ARCHIVO_FS

	pthread_mutex_unlock(&mutex_conexion_filesystem);
}

void escribir_archivo_fs(char *nombre_archivo, int puntero_archivo, int direccion_fisica)
{
	pthread_mutex_lock(&mutex_conexion_filesystem);

	// Enviar
	t_paquete *paquete_solicitud_escribir_archivo_fs = crear_paquete_solicitud_escribir_archivo_fs(logger, nombre_archivo, puntero_archivo, direccion_fisica);
	enviar_paquete(logger, conexion_con_filesystem, paquete_solicitud_escribir_archivo_fs, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);

	// Recibir
	op_code codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM, conexion_con_filesystem); // RESPUESTA_ESCRIBIR_ARCHIVO_FS

	pthread_mutex_unlock(&mutex_conexion_filesystem);
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

void agregar_pid_a_aux_pids_cola(t_pcb *pcb)
{
	int cantidad_digitos_pid = floor(log10(abs(pcb->pid))) + 1;
	char pid_string[cantidad_digitos_pid + 1];
	sprintf(pid_string, "%d,", pcb->pid);
	int tamanio_anterior = strlen(aux_pids_cola);
	int tamanio_a_aumentar = strlen(pid_string);
	aux_pids_cola = realloc(aux_pids_cola, (tamanio_anterior + tamanio_a_aumentar) * sizeof(char));
	strcpy(aux_pids_cola + (tamanio_anterior) * sizeof(char), pid_string);
}

void loguear_cola(t_queue *cola, const char *nombre_cola, pthread_mutex_t *mutex_cola)
{
	aux_pids_cola = malloc(sizeof(char));
	strcpy(aux_pids_cola, "");

	queue_iterate_thread_safe(cola, (void (*)(void *)) & agregar_pid_a_aux_pids_cola, mutex_cola);

	int tamanio_pids = strlen(aux_pids_cola);
	if (tamanio_pids > 0)
	{
		aux_pids_cola[tamanio_pids - 1] = '\0';
	}

	log_info(logger, "Cola %s %s: [%s]", nombre_cola, configuracion_kernel->algoritmo_planificacion, aux_pids_cola);
	free(aux_pids_cola);
}

void imprimir_proceso_en_consola(t_pcb *pcb)
{
	printf("Proceso PID: %d - Estado %s\n", pcb->pid, nombre_estado_proceso(pcb->estado));
}

void imprimir_bloqueados_por_recurso_en_consola(t_recurso *recurso)
{
	queue_iterate_thread_safe(recurso->pcbs_bloqueados, (void (*)(void *)) & imprimir_proceso_en_consola, &recurso->mutex_pcbs_bloqueados);
}

void listar_procesos()
{
	queue_iterate_thread_safe(cola_new, (void (*)(void *)) & imprimir_proceso_en_consola, &mutex_cola_new);
	queue_iterate_thread_safe(cola_ready, (void (*)(void *)) & imprimir_proceso_en_consola, &mutex_cola_ready);

	if (pcb_ejecutando != NULL)
	{
		imprimir_proceso_en_consola(pcb_ejecutando);
	}

	queue_iterate_thread_safe(cola_bloqueados_sleep, (void (*)(void *)) & imprimir_proceso_en_consola, &mutex_cola_bloqueados_sleep);
	queue_iterate_thread_safe(cola_bloqueados_pagefault, (void (*)(void *)) & imprimir_proceso_en_consola, &mutex_cola_bloqueados_pagefault);

	list_iterate(recursos, (void (*)(void *)) & imprimir_bloqueados_por_recurso_en_consola);
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
	pcb->quantum_finalizado = false;
	pcb->id_hilo_quantum = -1;

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

t_pcb *buscar_pcb_con_pid(int pid)
{
	t_pcb *pcb;

	pcb = buscar_pcb_con_pid_en_cola(pid, cola_new, &mutex_cola_new);
	if (pcb != NULL)
	{
		return pcb;
	}

	pcb = buscar_pcb_con_pid_en_cola(pid, cola_ready, &mutex_cola_ready);
	if (pcb != NULL)
	{
		return pcb;
	}

	pcb = buscar_pcb_con_pid_en_cola(pid, cola_bloqueados_sleep, &mutex_cola_bloqueados_sleep);
	if (pcb != NULL)
	{
		return pcb;
	}

	pcb = buscar_pcb_con_pid_en_cola(pid, cola_bloqueados_pagefault, &mutex_cola_bloqueados_pagefault);
	if (pcb != NULL)
	{
		return pcb;
	}

	for (int i = 0; i < configuracion_kernel->cantidad_de_recursos; i++)
	{
		t_recurso *recurso = list_get(recursos, i);
		pcb = buscar_pcb_con_pid_en_cola(pid, recurso->pcbs_bloqueados, &recurso->mutex_pcbs_bloqueados);

		if (pcb != NULL)
		{
			return pcb;
		}
	}

	if (pcb_ejecutando != NULL && pcb_ejecutando->pid == pid)
	{
		return pcb_ejecutando;
	}

	log_warning(logger, "No se encontro proceso con PID %d", pid);

	return NULL;
}

t_pcb *buscar_pcb_con_pid_en_cola(int pid, t_queue *cola, pthread_mutex_t *mutex)
{
	bool _filtro_proceso_por_id(t_pcb * pcb)
	{
		return pcb->pid == pid;
	};

	t_pcb *pcb = list_find_thread_safe(cola->elements, (void *)_filtro_proceso_por_id, mutex);
	return pcb;
}

void eliminar_pcb_de_cola(int pid, t_queue *cola, pthread_mutex_t *mutex)
{
	bool _filtro_proceso_por_id(t_pcb * pcb)
	{
		return pcb->pid == pid;
	};

	list_remove_by_condition_thread_safe(cola->elements, (void *)_filtro_proceso_por_id, mutex);
}

void push_cola_ready(t_pcb *pcb)
{
	queue_push_thread_safe(cola_ready, pcb, &mutex_cola_ready);

	if (planifico_con_prioridades)
	{
		bool _comparador_prioridades(t_pcb * pcb1, t_pcb * pcb2)
		{
			return pcb1->prioridad < pcb2->prioridad;
		}

		list_sort_thread_safe(cola_ready->elements, (void *)_comparador_prioridades, &mutex_cola_ready);
	}

	loguear_cola(cola_ready, "ready", &mutex_cola_ready);

	if (planifico_con_round_robin && pcb_ejecutando != NULL && pcb_ejecutando->quantum_finalizado)
	{
		interrumpir_proceso_en_cpu(INTERRUPCION_POR_DESALOJO);
		log_info(logger, "PID: %d - Desalojado por fin de Quantum", pcb_ejecutando->pid);
	}

	if (planifico_con_prioridades && pcb_ejecutando != NULL && pcb->prioridad < pcb_ejecutando->prioridad)
	{
		interrumpir_proceso_en_cpu(INTERRUPCION_POR_DESALOJO);
		log_info(logger, "PID: %d - Desalojado por proceso con mayor prioridad", pcb_ejecutando->pid);
	}

	sem_post(&semaforo_hay_algun_proceso_en_cola_ready);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* RECURSOS *//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////

void crear_recursos()
{
	recursos = list_create();

	for (int i = 0; i < configuracion_kernel->cantidad_de_recursos; i++)
	{
		list_add(recursos, crear_recurso(configuracion_kernel->recursos[i], configuracion_kernel->instancias_recursos[i]));
	}
}

t_recurso *crear_recurso(char *nombre, int instancias)
{
	t_recurso *recurso = malloc(sizeof(t_recurso));

	recurso->nombre = malloc(strlen(nombre));
	strcpy(recurso->nombre, nombre);

	recurso->instancias_iniciales = instancias;
	recurso->instancias_disponibles = instancias;
	recurso->pcbs_bloqueados = queue_create();
	recurso->pcbs_asignados = list_create();

	pthread_mutex_init(&recurso->mutex_pcbs_asignados, NULL);
	pthread_mutex_init(&recurso->mutex_pcbs_bloqueados, NULL);

	log_debug(logger, "Se crea el recurso %s con %d instancias", nombre, instancias);

	return recurso;
}

bool recurso_existe(char *nombre)
{
	bool _filtro_nombre_recurso(t_recurso * recurso)
	{
		return strcmp(recurso->nombre, nombre) == 0;
	}

	return list_any_satisfy(recursos, (void *)_filtro_nombre_recurso);
}

t_recurso *buscar_recurso_por_nombre(char *nombre_recurso)
{
	bool _filtro_recurso_por_nombre(t_recurso * recurso)
	{
		return strcmp(nombre_recurso, recurso->nombre) == 0;
	};

	t_recurso *recurso = list_find(recursos, (void *)_filtro_recurso_por_nombre);
	return recurso;
}

bool recurso_esta_asignado_a_pcb(char *nombre_recurso, int pid)
{
	t_recurso *recurso = buscar_recurso_por_nombre(nombre_recurso);

	bool _filtro_proceso_por_id(t_pcb * pcb)
	{
		return pcb->pid == pid;
	};

	return list_find_thread_safe(recurso->pcbs_asignados, (void *)_filtro_proceso_por_id, &recurso->mutex_pcbs_asignados) != NULL;
}

void desasignar_recurso_a_pcb(char *nombre_recurso, int pid)
{
	t_recurso *recurso = buscar_recurso_por_nombre(nombre_recurso);

	bool _filtro_proceso_por_id(t_pcb * pcb)
	{
		return pcb->pid == pid;
	};

	list_remove_by_condition_thread_safe(recurso->pcbs_asignados, (void *)_filtro_proceso_por_id, &recurso->mutex_pcbs_asignados);

	recurso->instancias_disponibles++;

	if (recurso->instancias_disponibles <= 0)
	{
		t_pcb *pcb_a_desbloquear = queue_pop_thread_safe(recurso->pcbs_bloqueados, &recurso->mutex_pcbs_bloqueados);
		transicionar_proceso(pcb_a_desbloquear, CODIGO_ESTADO_PROCESO_READY);
	}
}

void desasignar_todos_los_recursos_a_pcb(int pid)
{
	for (int i = 0; i < configuracion_kernel->cantidad_de_recursos; i++)
	{
		t_recurso *recurso = list_get(recursos, i);

		bool _filtro_proceso_por_id(t_pcb * pcb)
		{
			return pcb->pid == pid;
		};

		list_remove_by_condition_thread_safe(recurso->pcbs_bloqueados->elements, (void *)_filtro_proceso_por_id, &recurso->mutex_pcbs_bloqueados);
	}

	for (int i = 0; i < configuracion_kernel->cantidad_de_recursos; i++)
	{
		t_recurso *recurso = list_get(recursos, i);

		while (recurso_esta_asignado_a_pcb(recurso->nombre, pid))
		{
			desasignar_recurso_a_pcb(recurso->nombre, pid);
		}
	}
}

// t_list *obtener_procesos_analisis_deadlock()
// {
// 	t_list *resultado = list_create();
// 	t_pcb *pcb;
// 	t_pcb_analisis_deadlock *pcb_a_analizar_existente;
// 	t_pcb_analisis_deadlock *pcb_a_analizar_nuevo;
// 	t_recurso *recurso;
// 	t_list_iterator *iterador_pcbs_asignados;
// 	t_list_iterator *iterador_pcbs_bloqueados;
// 	int i, j;
// 	int cantidad_de_recursos = configuracion_kernel->cantidad_de_recursos;

// 	bool _filtro_pcb_por_id(t_pcb * unpcb)
// 	{
// 		return unpcb->pid == pcb->pid;
// 	};

// 	for (i = 0; i < cantidad_de_recursos; i++)
// 	{
// 		recurso = list_get(recursos, i);
// 		iterador_pcbs_asignados = list_iterator_create(recurso->pcbs_asignados);

// 		while (list_iterator_has_next(iterador_pcbs_asignados))
// 		{
// 			pcb = list_iterator_next(iterador_pcbs_asignados);
// 			pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);

// 			if (pcb_a_analizar_existente == NULL)
// 			{
// 				pcb_a_analizar_nuevo = malloc(sizeof(t_pcb_analisis_deadlock));

// 				pcb_a_analizar_nuevo->finalizado = false;
// 				pcb_a_analizar_nuevo->pid = pcb->pid;
// 				pcb_a_analizar_nuevo->recursos_asignados = malloc(cantidad_de_recursos * sizeof(int));
// 				pcb_a_analizar_nuevo->solicitudes_actuales = malloc(cantidad_de_recursos * sizeof(int));

// 				for (j = 0; j < cantidad_de_recursos; j++)
// 				{
// 					pcb_a_analizar_nuevo->recursos_asignados[j] = 0;
// 					pcb_a_analizar_nuevo->solicitudes_actuales[j] = 0;
// 				}

// 				pcb_a_analizar_nuevo->recursos_asignados[i]++;

// 				list_add(resultado, pcb_a_analizar_nuevo);
// 			}
// 			else
// 			{
// 				pcb_a_analizar_existente->recursos_asignados[i]++;
// 			}
// 		}
// 		list_iterator_destroy(iterador_pcbs_asignados);
// 	}

// 	for (i = 0; i < cantidad_de_recursos; i++)
// 	{
// 		recurso = list_get(recursos, i);
// 		iterador_pcbs_bloqueados = list_iterator_create(recurso->pcbs_bloqueados->elements);
// 		while (list_iterator_has_next(iterador_pcbs_bloqueados))
// 		{
// 			pcb = list_iterator_next(iterador_pcbs_bloqueados);
// 			pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);

// 			if (pcb_a_analizar_existente != NULL)
// 			{
// 				pcb_a_analizar_existente->solicitudes_actuales[i]++;
// 			}
// 		}
// 		list_iterator_destroy(iterador_pcbs_bloqueados);
// 	}

// 	return resultado;
// }

// int *obtener_vector_recursos_disponibles()
// {
// 	int cantidad_de_recursos = configuracion_kernel->cantidad_de_recursos;
// 	int *recursos_disponibles = (int *)malloc(cantidad_de_recursos * sizeof(int));

// 	for (int i = 0; i < cantidad_de_recursos; i++)
// 	{
// 		t_recurso *recurso = list_get(recursos, i);
// 		recursos_disponibles[i] = recurso->instancias_disponibles;
// 	}

// 	return recursos_disponibles;
// }

// // TODO: chequear frees!
// // free(recursos_disponibles);
// // list_destroy(procesos_a_analizar);
// void analisis_deadlock()
// {
// 	int *recursos_totales = configuracion_kernel->instancias_recursos;
// 	int *recursos_disponibles = obtener_vector_recursos_disponibles();
// 	t_list *procesos_a_analizar = obtener_procesos_analisis_deadlock();
// 	bool finalice_alguno = true;
// 	t_list_iterator *iterador_procesos_a_analizar;
// 	t_pcb_analisis_deadlock *pcb_analisis_deadlock;
// 	int cantidad_procesos_a_analizar = list_size(procesos_a_analizar);
// 	int cantidad_iteraciones_realizadas = 0;
// 	int cantidad_de_recursos = configuracion_kernel->cantidad_de_recursos;

// 	// INICIO LOGUEO
// 	loguear_vector(recursos_totales, cantidad_de_recursos, "RECURSOS TOTALES", -1);
// 	loguear_vector(recursos_disponibles, cantidad_de_recursos, "RECURSOS DISPONIBLES", -1);
// 	iterador_procesos_a_analizar = list_iterator_create(procesos_a_analizar);
// 	while (list_iterator_has_next(iterador_procesos_a_analizar))
// 	{
// 		pcb_analisis_deadlock = list_iterator_next(iterador_procesos_a_analizar);
// 		loguear_vector(pcb_analisis_deadlock->recursos_asignados, cantidad_de_recursos, "RECURSOS DISPONIBLES", pcb_analisis_deadlock->pid);
// 		loguear_vector(pcb_analisis_deadlock->solicitudes_actuales, cantidad_de_recursos, "SOLICITUDES ACTUALES", pcb_analisis_deadlock->pid);
// 	}
// 	list_iterator_destroy(iterador_procesos_a_analizar);
// 	// FIN LOGUEO

// 	// INICIO ALGORITMO
// 	if (list_is_empty(procesos_a_analizar))
// 	{
// 		return false;
// 	}

// 	while (finalice_alguno && cantidad_iteraciones_realizadas < cantidad_procesos_a_analizar)
// 	{
// 		finalice_alguno = false;

// 		log_info(logger, "ITERACION %d", cantidad_iteraciones_realizadas);
// 		iterador_procesos_a_analizar = list_iterator_create(procesos_a_analizar);
// 		while (list_iterator_has_next(iterador_procesos_a_analizar) && !finalice_alguno)
// 		{
// 			pcb_analisis_deadlock = list_iterator_next(iterador_procesos_a_analizar);

// 			if (!pcb_analisis_deadlock->finalizado)
// 			{
// 				log_info(logger, "SE PUEDEN SATISFACER LAS SOLICITUDES DE PID %d?", pcb_analisis_deadlock->pid);

// 				bool se_puede_satisfacer_solicitudes = true;
// 				for (int i = 0; i < cantidad_de_recursos; i++)
// 				{
// 					se_puede_satisfacer_solicitudes = se_puede_satisfacer_solicitudes && recursos_disponibles[i] >= pcb_analisis_deadlock->solicitudes_actuales[i];
// 				}

// 				if (se_puede_satisfacer_solicitudes)
// 				{
// 					finalice_alguno = true;
// 					log_info(logger, "SI!");

// 					pcb_analisis_deadlock->finalizado = true;

// 					for (int i = 0; i < cantidad_de_recursos; i++)
// 					{
// 						recursos_disponibles[i] = recursos_disponibles[i] + pcb_analisis_deadlock->recursos_asignados[i];
// 					}
// 				}
// 				else
// 				{
// 					log_info(logger, "NO!");
// 				}
// 			}
// 		}
// 		list_iterator_destroy(iterador_procesos_a_analizar);

// 		loguear_vector(recursos_disponibles, cantidad_de_recursos, "RECURSOS DISPONIBLES", -1);
// 		cantidad_iteraciones_realizadas++;
// 	}

// 	bool hay_deadlock = cantidad_iteraciones_realizadas < cantidad_procesos_a_analizar;

// 	if (hay_deadlock)
// 	{
// 		log_info(logger, "HAY DEADLOCK");

// 		iterador_procesos_a_analizar = list_iterator_create(procesos_a_analizar);
// 		while (list_iterator_has_next(iterador_procesos_a_analizar))
// 		{
// 			pcb_analisis_deadlock = list_iterator_next(iterador_procesos_a_analizar);
// 			if (!pcb_analisis_deadlock->finalizado)
// 			{
// 				log_info(logger, "PID %d ESTA EN DEADLOCK", pcb_analisis_deadlock->pid);
// 			}
// 		}
// 	}
// 	else
// 	{
// 		log_info(logger, "NO HAY DEADLOCK");
// 	}

// 	return hay_deadlock;
// }