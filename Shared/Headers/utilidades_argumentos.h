#ifndef UTILIDADES_ARGUMENTOS_H_
#define UTILIDADES_ARGUMENTOS_H_

#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

#include "enums.h"
#include "estructuras.h"
#include "constantes.h"

bool cantidad_de_argumentos_es_valida(t_log *logger, int cantidad_argumentos_recibidos, int cantidad_argumentos_esperados);
char *leer_argumento_string(t_log *logger, char **argumentos, int indice, char *nombre_argumento);

#endif /* UTILIDADES_ARGUMENTOS_H_ */