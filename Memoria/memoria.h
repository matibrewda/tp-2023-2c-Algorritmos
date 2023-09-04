#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>

#include "Source/Headers/argumentos_memoria.h"
#include "Source/Headers/configuracion_memoria.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/nombres_modulos.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/memoria.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

// Terminar
void terminar_memoria(t_log *logger, t_argumentos_memoria *argumentos_memoria, t_config_memoria *configuracion_memoria);

#endif /* MEMORIA_H_ */
