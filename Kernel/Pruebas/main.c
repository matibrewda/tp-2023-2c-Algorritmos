#include "main.h"

t_log *logger = NULL;
char *nombres_recursos[] = {"R1", "R2", "R3"};
int instancias_recursos[] = {3, 2, 3}; // Recursos totales
int cantidad_de_recursos = 3;

t_list *recursos;

int *recursos_disponibles;

int main()
{
	logger = crear_logger("pruebas.log", "PRUEBA", LOG_LEVEL_TRACE);
	crear_recursos();

	t_mipcb *p1 = malloc(sizeof(t_mipcb));
	p1->pid = 1;
	t_mipcb *p2 = malloc(sizeof(t_mipcb));
	p2->pid = 2;
	t_mipcb *p3 = malloc(sizeof(t_mipcb));
	p3->pid = 3;
	t_mipcb *p4 = malloc(sizeof(t_mipcb));
	p4->pid = 4;

	t_rec *r1 = buscar_recurso_por_nombre("R1");
	t_rec *r2 = buscar_recurso_por_nombre("R2");
	t_rec *r3 = buscar_recurso_por_nombre("R3");

	r1->instancias_disponibles = 0;
	r2->instancias_disponibles = 0;
	r3->instancias_disponibles = 1;

	list_add(r1->pcbs_asignados, p2);
	list_add(r1->pcbs_asignados, p2);
	list_add(r1->pcbs_asignados, p3);
	list_add(r2->pcbs_asignados, p1);
	list_add(r2->pcbs_asignados, p3);
	list_add(r3->pcbs_asignados, p2);
	list_add(r3->pcbs_asignados, p3);

	queue_push(r1->pcbs_bloqueados, p1);
	queue_push(r1->pcbs_bloqueados, p1);
	queue_push(r1->pcbs_bloqueados, p3);
	queue_push(r2->pcbs_bloqueados, p1);
	queue_push(r2->pcbs_bloqueados, p3);
	queue_push(r2->pcbs_bloqueados, p4);
	queue_push(r3->pcbs_bloqueados, p2);
	queue_push(r3->pcbs_bloqueados, p4);

	hay_deadlock();

	return 0;
}

// TODO: chequear frees!
// free(recursos_disponibles);
// list_destroy(procesos_a_analizar);
void analisis_deadlock()
{
	int *recursos_totales = instancias_recursos;
	int *recursos_disponibles = obtener_vector_recursos_disponibles();
	t_list *procesos_a_analizar = obtener_procesos_analisis_deadlock();
	bool finalice_alguno = true;
	t_list_iterator *iterador_procesos_a_analizar;
	t_pcb_analisis_deadlock *pcb_analisis_deadlock;
	int cantidad_procesos_a_analizar = list_size(procesos_a_analizar);
	int cantidad_iteraciones_realizadas = 0;

	// INICIO LOGUEO
	loguear_vector(recursos_totales, cantidad_de_recursos, "RECURSOS TOTALES", -1);
	loguear_vector(recursos_disponibles, cantidad_de_recursos, "RECURSOS DISPONIBLES", -1);
	iterador_procesos_a_analizar = list_iterator_create(procesos_a_analizar);
	while (list_iterator_has_next(iterador_procesos_a_analizar))
	{
		pcb_analisis_deadlock = list_iterator_next(iterador_procesos_a_analizar);
		loguear_vector(pcb_analisis_deadlock->recursos_asignados, cantidad_de_recursos, "RECURSOS DISPONIBLES", pcb_analisis_deadlock->pid);
		loguear_vector(pcb_analisis_deadlock->solicitudes_actuales, cantidad_de_recursos, "SOLICITUDES ACTUALES", pcb_analisis_deadlock->pid);
	}
	list_iterator_destroy(iterador_procesos_a_analizar);
	// FIN LOGUEO

	// INICIO ALGORITMO
	if (list_is_empty(procesos_a_analizar))
	{
		return false;
	}

	while (finalice_alguno && cantidad_iteraciones_realizadas < cantidad_procesos_a_analizar)
	{
		finalice_alguno = false;

		log_info(logger, "ITERACION %d", cantidad_iteraciones_realizadas);
		iterador_procesos_a_analizar = list_iterator_create(procesos_a_analizar);
		while (list_iterator_has_next(iterador_procesos_a_analizar) && !finalice_alguno)
		{
			pcb_analisis_deadlock = list_iterator_next(iterador_procesos_a_analizar);

			if (!pcb_analisis_deadlock->finalizado)
			{
				log_info(logger, "SE PUEDEN SATISFACER LAS SOLICITUDES DE PID %d?", pcb_analisis_deadlock->pid);

				bool se_puede_satisfacer_solicitudes = true;
				for (int i = 0; i < cantidad_de_recursos; i++)
				{
					se_puede_satisfacer_solicitudes = se_puede_satisfacer_solicitudes && recursos_disponibles[i] >= pcb_analisis_deadlock->solicitudes_actuales[i];
				}

				if (se_puede_satisfacer_solicitudes)
				{
					finalice_alguno = true;
					log_info(logger, "SI!");

					pcb_analisis_deadlock->finalizado = true;

					for (int i = 0; i < cantidad_de_recursos; i++)
					{
						recursos_disponibles[i] = recursos_disponibles[i] + pcb_analisis_deadlock->recursos_asignados[i];
					}
				}
				else
				{
					log_info(logger, "NO!");
				}
			}
		}
		list_iterator_destroy(iterador_procesos_a_analizar);

		loguear_vector(recursos_disponibles, cantidad_de_recursos, "RECURSOS DISPONIBLES", -1);
		cantidad_iteraciones_realizadas++;
	}

	bool hay_deadlock = cantidad_iteraciones_realizadas < cantidad_procesos_a_analizar;

	if (hay_deadlock)
	{
		log_info(logger, "HAY DEADLOCK");

		iterador_procesos_a_analizar = list_iterator_create(procesos_a_analizar);
		while (list_iterator_has_next(iterador_procesos_a_analizar))
		{
			pcb_analisis_deadlock = list_iterator_next(iterador_procesos_a_analizar);
			if (!pcb_analisis_deadlock->finalizado)
			{
				log_info(logger, "PID %d ESTA EN DEADLOCK", pcb_analisis_deadlock->pid);
			}
		}
	}
	else
	{
		log_info(logger, "NO HAY DEADLOCK");
	}

	return hay_deadlock;
}

void loguear_vector(int *vector, int tamanio, char *nombre, int pid)
{
	if (pid == -1)
	{
		log_debug(logger, "%s:", nombre);
	}
	else
	{

		log_debug(logger, "%s PID %d:", nombre, pid);
	}

	for (int i = 0; i < tamanio; i++)
	{
		log_debug(logger, "%d", vector[i]);
	}
}

void crear_recursos()
{
	recursos = list_create();

	for (int i = 0; i < cantidad_de_recursos; i++)
	{
		list_add(recursos, crear_recurso(nombres_recursos[i], instancias_recursos[i]));
	}
}

t_rec *crear_recurso(char *nombre, int instancias)
{
	t_rec *recurso = malloc(sizeof(t_rec));

	recurso->nombre = malloc(strlen(nombre));
	strcpy(recurso->nombre, nombre);

	recurso->instancias_iniciales = instancias;
	recurso->instancias_disponibles = instancias;
	recurso->pcbs_asignados = list_create();
	recurso->pcbs_bloqueados = queue_create();

	return recurso;
}

int *obtener_vector_recursos_disponibles()
{
	int *recursos_disponibles = (int *)malloc(cantidad_de_recursos * sizeof(int));

	for (int i = 0; i < cantidad_de_recursos; i++)
	{
		t_rec *recurso = list_get(recursos, i);
		recursos_disponibles[i] = recurso->instancias_disponibles;
	}

	return recursos_disponibles;
}

t_rec *buscar_recurso_por_nombre(char *nombre_recurso)
{
	bool _filtro_recurso_por_nombre(t_rec * recurso)
	{
		return strcmp(nombre_recurso, recurso->nombre) == 0;
	};

	t_rec *recurso = list_find(recursos, (void *)_filtro_recurso_por_nombre);
	return recurso;
}

t_list *obtener_procesos_analisis_deadlock()
{
	t_list *resultado = list_create();
	t_mipcb *pcb;
	t_pcb_analisis_deadlock *pcb_a_analizar_existente;
	t_pcb_analisis_deadlock *pcb_a_analizar_nuevo;
	t_rec *recurso;
	t_list_iterator *iterador_pcbs_asignados;
	t_list_iterator *iterador_pcbs_bloqueados;
	int i, j;

	bool _filtro_pcb_por_id(t_mipcb * unpcb)
	{
		return unpcb->pid == pcb->pid;
	};

	for (i = 0; i < cantidad_de_recursos; i++)
	{
		recurso = list_get(recursos, i);
		iterador_pcbs_asignados = list_iterator_create(recurso->pcbs_asignados);

		while (list_iterator_has_next(iterador_pcbs_asignados))
		{
			pcb = list_iterator_next(iterador_pcbs_asignados);
			pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);

			if (pcb_a_analizar_existente == NULL)
			{
				pcb_a_analizar_nuevo = malloc(sizeof(t_pcb_analisis_deadlock));

				pcb_a_analizar_nuevo->finalizado = false;
				pcb_a_analizar_nuevo->pid = pcb->pid;
				pcb_a_analizar_nuevo->recursos_asignados = malloc(cantidad_de_recursos * sizeof(int));
				pcb_a_analizar_nuevo->solicitudes_actuales = malloc(cantidad_de_recursos * sizeof(int));

				for (j = 0; j < cantidad_de_recursos; j++)
				{
					pcb_a_analizar_nuevo->recursos_asignados[j] = 0;
					pcb_a_analizar_nuevo->solicitudes_actuales[j] = 0;
				}

				pcb_a_analizar_nuevo->recursos_asignados[i]++;

				list_add(resultado, pcb_a_analizar_nuevo);
			}
			else
			{
				pcb_a_analizar_existente->recursos_asignados[i]++;
			}
		}
		list_iterator_destroy(iterador_pcbs_asignados);
	}

	for (i = 0; i < cantidad_de_recursos; i++)
	{
		recurso = list_get(recursos, i);
		iterador_pcbs_bloqueados = list_iterator_create(recurso->pcbs_bloqueados->elements);
		while (list_iterator_has_next(iterador_pcbs_bloqueados))
		{
			pcb = list_iterator_next(iterador_pcbs_bloqueados);
			pcb_a_analizar_existente = list_find(resultado, (void *)_filtro_pcb_por_id);

			if (pcb_a_analizar_existente != NULL)
			{
				pcb_a_analizar_existente->solicitudes_actuales[i]++;
			}
		}
		list_iterator_destroy(iterador_pcbs_bloqueados);
	}

	return resultado;
}

void debug_deadlock(int cantidad_de_recursos, int *recursos_totales, int *recursos_disponibles, t_list *procesos_a_analizar)
{
	t_list_iterator *iterador_procesos_a_analizar;
	t_pcb_analisis_deadlock *pcb_analisis_deadlock;

	// --
	crear_string_dinamico();
	for (int i = 0; i < cantidad_de_recursos; i++)
	{
		agregar_entero_a_string_dinamico(recursos_totales[i]);
		agregar_string_a_string_dinamico(" ");
	}
	log_warning(logger, "* VECTOR DE RECURSOS TOTALES: [ %s]", string_dinamico);
	liberar_string_dinamico();
	// --

	// --
	crear_string_dinamico();
	for (int i = 0; i < cantidad_de_recursos; i++)
	{
		agregar_entero_a_string_dinamico(recursos_disponibles[i]);
		agregar_string_a_string_dinamico(" ");
	}
	log_warning(logger, "* VECTOR DE RECURSOS DISPONIBLES: [ %s]", string_dinamico);
	liberar_string_dinamico();
	// --

	// --
	iterador_procesos_a_analizar = list_iterator_create(procesos_a_analizar);
	while (list_iterator_has_next(iterador_procesos_a_analizar))
	{
		pcb_analisis_deadlock = list_iterator_next(iterador_procesos_a_analizar);
		crear_string_dinamico();
		for (int i = 0; i < cantidad_de_recursos; i++)
		{
			agregar_entero_a_string_dinamico(pcb_analisis_deadlock->solicitudes_actuales[i]);
			agregar_string_a_string_dinamico(" ");
		}
		log_warning(logger, "* VECTOR DE PETICIONES ACTUALES PARA PID %d: [%s]", pcb_analisis_deadlock->pid, string_dinamico);
		liberar_string_dinamico();

		crear_string_dinamico();
		for (int i = 0; i < cantidad_de_recursos; i++)
		{
			agregar_entero_a_string_dinamico(pcb_analisis_deadlock->recursos_asignados[i]);
			agregar_string_a_string_dinamico(" ");
		}
		log_warning(logger, "* VECTOR DE RECURSOS ASIGNADOS PARA PID %d: [%s]", pcb_analisis_deadlock->pid, string_dinamico);
		liberar_string_dinamico();
	}
	// --
}