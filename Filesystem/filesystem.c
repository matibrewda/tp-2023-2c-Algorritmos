#include "filesystem.h"

pthread_t hilo_memoria;
pthread_t hilo_kernel;

t_log *logger = NULL;
t_argumentos_filesystem *argumentos_filesystem = NULL;
t_config_filesystem *configuracion_filesystem = NULL;

int socket_kernel = -1;
int conexion_con_kernel = -1;
int conexion_con_memoria = -1;

// FCBs de archivos que fueron abiertos
t_list *fcbs;

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	atexit(terminar_filesystem);

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

	fcbs = list_create();
	inicializar_archivo_de_bloques();
	inicializar_fat();

	pthread_create(&hilo_kernel, NULL, comunicacion_kernel, NULL);
	pthread_create(&hilo_memoria, NULL, comunicacion_memoria, NULL);
	pthread_join(hilo_kernel, NULL);
	pthread_join(hilo_memoria, NULL);

	terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
	return EXIT_SUCCESS;
}

void terminar_filesystem()
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
			char *nombre_archivo_abrir = leer_paquete_solicitud_abrir_archivo_fs(logger, conexion_con_kernel);
			log_info(logger, "Abrir Archivo: %s", nombre_archivo_abrir);
			int tamanio_archivo = abrir_archivo_fs(nombre_archivo_abrir);
			t_paquete *respuesta_abrir_archivo;
			if (tamanio_archivo < 0)
			{
				respuesta_abrir_archivo = crear_paquete_respuesta_abrir_archivo_fs(logger, false, 0);
			}
			else
			{
				respuesta_abrir_archivo = crear_paquete_respuesta_abrir_archivo_fs(logger, true, tamanio_archivo);
			}
			enviar_paquete(logger, conexion_con_kernel, respuesta_abrir_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
			break;

		case SOLICITUD_CREAR_ARCHIVO_FS:
			char *nombre_crear = leer_paquete_solicitud_crear_archivo_fs(logger, conexion_con_kernel);
			log_info(logger, "Crear Archivo: %s", nombre_archivo_abrir);
			crear_archivo_fs(nombre_crear);
			t_paquete *respuesta_crear_archivo = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_CREAR_ARCHIVO_FS, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
			enviar_paquete(logger, conexion_con_kernel, respuesta_crear_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
			break;

		case SOLICITUD_LEER_ARCHIVO_FS:
			char **nombre_leer;
			int *puntero_lectura;
			int *direccion_fisica_a_escribir;
			leer_paquete_solicitud_leer_archivo_fs(logger, conexion_con_kernel, nombre_leer, puntero_lectura, direccion_fisica_a_escribir);
			log_debug(logger, "entramos a f_read");
			int nro_bloque = *puntero_lectura / configuracion_filesystem->tam_bloques;
			uint32_t bloque_fat_leer = buscar_bloque_fat(nro_bloque, *nombre_leer);
			leer_bloque(bloque_fat_leer);
			break;
		case SOLICITUD_ESCRIBIR_ARCHIVO_FS:
			log_debug(logger, "entramos a f_write");
			char **nombre_escribir;
			int *puntero_archivo_a_escribir;
			int *direccion_fisica_a_leer;
			char *informacion;
			leer_paquete_solicitud_escribir_archivo_fs(logger, conexion_con_kernel, nombre_escribir, puntero_archivo_a_escribir, direccion_fisica_a_leer);
			int nro_bloque_esc = *puntero_lectura / configuracion_filesystem->tam_bloques;
			uint32_t bloque_fat = buscar_bloque_fat(nro_bloque_esc, *nombre_leer);
			escribir_bloque(bloque_fat, informacion);

			break;

		case SOLICITUD_TRUNCAR_ARCHIVO_FS:

			char **nombre_truncar;
			int *nuevo_tamanio_archivo;
			leer_paquete_solicitud_truncar_archivo_fs(logger, conexion_con_kernel, nombre_truncar, nuevo_tamanio_archivo);
			log_debug(logger, "entro a f_truncate");
			truncar_archivo(*nombre_truncar, *nuevo_tamanio_archivo);
			break;

		default:
			break;
		}
	}
}

void inicializar_fat()
{
	log_debug(logger, "Inicializando archivo de tabla FAT");

	// Creo archivo para tabla FAT con su correspondiente tamaño
	FILE *archivo_tabla_fat = fopen(configuracion_filesystem->path_fat, "rb");
	bool existia_archivo_tabla_fat = archivo_tabla_fat != NULL;
	if (existia_archivo_tabla_fat)
	{
		log_debug(logger, "Tabla FAT ya existia, no se escribiran 0s");
		fclose(archivo_tabla_fat);
		return;
	}

	log_debug(logger, "Tabla FAT NO existia, se escribiran 0s en TODA la tabla FAT");
	archivo_tabla_fat = fopen(configuracion_filesystem->path_fat, "wb");
	int cantidad_de_entradas_tabla_fat = configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap;

	// Inicializar tabla FAT con 0s
	uint32_t byte_en_cero = 0;
	for (uint32_t i = 0; i < cantidad_de_entradas_tabla_fat; i++)
	{
		fwrite(&byte_en_cero, sizeof(uint32_t), 1, archivo_tabla_fat);
	}

	fclose(archivo_tabla_fat);
	abrir_permisos_archivo(configuracion_filesystem->path_fat);
}

void inicializar_archivo_de_bloques()
{
	log_debug(logger, "Inicializando archivo de bloques");

	// Creo archivo para bloques con su correspondiente tamaño
	FILE *archivo_bloques = fopen(configuracion_filesystem->path_bloques, "ab+");
	int archivo_bloques_file_descriptor = fileno(archivo_bloques);
	int tamanio_bloques = configuracion_filesystem->cant_bloques_total * configuracion_filesystem->tam_bloques;
	ftruncate(archivo_bloques_file_descriptor, tamanio_bloques);

	fclose(archivo_bloques);
	abrir_permisos_archivo(configuracion_filesystem->path_bloques);
}

int abrir_archivo_fs(char *nombre_archivo)
{
	char path_fcb[200];
	sprintf(path_fcb, "%s/%s.fcb", configuracion_filesystem->path_fcb, nombre_archivo);
	FCB *fcb = abrir_fcb(path_fcb);
	if (fcb == NULL)
	{
		// No existe el archivo
		log_debug(logger, "No existe el archivo %s (ruta %s)", nombre_archivo, path_fcb);
		return -1;
	}
	else
	{
		// Existe el archivo
		log_debug(logger, "Existe el archivo: %s", nombre_archivo);

		// Agregar a la lista solo si no estaba antes
		bool _filtro_fcb_por_nombre_archivo(FCB * fcb)
		{
			return strcmp(fcb->nombre_archivo, nombre_archivo) == 0;
		};

		FCB *fcb_existente = list_find(fcbs, (void *)_filtro_fcb_por_nombre_archivo);
		if (fcb_existente == NULL)
		{
			list_add(fcbs, fcb);
		}

		return fcb->tamanio_archivo;
	}
}

void crear_archivo_fs(char *nombre_archivo)
{
	// Crear un archivo FCB (en ejecucion) con tamaño 0 y sin bloque inicial. Coloco bloque inicial -1 para indicar que inicia SIN bloque inicial.
	FCB *fcb_archivo_nuevo = iniciar_fcb(nombre_archivo, 0, -1);
	list_add(fcbs, fcb_archivo_nuevo);

	char path_fcb[200];
	sprintf(path_fcb, "%s/%s.fcb", configuracion_filesystem->path_fcb, nombre_archivo);

	FILE *archivo_fcb = fopen(path_fcb, "w+");
	fprintf(archivo_fcb, "NOMBRE_ARCHIVO=%s\n", fcb_archivo_nuevo->nombre_archivo);
	fprintf(archivo_fcb, "TAMANIO_ARCHIVO=%d\n", fcb_archivo_nuevo->tamanio_archivo);
	fprintf(archivo_fcb, "BLOQUE_INICIAL=%d\n", fcb_archivo_nuevo->bloque_inicial);
	fclose(archivo_fcb);
	abrir_permisos_archivo(path_fcb);
}

void abrir_permisos_archivo(char *path_archivo)
{
	char modo_todos_los_permisos[] = "0777";
	chmod(path_archivo, strtol(modo_todos_los_permisos, 0, 8));
}

/* void ampliar_tamano_archivo(FCB *fcb , t_config *config,int nuevo_tamano) {
	fcb->tamanio_archivo = nuevo_tamano;
	char nuevo_tamano_str[12]; // Suficientemente grande para un uint32_t
	sprintf(nuevo_tamano_str, "%u", fcb->tamanio_archivo);
	config_set_value(config, "TAMANIO_ARCHIVO", nuevo_tamano_str);
} */

void ampliar_tamano_archivo(FCB *fcb, int nuevo_tamano)
{
	// Actualizar tamaño en la estructura FCB
	int bloques_a_agregar = (nuevo_tamano - (fcb->tamanio_archivo)) / configuracion_filesystem->tam_bloques;
	uint32_t tamanio_FAT = (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap) * sizeof(uint32_t);
	uint32_t bloque_a_leer = fcb->bloque_inicial;
	int fatfd = open(configuracion_filesystem->path_fat, O_RDWR);
	uint32_t *entrada_fat = mmap(NULL, tamanio_FAT, PROT_READ | PROT_WRITE, MAP_SHARED, fatfd, 0);

	// busco el ultimo bloque del archivo
	while (entrada_fat[bloque_a_leer] != UINT32_MAX)
	{
		bloque_a_leer = entrada_fat[bloque_a_leer];
	}

	uint32_t ultimo_bloque = bloque_a_leer;
	bloque_a_leer = 1;

	while (bloques_a_agregar != 0)
	{
		if (entrada_fat[bloque_a_leer] == 0)
		{
			entrada_fat[ultimo_bloque] = bloque_a_leer;
			ultimo_bloque = bloque_a_leer;
			bloques_a_agregar--;
		}
		bloque_a_leer++;
	}
	entrada_fat[ultimo_bloque] = UINT32_MAX;
	munmap(entrada_fat, tamanio_FAT);
	close(fatfd);
	fcb->tamanio_archivo = nuevo_tamano;
	// Actualizar tamaño en la configuración
}

void reducir_tamano_archivo(FCB *fcb, int nuevo_tamano)
{
	int bloques_a_quitar = ((fcb->tamanio_archivo) - nuevo_tamano) / configuracion_filesystem->tam_bloques;
	fcb->tamanio_archivo = nuevo_tamano;
	uint32_t tamanio_FAT = (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap) * sizeof(uint32_t);
	uint32_t bloque_a_leer = fcb->bloque_inicial;
	int fatfd = open(configuracion_filesystem->path_fat, O_RDWR);
	uint32_t *entrada_fat = mmap(NULL, tamanio_FAT, PROT_READ | PROT_WRITE, MAP_SHARED, fatfd, 0);

	// busco el ultimo bloque del archivo
	while (entrada_fat[bloque_a_leer] != UINT32_MAX)
	{
		bloque_a_leer = entrada_fat[bloque_a_leer];
	}
	uint32_t ultimo_bloque = bloque_a_leer;
	while (bloques_a_quitar != 0)
	{
		bloque_a_leer = 1;
		while (entrada_fat[bloque_a_leer] != ultimo_bloque)
			;
		{
			bloque_a_leer++;
		}
		entrada_fat[ultimo_bloque] = 0; // Libero el ultimo bloque
		ultimo_bloque = bloque_a_leer;
		bloques_a_quitar--;
	}
	entrada_fat[ultimo_bloque] = UINT32_MAX;
	munmap(entrada_fat, tamanio_FAT);
	close(fatfd);
}

void truncar_archivo(char *nombre, int nuevo_tamano)
{
	// FCB *fcb = buscar_archivo(nombre);
	FCB *fcb;
	t_config *config;
	log_debug(logger, "Truncar Archivo: Iniciado.");
	// BUSCAR RUTA DEL ARCHIVO COMLETAR!!!
	// Agarro la ruta del fcb existente y el nuevo tamaño para truncarlo.
	if (fcb == NULL)
	{
		log_debug(logger, "Truncar Archivo: No existe el archivo: <%s>.", nombre);
		return;
	}
	if (nuevo_tamano > fcb->tamanio_archivo)
	{
		log_info(logger, "Truncar Archivo: <%s> - Tamaño: <%d>", fcb->nombre_archivo, nuevo_tamano);
		// Ampliar el tamaño del archivo
		ampliar_tamano_archivo(fcb, nuevo_tamano);
		return;
	}
	else if (nuevo_tamano < fcb->tamanio_archivo)
	{
		log_info(logger, "Truncar Archivo: <%s> - Tamaño: <%d>", fcb->nombre_archivo, nuevo_tamano);
		// Reducir el tamaño del archivo
		reducir_tamano_archivo(fcb, nuevo_tamano);
		return;
	}
	// Si el nuevo tamaño es igual al actual, no se requiere acción.
	return;
}

void escribir_bloque(uint32_t bloqueFAT, char *informacion)
{
	uint32_t bloque_real = configuracion_filesystem->cant_bloques_swap + bloqueFAT;
	int tamanio_archivo = configuracion_filesystem->cant_bloques_total * configuracion_filesystem->tam_bloques;
	int byte_inicial = bloque_real * configuracion_filesystem->tam_bloques;
	FILE *archivo_bloques = fopen(configuracion_filesystem->path_bloques, "rb+");
	fseek(archivo_bloques, byte_inicial, SEEK_SET);
	fwrite(informacion, configuracion_filesystem->tam_bloques, 1, archivo_bloques);
	fclose(archivo_bloques);
}

void leer_bloque(uint32_t bloqueFAT)
{
	uint32_t bloque_real = configuracion_filesystem->cant_bloques_swap + bloqueFAT;
	int tamanio_archivo = configuracion_filesystem->cant_bloques_total * configuracion_filesystem->tam_bloques;
	int byte_a_leer = bloque_real * configuracion_filesystem->tam_bloques;
	char *informacion = malloc(configuracion_filesystem->tam_bloques);
	FILE *archivo_bloques = fopen(configuracion_filesystem->path_bloques, "rb+");

	fseek(archivo_bloques, byte_a_leer, SEEK_SET);
	fread(informacion, 1, configuracion_filesystem->tam_bloques, archivo_bloques);
	log_debug(logger, "leo la siguiente info: %s", informacion);
	fclose(archivo_bloques);
	solicitar_escribir_memoria(informacion);

	/*int fdab = open(configuracion_filesystem->path_bloques, O_RDWR);
	char *bloque = mmap(NULL,tamanio_archivo,PROT_READ | PROT_WRITE, MAP_SHARED ,fdab, 0);
	char *informacion = bloque[bloque_real*configuracion_filesystem->tam_bloques];
	*/
	// ENVIAR A MEMORIA DESPUES
}

void solicitar_escribir_memoria(char *informacion)
{
	t_paquete paquete;
}

uint32_t buscar_bloque_fat(int nro_bloque, char *nombre_archivo)
{
	// FCB *fcb = buscar_archivo(nombre_archivo);
	FCB *fcb;
	uint32_t bloque_a_leer = fcb->bloque_inicial;
	uint32_t tamanio_FAT = (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap) * sizeof(uint32_t);
	int fatfd = open(configuracion_filesystem->path_fat, O_RDWR);
	uint32_t *entrada_fat = mmap(NULL, tamanio_FAT, PROT_READ | PROT_WRITE, MAP_SHARED, fatfd, 0);
	for (uint32_t i = 0; i < nro_bloque; i++)
	{
		log_info(logger, "Acceso FAT - Entrada: <%d> - Valor: <%d>", i, entrada_fat[bloque_a_leer]);
		bloque_a_leer = entrada_fat[bloque_a_leer];
	}
	return bloque_a_leer;
}

char *concatenarRutas(const char *rutaFat, const char *nombreFCB)
{
	// Asegúrate de tener suficiente espacio para ambas cadenas más el carácter nulo
	size_t longitudTotal = strlen(rutaFat) + strlen(nombreFCB) + 1;

	// Reserva memoria para la nueva cadena
	char *rutaCompleta = (char *)malloc(longitudTotal);

	// Verifica si la asignación de memoria fue exitosa
	if (rutaCompleta == NULL)
	{
		perror("Error al asignar memoria");
		exit(EXIT_FAILURE);
	}

	// Copia la primera cadena en la nueva cadena
	strcpy(rutaCompleta, rutaFat);

	// Concatena la segunda cadena en la nueva cadena
	strcat(rutaCompleta, nombreFCB);

	return rutaCompleta;
}