#include "fat.h"
#include "fcb.h"
#include "bloque.h"
#include "directorio.h"


int32_t contarBloqueDisponibles(FATEntry fat[], size_t cantBloquesFAT) {
    int32_t bloquesDisponibles = 0;

    for (size_t i = 0; i < cantBloquesFAT; ++i) {
        if (fat[i].block_value == 0) {
            bloquesDisponibles++;
        }
    }

    return bloquesDisponibles;
}


int32_t validarEspacioDisponible (int32_t tamanioArchivo,FATEntry fat[],size_t cantBloquesFAT){
    
    if (tamanioArchivo > contarBloqueDisponibles(fat,cantBloquesFAT)){
        return -1;
    } else return 0;
}



void modificarFATenArchivoFAT(const char* pathFAT, uint32_t numeroBloque, FATEntry *nuevaEntrada);

// Implementacion de funciones

// INICIA POR PRIMERA VEZ LA FAT SI ES QUE NO EXISTE, ESCRIBE LA FAT CON TODOS LOS BLOQUES EN 0
int iniciarFAT(t_log *logger, char *fat_path, uint32_t cant_bloques_total, uint32_t cant_bloques_swap,uint32_t tamanio_bloque)
{
    size_t total_blocks = cant_bloques_total - cant_bloques_swap;

    // Abro archivo FAT para escribir en el.
    FILE *fat_file = fopen(fat_path, "r+");
    if (fat_file == NULL)
    {
        // No se pudo abrir el archivo, entonces lo creamos y lo inicializamos
        fat_file = fopen(fat_path, "w+");





        if (fat_file == NULL)
        { 
            log_debug(logger, "FS FAT: No se pudo crear el archivo fat.bin");
            return 1;
        }

        // solicito memoria para el arreglo de bloques FAT

        FATEntry *arreglo = (FATEntry *)malloc(total_blocks * sizeof(FATEntry));

        if (arreglo == NULL)
        {
            // Error: no se pudo asignar memoria
        }
        else
        {
            // Inicializar el arreglo con 0 en todos sus bloques.
            for (size_t i = 0; i < total_blocks; i++)
            {
                arreglo[i].block_value = 0;
            }

            arreglo[0].block_value = INT32_MAX;

            // EScribimos el contenido del array en LOS archivos

            for (size_t j = 0; j < total_blocks; j++)
            {

                fwrite(&arreglo[j], sizeof(int32_t), 1, fat_file);
            }

            log_debug(logger, "FS FAT: La tabla FAT se ha creado y inicializado con %ld entradas (bloques), todos ellos con direccion 0.\n", total_blocks);
            free(arreglo);

        
        }
    }
    else
    {
        log_debug(logger, "FS FAT: La tabla FAT ya existe.\n");
    }

    fclose(fat_file);

    return 0;
}

#include "fat.h"

// LEE ARCHIVO FAT Y LO PASA A UN ARRAY DE ENTRADAS FAT
FATEntry* abrirFAT(char *fat_path, uint32_t cant_bloques_total, uint32_t cant_bloques_swap) {
    size_t total_blocks = cant_bloques_total - cant_bloques_swap;

    // Abro archivo FAT para lectura
    FILE *fat_file = fopen(fat_path, "rb");
    if (fat_file == NULL) {
        // No se pudo abrir el archivo, retorna NULL
        return NULL;
    }

    // Solicito memoria para el arreglo de bloques FAT
    FATEntry *arreglo = (FATEntry *)malloc(total_blocks * sizeof(FATEntry));

    if (arreglo == NULL) {
        // Error: no se pudo asignar memoria
        fclose(fat_file);
        return NULL;
    }

    // Leo el contenido del archivo y almaceno en el array
    size_t read_blocks = fread(arreglo, sizeof(uint32_t), total_blocks, fat_file);

    if (read_blocks != total_blocks) {
        // Error al leer la FAT, retorna NULL
        free(arreglo);
        fclose(fat_file);
        return NULL;
    }

    fclose(fat_file);

    return arreglo;
}


void cerrarFAT(FATEntry *arreglo) {
    // Liberar la memoria asignada para el array
    free(arreglo);
}
/* 
int buscarBloqueLibre(FATEntry fat[], size_t total_blocks) {
    for (size_t i = 0; i < total_blocks; i++) {
        if (fat[i].block_value == 0) {
            return i;  // Devuelve el índice del bloque libre
        }
    }
    return -1;  // No hay bloques libres
} */

u_int32_t buscarBloqueLibre(FATEntry fat[], size_t total_blocks) {
    for (size_t i = 0; i < total_blocks; i++) {
        if (fat[i].block_value == 0) {
            return i;  // Devuelve el índice del bloque libre
        }
    }
    return -1;  // No hay bloques libres
}


void asignarBloques(char* pathFAT, char* pathBLOQUES, char* pathFCB, BLOQUE *bloques[],FATEntry fat[],size_t cantBloquesTotales,size_t cantBLoquesSWAP, size_t tamanioBloque){
    
    size_t cantBloquesTotalesFAT = cantBloquesTotales - cantBLoquesSWAP;
    FCB *fcb = crear_fcb(pathFCB);

    int32_t espacioChecker = validarEspacioDisponible (fcb->tamanio_archivo,fat,cantBloquesTotalesFAT);

    if (fcb->bloque_inicial == INT32_MAX && espacioChecker == 0) {
    int32_t cantBloquesAAsignar = fcb->tamanio_archivo;
    int32_t ultBloqueAsignado;

    
for (cantBloquesAAsignar = fcb->tamanio_archivo; 0 <  cantBloquesAAsignar; cantBloquesAAsignar--) {
        
        int8_t bloqueLibre = buscarBloqueLibre(fat,cantBloquesTotalesFAT);
        int8_t bloqueLibreAnt;

        if (bloqueLibre != -1){

        bloqueLibreAnt = bloqueLibre;
       
        if (1 < cantBloquesAAsignar ){
	    fat[bloqueLibre].block_value  = -1;
        bloqueLibre = buscarBloqueLibre(fat,cantBloquesTotalesFAT);
        fat[bloqueLibreAnt].block_value = bloqueLibre;
        memset(bloques[bloqueLibreAnt + cantBLoquesSWAP]->valorDeBloque, obtenerContenidoRandom(), tamanioBloque);
        ultBloqueAsignado = bloqueLibreAnt;
        modificarFATenArchivoFAT(pathFAT, bloqueLibreAnt, &fat[bloqueLibreAnt]);
        modificarBLOQUEenArchivoBLOQUE(pathBLOQUES,(bloqueLibreAnt + cantBLoquesSWAP),bloques[bloqueLibreAnt + cantBLoquesSWAP],tamanioBloque);
        } else {
        fat[bloqueLibre].block_value = INT32_MAX;
        modificarFATenArchivoFAT(pathFAT, bloqueLibre, &fat[bloqueLibre]);
        memset(bloques[bloqueLibre + cantBLoquesSWAP]->valorDeBloque, obtenerContenidoRandom(), tamanioBloque);
        modificarBLOQUEenArchivoBLOQUE(pathBLOQUES,bloqueLibre + cantBLoquesSWAP,bloques[bloqueLibre + cantBLoquesSWAP],tamanioBloque);
   }
        } else printf("NO hay espacio");

    if (cantBloquesAAsignar ==  fcb->tamanio_archivo) {
        fcb->bloque_inicial = bloqueLibreAnt;
        guardar_fcb_en_archivo(fcb,pathFCB);
} 
}

/* fat[ultBloqueAsignado].block_value = INT32_MAX;
modificarFATenArchivoFAT(pathFAT, ultBloqueAsignado, &fat[ultBloqueAsignado]);
memset(bloques[ultBloqueAsignado + cantBLoquesSWAP]->valorDeBloque, 'U', tamanioBloque);
modificarBLOQUEenArchivoBLOQUE(pathBLOQUES,ultBloqueAsignado + cantBLoquesSWAP,bloques[ultBloqueAsignado + cantBLoquesSWAP],tamanioBloque);
   */      
}
else printf("Archivo ya asignado o NO HAY ESPACIO total para incluirlo.");
}

/* void eliminarBloques(char* pathFAT, char* pathFCB, FATEntry fat[],size_t cantBloquesTotales,size_t cantBLoquesSWAP){
    FCB *fcb = crear_fcb(pathFCB);

    // validar que no sea max uint
    int32_t numBloqueSiguiente = fat [fcb->bloque_inicial].block_value;

    fat [fcb->bloque_inicial].block_value = 0;
    modificarFATenArchivoFAT(pathFAT,fcb->bloque_inicial, &fat[fcb->bloque_inicial]);

    while (fat[numBloqueSiguiente].block_value != UINT32_MAX){
        int32_t numBloqueSiguienteAux = fat[numBloqueSiguiente].block_value;
        fat [numBloqueSiguiente].block_value = 0;
        modificarFATenArchivoFAT(pathFAT,fcb->bloque_inicial, &fat[numBloqueSiguiente]);
        numBloqueSiguiente = numBloqueSiguienteAux;
        
    }
    fat[numBloqueSiguiente].block_value = 0;
}


 */

void eliminarBloques(char* pathFAT, char* pathFCB, char* pathBLOQUES, FATEntry fat[], BLOQUE *bloques[],size_t cantBloquesTotales, size_t cantBLoquesSWAP,size_t tamanioBloque) {
    FCB *fcb = crear_fcb(pathFCB);
    int32_t numBloqueActual;
    int32_t numBloqueSiguiente;
    int32_t cantBLoquesUsadosPorArchivo;
    if (fcb != NULL) {
    numBloqueActual = fcb->bloque_inicial;
    cantBLoquesUsadosPorArchivo = fcb->tamanio_archivo;
    

    printf("%i\n", numBloqueActual);

} else {
    printf("Error: No se pudo crear el FCB\n");
    return;
}

if (numBloqueActual != INT32_MAX){

      for (int i = 0; i < cantBLoquesUsadosPorArchivo; i++) {

        numBloqueSiguiente = fat[numBloqueActual].block_value;
        fat[numBloqueActual].block_value = 0;

        printf("%i\n", numBloqueActual);
    
        modificarFATenArchivoFAT(pathFAT, numBloqueActual, &fat[numBloqueActual]);
        
        memset(bloques[numBloqueActual + cantBLoquesSWAP]->valorDeBloque,0, tamanioBloque);
        modificarBLOQUEenArchivoBLOQUE(pathBLOQUES,(numBloqueActual + cantBLoquesSWAP),bloques[numBloqueActual + cantBLoquesSWAP],tamanioBloque);
        
        numBloqueActual = numBloqueSiguiente;
    }

    //Actualizar el bloque inicial en el FCB
    fcb->bloque_inicial = INT32_MAX;
    guardar_fcb_en_archivo(fcb, pathFCB);
}}



void modificarFATenArchivoFAT(const char* pathFAT, uint32_t numeroBloque, FATEntry *nuevaEntrada){

    
    // Abrir el archivo en modo lectura y escritura binaria
    FILE *archivoFAT = fopen(pathFAT, "r+b");

    if (archivoFAT == NULL) {
        perror("Error al abrir el archivo FAT");
    }

    // Mover el puntero de archivo a la posición de la entrada deseada
    fseek(archivoFAT, (numeroBloque) * sizeof(int32_t), SEEK_SET);

    // Escribir la nueva entrada en el archivo
    fwrite(nuevaEntrada, sizeof(int32_t), 1, archivoFAT);

    // Cerrar el archivo
    fclose(archivoFAT);

}

int32_t obtenerUltimoBloque(FATEntry fat[],FCB *fcb) {

	if (fcb->bloque_inicial != INT32_MAX || fcb->bloque_inicial != 0){

	int32_t bloqueEvaluado =  fcb->bloque_inicial;
    while (fat[bloqueEvaluado].block_value != INT32_MAX){
		bloqueEvaluado = fat[bloqueEvaluado].block_value;
	}
    
    return bloqueEvaluado;
}}



void sumarBloques (char* pathFAT, char* pathBLOQUES, char* pathFCB, FATEntry fat[], BLOQUE *bloques[], size_t cantBloquesTotales, size_t cantBLoquesSWAP,size_t tamanioBloque, uint32_t bloquesASumar){
    FCB *fcb = crear_fcb(pathFCB);
    int32_t ultBloque = obtenerUltimoBloque(fat,fcb);

    printf("UltBloque: %i \n",ultBloque);

    while (bloquesASumar > 1) {
    fat[ultBloque].block_value = buscarBloqueLibre(fat,cantBloquesTotales);
    memset(bloques[ultBloque + cantBLoquesSWAP]->valorDeBloque, obtenerContenidoRandom(), tamanioBloque);
    modificarFATenArchivoFAT(pathFAT, ultBloque, &fat[ultBloque]);
    modificarBLOQUEenArchivoBLOQUE(pathBLOQUES,(ultBloque + cantBLoquesSWAP),bloques[ultBloque + cantBLoquesSWAP],tamanioBloque);
    ultBloque = fat[ultBloque].block_value;
    bloquesASumar--;
    }

    fat[ultBloque].block_value = INT32_MAX;
    memset(bloques[ultBloque + cantBLoquesSWAP]->valorDeBloque, obtenerContenidoRandom(), tamanioBloque);
    modificarFATenArchivoFAT(pathFAT, ultBloque, &fat[ultBloque]);
    modificarBLOQUEenArchivoBLOQUE(pathBLOQUES,(ultBloque + cantBLoquesSWAP),bloques[ultBloque + cantBLoquesSWAP],tamanioBloque);
    
}

void restarBloques (char* pathFAT, char* pathBLOQUES, char* pathFCB, FATEntry fat[], BLOQUE *bloques[], size_t cantBloquesTotales, size_t cantBLoquesSWAP,size_t tamanioBloque, uint32_t bloquesARestar){
    FCB *fcb = crear_fcb(pathFCB);
    int32_t vecesAIterarAlUltBloque = fcb->tamanio_archivo - bloquesARestar;
    printf("vecesAIterarAlUltBloque: %i\n",vecesAIterarAlUltBloque);
    int32_t bloqueEvaluado = fat[fcb->bloque_inicial].block_value;
    printf("bloqueEvaluado: %i\n",bloqueEvaluado);
    int32_t bloqueAnt;

    for (int32_t i = 1;vecesAIterarAlUltBloque > i;i++) {
        bloqueAnt = bloqueEvaluado;
        bloqueEvaluado = fat[bloqueEvaluado].block_value;
    }

    fat[bloqueAnt].block_value = INT32_MAX;
    printf("INT32_MAX: %i\n",fat[bloqueAnt].block_value);
    memset(bloques[bloqueAnt + cantBLoquesSWAP]->valorDeBloque, obtenerContenidoRandom(), tamanioBloque);
    modificarFATenArchivoFAT(pathFAT, bloqueAnt, &fat[bloqueAnt]);
    modificarBLOQUEenArchivoBLOQUE(pathBLOQUES,(bloqueAnt + cantBLoquesSWAP),bloques[bloqueAnt + cantBLoquesSWAP],tamanioBloque);

    int32_t bloqueSiguiente;

    while (fat[bloqueEvaluado].block_value != INT32_MAX){
        bloqueSiguiente = fat[bloqueEvaluado].block_value;
        fat[bloqueEvaluado].block_value = 0;
        memset(bloques[bloqueEvaluado + cantBLoquesSWAP]->valorDeBloque, 0, tamanioBloque);
        modificarFATenArchivoFAT(pathFAT, bloqueEvaluado, &fat[bloqueEvaluado]);
        modificarBLOQUEenArchivoBLOQUE(pathBLOQUES,(bloqueEvaluado + cantBLoquesSWAP),bloques[bloqueEvaluado + cantBLoquesSWAP],tamanioBloque);

        bloqueEvaluado = bloqueSiguiente;
    }

    printf("bloqueEvaluado: %i\n",bloqueEvaluado);

    fat[bloqueEvaluado].block_value = 0;
    memset(bloques[bloqueEvaluado + cantBLoquesSWAP]->valorDeBloque, 0, tamanioBloque);
    modificarFATenArchivoFAT(pathFAT, bloqueEvaluado, &fat[bloqueEvaluado]);
    modificarBLOQUEenArchivoBLOQUE(pathBLOQUES,(bloqueEvaluado + cantBLoquesSWAP),bloques[bloqueEvaluado + cantBLoquesSWAP],tamanioBloque);

}
    




