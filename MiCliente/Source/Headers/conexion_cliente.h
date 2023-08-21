#ifndef CONEXION_CLIENTE_H_
#define CONEXION_CLIENTE_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#include<commons/log.h>

#include "../../../Shared/Headers/utilidades_conexion.h"
#include "../../../Shared/Headers/nombres_modulos.h"

int conectar_cliente_con_servidor(t_log* logger, char* ip_servidor, char* puerto_servidor);
void destruir_conexion_con_servidor(int conexion_con_servidor);

#endif /* CONEXION_CLIENTE_H_ */
