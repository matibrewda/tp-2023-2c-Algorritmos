#include "filesystem.h"

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


	

	// INICIAR ARCHIVO DE BLOQUES V2
	// liberar memoria!
  	// PRUEBA DE TRUNCAR truncar_archivo
 

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

		switch (operacion_recibida_memoria)
		{
		case SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM:

			break;
		case SOLICITUD_LIBERAR_BLOQUES_EN_FILESYSTEM:
		
			break;
		case SOLICITUD_CONTENIDO_BLOQUE_EN_FILESYSTEM:
		
			break;
		case SOLICITUD_ESCRIBIR_PAGINA_EN_SWAP:
		
			break;	
		default:
			break;
		}
		
		// enviar_operacion_sin_paquete(logger,conexion_con_memoria,operacion_recibida_memoria,NOMBRE_MODULO_FILESYSTEM,NOMBRE_MODULO_MEMORIA);
		// log_debug(logger,"le devuelvo la operacion recibida: %s a %s",operacion_recibida_memoria,NOMBRE_MODULO_MEMORIA);
	}
}

void *comunicacion_kernel()
{
	
	while (true)
	{
		op_code operacion_recibida_kernel = esperar_operacion(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);
		
		log_debug(logger, "se recibio la operacion %s de %s", nombre_opcode(operacion_recibida_kernel), NOMBRE_MODULO_KERNEL);
		switch (operacion_recibida_kernel)
		{
		case SOLICITUD_ABRIR_ARCHIVO_FS:
		char *nombre_abrir = leer_paquete_solicitud_abrir_archivo_fs(logger, conexion_con_kernel);
		log_debug(logger,"Entramos a F_CREATE");
		int tam = abrirarchivo(nombre_abrir);
		if (tam<0)
		{
			t_paquete *respuesta_abrir = crear_paquete_respuesta_abrir_archivo_fs(logger, false, 0);
			enviar_paquete(logger,conexion_con_kernel,respuesta_abrir,NOMBRE_MODULO_FILESYSTEM,NOMBRE_MODULO_KERNEL);
		}else
		{
			t_paquete *respuesta_abrir = crear_paquete_respuesta_abrir_archivo_fs(logger, true, tam);
		}
		break;



		case SOLICITUD_CREAR_ARCHIVO_FS:
		char *nombre_crear = leer_paquete_solicitud_crear_archivo_fs(logger,conexion_con_kernel);
		log_debug(logger,"Entramos a F_CREATE");
		crear_archivo(nombre_crear);
		t_paquete *paquete_crear = crear_paquete(logger,OK);
		enviar_paquete(logger,conexion_con_kernel,paquete_crear,NOMBRE_MODULO_FILESYSTEM,NOMBRE_MODULO_KERNEL);
		break;


		case SOLICITUD_LEER_ARCHIVO_FS:
		char **nombre_leer;
		int *puntero_lectura;
		int* direccion_fisica_a_escribir;
		leer_paquete_solicitud_leer_archivo_fs(logger, conexion_con_kernel, nombre_leer, puntero_lectura, direccion_fisica_a_escribir);
		log_debug(logger, "entramos a f_read");
		int nro_bloque = *puntero_lectura / configuracion_filesystem->tam_bloques;
		uint32_t bloque_fat = buscar_bloque_fat(nro_bloque,nombre_leer);
		leer_bloque(bloque_fat);
		t_paquete *paquete_leer = crear_paquete(logger,OK);
		enviar_paquete(logger,conexion_con_kernel,paquete_leer,NOMBRE_MODULO_FILESYSTEM,NOMBRE_MODULO_KERNEL);
		free(paquete_leer);
		break;
		case SOLICITUD_ESCRIBIR_ARCHIVO_FS:
		log_debug(logger, "entramos a f_write");
		char **nombre_escribir;
		int *puntero_archivo_a_escribir;
		int* direccion_fisica_a_leer;
		char* informacion;
		leer_paquete_solicitud_escribir_archivo_fs(logger, conexion_con_kernel,nombre_escribir,puntero_archivo_a_escribir, direccion_fisica_a_leer);
		int nro_bloque = *puntero_lectura / configuracion_filesystem->tam_bloques;
		uint32_t bloque_fat = buscar_bloque_fat(nro_bloque,nombre_leer);
		escribir_bloque(bloque_fat,informacion);



		break;


		case SOLICITUD_TRUNCAR_ARCHIVO_FS:

		char** nombre_truncar;
		int* nuevo_tamanio_archivo;
		leer_paquete_solicitud_truncar_archivo_fs(logger,conexion_con_kernel, nombre_truncar,nuevo_tamanio_archivo);
		log_debug(logger,"entro a f_truncate");
		truncar_archivo(nombre_truncar,nuevo_tamanio_archivo);
		break;

		default:
			break;
		}
		
		/*
		t_paquete *paquete_para_cpu = crear_paquete(logger, operacion_recibida_kernel);
		log_debug(logger,"creo un paquete con el codigo de operacion recibido: %s , y se lo devuelvo a",operacion_recibida_kernel, NOMBRE_MODULO_KERNEL);
		enviar_paquete(logger,conexion_con_kernel,paquete_para_cpu,NOMBRE_MODULO_FILESYSTEM,NOMBRE_MODULO_KERNEL);
		*/
	}
}


int crear_archivo (char* nombreNuevoArchivo) {
	char* path = configuracion_filesystem->path_fcb;
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
		list_add(archivos,fcbArchivoNuevo);


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




void ampliar_tamano_archivo(FCB *fcb, int nuevo_tamano) {
    // Actualizar tamaño en la estructura FCB
	int bloques_a_agregar = (nuevo_tamano-(fcb->tamanio_archivo))/configuracion_filesystem->tam_bloques;
	uint32_t tamanio_FAT = (configuracion_filesystem->cant_bloques_total-configuracion_filesystem->cant_bloques_swap)*sizeof(uint32_t);
	uint32_t bloque_a_leer = fcb->bloque_inicial;
	int fatfd = open(configuracion_filesystem->path_fat,O_RDWR);
	uint32_t *entrada_fat = mmap(NULL,tamanio_FAT,PROT_READ | PROT_WRITE,MAP_SHARED,fatfd,0);

	//busco el ultimo bloque del archivo
	while (entrada_fat[bloque_a_leer]!=UINT32_MAX)
	{
		bloque_a_leer=entrada_fat[bloque_a_leer];
	}

	uint32_t ultimo_bloque = bloque_a_leer;
	bloque_a_leer = 1;

	while (bloques_a_agregar != 0)
	{
		if (entrada_fat[bloque_a_leer]==0)
		{
			entrada_fat[ultimo_bloque]= bloque_a_leer;
			ultimo_bloque = bloque_a_leer;
			bloques_a_agregar--;
		}
		bloque_a_leer++;
	}
	entrada_fat[ultimo_bloque]=UINT32_MAX;
	munmap(entrada_fat,tamanio_FAT);
	close(fatfd);
	fcb->tamanio_archivo = nuevo_tamano;
    // Actualizar tamaño en la configuración

}

void reducir_tamano_archivo(FCB *fcb, int nuevo_tamano) {
	int bloques_a_quitar = ((fcb->tamanio_archivo)-nuevo_tamano)/configuracion_filesystem->tam_bloques;
    fcb->tamanio_archivo = nuevo_tamano;
	uint32_t tamanio_FAT = (configuracion_filesystem->cant_bloques_total-configuracion_filesystem->cant_bloques_swap)*sizeof(uint32_t);
	uint32_t bloque_a_leer = fcb->bloque_inicial;
	int fatfd = open(configuracion_filesystem->path_fat,O_RDWR);
	uint32_t *entrada_fat = mmap(NULL,tamanio_FAT,PROT_READ | PROT_WRITE,MAP_SHARED,fatfd,0);

	//busco el ultimo bloque del archivo
	while (entrada_fat[bloque_a_leer]!=UINT32_MAX)
	{
		bloque_a_leer=entrada_fat[bloque_a_leer];
	}
	uint32_t ultimo_bloque = bloque_a_leer;
	while (bloques_a_quitar!=0)
	{
		bloque_a_leer=1;
		while (entrada_fat[bloque_a_leer]!=ultimo_bloque);{
			bloque_a_leer++;
		} 
		entrada_fat[ultimo_bloque]=0; //Libero el ultimo bloque
		ultimo_bloque = bloque_a_leer;
		bloques_a_quitar--;
	}
	entrada_fat[ultimo_bloque]=UINT32_MAX;
	munmap(entrada_fat,tamanio_FAT);
	close(fatfd);
	
}

void truncar_archivo(char* nombre, int nuevo_tamano){
	char* nombre;
	FCB *fcb = buscar_archivo(nombre);
	t_config *config;
	log_debug(logger,"Truncar Archivo: Iniciado.");
	//BUSCAR RUTA DEL ARCHIVO COMLETAR!!!
	//Agarro la ruta del fcb existente y el nuevo tamaño para truncarlo.
	if (fcb	== NULL) {
		log_debug(logger,"Truncar Archivo: No existe el archivo: <%s>.",nombre);
		return ;
	}
    if (nuevo_tamano > fcb->tamanio_archivo) {
		log_info(logger,"Truncar Archivo: <%s> - Tamaño: <%d>",fcb->nombre_archivo,nuevo_tamano);
        // Ampliar el tamaño del archivo
        ampliar_tamano_archivo(fcb, nuevo_tamano);
		return ;
    } else if (nuevo_tamano < fcb->tamanio_archivo) {
		log_info(logger,"Truncar Archivo: <%s> - Tamaño: <%d>",fcb->nombre_archivo,nuevo_tamano);
        // Reducir el tamaño del archivo
        reducir_tamano_archivo(fcb, nuevo_tamano);
		return ;
    }
    // Si el nuevo tamaño es igual al actual, no se requiere acción.
	return 0;
}

void inicializar_fat(){
	FILE* fat ;
	log_debug(logger,"Inicializando Tabla FAT");
	fat = fopen(configuracion_filesystem->path_fat,"rb");
	if(!fat){
		fat = fopen(configuracion_filesystem->path_fat,"wb");
		if (fat)
		{	
			uint32_t a = 0;
			uint32_t tamanio_fat = (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap);
			for (uint32_t i = 0; i < tamanio_fat ; i++)
			{
				fwrite(&a,sizeof(uint32_t),1,fat);
			}

		}else
	{
		log_debug(logger,"No se pudo crear el archivo FAT");
	}

	} else
	{
		log_debug(logger,"ya existe el archivo FAT");
	}
	
}

void inicializar_archivo_de_bloques(){
	int fd = open(configuracion_filesystem->path_bloques, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	int tamanio_bloques = configuracion_filesystem->cant_bloques_total*configuracion_filesystem->tam_bloques;
	ftruncate(fd,tamanio_bloques);
	void *ptrblq = mmap(NULL, tamanio_bloques,PROT_READ | PROT_WRITE, MAP_SHARED ,fd, 0);
	void *bloques = malloc(tamanio_bloques);
	memcpy(ptrblq,bloques,tamanio_bloques);
	munmap(ptrblq,tamanio_bloques);
	free(bloques);
	close(fd);
}

void escribir_bloque(uint32_t bloqueFAT,char* informacion){
	uint32_t bloque_real = configuracion_filesystem->cant_bloques_swap+bloqueFAT;
	int tamanio_archivo = configuracion_filesystem->cant_bloques_total*configuracion_filesystem->tam_bloques;
	int byte_inicial = bloque_real * configuracion_filesystem->tam_bloques;
	FILE *archivo_bloques= fopen(configuracion_filesystem->path_bloques,"rb+");
	fseek(archivo_bloques,byte_inicial,SEEK_SET);
	fwrite(informacion,configuracion_filesystem->tam_bloques,1,archivo_bloques);
	fclose(archivo_bloques);
}


void leer_bloque(uint32_t bloqueFAT){
	uint32_t bloque_real = configuracion_filesystem->cant_bloques_swap+bloqueFAT;
	int tamanio_archivo = configuracion_filesystem->cant_bloques_total*configuracion_filesystem->tam_bloques;
	int byte_a_leer = bloque_real * configuracion_filesystem->tam_bloques;
	char *informacion = malloc(configuracion_filesystem->tam_bloques);
	FILE *archivo_bloques= fopen(configuracion_filesystem->path_bloques,"rb+");

	fseek(archivo_bloques,byte_a_leer,SEEK_SET);
	fread(informacion,1,configuracion_filesystem->tam_bloques,archivo_bloques);
	log_debug(logger,"leo la siguiente info: %s", informacion);
	fclose(archivo_bloques);
	solicitar_escribir_memoria(informacion);

	/*int fdab = open(configuracion_filesystem->path_bloques, O_RDWR);
	char *bloque = mmap(NULL,tamanio_archivo,PROT_READ | PROT_WRITE, MAP_SHARED ,fdab, 0);
	char *informacion = bloque[bloque_real*configuracion_filesystem->tam_bloques];
	*/
	// ENVIAR A MEMORIA DESPUES
}

int abrirarchivo(char* nombre_archivo){
	log_info(logger,"Abrir Archivo: %s",nombre_archivo);
	FCB* fcb = buscar_archivo(nombre_archivo); 
	if (fcb)
	{
		log_debug(logger, "Existe el archivo: %s",nombre_archivo);
		return fcb->tamanio_archivo;
	}
	else
	{
		return -1;
	}	
}

FCB* buscar_archivo(char* nombre_archivo){
	for (int i = 0; i < list_size(archivos); i++)
	{
		FCB* fcb = list_get(archivos,i);
		if (fcb!=NULL && fcb->nombre_archivo!=NULL && strcmp(fcb->nombre_archivo,nombre_archivo)==0 )
		{
			return fcb;
		}
	}
	return NULL;
}

void solicitar_escribir_memoria(char *informacion){
	t_paquete paquete;
}

uint32_t buscar_bloque_fat(int nro_bloque,char* nombre_archivo){
	FCB* fcb = buscar_archivo(nombre_archivo); 
	uint32_t bloque_a_leer = fcb->bloque_inicial;
	uint32_t tamanio_FAT = (configuracion_filesystem->cant_bloques_total-configuracion_filesystem->cant_bloques_swap)*sizeof(uint32_t);
	int fatfd = open(configuracion_filesystem->path_fat,O_RDWR);
	uint32_t *entrada_fat = mmap(NULL,tamanio_FAT,PROT_READ | PROT_WRITE,MAP_SHARED,fatfd,0);
	for (uint32_t i = 0; i < nro_bloque; i++)
	{
		log_info(logger, "Acceso FAT - Entrada: <%s> - Valor: <%d>",i,entrada_fat[bloque_a_leer]);
		bloque_a_leer = entrada_fat[bloque_a_leer];
	}
	return bloque_a_leer;

}