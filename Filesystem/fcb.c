
#include "fcb.h"

FCB *abrir_fcb(char *ruta_archivo)
{
    t_config *config = config_create(ruta_archivo);

    if (config == NULL)
    {
        return NULL;
    }

    char *nombre_archivo = config_get_string_value(config, "NOMBRE_ARCHIVO");
    uint32_t tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");
    uint32_t bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");
    FCB *fcb = iniciar_fcb(nombre_archivo, tamanio_archivo, bloque_inicial);

    config_destroy(config);
    return fcb;
}

// Crear un nuevo FCB
FCB *iniciar_fcb(char *nombre_archivo, uint32_t tamanio_archivo, uint32_t bloque_inicial)
{
    FCB *fcb = malloc(sizeof(FCB));
    fcb->nombre_archivo = strdup(nombre_archivo); // Revisar
    fcb->tamanio_archivo = tamanio_archivo;
    fcb->bloque_inicial = bloque_inicial;
    return fcb;
}

// Guardar un FCB en un archivo de configuración

void generar_archivo_fcb_vacio(const char *rutaCompleta)
{
    FILE *archivo = fopen(rutaCompleta, "w");

    if (archivo != NULL)
    {
        fclose(archivo);
        printf("Archivo %s creado con éxito.\n", rutaCompleta);
    }
    else
    {
        printf("Error al crear el archivo %s.\n", rutaCompleta);
    }
}

void guardar_fcb_en_archivo(FCB *fcb, char *ruta_archivo)
{

    generar_archivo_fcb_vacio(ruta_archivo);

    t_config *config = config_create(ruta_archivo);

    char tamanio_archivo_str[12]; // Suficientemente grande para un uint32_t
    char bloque_inicial_str[12];  // Suficientemente grande para un uint32_t
    sprintf(tamanio_archivo_str, "%u", fcb->tamanio_archivo);
    sprintf(bloque_inicial_str, "%u", fcb->bloque_inicial); // Cambio %u a %d para valores negativos

    // Asigna los valores a la configuración
    config_set_value(config, "NOMBRE_ARCHIVO", fcb->nombre_archivo);
    config_set_value(config, "TAMANIO_ARCHIVO", tamanio_archivo_str);
    config_set_value(config, "BLOQUE_INICIAL", bloque_inicial_str);

    // Guarda la configuración en el archivo
    config_save(config);

    // Destruye la configuración
    config_destroy(config);
}

uint32_t verificarSiExisteFCBdeArchivo(char *rutaCompleta)
{

    // Intentar abrir el archivo
    FILE *archivo = fopen(rutaCompleta, "r");

    if (archivo != NULL)
    {
        // El archivo se abrió correctamente, cerramos el archivo y devolvemos 0
        fclose(archivo);
        return 0;
    }
    else
    {
        // El archivo no se pudo abrir, devolvemos -1
        return -1;
    }
}