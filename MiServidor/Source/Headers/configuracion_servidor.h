#ifndef CONFIGURACION_SERVIDOR_H_
#define CONFIGURACION_SERVIDOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#include<commons/log.h>
#include<commons/config.h>

#include "../../../Shared/Headers/utilidades_configuracion.h"

#define CLAVE_CONFIGURACION_IP_SERVIDOR "IP_SERVIDOR"
#define CLAVE_CONFIGURACION_PUERTO_ESCUCHA_A_CLIENTE "PUERTO_ESCUCHA_A_CLIENTE"

typedef struct
{
  	char* ip_servidor;
	char* puerto_escucha_a_cliente;
} t_config_servidor;

t_config_servidor* leer_configuracion(t_log* logger, char* ruta_archivo_configuracion);
void destruir_configuracion(t_config_servidor* config_servidor);

#endif /* CONFIGURACION_SERVIDOR_H_ */
