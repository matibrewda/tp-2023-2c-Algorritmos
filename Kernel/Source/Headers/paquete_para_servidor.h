#ifndef PAQUETE_PARA_SERVIDOR_H_
#define PAQUETE_PARA_SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <commons/log.h>

#include "../../../Shared/Headers/utilidades_conexion.h"
#include "../../../Shared/Headers/nombres_modulos.h"

t_paquete *crear_paquete_para_servidor(t_log *logger);

#endif /* PAQUETE_PARA_SERVIDOR_H_ */
