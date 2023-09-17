#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdint.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/config.h>

#include "Source/Headers/argumentos_memoria.h"
#include "Source/Headers/configuracion_memoria.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/utilidades_serializacion.h"
#include "../Shared/Headers/utilidades_deserializacion.h"
#include "../Shared/Headers/enums.h"
#include "../Shared/Headers/estructuras.h"
#include "../Shared/Headers/constantes.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/memoria.log"
#define LOG_LEVEL LOG_LEVEL_TRACE


// Handshake con CPU
void realizar_handshake_cpu(t_log *logger, t_config_memoria *configuracion_memoria, int conexion_con_cpu);
//Iniciar proceso
void iniciar_proceso_memoria(t_log *logger, int conexion_con_kernel);
// Terminar
void terminar_memoria(t_log *logger, t_argumentos_memoria *argumentos_memoria, t_config_memoria *configuracion_memoria, int socket_kernel, int conexion_con_kernel, int socket_cpu, int conexion_con_cpu, int socket_filesystem, int conexion_con_filesystem);


#endif /* MEMORIA_H_ */
