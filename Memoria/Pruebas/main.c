#include "main.h"

t_log *logger = NULL;
char **nombres_recursos = {"R1", "R2", "R3"};
int *instancias_recursos = {3, 2, 3}; // Recursos totales
int cantidad_de_recursos = 3;

t_list *recursos;

int *recursos_disponibles;

int main()
{
    logger = crear_logger("pruebas.log", "PRUEBA", LOG_LEVEL_TRACE);
    crear_recursos();

    return 0;
}

int indice_recurso(char *nombre_recurso)
{
    for (int i = 0; i < cantidad_de_recursos; i++)
    {
        if (strcmp(recursos[i], nombre_recurso) == 0)
        {
            return i;
        }
    }

    return -1;
}

void crear_recursos()
{
	recursos = list_create();

	for (int i = 0; i < cantidad_de_recursos; i++)
	{
		list_add(recursos, crear_recurso(nombres_recursos[i], instancias_recursos[i]));
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

	log_debug(logger, "Se crea el recurso %s con %d instancias", nombre, instancias);

	return recurso;
}

int* obtener_vector_recursos_disponibles()
{

}

int* obtener_vector_recursos_totales()
{
    return instancias_recursos;
}