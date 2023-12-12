#include "Headers/configuracion_filesystem.h"

t_config_filesystem *leer_configuracion(t_log *logger, char *ruta_archivo_configuracion)
{
	log_trace(logger, "Comenzando la lectura de archivo de configuracion");

	t_config *configuracion = leer_archivo_configuracion(logger, ruta_archivo_configuracion);

	if (configuracion == NULL)
	{
		return NULL;
	}

	char *claves[] = {
		CLAVE_CONFIGURACION_IP_MEMORIA,
		CLAVE_CONFIGURACION_PUERTO_MEMORIA,
		CLAVE_CONFIGURACION_PUERTO_ESCUCHA_KERNEL,
		CLAVE_CONFIGURACION_PATH_FAT,
		CLAVE_CONFIGURACION_PATH_BLOQUES,
		CLAVE_CONFIGURACION_PATH_FCB,
		CLAVE_CONFIGURACION_CANT_BLOQUES_TOTAL,
		CLAVE_CONFIGURACION_CANT_BLOQUES_SWAP,
		CLAVE_CONFIGURACION_TAM_BLOQUE,
		CLAVE_CONFIGURACION_RETARDO_ACCESO_BLOQUE,
		CLAVE_CONFIGURACION_RETARDO_ACCESO_FAT,
		NULL};

	if (!existen_claves_en_configuracion(logger, configuracion, claves))
	{
		return NULL;
	}

	t_config_filesystem *configuracion_filesystem = malloc(sizeof(t_config_filesystem));
	configuracion_filesystem->ip_memoria = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_IP_MEMORIA);
	configuracion_filesystem->puerto_memoria = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_MEMORIA);
	configuracion_filesystem->puerto_escucha_kernel = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PUERTO_ESCUCHA_KERNEL);
	configuracion_filesystem->path_fat = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PATH_FAT);
	configuracion_filesystem->path_bloques = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PATH_BLOQUES);
	configuracion_filesystem->path_fcb = leer_clave_string(logger, configuracion, CLAVE_CONFIGURACION_PATH_FCB);
	configuracion_filesystem->cant_bloques_total = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_CANT_BLOQUES_TOTAL);
	configuracion_filesystem->cant_bloques_swap = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_CANT_BLOQUES_SWAP);
	configuracion_filesystem->tam_bloques = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_TAM_BLOQUE);
	configuracion_filesystem->retardo_acceso_bloques = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_RETARDO_ACCESO_BLOQUE);
	configuracion_filesystem->retardo_acceso_fat = leer_clave_int(logger, configuracion, CLAVE_CONFIGURACION_RETARDO_ACCESO_FAT);

	log_trace(logger, "Exito en la lectura de archivo de configuracion");

	config_destroy(configuracion);
	return configuracion_filesystem;
}

void destruir_configuracion(t_config_filesystem *configuracion_filesystem)
{
	if (configuracion_filesystem == NULL)
	{
		return;
	}

	free(configuracion_filesystem->ip_memoria);
	free(configuracion_filesystem->puerto_memoria);
	free(configuracion_filesystem->puerto_escucha_kernel);
	free(configuracion_filesystem->path_fat);
	free(configuracion_filesystem->path_bloques);
	free(configuracion_filesystem->path_fcb);

	free(configuracion_filesystem);
}