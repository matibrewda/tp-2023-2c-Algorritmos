#include "kernel.h"

// Variables globales
t_log *logger = NULL;
t_argumentos_kernel *argumentos_kernel = NULL;
t_config_kernel *configuracion_kernel = NULL;
int proximo_pid = 0;
int id_hilo_quantum = 0;
bool planifico_con_round_robin = false;
bool planifico_con_prioridades = false;
bool en_deadlock = false;
int grado_max_multiprogramacion_actual;
bool planificacion_detenida = false;

// Procesos
t_list *procesos;

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

pthread_mutex_t mutex_conexion_memoria;
pthread_mutex_t mutex_conexion_filesystem;
pthread_mutex_t mutex_bool_planificacion_detenida;
pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_cola_bloqueados_sleep;
pthread_mutex_t mutex_cola_bloqueados_pagefault;
pthread_mutex_t mutex_cola_bloqueados_operaciones_archivos;
pthread_mutex_t mutex_proceso_ejecutando;
pthread_mutex_t mutex_detener_planificacion_corto_plazo;
pthread_mutex_t mutex_detener_planificacion_largo_plazo;
pthread_mutex_t mutex_recursos;
pthread_mutex_t mutex_procesos;

// Colas de planificacion
t_queue *cola_new = NULL;
t_queue *cola_ready = NULL;
t_queue *cola_bloqueados_sleep = NULL;
t_queue *cola_bloqueados_pagefault = NULL;
t_queue *cola_bloqueados_operaciones_archivos = NULL;
t_pcb *pcb_ejecutando = NULL;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	setbuf(stdout, NULL);
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
	procesos = list_create();
	recursos = list_create();
	for (int i = 0; i < configuracion_kernel->cantidad_de_recursos; i++)
	{
		list_add(recursos, crear_recurso(configuracion_kernel->recursos[i], configuracion_kernel->instancias_recursos[i]));
	}

	// Semaforos
	grado_max_multiprogramacion_actual = configuracion_kernel->grado_multiprogramacion_inicial;
	sem_init(&semaforo_grado_max_multiprogramacion, false, configuracion_kernel->grado_multiprogramacion_inicial);
	sem_init(&semaforo_hay_algun_proceso_en_cola_new, false, 0);
	sem_init(&semaforo_hay_algun_proceso_en_cola_ready, false, 0);

	pthread_mutex_init(&mutex_cola_new, NULL);
	pthread_mutex_init(&mutex_cola_ready, NULL);
	pthread_mutex_init(&mutex_cola_bloqueados_sleep, NULL);
	pthread_mutex_init(&mutex_cola_bloqueados_pagefault, NULL);
	pthread_mutex_init(&mutex_conexion_memoria, NULL);
	pthread_mutex_init(&mutex_conexion_filesystem, NULL);
	pthread_mutex_init(&mutex_recursos, NULL);
	pthread_mutex_init(&mutex_detener_planificacion_corto_plazo, NULL);
	pthread_mutex_init(&mutex_detener_planificacion_largo_plazo, NULL);
	pthread_mutex_init(&mutex_cola_bloqueados_operaciones_archivos, NULL);
	pthread_mutex_init(&mutex_procesos, NULL);
	pthread_mutex_init(&mutex_bool_planificacion_detenida, NULL);
	pthread_mutex_init(&mutex_proceso_ejecutando, NULL);

	// Colas de planificacion
	cola_new = queue_create();
	cola_ready = queue_create();
	cola_bloqueados_sleep = queue_create();
	cola_bloqueados_pagefault = queue_create();
	cola_bloqueados_operaciones_archivos = queue_create();

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
		sem_wait(&semaforo_grado_max_multiprogramacion);
		sem_wait(&semaforo_hay_algun_proceso_en_cola_new);
		pthread_mutex_lock(&mutex_detener_planificacion_largo_plazo);
		t_pcb *pcb = queue_pop_thread_safe(cola_new, &mutex_cola_new);
		if (pcb == NULL)
		{
			sem_post(&semaforo_grado_max_multiprogramacion);
		}
		log_debug(logger, "Planificador largo plazo - push a cola de ready de PID %d", pcb->pid);
		transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_READY);
		pthread_mutex_unlock(&mutex_detener_planificacion_largo_plazo);
	}
}

void *planificador_corto_plazo()
{
	t_pcb *pcb;
	op_code codigo_operacion_recibido;
	int pid_pcb = -1;
	int tiempo_sleep = -1;
	int motivo_interrupcion = -1;
	int codigo_error = -1;
	int numero_pagina = -1;
	int posicion_puntero_archivo = -1;
	int direccion_fisica = -1;
	int nuevo_tamanio_archivo = -1;
	int fs_opcode = -1;
	int modo_apertura = -1;
	bool mantener_proceso_ejecutando = false;
	bool correr_deteccion_deadlock = false;
	char *nombre_recurso = NULL;
	char *nombre_archivo = NULL;

	while (true)
	{
		if (!mantener_proceso_ejecutando)
		{
			sem_wait(&semaforo_hay_algun_proceso_en_cola_ready);
			pcb = queue_pop_thread_safe(cola_ready, &mutex_cola_ready);
			if (pcb == NULL)
			{
				continue;
			}
		}
		else
		{
			if (pcb->quantum_finalizado)
			{
				mantener_proceso_ejecutando = false;
				transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_READY);
				continue;
			}
		}

		mantener_proceso_ejecutando = false;
		correr_deteccion_deadlock = false;
		pid_pcb = pcb->pid;
		transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXECUTING);
		t_contexto_de_ejecucion *contexto_de_ejecucion = recibir_paquete_de_cpu_dispatch(&codigo_operacion_recibido, &tiempo_sleep, &motivo_interrupcion, &nombre_recurso, &codigo_error, &numero_pagina, &nombre_archivo, &modo_apertura, &posicion_puntero_archivo, &direccion_fisica, &nuevo_tamanio_archivo, &fs_opcode);
		pthread_mutex_lock(&mutex_proceso_ejecutando);
		pcb_ejecutando = NULL;
		pthread_mutex_unlock(&mutex_proceso_ejecutando);
		actualizar_pcb(pcb, contexto_de_ejecucion);
		free(contexto_de_ejecucion);

		pthread_mutex_lock(&mutex_detener_planificacion_corto_plazo);

		if (!buscar_pcb_con_pid(pid_pcb))
		{
			pthread_mutex_unlock(&mutex_detener_planificacion_corto_plazo);
			
			if (nombre_recurso != NULL)
			{
				free(nombre_recurso);
				nombre_recurso = NULL;
			}

			if (nombre_archivo != NULL)
			{
				free(nombre_archivo);
				nombre_archivo = NULL;
			}

			continue;
		}

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
			bool _filtro_archivo_abierto_proceso_por_nombre(t_archivo_abierto_proceso * archivo_abierto_proceso)
			{
				return strcmp(archivo_abierto_proceso->nombre_archivo, nombre_archivo) == 0;
			};

			t_recurso *recurso_archivo = buscar_recurso_por_nombre(nombre_archivo);
			t_archivo_abierto_proceso *archivo_abierto_proceso = list_find_thread_safe(pcb->tabla_archivos, (void *)_filtro_archivo_abierto_proceso_por_nombre, &pcb->mutex_tabla_archivos);

			if (fs_opcode == FOPEN_OPCODE)
			{
				log_info(logger, "Abrir Archivo: PID: %d - Abrir Archivo: %s", pcb->pid, nombre_archivo);

				bool archivo_a_abrir_existe_y_estaba_abierto = recurso_archivo != NULL;
				if (archivo_a_abrir_existe_y_estaba_abierto)
				{
					t_archivo_abierto_proceso *archivo_abierto_pcb = malloc(sizeof(t_archivo_abierto_proceso));
					archivo_abierto_pcb->nombre_archivo = malloc(strlen(nombre_archivo)+1);
					strcpy(archivo_abierto_pcb->nombre_archivo, nombre_archivo);
					archivo_abierto_pcb->puntero = 0;
					archivo_abierto_pcb->modo_apertura = modo_apertura;
					list_add_thread_safe(pcb->tabla_archivos, archivo_abierto_pcb, &pcb->mutex_tabla_archivos);

					if (modo_apertura == LOCK_ESCRITURA)
					{
						pthread_mutex_lock(&recurso_archivo->mutex_recurso);
						if (recurso_archivo->pcb_lock_escritura != NULL || !queue_is_empty(recurso_archivo->pcbs_bloqueados_por_archivo))
						{
							transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_BLOCKED);
							t_pcb_bloqueado_archivo *pcb_bloqueado_archivo = malloc(sizeof(t_pcb_bloqueado_archivo));
							pcb_bloqueado_archivo->pcb = pcb;
							pcb_bloqueado_archivo->lock = modo_apertura;
							pcb->ultimo_recurso_pedido = malloc(strlen(nombre_archivo)+1);
							strcpy(pcb->ultimo_recurso_pedido, nombre_archivo);
							queue_push(recurso_archivo->pcbs_bloqueados_por_archivo, pcb_bloqueado_archivo);
							correr_deteccion_deadlock = true;
						}
						else
						{
							recurso_archivo->pcb_lock_escritura = pcb;
							recurso_archivo->instancias_iniciales = 1;
							recurso_archivo->instancias_disponibles = 0;
							mantener_proceso_ejecutando = true;
						}
						pthread_mutex_unlock(&recurso_archivo->mutex_recurso);
					}
					else
					{
						pthread_mutex_lock(&recurso_archivo->mutex_recurso);
						if (recurso_archivo->pcb_lock_escritura != NULL)
						{
							transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_BLOCKED);
							t_pcb_bloqueado_archivo *pcb_bloqueado_archivo = malloc(sizeof(t_pcb_bloqueado_archivo));
							pcb_bloqueado_archivo->pcb = pcb;
							pcb_bloqueado_archivo->lock = modo_apertura;
							pcb->ultimo_recurso_pedido = malloc(strlen(nombre_archivo)+1);
							strcpy(pcb->ultimo_recurso_pedido, nombre_archivo);
							queue_push(recurso_archivo->pcbs_bloqueados_por_archivo, pcb_bloqueado_archivo);
							correr_deteccion_deadlock = true;
						}
						else
						{
							list_add(recurso_archivo->pcbs_lock_lectura, pcb);
							mantener_proceso_ejecutando = true;
							recurso_archivo->instancias_iniciales = list_size(recurso_archivo->pcbs_lock_lectura);
							recurso_archivo->instancias_disponibles = 0;
						}
						pthread_mutex_unlock(&recurso_archivo->mutex_recurso);
					}
				}
				else
				{
					mantener_proceso_ejecutando = true;
					int existe_archivo_a_abrir;
					int tamanio_archivo_abierto;
					abrir_archivo_fs(nombre_archivo, &existe_archivo_a_abrir, &tamanio_archivo_abierto);
					if (!existe_archivo_a_abrir)
					{
						crear_archivo_fs(nombre_archivo);
						tamanio_archivo_abierto = 0;
						existe_archivo_a_abrir = true;
					}
					list_add_thread_safe(recursos, crear_recurso_archivo(nombre_archivo, modo_apertura, tamanio_archivo_abierto, pcb), &mutex_recursos);
					t_archivo_abierto_proceso *archivo_abierto_pcb = malloc(sizeof(t_archivo_abierto_proceso));
					archivo_abierto_pcb->nombre_archivo = malloc(strlen(nombre_archivo)+1);
					strcpy(archivo_abierto_pcb->nombre_archivo, nombre_archivo);
					archivo_abierto_pcb->puntero = 0;
					archivo_abierto_pcb->modo_apertura = modo_apertura;
					list_add_thread_safe(pcb->tabla_archivos, archivo_abierto_pcb, &pcb->mutex_tabla_archivos);
				}
			}
			else if (fs_opcode == FCLOSE_OPCODE)
			{
				log_info(logger, "Cerrar Archivo: PID: %d - Cerrar Archivo: %s", pcb->pid, nombre_archivo);
				desasignar_recurso_a_pcb(recurso_archivo, pcb->pid);
				correr_deteccion_deadlock = true;
				mantener_proceso_ejecutando = true;
			}
			else if (fs_opcode == FSEEK_OPCODE)
			{
				log_info(logger, "Actualizar Puntero Archivo: PID: %d - Actualizar puntero Archivo: %s - Puntero: %d - Puntero Anterior: %d", pcb->pid, nombre_archivo, posicion_puntero_archivo, archivo_abierto_proceso->puntero);
				mantener_proceso_ejecutando = true;
				archivo_abierto_proceso->puntero = posicion_puntero_archivo;
			}
			else if (fs_opcode == FTRUNCATE_OPCODE)
			{
				log_info(logger, "Truncar Archivo: PID: %d - Truncar Archivo: %s - Tamaño: %d", pcb->pid, nombre_archivo, nuevo_tamanio_archivo);
				transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_BLOCKED);
				queue_push_thread_safe(cola_bloqueados_operaciones_archivos, pcb, &mutex_cola_bloqueados_operaciones_archivos);
				crear_hilo_operacion_archivo(pcb, TRUNCAR_ARCHIVO, nombre_archivo, archivo_abierto_proceso->modo_apertura, -1, -1, nuevo_tamanio_archivo);
				pthread_mutex_lock(&recurso_archivo->mutex_recurso);
				recurso_archivo->tamanio_archivo = nuevo_tamanio_archivo;
				pthread_mutex_unlock(&recurso_archivo->mutex_recurso);
			}
			else if (fs_opcode == FWRITE_OPCODE)
			{
				log_info(logger, "Escribir Archivo: PID: %d - Escribir Archivo: %s - Puntero: %d - Direccion Memoria: %d - Tamaño: %d", pcb->pid, nombre_archivo, archivo_abierto_proceso->puntero, direccion_fisica, recurso_archivo->tamanio_archivo);
				if (archivo_abierto_proceso->modo_apertura != LOCK_ESCRITURA)
				{
					pcb->motivo_finalizacion = FINALIZACION_INVALID_WRITE;
					transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
				}
				else
				{
					transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_BLOCKED);
					queue_push_thread_safe(cola_bloqueados_operaciones_archivos, pcb, &mutex_cola_bloqueados_operaciones_archivos);
					crear_hilo_operacion_archivo(pcb, ESCRIBIR_ARCHIVO, nombre_archivo, modo_apertura, archivo_abierto_proceso->puntero, direccion_fisica, -1);
				}
			}
			else if (fs_opcode == FREAD_OPCODE)
			{
				log_info(logger, "Leer Archivo: PID: %d - Leer Archivo: %s - Puntero: %d - Direccion Memoria: %d - Tamaño: %d", pcb->pid, nombre_archivo, archivo_abierto_proceso->puntero, direccion_fisica, recurso_archivo->tamanio_archivo);
				transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_BLOCKED);
				queue_push_thread_safe(cola_bloqueados_operaciones_archivos, pcb, &mutex_cola_bloqueados_operaciones_archivos);
				crear_hilo_operacion_archivo(pcb, LEER_ARCHIVO, nombre_archivo, modo_apertura, archivo_abierto_proceso->puntero, direccion_fisica, -1);
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
			t_recurso *recurso_para_wait = buscar_recurso_por_nombre(nombre_recurso);

			if (recurso_para_wait == NULL)
			{
				pcb->motivo_finalizacion = FINALIZACION_INVALID_RESOURCE;
				transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
			}
			else
			{
				recurso_para_wait->instancias_disponibles--;
				log_info(logger, "Wait: PID: %d - Wait: %s - Instancias: %d", pcb->pid, nombre_recurso, recurso_para_wait->instancias_disponibles);

				if (recurso_para_wait->instancias_disponibles < 0)
				{
					transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_BLOCKED);

					pthread_mutex_lock(&recurso_para_wait->mutex_recurso);

					pcb->ultimo_recurso_pedido = malloc(strlen(recurso_para_wait->nombre)+1);
					strcpy(pcb->ultimo_recurso_pedido, recurso_para_wait->nombre);

					queue_push(recurso_para_wait->pcbs_bloqueados, pcb);
					log_info(logger, "Motivo de Bloqueo: PID: %d - Bloqueado por: %s", pcb->pid, nombre_recurso);
					pthread_mutex_unlock(&recurso_para_wait->mutex_recurso);

					correr_deteccion_deadlock = true;
				}
				else
				{
					list_add_thread_safe(recurso_para_wait->pcbs_asignados, pcb, &recurso_para_wait->mutex_recurso);
					mantener_proceso_ejecutando = true;
				}
			}
		}
		else if (codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL)
		{
			t_recurso *recurso_para_signal = buscar_recurso_por_nombre(nombre_recurso);
			if (recurso_para_signal == NULL || !recurso_esta_asignado_a_pcb(recurso_para_signal, pcb->pid))
			{
				pcb->motivo_finalizacion = FINALIZACION_INVALID_RESOURCE;
				transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
			}
			else
			{
				pthread_mutex_lock(&recurso_para_signal->mutex_recurso);
				log_info(logger, "Signal: PID: %d - Signal: %s - Instancias: %d", pcb->pid, nombre_recurso, recurso_para_signal->instancias_disponibles + 1);
				pthread_mutex_unlock(&recurso_para_signal->mutex_recurso);

				mantener_proceso_ejecutando = true;
				desasignar_recurso_a_pcb(recurso_para_signal, pcb->pid);
			}
		}

		if (correr_deteccion_deadlock)
		{
			hay_deadlock();
		}

		pthread_mutex_unlock(&mutex_detener_planificacion_corto_plazo);

		if (nombre_recurso != NULL)
		{
			free(nombre_recurso);
			nombre_recurso = NULL;
		}

		if (nombre_archivo != NULL)
		{
			free(nombre_archivo);
			nombre_archivo = NULL;
		}
	}
}

void *contador_quantum(void *argumentos)
{
	t_arg_hilo_quantum *arg_hilo_quantum = (t_arg_hilo_quantum *)argumentos;
	log_debug(logger, "Creo hilo para interrumpir PID %d (HILO ID %d)", arg_hilo_quantum->pid_a_interrumpir, arg_hilo_quantum->id_hilo_quantum);

	usleep((configuracion_kernel->quantum) * 1000);

	log_debug(logger, "Soy el hilo %d y me fijo si tengo que interrumpir PID %d", arg_hilo_quantum->id_hilo_quantum, arg_hilo_quantum->pid_a_interrumpir);

	t_pcb *pcb_quantum_finalizado = buscar_pcb_con_pid(arg_hilo_quantum->pid_a_interrumpir);
	if (pcb_quantum_finalizado != NULL && pcb_quantum_finalizado->pid == arg_hilo_quantum->pid_a_interrumpir && pcb_quantum_finalizado->id_hilo_quantum == arg_hilo_quantum->id_hilo_quantum)
	{
		pcb_quantum_finalizado->quantum_finalizado = true;

		pthread_mutex_lock(&mutex_proceso_ejecutando);
		if (pcb_ejecutando != NULL && pcb_ejecutando->pid == arg_hilo_quantum->pid_a_interrumpir && pcb_ejecutando->id_hilo_quantum == arg_hilo_quantum->id_hilo_quantum)
		{
			pthread_mutex_unlock(&mutex_proceso_ejecutando);
			log_info(logger, "Fin de Quantum: PID: %d - Desalojado por fin de Quantum", pcb_quantum_finalizado->pid);
			interrumpir_proceso_en_cpu(INTERRUPCION_POR_DESALOJO);
		}
		else
		{
			log_debug(logger, "Soy el hilo %d y NO tuve que interrumpir PID %d", arg_hilo_quantum->id_hilo_quantum, arg_hilo_quantum->pid_a_interrumpir);
			pthread_mutex_unlock(&mutex_proceso_ejecutando);
		}
	}
	else
	{
		log_debug(logger, "Soy el hilo %d y NO tuve que interrumpir PID %d", arg_hilo_quantum->id_hilo_quantum, arg_hilo_quantum->pid_a_interrumpir);
	}

	free(arg_hilo_quantum);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* TRANSICIONES *//////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
void transicionar_proceso(t_pcb *pcb, char nuevo_estado_proceso)
{
	if (pcb->estado == CODIGO_ESTADO_PROCESO_NEW)
	{
		if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_NEW)
		{
			transicionar_proceso_a_new(pcb);
		}
		else if (nuevo_estado_proceso == CODIGO_ESTADO_PROCESO_READY)
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
	if (!iniciar_estructuras_de_proceso_en_memoria(pcb))
	{
		log_error(logger, "Ocurrio un error al inicializar las estructuras en memoria del proceso %d", pcb->pid);
		destruir_pcb(pcb);
		return;
	}

	log_info(logger, "Creacion de Proceso: Se crea el proceso %d en NEW", pcb->pid);
	queue_push_thread_safe(cola_new, pcb, &mutex_cola_new);
	list_add_thread_safe(procesos, pcb, &mutex_procesos);
	sem_post(&semaforo_hay_algun_proceso_en_cola_new);
}

void transicionar_proceso_de_new_a_ready(t_pcb *pcb)
{
	log_info(logger, "Cambio de Estado: PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_READY));
	pcb->estado = CODIGO_ESTADO_PROCESO_READY;
	push_cola_ready(pcb);
}

void transicionar_proceso_de_new_a_exit(t_pcb *pcb)
{
	log_info(logger, "Cambio de Estado: PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXIT));
	log_fin_de_proceso(pcb);
	eliminar_pcb_de_cola(pcb->pid, cola_new, &mutex_cola_new);
	eliminar_pcb_de_lista(pcb->pid, procesos, &mutex_procesos);
	destruir_pcb(pcb);
}

void transicionar_proceso_de_ready_a_executing(t_pcb *pcb)
{
	log_info(logger, "Cambio de Estado: PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXECUTING));

	pcb->estado = CODIGO_ESTADO_PROCESO_EXECUTING;

	if (planifico_con_round_robin)
	{
		pcb->quantum_finalizado = false;
		pcb->id_hilo_quantum = ++id_hilo_quantum;

		// Creo hilo quantum
		pthread_t hilo_contador_quantum;
		t_arg_hilo_quantum *arg_hilo_quantum = malloc(sizeof(t_arg_hilo_quantum));
		arg_hilo_quantum->id_hilo_quantum = id_hilo_quantum;
		arg_hilo_quantum->pid_a_interrumpir = pcb->pid;
		pthread_create(&hilo_contador_quantum, NULL, contador_quantum, (void *)arg_hilo_quantum);
	}

	ejecutar_proceso_en_cpu(pcb);
}

void transicionar_proceso_de_ready_a_exit(t_pcb *pcb)
{
	log_info(logger, "Cambio de Estado: PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXIT));
	log_fin_de_proceso(pcb);
	desasignar_todos_los_recursos_a_pcb(pcb->pid);
	destruir_estructuras_de_proceso_en_memoria(pcb);
	eliminar_pcb_de_cola(pcb->pid, cola_ready, &mutex_cola_ready);
	eliminar_pcb_de_lista(pcb->pid, procesos, &mutex_procesos);
	destruir_pcb(pcb);
	if (en_deadlock)
	{
		hay_deadlock();
	}
	sem_post(&semaforo_grado_max_multiprogramacion);
}

void transicionar_proceso_de_executing_a_ready(t_pcb *pcb)
{
	log_info(logger, "Cambio de Estado: PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_READY));
	pcb->estado = CODIGO_ESTADO_PROCESO_READY;
	push_cola_ready(pcb);
}

void transicionar_proceso_de_executing_a_bloqueado(t_pcb *pcb)
{
	log_info(logger, "Cambio de Estado: PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_BLOCKED));
	pcb->estado = CODIGO_ESTADO_PROCESO_BLOCKED;
}

void transicionar_proceso_de_executing_a_executing(t_pcb *pcb)
{
	ejecutar_proceso_en_cpu(pcb);
}

void transicionar_proceso_de_executing_a_exit(t_pcb *pcb)
{
	log_info(logger, "Cambio de Estado: PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXIT));
	log_fin_de_proceso(pcb);
	desasignar_todos_los_recursos_a_pcb(pcb->pid);
	destruir_estructuras_de_proceso_en_memoria(pcb);
	eliminar_pcb_de_lista(pcb->pid, procesos, &mutex_procesos);
	destruir_pcb(pcb);
	if (en_deadlock)
	{
		hay_deadlock();
	}
	sem_post(&semaforo_grado_max_multiprogramacion);
}

void transicionar_proceso_de_bloqueado_a_ready(t_pcb *pcb)
{
	log_info(logger, "Cambio de Estado: PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_READY));
	pcb->estado = CODIGO_ESTADO_PROCESO_READY;
	push_cola_ready(pcb);
}

void transicionar_proceso_de_bloqueado_a_exit(t_pcb *pcb)
{
	log_info(logger, "Cambio de Estado: PID: %d - Estado Anterior: '%s' - Estado Actual: '%s'", pcb->pid, nombre_estado_proceso(pcb->estado), nombre_estado_proceso(CODIGO_ESTADO_PROCESO_EXIT));
	log_fin_de_proceso(pcb);
	eliminar_pcb_de_cola(pcb->pid, cola_bloqueados_sleep, &mutex_cola_bloqueados_sleep);
	eliminar_pcb_de_cola(pcb->pid, cola_bloqueados_pagefault, &mutex_cola_bloqueados_pagefault);
	eliminar_pcb_de_cola(pcb->pid, cola_bloqueados_operaciones_archivos, &mutex_cola_bloqueados_operaciones_archivos);
	desasignar_todos_los_recursos_a_pcb(pcb->pid);
	destruir_estructuras_de_proceso_en_memoria(pcb);
	eliminar_pcb_de_lista(pcb->pid, procesos, &mutex_procesos);
	destruir_pcb(pcb);
	if (en_deadlock)
	{
		hay_deadlock();
	}
	sem_post(&semaforo_grado_max_multiprogramacion);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* BLOQUEOS *//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
void crear_hilo_sleep(t_pcb *pcb, int tiempo_sleep)
{
	log_info(logger, "Motivo de Bloqueo: PID: %d - Bloqueado por: SLEEP", pcb->pid);
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

	log_debug(logger, "Sleep de %d segundos", bloqueo_sleep->tiempo_sleep);
	usleep(bloqueo_sleep->tiempo_sleep * 1000 * 1000);

	pthread_mutex_lock(&mutex_detener_planificacion_corto_plazo);
	t_pcb *pcb = buscar_pcb_con_pid_en_cola(pid_proceso_bloqueado, cola_bloqueados_sleep, &mutex_cola_bloqueados_sleep);
	if (pcb != NULL)
	{
		eliminar_pcb_de_cola(pid_proceso_bloqueado, cola_bloqueados_sleep, &mutex_cola_bloqueados_sleep);
		transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_READY);
	}
	pthread_mutex_unlock(&mutex_detener_planificacion_corto_plazo);

	free(bloqueo_sleep);
}

void crear_hilo_operacion_archivo(t_pcb *pcb, int operacion_archivo, char *nombre_archivo, int modo_apertura, int puntero, int direccion_fisica, int nuevo_tamanio)
{
	log_info(logger, "Motivo de Bloqueo: PID: %d - Bloqueado por: %s", pcb->pid, nombre_archivo);
	pthread_t hilo_operacion_archivo;
	t_operacion_archivo *operacion_archivo_parametros = malloc(sizeof(t_operacion_archivo));

	operacion_archivo_parametros->nombre_archivo = malloc(strlen(nombre_archivo)+1);
	strcpy(operacion_archivo_parametros->nombre_archivo, nombre_archivo);

	operacion_archivo_parametros->pcb = pcb;
	operacion_archivo_parametros->codigo_operacion_archivo = operacion_archivo;
	operacion_archivo_parametros->modo_apertura = modo_apertura;
	operacion_archivo_parametros->puntero = puntero;
	operacion_archivo_parametros->direccion_fisica = direccion_fisica;
	operacion_archivo_parametros->nuevo_tamanio = nuevo_tamanio;

	pthread_create(&hilo_operacion_archivo, NULL, operacion_archivo_h, (void *)operacion_archivo_parametros);
}

void *operacion_archivo_h(void *argumentos)
{
	t_operacion_archivo *parametros_operacion_archivo = (t_operacion_archivo *)argumentos;

	int pid_proceso_operacion_archivo = parametros_operacion_archivo->pcb->pid;
	int existe_archivo_a_abrir;
	int tamanio_archivo_abierto;
	int lock = parametros_operacion_archivo->modo_apertura;

	if (parametros_operacion_archivo->codigo_operacion_archivo == TRUNCAR_ARCHIVO)
	{
		truncar_archivo_fs(parametros_operacion_archivo->nombre_archivo, parametros_operacion_archivo->nuevo_tamanio);
	}
	else if (parametros_operacion_archivo->codigo_operacion_archivo == LEER_ARCHIVO)
	{
		leer_archivo_fs(parametros_operacion_archivo->nombre_archivo, parametros_operacion_archivo->puntero, parametros_operacion_archivo->direccion_fisica);
	}
	else if (parametros_operacion_archivo->codigo_operacion_archivo == ESCRIBIR_ARCHIVO)
	{
		escribir_archivo_fs(parametros_operacion_archivo->nombre_archivo, parametros_operacion_archivo->puntero, parametros_operacion_archivo->direccion_fisica);
	}

	pthread_mutex_lock(&mutex_detener_planificacion_corto_plazo);
	t_pcb *pcb = buscar_pcb_con_pid_en_cola(pid_proceso_operacion_archivo, cola_bloqueados_operaciones_archivos, &mutex_cola_bloqueados_operaciones_archivos);
	if (pcb != NULL)
	{
		eliminar_pcb_de_cola(pid_proceso_operacion_archivo, cola_bloqueados_operaciones_archivos, &mutex_cola_bloqueados_operaciones_archivos);
		transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_READY);
	}
	pthread_mutex_unlock(&mutex_detener_planificacion_corto_plazo);

	free(parametros_operacion_archivo->nombre_archivo);
	free(parametros_operacion_archivo);
}

void crear_hilo_page_fault(t_pcb *pcb, int numero_pagina)
{
	log_info(logger, "Page Fault: Page Fault PID: %d - Pagina: %d", pcb->pid, numero_pagina);
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
	pthread_mutex_lock(&mutex_detener_planificacion_corto_plazo);
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
	pthread_mutex_unlock(&mutex_detener_planificacion_corto_plazo);

	free(bloqueo_page_fault);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* CONSOLA *///////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
void consola()
{
	char *valor_ingresado_por_teclado, *saveptr, *funcion_seleccionada, *path, *size_str, *nuevo_grado_multiprogramacion_str, *pid_str, *prioridad_str;
	int size, prioridad, pid, nuevo_grado_multiprogramacion;

	while (true)
	{
		valor_ingresado_por_teclado = NULL;

		do
		{
			valor_ingresado_por_teclado = readline("KERNEL> ");

		} while (valor_ingresado_por_teclado == NULL || strlen(valor_ingresado_por_teclado) == 0);

		log_debug(logger, "%s", valor_ingresado_por_teclado);

		saveptr = valor_ingresado_por_teclado;
		funcion_seleccionada = strtok_r(saveptr, " ", &saveptr);

		if (strcmp(funcion_seleccionada, DETENER_PLANIFICACION) == 0)
		{
			detener_planificacion();
		}
		else if (strcmp(funcion_seleccionada, INICIAR_PLANIFICACION) == 0)
		{
			iniciar_planificacion();
		}
		else if (strcmp(funcion_seleccionada, PROCESO_ESTADO) == 0)
		{
			listar_procesos();
		}
		else if (strcmp(funcion_seleccionada, INICIAR_PROCESO) == 0)
		{
			if ((path = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'path' para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'path' para la funcion %s.", funcion_seleccionada);
				continue;
			}

			if ((size_str = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'size' para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'size' para la funcion %s.", funcion_seleccionada);
				continue;
			}
			else
			{
				if (!(size = atoi(size_str)))
				{
					log_error(logger, "No se pudo convertir el argumento 'size' = '%s' de funcion %s a entero", size_str, funcion_seleccionada);
					printf("No se pudo convertir el argumento 'size' = '%s' de funcion %s a entero\n", size_str, funcion_seleccionada);
					continue;
				}
			}

			if ((prioridad_str = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'prioridad' para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'prioridad' para la funcion %s.", funcion_seleccionada);
				continue;
			}
			else
			{
				if (!(prioridad = atoi(prioridad_str)))
				{
					log_error(logger, "No se pudo convertir el argumento 'prioridad' = '%s' de funcion %s a entero", prioridad_str, funcion_seleccionada);
					printf("No se pudo convertir el argumento 'prioridad' = '%s' de funcion %s a entero\n", prioridad_str, funcion_seleccionada);
					continue;
				}
			}

			iniciar_proceso(path, size, prioridad);
		}
		else if (strcmp(funcion_seleccionada, FINALIZAR_PROCESO) == 0)
		{

			if ((pid_str = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'pid' para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'pid' para la funcion %s.", funcion_seleccionada);
				continue;
			}
			else
			{
				if (!(pid = atoi(pid_str)))
				{
					log_trace(logger, "No se pudo convertir el argumento 'pid' = '%s' de funcion %s a entero", pid_str, funcion_seleccionada);
					printf("No se pudo convertir el argumento 'pid' = '%s' de funcion %s a entero\n", pid_str, funcion_seleccionada);
					continue;
				}
			}

			finalizar_proceso(pid);
		}

		else if (strcmp(funcion_seleccionada, MULTIPROGRAMACION) == 0)
		{

			if ((nuevo_grado_multiprogramacion_str = strtok_r(saveptr, " ", &saveptr)) == NULL)
			{
				printf("No se encontro parametro 'nuevo_grado_multiprogramacion para la funcion %s.\n", funcion_seleccionada);
				log_error(logger, "No se encontro parametro 'nuevo_grado_multiprogramacion' para la funcion %s.", funcion_seleccionada);
				continue;
			}
			else
			{
				if (!(nuevo_grado_multiprogramacion = atoi(nuevo_grado_multiprogramacion_str)))
				{
					log_trace(logger, "No se pudo convertir el argumento 'nuevo_grado_multiprogramacion' = '%s' de funcion %s a entero", nuevo_grado_multiprogramacion_str, funcion_seleccionada);
					printf("No se pudo convertir el argumento 'nuevo_grado_multiprogramacion' = '%s' de funcion %s a entero\n", nuevo_grado_multiprogramacion_str, funcion_seleccionada);
					continue;
				}
			}

			modificar_grado_max_multiprogramacion(nuevo_grado_multiprogramacion);
		}
		else
		{
			log_error(logger, "'%s' no coincide con ninguna funcion conocida.", funcion_seleccionada);
			printf("'%s' no coincide con ninguna funcion conocida.\n", funcion_seleccionada);
		}

		free(valor_ingresado_por_teclado);
	}
}

void iniciar_proceso(char *path, int size, int prioridad)
{
	pthread_t iniciar_proceso_hilo;
	t_iniciar_proceso *iniciar_proceso_parametros = malloc(sizeof(t_iniciar_proceso));
	iniciar_proceso_parametros->path = path;
	iniciar_proceso_parametros->size = size;
	iniciar_proceso_parametros->prioridad = prioridad;
	pthread_create(&iniciar_proceso_hilo, NULL, hilo_iniciar_proceso, (void *)iniciar_proceso_parametros);
}

void *hilo_iniciar_proceso(void *argumentos)
{
	t_iniciar_proceso *iniciar_proceso_parametros = (t_iniciar_proceso *)argumentos;
	t_pcb *pcb = malloc(sizeof(t_pcb));

	pcb->path = malloc(sizeof(char) * strlen(iniciar_proceso_parametros->path) + 1);
	strcpy(pcb->path, iniciar_proceso_parametros->path);

	pcb->quantum_finalizado = false;
	pcb->estado = CODIGO_ESTADO_PROCESO_NEW;
	pcb->pid = ++proximo_pid;
	pcb->program_counter = 0;
	pcb->registro_ax = 0;
	pcb->registro_bx = 0;
	pcb->registro_cx = 0;
	pcb->registro_dx = 0;
	pcb->prioridad = iniciar_proceso_parametros->prioridad;
	pcb->size = iniciar_proceso_parametros->size;
	pcb->id_hilo_quantum = -1;
	pcb->motivo_finalizacion = FINALIZACION_SUCCESS;
	pcb->ultimo_recurso_pedido = NULL;
	pcb->tabla_archivos = list_create();
	pthread_mutex_init(&pcb->mutex_tabla_archivos, NULL);

	transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_NEW);
	free(iniciar_proceso_parametros);
}

void finalizar_proceso(int pid)
{
	pthread_t finalizar_proceso_hilo;
	t_finalizar_proceso *finalizar_proceso_parametros = malloc(sizeof(t_finalizar_proceso));
	finalizar_proceso_parametros->pid = pid;
	pthread_create(&finalizar_proceso_hilo, NULL, hilo_finalizar_proceso, (void *)finalizar_proceso_parametros);
}

void *hilo_finalizar_proceso(void *argumentos)
{
	t_finalizar_proceso *finalizar_proceso_parametros = (t_finalizar_proceso *)argumentos;
	t_pcb *pcb = buscar_pcb_con_pid(finalizar_proceso_parametros->pid);

	if (pcb != NULL)
	{
		if (pcb->estado == CODIGO_ESTADO_PROCESO_EXECUTING)
		{
			pthread_mutex_lock(&mutex_bool_planificacion_detenida);
			if (planificacion_detenida)
			{
				pthread_mutex_unlock(&mutex_bool_planificacion_detenida);
				transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
			}
			else
			{
				pthread_mutex_unlock(&mutex_bool_planificacion_detenida);
				interrumpir_proceso_en_cpu(INTERRUPCION_POR_KILL);
			}
		}
		else
		{
			transicionar_proceso(pcb, CODIGO_ESTADO_PROCESO_EXIT);
		}
	}

	free(finalizar_proceso_parametros);
}

void iniciar_planificacion()
{
	pthread_mutex_lock(&mutex_bool_planificacion_detenida);
	if (planificacion_detenida)
	{
		log_info(logger, "Inicio de planificacion: INICIO DE PLANIFICACIÓN");
		planificacion_detenida = false;
		pthread_mutex_unlock(&mutex_detener_planificacion_corto_plazo);
		pthread_mutex_unlock(&mutex_detener_planificacion_largo_plazo);
	}
	pthread_mutex_unlock(&mutex_bool_planificacion_detenida);
}

void detener_planificacion()
{
	pthread_mutex_lock(&mutex_bool_planificacion_detenida);
	if (!planificacion_detenida)
	{
		log_info(logger, "Pausa planificacion: PAUSA DE PLANIFICACIÓN");
		planificacion_detenida = true;
		pthread_mutex_lock(&mutex_detener_planificacion_corto_plazo);
		pthread_mutex_lock(&mutex_detener_planificacion_largo_plazo);
	}
	pthread_mutex_unlock(&mutex_bool_planificacion_detenida);
}

void modificar_grado_max_multiprogramacion(int nuevo_grado_max_multiprogramacion)
{
	int grado_max_multiprogramacion_anterior = grado_max_multiprogramacion_actual;
	grado_max_multiprogramacion_actual = nuevo_grado_max_multiprogramacion;
	log_info(logger, "Cambio de Grado de Multiprogramacion: Grado anterior: %d - Grado actual: %d", grado_max_multiprogramacion_anterior, grado_max_multiprogramacion_actual);
	sem_destroy(&semaforo_grado_max_multiprogramacion);
	sem_init(&semaforo_grado_max_multiprogramacion, false, nuevo_grado_max_multiprogramacion - 1);
}

void listar_procesos()
{
	pthread_mutex_lock(&mutex_procesos);

	char *procesos_en_new_string_dinamico = crear_string_dinamico();
	char *procesos_en_ready_string_dinamico = crear_string_dinamico();
	char *procesos_en_executing_string_dinamico = crear_string_dinamico();
	char *procesos_en_blocked_string_dinamico = crear_string_dinamico();

	t_list_iterator *iterador = list_iterator_create(procesos);
	while (list_iterator_has_next(iterador))
	{
		t_pcb *pcb_a_loguear = list_iterator_next(iterador);
		if (pcb_a_loguear->estado == CODIGO_ESTADO_PROCESO_NEW)
		{
			procesos_en_new_string_dinamico = agregar_entero_a_string_dinamico(procesos_en_new_string_dinamico, pcb_a_loguear->pid);
			procesos_en_new_string_dinamico = agregar_string_a_string_dinamico(procesos_en_new_string_dinamico, ",");
		}
		else if (pcb_a_loguear->estado == CODIGO_ESTADO_PROCESO_READY)
		{
			procesos_en_ready_string_dinamico = agregar_entero_a_string_dinamico(procesos_en_ready_string_dinamico, pcb_a_loguear->pid);
			procesos_en_ready_string_dinamico = agregar_string_a_string_dinamico(procesos_en_ready_string_dinamico, ",");
		}
		else if (pcb_a_loguear->estado == CODIGO_ESTADO_PROCESO_EXECUTING)
		{
			procesos_en_executing_string_dinamico = agregar_entero_a_string_dinamico(procesos_en_executing_string_dinamico, pcb_a_loguear->pid);
			procesos_en_executing_string_dinamico = agregar_string_a_string_dinamico(procesos_en_executing_string_dinamico, ",");
		}
		else if (pcb_a_loguear->estado == CODIGO_ESTADO_PROCESO_BLOCKED)
		{
			procesos_en_blocked_string_dinamico = agregar_entero_a_string_dinamico(procesos_en_blocked_string_dinamico, pcb_a_loguear->pid);
			procesos_en_blocked_string_dinamico = agregar_string_a_string_dinamico(procesos_en_blocked_string_dinamico, ",");
		}
	}
	list_iterator_destroy(iterador);

	if (strlen(procesos_en_new_string_dinamico))
	{
		procesos_en_new_string_dinamico[strlen(procesos_en_new_string_dinamico) - 1] = '\0';
	}

	if (strlen(procesos_en_ready_string_dinamico))
	{
		procesos_en_ready_string_dinamico[strlen(procesos_en_ready_string_dinamico) - 1] = '\0';
	}

	if (strlen(procesos_en_executing_string_dinamico))
	{
		procesos_en_executing_string_dinamico[strlen(procesos_en_executing_string_dinamico) - 1] = '\0';
	}

	if (strlen(procesos_en_blocked_string_dinamico))
	{
		procesos_en_blocked_string_dinamico[strlen(procesos_en_blocked_string_dinamico) - 1] = '\0';
	}

	printf("Estado: NEW     - Procesos: %s", procesos_en_new_string_dinamico);
	printf("\nEstado: READY   - Procesos: %s", procesos_en_ready_string_dinamico);
	printf("\nEstado: EXEC    - Procesos: %s", procesos_en_executing_string_dinamico);
	printf("\nEstado: BLOCKED - Procesos: %s\n", procesos_en_blocked_string_dinamico);

	log_info(logger, "Listar procesos por estado:");
	log_info(logger, "Estado: NEW - Procesos: %s", procesos_en_new_string_dinamico);
	log_info(logger, "Estado: READY - Procesos: %s", procesos_en_ready_string_dinamico);
	log_info(logger, "Estado: EXEC - Procesos: %s", procesos_en_executing_string_dinamico);
	log_info(logger, "Estado: BLOCKED - Procesos: %s", procesos_en_blocked_string_dinamico);

	free(procesos_en_new_string_dinamico);
	free(procesos_en_ready_string_dinamico);
	free(procesos_en_executing_string_dinamico);
	free(procesos_en_blocked_string_dinamico);

	pthread_mutex_unlock(&mutex_procesos);
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* CPU *///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
t_contexto_de_ejecucion *recibir_paquete_de_cpu_dispatch(op_code *codigo_operacion_recibido, int *tiempo_sleep, int *motivo_interrupcion, char **nombre_recurso, int *codigo_error, int *numero_pagina, char **nombre_archivo, int *modo_apertura, int *posicion_puntero_archivo, int *direccion_fisica, int *nuevo_tamanio_archivo, int *fs_opcode)
{
	// Esperar operacion
	*codigo_operacion_recibido = esperar_operacion(logger, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_DISPATCH, conexion_con_cpu_dispatch);
	t_contexto_de_ejecucion *contexto_de_ejecucion;

	if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION)
	{
		contexto_de_ejecucion = leer_paquete_contexto_de_ejecucion(logger, conexion_con_cpu_dispatch, *codigo_operacion_recibido, NOMBRE_MODULO_CPU_DISPATCH, NOMBRE_MODULO_KERNEL);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_ser_interrumpido(logger, conexion_con_cpu_dispatch, motivo_interrupcion);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_sleep(logger, conexion_con_cpu_dispatch, tiempo_sleep);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_WAIT)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_wait(logger, conexion_con_cpu_dispatch, nombre_recurso);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_signal(logger, conexion_con_cpu_dispatch, nombre_recurso);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_ERROR)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_error(logger, conexion_con_cpu_dispatch, codigo_error);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_pagefault(logger, conexion_con_cpu_dispatch, numero_pagina);
	}
	else if (*codigo_operacion_recibido == SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM)
	{
		contexto_de_ejecucion = leer_paquete_solicitud_devolver_proceso_por_operacion_filesystem(logger, conexion_con_cpu_dispatch, nombre_archivo, modo_apertura, posicion_puntero_archivo, direccion_fisica, nuevo_tamanio_archivo, fs_opcode);
	}

	return contexto_de_ejecucion;
}

void ejecutar_proceso_en_cpu(t_pcb *pcb_proceso_a_ejecutar)
{
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

	pthread_mutex_lock(&mutex_proceso_ejecutando);
	pcb_ejecutando = pcb_proceso_a_ejecutar;
	pthread_mutex_unlock(&mutex_proceso_ejecutando);
}

void interrumpir_proceso_en_cpu(int motivo_interrupcion)
{
	log_debug(logger, "-INTERRUPT-");
	t_paquete *paquete_solicitud_interrumpir_ejecucion = crear_paquete_solicitud_interrumpir_proceso(logger, motivo_interrupcion);
	enviar_paquete(logger, conexion_con_cpu_interrupt, paquete_solicitud_interrumpir_ejecucion, NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_CPU_INTERRUPT);
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
	bool _filtro_proceso_por_id(t_pcb * pcb)
	{
		return pcb->pid == pid;
	};

	t_pcb *pcb = list_find_thread_safe(procesos, (void *)_filtro_proceso_por_id, &mutex_procesos);
	return pcb;
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
	eliminar_pcb_de_lista(pid, cola->elements, mutex);
}

void eliminar_pcb_de_lista(int pid, t_list *lista, pthread_mutex_t *mutex)
{
	bool _filtro_proceso_por_id(t_pcb * pcb)
	{
		return pcb->pid == pid;
	};

	list_remove_by_condition_thread_safe(lista, (void *)_filtro_proceso_por_id, mutex);
}

void destruir_pcb(t_pcb *pcb)
{
	if (pcb->ultimo_recurso_pedido != NULL)
	{
		free(pcb->ultimo_recurso_pedido);
	}

	void _destruir_archivo_abierto(t_archivo_abierto_proceso * archivo_abierto_proceso)
	{
		free(archivo_abierto_proceso->nombre_archivo);
		free(archivo_abierto_proceso);
	};

	list_destroy_and_destroy_elements_thread_safe(pcb->tabla_archivos, (void *)_destruir_archivo_abierto, &pcb->mutex_tabla_archivos);
	free(pcb->path);
	free(pcb);
}

void push_cola_ready(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_cola_ready);

	// Agrego PCB a cola de ready
	queue_push(cola_ready, pcb);

	// Si planifico con prioridades, reordeno la cola de ready por prioridad
	if (planifico_con_prioridades)
	{
		bool _comparador_prioridades(t_pcb * pcb1, t_pcb * pcb2)
		{
			return pcb1->prioridad < pcb2->prioridad;
		}
		list_sort(cola_ready->elements, (void *)_comparador_prioridades);
	}

	// Logueo la cola de ready
	char *string_dinamico = crear_string_dinamico();
	t_list_iterator *iterador = list_iterator_create(cola_ready->elements);
	while (list_iterator_has_next(iterador))
	{
		t_pcb *pcb_a_loguear = list_iterator_next(iterador);
		string_dinamico = agregar_entero_a_string_dinamico(string_dinamico, pcb_a_loguear->pid);
		string_dinamico = agregar_string_a_string_dinamico(string_dinamico, ",");
	}
	list_iterator_destroy(iterador);

	if (strlen(string_dinamico))
	{
		string_dinamico[strlen(string_dinamico) - 1] = '\0';
	}
	log_info(logger, "Ingreso a Ready: Cola Ready %s: [%s]", configuracion_kernel->algoritmo_planificacion, string_dinamico);
	free(string_dinamico);

	// Si planifico con prioridades y el PCB que llego tiene mayor prioridad que el que esta ejecutando, lo desalojo
	pthread_mutex_lock(&mutex_proceso_ejecutando);
	if (planifico_con_prioridades && pcb_ejecutando != NULL && pcb->prioridad < pcb_ejecutando->prioridad)
	{
		interrumpir_proceso_en_cpu(INTERRUPCION_POR_DESALOJO);
		log_info(logger, "PID: %d - Desalojado por proceso con mayor prioridad", pcb_ejecutando->pid);
		pthread_mutex_unlock(&mutex_proceso_ejecutando);
	}
	else
	{
		pthread_mutex_unlock(&mutex_proceso_ejecutando);
	}

	// Notifico al planificador de corto plazo
	sem_post(&semaforo_hay_algun_proceso_en_cola_ready);

	pthread_mutex_unlock(&mutex_cola_ready);
}

void log_fin_de_proceso(t_pcb *pcb)
{
	if (pcb->motivo_finalizacion == FINALIZACION_SUCCESS)
	{
		log_info(logger, "Fin de Proceso: Finaliza el proceso PID: %d - Motivo SUCCESS", pcb->pid);
	}
	else if (pcb->motivo_finalizacion == FINALIZACION_INVALID_RESOURCE)
	{
		log_info(logger, "Fin de Proceso: Finaliza el proceso PID: %d - Motivo INVALID_RESOURCE", pcb->pid);
	}
	else if (pcb->motivo_finalizacion == FINALIZACION_INVALID_WRITE)
	{
		log_info(logger, "Fin de Proceso: Finaliza el proceso PID: %d - Motivo INVALID_WRITE", pcb->pid);
	}
}

char *crear_string_dinamico()
{
	char *string_dinamico = malloc(sizeof(char));
	strcpy(string_dinamico, "");
	return string_dinamico;
}

char *agregar_string_a_string_dinamico(char *string_dinamico, char *string_a_agregar)
{
	int tamanio_anterior = strlen(string_dinamico) + 1;
	int tamanio_a_aumentar = strlen(string_a_agregar);
	int nuevo_tamanio = tamanio_anterior + tamanio_a_aumentar;
	string_dinamico = realloc(string_dinamico, nuevo_tamanio * sizeof(char));
	strcpy(string_dinamico + (tamanio_anterior - 1) * sizeof(char), string_a_agregar);
	return string_dinamico;
}

char *agregar_entero_a_string_dinamico(char *string_dinamico, int entero)
{
	int cantidad_digitos_entero;
	if (entero == 0)
	{
		cantidad_digitos_entero = 1;
	}
	else
	{
		cantidad_digitos_entero = floor(log10(abs(entero))) + 1;
		if (entero < 0)
		{
			cantidad_digitos_entero++;
		}
	}
	char entero_como_string[cantidad_digitos_entero + 1];
	sprintf(entero_como_string, "%d", entero);
	int tamanio_anterior = strlen(string_dinamico) + 1;
	int tamanio_a_aumentar = strlen(entero_como_string);
	int nuevo_tamanio = tamanio_anterior + tamanio_a_aumentar;
	string_dinamico = realloc(string_dinamico, nuevo_tamanio * sizeof(char));
	strcpy(string_dinamico + (tamanio_anterior - 1) * sizeof(char), entero_como_string);
	return string_dinamico;
}

////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* RECURSOS *//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////* ////////// *////////////////////////////////////////////////////////////////////////
t_recurso *crear_recurso(char *nombre, int instancias)
{
	t_recurso *recurso = malloc(sizeof(t_recurso));

	recurso->nombre = nombre;
	recurso->instancias_iniciales = instancias;
	recurso->instancias_disponibles = instancias;
	recurso->pcbs_bloqueados = queue_create();
	recurso->pcbs_asignados = list_create();

	recurso->es_archivo = false;
	recurso->tamanio_archivo = -1;
	recurso->pcb_lock_escritura = NULL;
	recurso->pcbs_lock_lectura = list_create();
	recurso->pcbs_bloqueados_por_archivo = queue_create();

	pthread_mutex_init(&recurso->mutex_recurso, NULL);

	return recurso;
}

t_recurso *crear_recurso_archivo(char *nombre_archivo, int lock_actual, int tamanio, t_pcb *pcb)
{
	t_recurso *recurso = malloc(sizeof(t_recurso));

	recurso->nombre = malloc(strlen(nombre_archivo)+1);
	strcpy(recurso->nombre, nombre_archivo);
	recurso->instancias_iniciales = 1;
	recurso->instancias_disponibles = 0;
	recurso->pcbs_bloqueados = queue_create();
	recurso->pcbs_asignados = list_create();

	recurso->es_archivo = true;
	recurso->tamanio_archivo = tamanio;
	recurso->pcb_lock_escritura = NULL;
	recurso->pcbs_lock_lectura = list_create();
	recurso->pcbs_bloqueados_por_archivo = queue_create();

	pthread_mutex_init(&recurso->mutex_recurso, NULL);

	if (lock_actual == LOCK_ESCRITURA)
	{
		recurso->pcb_lock_escritura = pcb;
	}
	else
	{
		list_add(recurso->pcbs_lock_lectura, pcb);
	}

	return recurso;
}

t_recurso *buscar_recurso_por_nombre(char *nombre_recurso)
{
	bool _filtro_recurso_por_nombre(t_recurso * recurso)
	{
		return strcmp(nombre_recurso, recurso->nombre) == 0;
	};

	t_recurso *recurso = list_find_thread_safe(recursos, (void *)_filtro_recurso_por_nombre, &mutex_recursos);
	return recurso;
}

bool recurso_esta_asignado_a_pcb(t_recurso *recurso, int pid)
{
	bool _filtro_proceso_por_id(t_pcb * pcb)
	{
		return pcb->pid == pid;
	};

	bool recurso_kernel_asignado = false;
	bool recurso_archivo_asignado = false;

	pthread_mutex_lock(&recurso->mutex_recurso);
	if (recurso->es_archivo)
	{
		if (recurso->pcb_lock_escritura != NULL)
		{
			if (recurso->pcb_lock_escritura->pid == pid)
			{
				recurso_archivo_asignado = true;
			}
		}

		t_pcb *pcb_lock_lectura = list_find(recurso->pcbs_lock_lectura, (void *)_filtro_proceso_por_id);
		if (pcb_lock_lectura != NULL)
		{
			recurso_archivo_asignado = true;
		}
	}
	else
	{
		recurso_kernel_asignado = (list_find(recurso->pcbs_asignados, (void *)_filtro_proceso_por_id) != NULL);
	}
	pthread_mutex_unlock(&recurso->mutex_recurso);

	return recurso_kernel_asignado || recurso_archivo_asignado;
}

bool desasignar_recurso_a_pcb(t_recurso *recurso, int pid)
{
	int pid_lectura_filtro;
	bool elimino_el_recurso = false;

	bool _filtro_proceso_por_id(t_pcb * pcb)
	{
		return pcb->pid == pid;
	};

	bool _filtro_archivo_abierto_por_nombre(t_archivo_abierto_proceso * archivo_abierto_proceso)
	{
		return strcmp(recurso->nombre, archivo_abierto_proceso->nombre_archivo) == 0;
	};

	bool _filtro_recurso_por_nombre(t_recurso * recurso_filtro)
	{
		return strcmp(recurso->nombre, recurso_filtro->nombre) == 0;
	};

	void _destruir_archivo_abierto(t_archivo_abierto_proceso * archivo_abierto_proceso)
	{
		free(archivo_abierto_proceso->nombre_archivo);
		free(archivo_abierto_proceso);
	};

	void _destruir_recurso(t_recurso * recurso)
	{
		free(recurso->nombre);

		queue_destroy(recurso->pcbs_bloqueados);
		list_destroy(recurso->pcbs_asignados);

		queue_destroy(recurso->pcbs_bloqueados_por_archivo);
		list_destroy(recurso->pcbs_lock_lectura);

		free(recurso);
	};

	bool _filtro_pcb_bloqueado_archivo_por_modo_lectura(t_pcb_bloqueado_archivo * pcb_bloqueado_archivo)
	{
		return pcb_bloqueado_archivo->lock == LOCK_LECTURA;
	};

	bool _filtro_pcb_bloqueado_archivo_por_pid(t_pcb_bloqueado_archivo * pcb_bloqueado_archivo)
	{
		return pcb_bloqueado_archivo->pcb->pid == pid_lectura_filtro;
	};

	pthread_mutex_lock(&recurso->mutex_recurso);
	log_debug(logger, "Desasignando %s a PID %d", recurso->nombre, pid);
	if (recurso->es_archivo)
	{
		// Elimino lock de escritura (si pid lo tenia)
		if (recurso->pcb_lock_escritura != NULL)
		{
			if (recurso->pcb_lock_escritura->pid == pid)
			{
				list_remove_and_destroy_by_condition_thread_safe(recurso->pcb_lock_escritura->tabla_archivos, (void *)_filtro_archivo_abierto_por_nombre, (void *)_destruir_archivo_abierto, &recurso->pcb_lock_escritura->mutex_tabla_archivos);
				recurso->pcb_lock_escritura = NULL;
				recurso->instancias_iniciales = 1;
				recurso->instancias_disponibles = 1;
			}
		}

		// Elimino locks de lectura (si pid lo tenia)
		t_pcb *pcb_lock_lectura = list_find(recurso->pcbs_lock_lectura, (void *)_filtro_proceso_por_id);
		if (pcb_lock_lectura != NULL)
		{
			list_remove_by_condition(recurso->pcbs_lock_lectura, (void *)_filtro_proceso_por_id);
			recurso->instancias_iniciales--;
			list_remove_and_destroy_by_condition_thread_safe(pcb_lock_lectura->tabla_archivos, (void *)_filtro_archivo_abierto_por_nombre, (void *)_destruir_archivo_abierto, &pcb_lock_lectura->mutex_tabla_archivos);
		}

		if (recurso->pcb_lock_escritura == NULL && list_is_empty(recurso->pcbs_lock_lectura))
		{
			if (queue_is_empty(recurso->pcbs_bloqueados_por_archivo))
			{
				// Si me quedo TODO vacio (lock escritura, locks de lectura, pcbs bloqueados por archivo), destruyo el recurso
				list_remove_and_destroy_by_condition_thread_safe(recursos, (void *)_filtro_recurso_por_nombre, (void *)_destruir_recurso, &mutex_recursos);
				elimino_el_recurso = true;
			}
			else
			{
				t_pcb_bloqueado_archivo *pcb_a_desbloquear = queue_pop(recurso->pcbs_bloqueados_por_archivo);

				if (pcb_a_desbloquear->lock == LOCK_ESCRITURA)
				{
					recurso->pcb_lock_escritura = pcb_a_desbloquear->pcb;
					recurso->instancias_iniciales = 1;
					recurso->instancias_disponibles = 0;
					transicionar_proceso(pcb_a_desbloquear->pcb, CODIGO_ESTADO_PROCESO_READY);
					free(pcb_a_desbloquear);
				}
				else
				{
					list_add(recurso->pcbs_lock_lectura, pcb_a_desbloquear->pcb);
					recurso->instancias_iniciales = 1;
					recurso->instancias_disponibles = 0;
					transicionar_proceso(pcb_a_desbloquear->pcb, CODIGO_ESTADO_PROCESO_READY);
					free(pcb_a_desbloquear);

					t_pcb_bloqueado_archivo *pcb_a_desbloquear_por_lectura = list_find(recurso->pcbs_bloqueados_por_archivo->elements, (void *)_filtro_pcb_bloqueado_archivo_por_modo_lectura);
					while (pcb_a_desbloquear_por_lectura != NULL)
					{
						pid_lectura_filtro = pcb_a_desbloquear_por_lectura->pcb->pid;
						list_remove_by_condition(recurso->pcbs_bloqueados_por_archivo->elements, (void *)_filtro_pcb_bloqueado_archivo_por_pid);
						list_add(recurso->pcbs_lock_lectura, pcb_a_desbloquear_por_lectura->pcb);
						recurso->instancias_iniciales++;
						transicionar_proceso(pcb_a_desbloquear_por_lectura->pcb, CODIGO_ESTADO_PROCESO_READY);
						free(pcb_a_desbloquear_por_lectura);
						pcb_a_desbloquear_por_lectura = list_find(recurso->pcbs_bloqueados_por_archivo->elements, (void *)_filtro_pcb_bloqueado_archivo_por_modo_lectura);
					}
				}
			}
		}
	}
	else
	{
		list_remove_by_condition(recurso->pcbs_asignados, (void *)_filtro_proceso_por_id);
		recurso->instancias_disponibles++;
		if (recurso->instancias_disponibles <= 0)
		{
			if (!queue_is_empty(recurso->pcbs_bloqueados))
			{
				t_pcb *pcb_a_desbloquear = queue_pop(recurso->pcbs_bloqueados);
				list_add(recurso->pcbs_asignados, pcb_a_desbloquear);
				transicionar_proceso(pcb_a_desbloquear, CODIGO_ESTADO_PROCESO_READY);
			}
		}
	}
	pthread_mutex_unlock(&recurso->mutex_recurso);
	log_debug(logger, "Desasignado recurso a PID %d", pid);
	return elimino_el_recurso;
}

void desasignar_todos_los_recursos_a_pcb(int pid)
{
	//pthread_mutex_lock(&mutex_recursos);
	for (int i = 0; i < list_size(recursos); i++)
	{
		t_recurso *recurso = list_get(recursos, i);

		bool _filtro_proceso_por_id(t_pcb * pcb)
		{
			return pcb->pid == pid;
		};
		list_remove_by_condition_thread_safe(recurso->pcbs_bloqueados->elements, (void *)_filtro_proceso_por_id, &recurso->mutex_recurso);

		while (recurso_esta_asignado_a_pcb(recurso, pid))
		{
			if (desasignar_recurso_a_pcb(recurso, pid))
			{
				// Si elimino el recurso, salgo
				break;
			}
		}
	}
	//pthread_mutex_unlock(&mutex_recursos);
}

t_list *obtener_procesos_analisis_deadlock()
{
	t_list *resultado = list_create();
	t_pcb *pcb;
	t_pcb_analisis_deadlock *pcb_a_analizar_existente;
	t_pcb_analisis_deadlock *pcb_a_analizar_nuevo;
	t_recurso *recurso;
	t_list_iterator *iterador_pcbs_asignados;
	t_list_iterator *iterador_pcbs_bloqueados;
	t_pcb_bloqueado_archivo *pcb_bloqueado_archivo;
	int i, j;
	int cantidad_de_recursos = list_size(recursos);

	bool _filtro_pcb_por_id(t_pcb * unpcb)
	{
		return unpcb->pid == pcb->pid;
	};

	void _crear_pcb_a_analizar()
	{
		pcb_a_analizar_nuevo = malloc(sizeof(t_pcb_analisis_deadlock));

		pcb_a_analizar_nuevo->finalizado = false;
		pcb_a_analizar_nuevo->pid = pcb->pid;
		pcb_a_analizar_nuevo->recursos_asignados = malloc(cantidad_de_recursos * sizeof(int));
		pcb_a_analizar_nuevo->solicitudes_actuales = malloc(cantidad_de_recursos * sizeof(int));

		if (pcb->ultimo_recurso_pedido != NULL)
		{
			pcb_a_analizar_nuevo->ultimo_recurso_pedido = malloc(strlen(pcb->ultimo_recurso_pedido)+1);
			strcpy(pcb_a_analizar_nuevo->ultimo_recurso_pedido, pcb->ultimo_recurso_pedido);
		}

		for (j = 0; j < cantidad_de_recursos; j++)
		{
			pcb_a_analizar_nuevo->recursos_asignados[j] = 0;
			pcb_a_analizar_nuevo->solicitudes_actuales[j] = 0;
		}

		list_add(resultado, pcb_a_analizar_nuevo);
	};

	for (i = 0; i < cantidad_de_recursos; i++)
	{
		recurso = list_get(recursos, i);

		log_debug(logger, "ANALISIS DE DETECCION DE DEADLOCK: ANALIZANDO RECURSO %s", recurso->nombre);

		// INSTANCIAS ASIGNADAS NO ARCHIVOS
		iterador_pcbs_asignados = list_iterator_create(recurso->pcbs_asignados);
		while (list_iterator_has_next(iterador_pcbs_asignados))
		{
			pcb = list_iterator_next(iterador_pcbs_asignados);
			pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);

			if (pcb_a_analizar_existente == NULL)
			{
				_crear_pcb_a_analizar();
				pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);
			}

			pcb_a_analizar_existente->recursos_asignados[i]++;
		}
		list_iterator_destroy(iterador_pcbs_asignados);

		// INSTANCIAS ASIGNADAS ARCHIVOS (lock escritura)
		if (recurso->pcb_lock_escritura != NULL)
		{
			pcb = recurso->pcb_lock_escritura;
			pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);

			if (pcb_a_analizar_existente == NULL)
			{
				_crear_pcb_a_analizar();
				pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);
			}
			pcb_a_analizar_existente->recursos_asignados[i]++;
		}

		// INSTANCIAS ASIGNADAS ARCHIVOS (locks lectura)
		iterador_pcbs_asignados = list_iterator_create(recurso->pcbs_lock_lectura);
		while (list_iterator_has_next(iterador_pcbs_asignados))
		{
			pcb = list_iterator_next(iterador_pcbs_asignados);
			pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);

			if (pcb_a_analizar_existente == NULL)
			{
				_crear_pcb_a_analizar();
				pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);
			}
			pcb_a_analizar_existente->recursos_asignados[i]++;
		}
		list_iterator_destroy(iterador_pcbs_asignados);

		// SOLICITUDES ACTUALES PARA ARCHIVOS
		iterador_pcbs_bloqueados = list_iterator_create(recurso->pcbs_bloqueados_por_archivo->elements);
		while (list_iterator_has_next(iterador_pcbs_bloqueados))
		{
			t_pcb_bloqueado_archivo *pcb_bloqueado_archivo = list_iterator_next(iterador_pcbs_bloqueados);
			pcb = pcb_bloqueado_archivo->pcb;
			pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);

			if (pcb_a_analizar_existente == NULL)
			{
				_crear_pcb_a_analizar();
				pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);
			}
			pcb_a_analizar_existente->solicitudes_actuales[i]++;
		}
		list_iterator_destroy(iterador_pcbs_bloqueados);

		// SOLICITUDES ACTUALES PARA NO ARCHIVOS
		iterador_pcbs_bloqueados = list_iterator_create(recurso->pcbs_bloqueados->elements);
		while (list_iterator_has_next(iterador_pcbs_bloqueados))
		{
			log_debug(logger, "Analizando PCBs bloqueados de recurso %s", recurso->nombre);
			pcb = list_iterator_next(iterador_pcbs_bloqueados);
			pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);

			if (pcb_a_analizar_existente == NULL)
			{
				_crear_pcb_a_analizar();
				pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);
			}
			pcb_a_analizar_existente->solicitudes_actuales[i]++;
		}
		list_iterator_destroy(iterador_pcbs_bloqueados);
	}

	return resultado;
}

int *obtener_vector_recursos_disponibles()
{
	int cantidad_de_recursos = list_size(recursos);
	int *recursos_disponibles = malloc(cantidad_de_recursos * sizeof(int));

	for (int i = 0; i < cantidad_de_recursos; i++)
	{
		t_recurso *recurso = list_get(recursos, i);

		if (recurso->instancias_disponibles < 0)
		{
			recursos_disponibles[i] = 0;
		}
		else
		{
			recursos_disponibles[i] = recurso->instancias_disponibles;
		}
	}

	return recursos_disponibles;
}

bool hay_deadlock()
{
	log_info(logger, "Proceso de deteccion de deadlock: ANALISIS DE DETECCION DE DEADLOCK");

	pthread_mutex_lock(&mutex_recursos);
	int cantidad_de_recursos = list_size(recursos);
	log_debug(logger, "ANALISIS DE DETECCION DE DEADLOCK: CANTIDAD DE RECURSOS %d", cantidad_de_recursos);
	int *recursos_disponibles = obtener_vector_recursos_disponibles();
	//-
	for (int i = 0; i < cantidad_de_recursos; i++)
	{
		log_debug(logger, "ANALISIS DE DETECCION DE DEADLOCK: RECURSO DISPONIBLE[%d] %d", i, recursos_disponibles[i]);
	}
	//-
	t_list *procesos_a_analizar = obtener_procesos_analisis_deadlock();
	//--
	t_list_iterator *mi_iterador_log = list_iterator_create(procesos_a_analizar);
	while (list_iterator_has_next(mi_iterador_log))
	{
		t_pcb_analisis_deadlock *pcb_analisis_log = list_iterator_next(mi_iterador_log);
		for (int i = 0; i < cantidad_de_recursos; i++)
		{
			log_debug(logger, "ANALISIS DE DETECCION DE DEADLOCK: PID %d PETICIONES ACTUALES[%d] %d", pcb_analisis_log->pid, i, pcb_analisis_log->solicitudes_actuales[i]);
		}
		for (int i = 0; i < cantidad_de_recursos; i++)
		{
			log_debug(logger, "ANALISIS DE DETECCION DE DEADLOCK: PID %d RECURSOS ASIGNADOS[%d] %d", pcb_analisis_log->pid, i, pcb_analisis_log->recursos_asignados[i]);
		}
	}
	list_iterator_destroy(mi_iterador_log);
	//--

	bool finalice_alguno = true;
	t_list_iterator *iterador_procesos_a_analizar;
	t_pcb_analisis_deadlock *pcb_analisis_deadlock;
	int cantidad_procesos_a_analizar = list_size(procesos_a_analizar);
	int cantidad_iteraciones_realizadas = 0;
	bool ya_estaba_en_deadlock = en_deadlock;

	if (list_is_empty(procesos_a_analizar))
	{
		log_debug(logger, "NO HAY DEADLOCK");
		pthread_mutex_unlock(&mutex_recursos);
		return false;
	}

	while (finalice_alguno && cantidad_iteraciones_realizadas < cantidad_procesos_a_analizar)
	{
		finalice_alguno = false;
		iterador_procesos_a_analizar = list_iterator_create(procesos_a_analizar);

		while (list_iterator_has_next(iterador_procesos_a_analizar) && !finalice_alguno)
		{
			pcb_analisis_deadlock = list_iterator_next(iterador_procesos_a_analizar);

			if (!pcb_analisis_deadlock->finalizado)
			{
				bool se_puede_satisfacer_solicitudes = true;

				for (int i = 0; i < cantidad_de_recursos; i++)
				{
					se_puede_satisfacer_solicitudes = se_puede_satisfacer_solicitudes && recursos_disponibles[i] >= pcb_analisis_deadlock->solicitudes_actuales[i];
				}

				if (se_puede_satisfacer_solicitudes)
				{
					finalice_alguno = true;
					pcb_analisis_deadlock->finalizado = true;

					for (int i = 0; i < cantidad_de_recursos; i++)
					{
						recursos_disponibles[i] = recursos_disponibles[i] + pcb_analisis_deadlock->recursos_asignados[i];
					}
				}
			}
		}
		list_iterator_destroy(iterador_procesos_a_analizar);

		cantidad_iteraciones_realizadas++;
	}

	en_deadlock = cantidad_iteraciones_realizadas < cantidad_procesos_a_analizar;

	// INICIO LOG/IMPRIMIR DEADLOCK
	if (en_deadlock)
	{
		printf("\n-");
		iterador_procesos_a_analizar = list_iterator_create(procesos_a_analizar);
		while (list_iterator_has_next(iterador_procesos_a_analizar))
		{
			pcb_analisis_deadlock = list_iterator_next(iterador_procesos_a_analizar);
			if (!pcb_analisis_deadlock->finalizado)
			{
				char *string_dinamico = crear_string_dinamico();
				string_dinamico = agregar_string_a_string_dinamico(string_dinamico, "Deteccion de deadlock: Deadlock detectado: ");
				string_dinamico = agregar_entero_a_string_dinamico(string_dinamico, pcb_analisis_deadlock->pid);
				string_dinamico = agregar_string_a_string_dinamico(string_dinamico, " - Recursos en posesion: ");

				bool agregue_recurso_a_lista_posesion = false;
				for (int i = 0; i < cantidad_de_recursos; i++)
				{
					t_recurso *recurso = list_get(recursos, i);

					if (recurso_esta_asignado_a_pcb(recurso, pcb_analisis_deadlock->pid))
					{
						if (agregue_recurso_a_lista_posesion)
						{
							string_dinamico = agregar_string_a_string_dinamico(string_dinamico, ",");
						}
						else
						{
							agregue_recurso_a_lista_posesion = true;
						}

						pthread_mutex_lock(&recurso->mutex_recurso);
						string_dinamico = agregar_string_a_string_dinamico(string_dinamico, recurso->nombre);
						pthread_mutex_unlock(&recurso->mutex_recurso);
					}
				}

				string_dinamico = agregar_string_a_string_dinamico(string_dinamico, " - Recurso requerido: ");

				if (pcb_analisis_deadlock->ultimo_recurso_pedido != NULL)
				{
					string_dinamico = agregar_string_a_string_dinamico(string_dinamico, pcb_analisis_deadlock->ultimo_recurso_pedido);
				}

				log_info(logger, "%s", string_dinamico);
				printf("\n%s", string_dinamico);
				free(string_dinamico);
			}
		}
		list_iterator_destroy(iterador_procesos_a_analizar);
		printf("\n-\n");
	}
	else
	{
		log_debug(logger, "NO HAY DEADLOCK");
	}
	// FIN LOG/IMPRIMIR DEADLOCK

	pthread_mutex_unlock(&mutex_recursos);

	// Limpieza
	free(recursos_disponibles);
	void _destruir_pcb_analisis_deadlock(t_pcb_analisis_deadlock * pcb_analisis_deadlock)
	{
		free(pcb_analisis_deadlock->recursos_asignados);
		free(pcb_analisis_deadlock->solicitudes_actuales);
		free(pcb_analisis_deadlock);
	};
	list_destroy_and_destroy_elements(procesos_a_analizar, (void *)_destruir_pcb_analisis_deadlock);

	return en_deadlock;
}