#ifndef UTILIDADES_ARCHIVOS_H_
#define UTILIDADES_ARCHIVOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <string.h>

#include "enums.h"
#include "estructuras.h"
#include "constantes.h"

char *leer_archivo(t_log *logger, char *ruta_archivo);
FILE* abrir_archivo(t_log* logger, char* ruta_archivo);
void cerrar_archivo(t_log *logger, FILE *archivo);
char *buscar_linea(t_log *logger, FILE *archivo, int pc);
bool existe_archivo(t_log *logger, char *ruta_archivo);

#endif /* UTILIDADES_ARCHIVOS_H_ */
