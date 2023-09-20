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
        while ((linea = leer_linea(logger, archivo)) != NULL)
        {
            log_debug(logger, "Línea leída: %s", linea);
            free(linea); // Liberar la memoria asignada a la línea
        }
        cerrar_archivo(logger, archivo);
    }
    return 0;
}