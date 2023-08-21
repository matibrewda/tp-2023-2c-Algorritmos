#include "Headers/utilidades_archivos.h"

char* leer_archivo(t_log* logger, char* ruta_archivo)
{
    FILE* fp = fopen(ruta_archivo, "r+");

    if (fp == NULL)
	{
        log_error(logger, "Error al leer archivo con la siguiente ruta: %s", ruta_archivo);
        return NULL;
	}
    
    fseek(fp, 0L, SEEK_END);
    long lSize = ftell(fp);
    rewind(fp);

    char* text = calloc(1, lSize + 1);
    fread(text, lSize, 1, fp);
    fclose(fp);
    
    return text;
}