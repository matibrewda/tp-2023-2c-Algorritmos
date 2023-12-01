#ifndef MEMORIA_H_
#define MEMORIA_H_

// Para incluir la funcion asprintf()
#define _GNU_SOURCE

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

// ?????
void cargar_datos_de_pagina_en_memoria_real(t_entrada_de_tabla_de_pagina *pagina);
void borrar_contenido_de_marco_en_memoria_real(int numero_de_marco);
void reemplazar_pagina(int pid, int numero_de_pagina);
void enviar_valor_leido_a_cpu(uint32_t valor_leido);
void notificar_escritura_a_cpu();
void escribir_valor_en_memoria(int direccion_fisica, uint32_t valor);
uint32_t leer_valor_en_memoria(int direccion_fisica);

// Handshake con CPU
void enviar_info_de_memoria_inicial_para_cpu();

// Inicializacion de estructuras
void inicializar_espacio_contiguo_de_memoria();

// Iniciar proceso
void iniciar_proceso_memoria(char *path, int size, int prioridad, int pid);
void enviar_instruccion_a_cpu(int pid, int pc);
t_list *pedir_bloques_a_filesystem(int cantidad_de_bloques);
t_list *recibir_paquete_pedir_bloques_a_filesystem();
void crear_entrada_de_tabla_de_paginas_de_proceso(int cantidad_de_paginas, t_list *posiciones_swap, int pid);

// Finalizar proceso
void finalizar_proceso_en_memoria(int pid);
void pedir_liberacion_de_bloques_a_filesystem();
void limpiar_entradas_tabla_de_paginas(int pid);

// Notificacion filesystem
void notificar_lectura_a_filesystem();
void notificar_escritura_a_filesystem();

// Busqueda
t_archivo_proceso *buscar_archivo_con_pid(int pid);
void cerrar_archivo_con_pid(int pid);
void enviar_numero_de_marco_a_cpu(int pid, int numero_de_pagina);
t_list *obtener_entradas_de_tabla_de_pagina_por_pid(int pid);
t_entrada_de_tabla_de_pagina *obtener_entrada_de_tabla_de_pagina_por_pid_y_numero(int pid, int numero_de_pagina);
void* buscar_contenido_marco(int numero_de_marco);

// Manejo de Paginas
void cargar_pagina_en_memoria(int pid, int numero_de_pagina);
int es_pagina_modificada(t_entrada_de_tabla_de_pagina *pagina);
int es_pagina_presente(t_entrada_de_tabla_de_pagina *pagina);
void* obtener_contenido_de_pagina_en_swap(int posicion_en_swap);

// Comunicacion con Kernel
void enviar_paquete_respuesta_iniciar_proceso_en_memoria_a_kernel(bool resultado_iniciar_proceso_en_memoria);
void enviar_paquete_respuesta_finalizar_proceso_en_memoria_a_kernel();
void enviar_paquete_respuesta_cargar_pagina_en_memoria_a_kernel(bool resultado_cargar_pagina_en_memoria);

// Terminar
void destruir_listas();
void terminar_memoria();

// Hilos
void *atender_kernel();
void *atender_cpu();
void *atender_filesystem();

#endif /* MEMORIA_H_ */
