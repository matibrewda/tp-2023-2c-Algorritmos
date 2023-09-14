#ifndef UTILIDADES_DESERIALIZACION_H_
#define UTILIDADES_DESERIALIZACION_H_

#include <commons/log.h>
#include <commons/collections/list.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "enums.h"
#include "estructuras.h"
#include "constantes.h"
#include "utilidades_conexion.h"

// Especificas
t_contexto_de_ejecucion *leer_paquete_ejecutar_proceso(t_log *logger, int conexion_con_kernel_dispatch);

// Comunes
t_contexto_de_ejecucion *leer_paquete_contexto_de_ejecucion(t_log *logger, int conexion, op_code codigo_operacion, char *nombre_proceso_origen, char *nombre_proceso_destino);

#endif /* UTILIDADES_DESERIALIZACION_H_ */