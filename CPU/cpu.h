#ifndef CPU_H_
#define CPU_H_

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

#include "Source/Headers/argumentos_cpu.h"
#include "Source/Headers/configuracion_cpu.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/nombres_modulos.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/cpu.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

// Terminar
void terminar_cpu(t_log *logger, t_argumentos_cpu *argumentos_cpu, t_config_cpu *configuracion_cpu, int cpu_para_cliente);

#endif /* CPU_H_ */
