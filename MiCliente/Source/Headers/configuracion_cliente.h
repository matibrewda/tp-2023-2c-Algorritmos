#ifndef CONFIGURACION_CLIENTE_H_
#define CONFIGURACION_CLIENTE_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#include<commons/log.h>
#include<commons/config.h>

#include "../../../Shared/Headers/utilidades_configuracion.h"

#define CLAVE_CONFIGURACION_IP_SERVIDOR "IP_SERVIDOR"
#define CLAVE_CONFIGURACION_PUERTO_SERVIDOR "PUERTO_SERVIDOR"

typedef struct
{
  	char* ip_servidor;
	char* puerto_servidor;
} t_config_cliente;

t_config_cliente* leer_configuracion(t_log* logger, char* ruta_archivo_configuracion);
void destruir_configuracion(t_config_cliente* config_cliente);

#endif /* CONFIGURACION_CLIENTE_H_ */
