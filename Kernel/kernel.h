#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>
#include<semaphore.h>
#include<math.h>

#include<commons/config.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>

#include<readline/readline.h>

#include "Source/Headers/argumentos_kernel.h"
#include "Source/Headers/configuracion_kernel.h"
#include "Source/Headers/paquete_para_servidor.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_argumentos.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/nombres_modulos.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/kernel.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

typedef struct
{
	t_log* logger;
    t_config_kernel* configuracion_kernel;
} t_argumentos_hilo_consola;

void terminar_kernel(t_log* logger, t_argumentos_kernel* argumentos_kernel, t_config_kernel* configuracion_kernel, int conexion_con_servidor, t_paquete* paquete_para_servidor);
void crear_hilo_consola(t_log* logger, t_config_kernel* config_kernel);
void consola(void* argumentos);

#endif /* KERNEL_H_ */
