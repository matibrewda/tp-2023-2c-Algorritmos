#include "main.h"

t_log *logger = NULL;

int main()
{
    logger = crear_logger("pruebas.log", "PRUEBA", 0);
	if (logger == NULL)
	{
		return EXIT_FAILURE;
	}
    FILE *archivo = abrir_archivo(logger, "prueba.txt");
    if (archivo != NULL)
    {
        char *linea;
        /*while ((linea = buscar_linea(logger, archivo)) != NULL)
        {
            log_debug(logger, "Línea leída: %s", linea);
            free(linea); // Liberar la memoria asignada a la línea
        }*/
        linea = buscar_linea(logger, archivo, 7);
        log_debug(logger, "Línea leída: %s, tamanio: %ld", linea, strlen(linea));
        free(linea);
        cerrar_archivo(logger, archivo);
    }
    return 0;
}