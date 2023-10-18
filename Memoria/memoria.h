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
#include "../Shared/Headers/utilidades_archivos.h"
#include "../Shared/Headers/enums.h"
#include "../Shared/Headers/estructuras.h"
#include "../Shared/Headers/constantes.h"

#define RUTA_ARCHIVO_DE_LOGS "Logs/memoria.log"
#define LOG_LEVEL LOG_LEVEL_TRACE

// Handshake con CPU
void enviar_info_de_memoria_inicial_para_cpu();


//Iniciar proceso
int iniciar_proceso_memoria(char* path, int size, int prioridad, int pid);
void enviar_instruccion_a_cpu(int pid, int pc);
void pedir_bloques_a_fylesystem(int cantidad_de_bloques);
void finalizar_proceso_en_memoria(int pid);
void pedir_liberacion_de_bloques_a_filesystem(int bloques);

//Notificacion filesystem
void notificar_lectura_a_filesystem();
void notificar_escritura_a_filesystem();

// Busqueda
t_archivo_proceso *buscar_archivo_con_pid(int pid);

// Terminar
void destruir_listas();
void terminar_memoria();

// Hilos
void* atender_kernel();
void* atender_cpu();
void* atender_filesystem();

#endif /* MEMORIA_H_ */
