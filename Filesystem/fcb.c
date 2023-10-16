	
#include "fcb.h"

    // Leer un FCB desde un archivo de configuración


// Leer FCB desde archivo xxxxxxxx.fcb
FCB* crear_fcb(char* ruta_archivo) {

    if (ruta_archivo != NULL) {
    t_config* config = config_create(ruta_archivo);
    char* nombre_archivo = config_get_string_value(config, "NOMBRE_ARCHIVO");
    uint32_t tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");
    uint32_t bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");
    config_destroy(config);
    FCB* fcb  = iniciar_fcb(nombre_archivo, tamanio_archivo, bloque_inicial);
    return fcb;}
    else return NULL;
}

// Crear un nuevo FCB
FCB* iniciar_fcb(char* nombre_archivo, uint32_t tamanio_archivo, uint32_t bloque_inicial) {
    FCB* fcb = malloc(sizeof(FCB));
    fcb->nombre_archivo = strdup(nombre_archivo);
    fcb->tamanio_archivo = tamanio_archivo;
    fcb->bloque_inicial = bloque_inicial;
    return fcb;
}


// Guardar un FCB en un archivo de configuración

void guardar_fcb_en_archivo(FCB* fcb, char* ruta_archivo) {
    t_config* config = config_create(ruta_archivo);

    // Convierte los valores numéricos a cadenas de caracteres
    char tamanio_archivo_str[12];  // Suficientemente grande para un uint32_t
    char bloque_inicial_str[12];   // Suficientemente grande para un uint32_t
    sprintf(tamanio_archivo_str, "%u", fcb->tamanio_archivo);
    sprintf(bloque_inicial_str, "%u", fcb->bloque_inicial);

    // Asigna los valores a la configuración
    config_set_value(config, "NOMBRE_ARCHIVO", fcb->nombre_archivo);
    config_set_value(config, "TAMANIO_ARCHIVO", tamanio_archivo_str);
    config_set_value(config, "BLOQUE_INICIAL", bloque_inicial_str);

    // Guarda la configuración en el archivo
    config_save(config);

    // Destruye la configuración
    config_destroy(config);
}


