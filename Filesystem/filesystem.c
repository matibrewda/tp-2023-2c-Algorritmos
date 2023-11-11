#include "filesystem.h"
#include "commons/config.h"
#include "fcb.h"
#include "fat.h"
#include "swap.h"

pthread_t hilo_memoria;
pthread_t hilo_kernel;

t_log *logger = NULL;
t_argumentos_filesystem *argumentos_filesystem = NULL;
t_config_filesystem *configuracion_filesystem = NULL;

int socket_kernel = -1;
int conexion_con_kernel = -1;
int conexion_con_memoria = -1;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_FILESYSTEM, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_FILESYSTEM);

	argumentos_filesystem = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_filesystem == NULL)
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	configuracion_filesystem = leer_configuracion(logger, argumentos_filesystem->ruta_archivo_configuracion);

	if (configuracion_filesystem == NULL)
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	// INICIAR SU FCB
	char *ruta_fcb_completa = NULL;
	asprintf(&ruta_fcb_completa, "%s/%s", configuracion_filesystem->path_fcb, "Notas1erParcialK9999.fcb");
	FCB *fcb = crear_fcb(ruta_fcb_completa);
	if (fcb != NULL)
	{
		log_info(logger, "Pude cargar el siguiente archivo fcb");
		log_info(logger, "Nombre archivo: %s - Tamaño Archivo: %d - Inicio de bloque: %d", fcb->nombre_archivo, fcb->tamanio_archivo, fcb->bloque_inicial);
	}
	else
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
		return EXIT_FAILURE;
	}


	crear_archivo (configuracion_filesystem->path_fcb,"LuchoPruebaGenerarFCBNUEVO.fcb");


	// INICIAR PARTICION SWAP EN ARCHIVO DE BLOQUES

	int blocks_file_checker = iniciarSWAP(logger, configuracion_filesystem->path_bloques, configuracion_filesystem->cant_bloques_swap);
	if (blocks_file_checker == 0)
	{
		log_debug(logger, "FS BLOCKS FILE: BLOCKS FILE con particion SWAP iniciado correctamente.");
	}
	else
		log_debug(logger, "FS BLOCKS FILE: No fue posible iniciar BLOCKS FILE.");


	// INICIAR BLOQUE FAT Y PARTICION FAT EN ARCHIVO DE BLOQUES.

	int fat_checker = iniciarFAT(logger, configuracion_filesystem->path_fat, configuracion_filesystem->path_bloques, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->cant_bloques_swap);
	if (fat_checker == 0)
	{
		log_debug(logger, "FS FAT: FAT iniciado correctamente.");
	}
	else
		log_debug(logger, "FS FAT: No fue posible iniciar FAT.");

	socket_kernel = crear_socket_servidor(logger, configuracion_filesystem->puerto_escucha_kernel, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
	if (socket_kernel == -1)
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	conexion_con_memoria = crear_socket_cliente(logger, configuracion_filesystem->ip_memoria, configuracion_filesystem->puerto_memoria, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);
	if (conexion_con_memoria == -1)
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	conexion_con_kernel = esperar_conexion_de_cliente(logger, socket_kernel, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
	if (conexion_con_kernel == -1)
	{
		terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
		return EXIT_FAILURE;
	}

	pthread_create(&hilo_kernel, NULL, comunicacion_kernel, NULL);
	pthread_create(&hilo_memoria, NULL, comunicacion_memoria, NULL);

	pthread_join(hilo_kernel, NULL);
	pthread_join(hilo_memoria, NULL);

	// Finalizacion
	terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
	return EXIT_SUCCESS;
}

void terminar_filesystem(t_log *logger, t_argumentos_filesystem *argumentos_filesystem, t_config_filesystem *configuracion_filesystem, int socket_kernel, int conexion_con_kernel, int conexion_con_memoria)
{
	if (logger != NULL)
	{
		log_debug(logger, "Finalizando %s", NOMBRE_MODULO_FILESYSTEM);
	}

	destruir_logger(logger);
	destruir_argumentos(argumentos_filesystem);
	destruir_configuracion(configuracion_filesystem);

	if (socket_kernel != -1)
	{
		close(socket_kernel);
	}

	if (conexion_con_kernel != -1)
	{
		close(conexion_con_kernel);
	}

	if (conexion_con_memoria != -1)
	{
		close(conexion_con_memoria);
	}
}

void *comunicacion_memoria()
{
	while (true)
	{
		op_code operacion_recibida_memoria = esperar_operacion(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
		log_debug(logger, "se recibió la operacion %s de %s", nombre_opcode(operacion_recibida_memoria), NOMBRE_MODULO_MEMORIA);
		
		// enviar_operacion_sin_paquete(logger,conexion_con_memoria,operacion_recibida_memoria,NOMBRE_MODULO_FILESYSTEM,NOMBRE_MODULO_MEMORIA);
		// log_debug(logger,"le devuelvo la operacion recibida: %s a %s",operacion_recibida_memoria,NOMBRE_MODULO_MEMORIA);
	}
}

void *comunicacion_kernel()
{
	while (true)
	{
		int operacion_recibida_kernel = esperar_operacion(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, conexion_con_kernel);
		log_debug(logger, "se recibio la operacion %s de %s", nombre_opcode(operacion_recibida_kernel), NOMBRE_MODULO_KERNEL);
		
		/*
		t_paquete *paquete_para_cpu = crear_paquete(logger, operacion_recibida_kernel);
		log_debug(logger,"creo un paquete con el codigo de operacion recibido: %s , y se lo devuelvo a",operacion_recibida_kernel, NOMBRE_MODULO_KERNEL);
		enviar_paquete(logger,conexion_con_kernel,paquete_para_cpu,NOMBRE_MODULO_FILESYSTEM,NOMBRE_MODULO_KERNEL);
		*/
	}
}


 int crear_archivo (char* path,char* nombreNuevoArchivo) {

	//Crear un archivo FCB (en ejecucion) con tamaño 0 y sin bloque inicial. Coloco bloque inicial -1 para indicar que inicia SIN bloque inicial.
	FCB *fcbArchivoNuevo = iniciar_fcb(nombreNuevoArchivo, 0, UINT32_MAX);

	if (fcbArchivoNuevo!= NULL){
		log_debug(logger,"FS: EStructura FCB de nuevo archivo creado correctamente.");
		//Guardar el fcb creado en un archivo .fcb en el disco.

		 char rutaCompleta[256];  // Tamaño suficientemente grande para contener la ruta completa

    	// Concatenar la ruta base con el nombre del nuevo archivo
    	snprintf(rutaCompleta, sizeof(rutaCompleta), "%s%s", path, nombreNuevoArchivo);

		guardar_fcb_en_archivo (fcbArchivoNuevo,rutaCompleta);
		//Liberar la memoria no utilizada.
		log_debug(logger,"FS: Archivo FCB, del nuevo archivo, creado correctamente.");

		//LOG MINIMO PEDIDO POR TP:
		log_info(logger,"Crear Archivo: Crear Archivo: %s",nombreNuevoArchivo);
		free(fcbArchivoNuevo);
	}

	// Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK.
	return 0;
} 


int truncar_archivo(char* path, uint32_t nuevo_tamano) {
	FCB *fcb;
	//Agarro la ruta del fcb existente y el nuevo tamaño para truncarlo.
	if (path!= NULL) {
		// creo la estructura para manejarlo.
		FCB *fcb = crear_fcb(path);
	} else {return -1;}
		//Pueden pasar dos cosas, 1) Que se trate de ampliar o 2) que se trate de reducir.
    if (nuevo_tamano > fcb->tamanio_archivo) {
        // Ampliar el tamaño del archivo
        ampliar_tamano_archivo(fcb, nuevo_tamano);
		return 0;
    } else if (nuevo_tamano < fcb->tamanio_archivo) {
        // Reducir el tamaño del archivo
        reducir_tamano_archivo(fcb, nuevo_tamano);
    }
    // Si el nuevo tamaño es igual al actual, no se requiere acción.
}

char* obtener_nombre_archivo(char* ruta) {
    char* nombre_archivo = strrchr(ruta, '/'); // Busca la última aparición de '/' en la cadena
    if (nombre_archivo != NULL) {
        // Avanza al siguiente carácter para obtener el nombre del archivo
        nombre_archivo++;
    } else {
        // Si no hay '/', asumimos que toda la ruta es el nombre del archivo
        nombre_archivo = ruta;
    }

    return nombre_archivo;
}


// PETICIONES KERNEL...

/* void abrirArchivo(char* ruta_archivo){
Abrir archivo:
La operación de abrir archivo consistirá en verificar que exista el
FCB correspondiente al archivo.

En caso de que exista deberá devolver el tamaño del archivo.
En caso de que no exista, deberá informar que el archivo no existe.

// Abrir FCB con nombre ruta_archivo
// Si no existe DEVOLVER ARCHIVO NO EXISTE
// Si existe DEVOLVER TAMAÑO DEL ARCHIVO
} */

/*int crearArchivo(){
Crear Archivo
En la operación crear archivo, se deberá crear un archivo FCB con tamaño 0 y sin bloque inicial.
Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK.

return EXIT_SUCCESS;
}*/

/* void truncarArchivo(char* ruta_archivo){
	Truncar Archivo:
Al momento de truncar un archivo, pueden ocurrir 2 situaciones:
1) Ampliar el tamaño del archivo: Al momento de ampliar el tamaño del archivo deberá
actualizar el tamaño del archivo en el FCB y se le deberán asignar tantos
bloques como sea necesario para poder direccionar el nuevo tamaño.

2) Reducir el tamaño del archivo: Se deberá asignar el nuevo tamaño del archivo en el FCB y
se deberán marcar como libres todos los bloques que ya no sean necesarios para direccionar
el tamaño del archivo (descartando desde el final del archivo hacia el principio).
Siempre se van a poder truncar archivos para ampliarlos, no se realizará la prueba de llenar el FS.
}*/

/* void leerArchivo(char* ruta_archivo){

}

void escribirArchivo(char* ruta_archivo){

} */

// Peticiones MEMORIA:

/* int inicarProceso(){
return EXIT_SUCCESS;
}

int finalizarProceso(){
return EXIT_SUCCESS;
} */

// PETICIONES KERNEL

/* void abrirArchivo(char* ruta_archivo){
Abrir archivo:
La operación de abrir archivo consistirá en verificar que exista el
FCB correspondiente al archivo.

En caso de que exista deberá devolver el tamaño del archivo.
En caso de que no exista, deberá informar que el archivo no existe.

// Abrir FCB con nombre ruta_archivo
// Si no existe DEVOLVER ARCHIVO NO EXISTE
// Si existe DEVOLVER TAMAÑO DEL ARCHIVO
}*/

/* int crearArchivo(){
 Crear Archivo
En la operación crear archivo, se deberá crear un archivo FCB con tamaño 0 y sin bloque inicial.
Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK.

return EXIT_SUCCESS;
} */

/*void truncarArchivo(char* ruta_archivo){
		Truncar Archivo:
Al momento de truncar un archivo, pueden ocurrir 2 situaciones:
1) Ampliar el tamaño del archivo: Al momento de ampliar el tamaño del archivo deberá
actualizar el tamaño del archivo en el FCB y se le deberán asignar tantos
bloques como sea necesario para poder direccionar el nuevo tamaño.

2) Reducir el tamaño del archivo: Se deberá asignar el nuevo tamaño del archivo en el FCB y
se deberán marcar como libres todos los bloques que ya no sean necesarios para direccionar
el tamaño del archivo (descartando desde el final del archivo hacia el principio).
Siempre se van a poder truncar archivos para ampliarlos, no se realizará la prueba de llenar el FS.

}*/

/* void leerArchivo(char* ruta_archivo){

}

void escribirArchivo(char* ruta_archivo){

} */

// Peticiones MEMORIA:

/* int inicarProceso(){
return EXIT_SUCCESS;
}

int finalizarProceso(){
return EXIT_SUCCESS;
} */