#ifndef UTILIDADES_CONFIGURACION_H_
#define UTILIDADES_CONFIGURACION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/log.h>
#include <string.h>

#include "enums.h"
#include "estructuras.h"
#include "constantes.h"

t_config *leer_archivo_configuracion(t_log *logger, char *ruta_archivo_configuracion);
bool existen_claves_en_configuracion(t_log *logger, t_config *configuracion, char **claves);
bool existe_clave_en_configuracion(t_log *logger, t_config *configuracion, char *clave);
char *leer_clave_string(t_log *logger, t_config *configuracion, char *clave);
int leer_clave_int(t_log *logger, t_config *configuracion, char *clave);
char **leer_clave_arreglo_de_strings(t_log *logger, t_config *configuracion, char *clave, int *tamanio_arreglo);
int *leer_clave_arreglo_de_enteros(t_log *logger, t_config *configuracion, char *clave, int *tamanio_arreglo);

#endif /* UTILIDADES_CONFIGURACION_H_ */
