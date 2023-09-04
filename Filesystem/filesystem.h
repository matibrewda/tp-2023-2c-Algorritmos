#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

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

#include "Source/Headers/argumentos_filesystem.h"
#include "Source/Headers/configuracion_filesystem.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/utilidades_serializacion.h"
#include "../Shared/Headers/utilidades_deserializacion.h"
#include "../Shared/Headers/nombres_modulos.h"
#include "../Shared/Headers/estructuras.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/filesystem.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

// Terminar
void terminar_filesystem(t_log *logger, t_argumentos_filesystem *argumentos_filesystem, t_config_filesystem *configuracion_filesystem, int socket_kernel, int conexion_con_kernel, int conexion_con_memoria);

#endif /* FILESYSTEM_H_ */
