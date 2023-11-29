#include "filesystem.h"
#include "commons/config.h"
#include "fcb.h"
#include "fat.h"
#include "swap.h"
#include "bloque.h"
#include "directorio.h"

pthread_t hilo_memoria;
pthread_t hilo_kernel;

t_log *logger = NULL;
t_argumentos_filesystem *argumentos_filesystem = NULL;
t_config_filesystem *configuracion_filesystem = NULL;

int socket_kernel = -1;
int conexion_con_kernel = -1;
int conexion_con_memoria = -1;

char* concatenarRutas(const char* rutaFat, const char* nombreFCB) {
    // Asegúrate de tener suficiente espacio para ambas cadenas más el carácter nulo
    size_t longitudTotal = strlen(rutaFat) + strlen(nombreFCB) + 1;

    // Reserva memoria para la nueva cadena
    char* rutaCompleta = (char*)malloc(longitudTotal);

    // Verifica si la asignación de memoria fue exitosa
    if (rutaCompleta == NULL) {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }

    // Copia la primera cadena en la nueva cadena
    strcpy(rutaCompleta, rutaFat);

    // Concatena la segunda cadena en la nueva cadena
    strcat(rutaCompleta, nombreFCB);

    return rutaCompleta;
}

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


	crear_archivo (configuracion_filesystem->path_fcb,"EstoEsUnNuevoArchivo");

	// INICIAR ARCHIVO DE BLOQUES V2

	 crearArchivoDeBloques("/home/utnso/tp-2023-2c-Algorritmos/Filesystem/BlocksFile/ARCHIVO_BLOQUES.bin", configuracion_filesystem->cant_bloques_total, configuracion_filesystem->tam_bloques);

	/* // INICIAR PARTICION SWAP EN ARCHIVO DE BLOQUES -- NO NECESARIO, POR EL MOMENTO NO MANEJO LA SWAP.

	int blocks_file_checker = iniciarSWAP(logger, configuracion_filesystem->path_bloques, configuracion_filesystem->cant_bloques_swap);
	if (blocks_file_checker == 0)
	{
		log_debug(logger, "FS BLOCKS FILE: BLOCKS FILE con particion SWAP iniciado correctamente.");
	}
	else
		log_debug(logger, "FS BLOCKS FILE: No fue posible iniciar BLOCKS FILE.");
 */

	// INICIAR BLOQUE FAT Y PARTICION FAT EN ARCHIVO DE BLOQUES.

	int fat_checker = iniciarFAT(logger, configuracion_filesystem->path_fat, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->cant_bloques_swap,configuracion_filesystem->tam_bloques);
	if (fat_checker == 0)
	{
		log_debug(logger, "FS FAT: Tabla FAT iniciada correctamente.");
	}
	else
		log_debug(logger, "FS FAT: No fue posible iniciar la tabla FAT.");

   
	// DIRECTORIO: Es una tabla con una entrada por archivo. Cada entrada tiene:
		//Nombre: El nombre de archivo, utilizado como ID de archivo.
		//Bloque inicial: El bloque logico inicial, se utilizará en la tabla FAT para buscar su bloque siguiente.		
    
	DirectorioArray directorio = inicializarDirectorioArray(logger);
    procesarArchivosEnDirectorio(&directorio, configuracion_filesystem->path_fcb);

	
    // Utilizar el array de directorios como desees
    for (size_t i = 0; i < directorio.cantidadDirectorios; i++) {
        printf("Nombre: %s, Bloque Inicial: %u\n", directorio.directorios[i].nombreArchivo, directorio.directorios[i].numBloqueInicial);
    }

    // Liberar la memoria utilizada por el array de directorios, al final, al terminar filesystem.
    
	// prueba asignar bloques
	FATEntry* fat = abrirFAT(configuracion_filesystem->path_fat,configuracion_filesystem->cant_bloques_total,configuracion_filesystem->cant_bloques_swap);
	
	// Prueba del contenido de cada ENTRADA FAT
	
	printf("Mostrando tabla FAT inicial"); // Luego borrar esta linea!
	for (size_t i = 0; i < (configuracion_filesystem->cant_bloques_total- configuracion_filesystem->cant_bloques_swap); ++i) {
        printf("Entrada FAT %zu: %d\n", i, fat[i].block_value);
    }
	
	char* ruta = concatenarRutas(configuracion_filesystem->path_fcb,"Notas1erParcialK9999.fcb");
	BLOQUE** bloques = leerBloquesDesdeArchivo(configuracion_filesystem->path_bloques,configuracion_filesystem->cant_bloques_total,configuracion_filesystem->tam_bloques);
	
	printf("Mostrando tabla BLOQUES inicial"); // Luego borrar esta linea!
	for (size_t i = 0; i < configuracion_filesystem->cant_bloques_total; ++i) {
    printf("Entrada BLOQUES %zu: %s\n", i, bloques[i]->valorDeBloque);
} 

	//modificarBloque (configuracion_filesystem->path_bloques,configuracion_filesystem->tam_bloques);
	asignarBloques(configuracion_filesystem->path_fat,"/home/utnso/tp-2023-2c-Algorritmos/Filesystem/BlocksFile/ARCHIVO_BLOQUES.bin",ruta,bloques,fat,configuracion_filesystem->cant_bloques_total,configuracion_filesystem->cant_bloques_swap,configuracion_filesystem->tam_bloques);
	
	printf("Mostrando tabla FAT post modificacion\n"); // Luego borrar esta linea!

	for (size_t i = 0; i < (configuracion_filesystem->cant_bloques_total- configuracion_filesystem->cant_bloques_swap); ++i) {
        printf("Entrada FAT %zu: %d\n", i, fat[i].block_value);
    }

	printf("Mostrando tabla BLOQUES post modificacion\n"); // Luego borrar esta linea!
	for (size_t i = 0; i < configuracion_filesystem->cant_bloques_total; ++i) {
    printf("Entrada BLOQUES %zu: %s\n", i, bloques[i]->valorDeBloque);
} 

	eliminarBloques(configuracion_filesystem->path_fat,ruta,configuracion_filesystem->path_bloques,fat,bloques,configuracion_filesystem->cant_bloques_total,configuracion_filesystem->cant_bloques_swap,configuracion_filesystem->tam_bloques);
	
	printf("Mostrando tabla FAT post eliminacion de bloques\n");
	for (size_t i = 0; i < (configuracion_filesystem->cant_bloques_total- configuracion_filesystem->cant_bloques_swap); ++i) {
        printf("Entrada FAT %zu: %d\n", i, fat[i].block_value);
    }

	printf("Mostrando tabla BLOQUES post eliminacion de bloques\n");
	for (size_t i = 0; i < configuracion_filesystem->cant_bloques_total; ++i) {
    printf("Entrada BLOQUES %zu: %s\n", i, bloques[i]->valorDeBloque);
} 

	// liberar memoria!
  	// PRUEBA DE TRUNCAR truncar_archivo

	asignarBloques(configuracion_filesystem->path_fat,"/home/utnso/tp-2023-2c-Algorritmos/Filesystem/BlocksFile/ARCHIVO_BLOQUES.bin","/home/utnso/tp-2023-2c-Algorritmos/Filesystem/Fcbs/estadisticas.fcb",bloques,fat,configuracion_filesystem->cant_bloques_total,configuracion_filesystem->cant_bloques_swap,configuracion_filesystem->tam_bloques);
	
	
	truncar_archivo ("/home/utnso/tp-2023-2c-Algorritmos/Filesystem/Fcbs/estadisticas.fcb",6,configuracion_filesystem,fat,bloques); 

	printf("Mostrando tabla FAT post truncamiento\n");
	for (size_t i = 0; i < (configuracion_filesystem->cant_bloques_total- configuracion_filesystem->cant_bloques_swap); ++i) {
        printf("Entrada FAT %zu: %d\n", i, fat[i].block_value);
    }

	printf("Mostrando tabla BLOQUES post truncamiento\n");
	for (size_t i = 0; i < configuracion_filesystem->cant_bloques_total; ++i) {
    printf("Entrada BLOQUES %zu: %s\n", i, bloques[i]->valorDeBloque);
} 

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
	liberarDirectorioArray(&directorio);
	cerrarFAT(fat);
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


 int32_t crear_archivo (char* path,char* nombreNuevoArchivo) {
	char nombreFormateado[256]; // Asegúrate de que este array es lo suficientemente grande
    sprintf(nombreFormateado, "%s.fcb", nombreNuevoArchivo);
	//Crear un archivo FCB (en ejecucion) con tamaño 0 y sin bloque inicial. Coloco bloque inicial -1 para indicar que inicia SIN bloque inicial.
	FCB *fcbArchivoNuevo = iniciar_fcb(nombreNuevoArchivo, 0, UINT32_MAX);

	if (fcbArchivoNuevo!= NULL){
		log_debug(logger,"FS: EStructura FCB de nuevo archivo creado correctamente.");
		//Guardar el fcb creado en un archivo .fcb en el disco.

		 char rutaCompleta[256];  // Tamaño suficientemente grande para contener la ruta completa

    	// Concatenar la ruta base con el nombre del nuevo archivo
    	snprintf(rutaCompleta, sizeof(rutaCompleta), "%s%s", path, nombreFormateado);

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



/* void ampliar_tamano_archivo(FCB *fcb , t_config *config,int nuevo_tamano) {
 	fcb->tamanio_archivo = nuevo_tamano;
	char nuevo_tamano_str[12]; // Suficientemente grande para un uint32_t
    sprintf(nuevo_tamano_str, "%u", fcb->tamanio_archivo);
	config_set_value(config, "TAMANIO_ARCHIVO", nuevo_tamano_str);
} */


int32_t truncar_archivo(char* path, uint32_t nuevo_tamano,t_config_filesystem *configuracion_filesystem,FATEntry fat[], BLOQUE *bloques[]) {

	// validar que haya espacio en FAT
	FCB *fcb = crear_fcb(path);

	printf("%d",fcb->tamanio_archivo);

	if (nuevo_tamano > fcb->tamanio_archivo){
		uint32_t aSumar = nuevo_tamano - fcb->tamanio_archivo;
		sumarBloques(configuracion_filesystem->path_fat,configuracion_filesystem->path_bloques,path,fat,bloques,configuracion_filesystem->cant_bloques_total,configuracion_filesystem->cant_bloques_swap,configuracion_filesystem->tam_bloques,aSumar);
		fcb->tamanio_archivo = nuevo_tamano;
	}

	if ( fcb->tamanio_archivo >  nuevo_tamano){
		uint32_t aRestar = fcb->tamanio_archivo - nuevo_tamano;
		restarBloques(configuracion_filesystem->path_fat,configuracion_filesystem->path_bloques,path,fat,bloques,configuracion_filesystem->cant_bloques_total,configuracion_filesystem->cant_bloques_swap,configuracion_filesystem->tam_bloques,aRestar);
		fcb->tamanio_archivo = nuevo_tamano;
	}

	guardar_fcb_en_archivo(fcb,path);
	log_info(logger,"Truncar Archivo: <%s> - Tamaño: <%d>",fcb->nombre_archivo,nuevo_tamano);
}

