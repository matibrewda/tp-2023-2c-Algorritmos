#ifndef CLIENTE_H_
#define CLIENTE_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>

#include<commons/config.h>
#include<commons/log.h>

#include<readline/readline.h>

#include "Source/Headers/argumentos_cliente.h"
#include "Source/Headers/configuracion_cliente.h"
#include "Source/Headers/paquete_para_servidor.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_argumentos.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/nombres_modulos.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/cliente.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

void terminar_cliente(t_log* logger, t_argumentos_cliente* argumentos_cliente, t_config_cliente* configuracion_cliente, int conexion_con_servidor, t_paquete* paquete_para_servidor);

#endif /* CLIENTE_H_ */
