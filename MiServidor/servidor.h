#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>
#include<semaphore.h>
#include<math.h>

#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<commons/config.h>

#include "Source/Headers/argumentos_servidor.h"
#include "Source/Headers/configuracion_servidor.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/nombres_modulos.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/servidor.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

// Terminar
void terminar_servidor(t_log* logger, t_argumentos_servidor* argumentos_servidor, t_config_servidor* configuracion_servidor, int servidor_para_cliente);

#endif /* SERVIDOR_H_ */
