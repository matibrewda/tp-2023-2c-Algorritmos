#include "Headers/conexion_cliente.h"

int conectar_cliente_con_servidor(t_log* logger, char* ip_servidor, char* puerto_servidor)
{
	log_debug(logger, "Intentando conectar %s con %s ...", NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_SERVIDOR);

	int conexion_con_servidor = -1;
	
	do
	{
		conexion_con_servidor = crear_socket_cliente(logger, ip_servidor, puerto_servidor, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_SERVIDOR);
	}
	while (conexion_con_servidor == -1);

	log_debug(logger, "Exito en conexion con %s", NOMBRE_MODULO_SERVIDOR);

	return conexion_con_servidor;
}

void destruir_conexion_con_servidor(int conexion_con_servidor)
{
	if (conexion_con_servidor != -1)
	{
    	close(conexion_con_servidor);
	}
}