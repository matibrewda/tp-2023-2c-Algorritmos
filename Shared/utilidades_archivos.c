#include "Headers/utilidades_archivos.h"
#include <stdio.h>
#include <stdlib.h>

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

FILE* abrir_archivo(t_log* logger, char* ruta_archivo){
    FILE* fp = fopen(ruta_archivo, "r+");

    if (fp == NULL)
	{
        log_error(logger, "Error al leer archivo con la siguiente ruta: %s", ruta_archivo);
        return NULL;
	}
    return fp;
}

void cerrar_arcivo(t_log* logger,FILE* archivo){
    if(archivo != NULL){
        fclose(archivo);
        return;
    }
    log_error(logger,"Error al cerrar el archivo");
}

char* leer_linea(t_log* logger,FILE* archivo){
    if (archivo == NULL)
	{
        log_error(logger,"No existe este archivo para leer");
        return NULL;
	}

    char* linea = NULL;
    size_t lenght = 0;
    ssize_t read;

    read = getline(&linea, &lenght, archivo);
    if (read == -1){
        free(linea);
        return NULL;
    }
    return linea;
}