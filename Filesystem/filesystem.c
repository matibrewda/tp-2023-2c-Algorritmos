#include "filesystem.h"

// VARIABLES GLOBALES
t_log *logger = NULL;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_FILESYSTEM, LOG_LEVEL);

	pthread_t mi_hilo_2;
	pthread_create(&mi_hilo_2, NULL, main_hilo_2, NULL);

	// HILO PRINCIPAL

	int a = 3;

	mi_memoria = malloc(12);

	log_info(logger, "HILO PRINCIPAL");

	// BLOQUEA
	pthread_join(mi_hilo_2, NULL);
	
	return EXIT_SUCCESS;
}

void* main_hilo_2()
{
	// ejecutar hilo 2
	log_info(logger, "HILO 2");
	
	while(true);
}

hilo_atiendo_kernel()
{
	
}

hilo_atiendo_memoria()
{

}