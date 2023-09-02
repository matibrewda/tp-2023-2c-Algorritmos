#include "Headers/paquete_para_servidor.h"

t_paquete *crear_paquete_para_servidor(t_log *logger)
{
    log_debug(logger, "Comenzando la creacion de paquete para enviar al servidor!");

    t_paquete *paquete = crear_paquete(logger, HACE_QUACK);

    // RESPETAR EL ORDEN -> SERIALIZACION!
    // agregar_int_a_paquete(logger, paquete, 1, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_SERVIDOR, HACE_QUACK);
    // agregar_int_a_paquete(logger, paquete, 2, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_SERVIDOR, HACE_QUACK);
    // agregar_int_a_paquete(logger, paquete, 3, NOMBRE_MODULO_CLIENTE, NOMBRE_MODULO_SERVIDOR, HACE_QUACK);

    log_debug(logger, "Exito en la creacion de paquete para enviar al servidor!");

    return paquete;
}