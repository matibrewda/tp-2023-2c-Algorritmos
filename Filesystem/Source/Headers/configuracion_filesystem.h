#ifndef CONFIGURACION_FILESYSTEM_H_
#define CONFIGURACION_FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <commons/log.h>
#include <commons/config.h>

#include "../../../Shared/Headers/utilidades_configuracion.h"

#define CLAVE_CONFIGURACION_IP_MEMORIA "IP_MEMORIA"
#define CLAVE_CONFIGURACION_PUERTO_MEMORIA "PUERTO_MEMORIA"
#define CLAVE_CONFIGURACION_PUERTO_ESCUCHA_KERNEL "PUERTO_ESCUCHA_KERNEL"
#define CLAVE_CONFIGURACION_PATH_FAT "PATH_FAT"
#define CLAVE_CONFIGURACION_PATH_BLOQUES "PATH_BLOQUES"
#define CLAVE_CONFIGURACION_PATH_FCB "PATH_FCB"
#define CLAVE_CONFIGURACION_CANT_BLOQUES_TOTAL "CANT_BLOQUES_TOTAL"
#define CLAVE_CONFIGURACION_CANT_BLOQUES_SWAP "CANT_BLOQUES_SWAP"
#define CLAVE_CONFIGURACION_TAM_BLOQUE "TAM_BLOQUE"
#define CLAVE_CONFIGURACION_RETARDO_ACCESO_BLOQUE "RETARDO_ACCESO_BLOQUE"
#define CLAVE_CONFIGURACION_RETARDO_ACCESO_FAT "RETARDO_ACCESO_FAT"

typedef struct
{
	char *ip_memoria;
	char *puerto_memoria;
	char *puerto_escucha_kernel;
	char *path_fat;
	char *path_bloques;
	char *path_fcb;
	int cant_bloques_total;
	int cant_bloques_swap;
	int tam_bloques;
	int retardo_acceso_bloques;
	int retardo_acceso_fat;
} t_config_filesystem;

t_config_filesystem *leer_configuracion(t_log *logger, char *ruta_archivo_configuracion);
void destruir_configuracion(t_config_filesystem *config_filesystem);

#endif /* CONFIGURACION_FILESYSTEM_H_ */
