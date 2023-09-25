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
#include "Source/Headers/estructuras_memoria.h"

#include "../Shared/Headers/utilidades_logger.h"
#include "../Shared/Headers/utilidades_configuracion.h"
#include "../Shared/Headers/utilidades_conexion.h"
#include "../Shared/Headers/utilidades_serializacion.h"
#include "../Shared/Headers/utilidades_deserializacion.h"
#include "../../Shared/Headers/utilidades_archivos.h"
#include "../Shared/Headers/enums.h"
#include "../Shared/Headers/estructuras.h"
#include "../Shared/Headers/constantes.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/memoria.log"
#define LOG_LEVEL LOG_LEVEL_TRACE


// Handshake con CPU
void realizar_handshake_cpu();
//Iniciar proceso
void iniciar_proceso_memoria();
void enviar_instruccion_a_cpu();
void finalizar_proceso_en_memoria();
// Busqueda
t_archivo_proceso *buscar_archivo_con_pid(int *pid);
// Terminar
void destruir_listas();
void terminar_memoria();


#endif /* MEMORIA_H_ */
