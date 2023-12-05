#include "filesystem.h"

pthread_t hilo_memoria;
pthread_t hilo_kernel;

t_log *logger = NULL;
t_argumentos_filesystem *argumentos_filesystem = NULL;
t_config_filesystem *configuracion_filesystem = NULL;

int socket_kernel = -1;
int conexion_con_kernel = -1;
int conexion_con_memoria = -1;

pthread_mutex_t mutex_archivo_bloques;

uint32_t cantidad_de_entradas_tabla_fat;
uint32_t tamanio_archivo_tabla_fat;
bool *bitmap_bloques_libres_swap;
t_list *fcbs; // Lista de FCBs de archivos que fueron abiertos

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

	pthread_mutex_init(&mutex_archivo_bloques, NULL);
	tamanio_archivo_tabla_fat = (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap) * sizeof(uint32_t);
	cantidad_de_entradas_tabla_fat = configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap;
	fcbs = list_create();
	inicializar_archivo_de_bloques();
	inicializar_fat();

	crear_archivo_fs("lucas");
	truncar_archivo_fs("lucas", 10); // Me reserva 5 bloques
	truncar_archivo_fs("lucas", 7);	 // Me reserva 4 bloques
	truncar_archivo_fs("lucas", 11); // Me reserva 5 bloques

	// socket_kernel = crear_socket_servidor(logger, configuracion_filesystem->puerto_escucha_kernel, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
	// if (socket_kernel == -1)
	// {
	// 	terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
	// 	return EXIT_FAILURE;
	// }

	// conexion_con_memoria = crear_socket_cliente(logger, configuracion_filesystem->ip_memoria, configuracion_filesystem->puerto_memoria, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);
	// if (conexion_con_memoria == -1)
	// {
	// 	terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
	// 	return EXIT_FAILURE;
	// }

	// conexion_con_kernel = esperar_conexion_de_cliente(logger, socket_kernel, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
	// if (conexion_con_kernel == -1)
	// {
	// 	terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
	// 	return EXIT_FAILURE;
	// }

	// pthread_mutex_init(&mutex_archivo_bloques, NULL);
	// tamanio_archivo_tabla_fat = (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap) * sizeof(uint32_t);
	// cantidad_de_entradas_tabla_fat = configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap;
	// fcbs = list_create();
	// inicializar_archivo_de_bloques();
	// inicializar_fat();

	// pthread_create(&hilo_kernel, NULL, comunicacion_kernel, NULL);
	// pthread_create(&hilo_memoria, NULL, comunicacion_memoria, NULL);
	// pthread_join(hilo_kernel, NULL);
	// pthread_join(hilo_memoria, NULL);

	// terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
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

		switch (operacion_recibida_memoria)
		{
		case SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM:
			int cantidad_de_bloques_a_reservar;
			int pid_pedido_bloques;
			leer_paquete_solicitud_pedir_bloques_a_fs(logger, conexion_con_memoria, &cantidad_de_bloques_a_reservar, &pid_pedido_bloques);
			t_list *bloques_reservados = reservar_bloques_en_swap(cantidad_de_bloques_a_reservar);
			bool pude_reservar_todos = bloques_reservados != NULL;
			// t_paquete* paquete_respuesta_pedirr_bloques_a_fs = crear_paquete_respuest_pedir
			list_destroy(bloques_reservados);
			break;
		case SOLICITUD_LIBERAR_BLOQUES_EN_FILESYSTEM:
			break;
		case SOLICITUD_LEER_PAGINA_EN_SWAP:

			break;
		case SOLICITUD_ESCRIBIR_PAGINA_EN_SWAP:

			break;
		default:
			log_trace(logger, "Se recibio una orden desconocida de %s en %s.", NOMBRE_MODULO_MEMORIA, NOMBRE_MODULO_FILESYSTEM);
			break;
		}
	}
}

void *comunicacion_kernel()
{
	while (true)
	{
		op_code operacion_recibida_kernel = esperar_operacion(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA, conexion_con_memoria);

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

		case SOLICITUD_TRUNCAR_ARCHIVO_FS:
			char *nombre_archivo_truncar;
			int nuevo_tamanio_archivo;
			leer_paquete_solicitud_truncar_archivo_fs(logger, conexion_con_kernel, &nombre_archivo_truncar, &nuevo_tamanio_archivo);
			log_info(logger, "Truncar Archivo: %s - Tamano: %d", nombre_archivo_truncar, nuevo_tamanio_archivo);
			truncar_archivo_fs(nombre_archivo_truncar, nuevo_tamanio_archivo);
			break;

		case SOLICITUD_LEER_ARCHIVO_FS:
			char **nombre_leer;
			int *puntero_lectura;
			int *direccion_fisica_a_escribir;
			leer_paquete_solicitud_leer_archivo_fs(logger, conexion_con_kernel, nombre_leer, puntero_lectura, direccion_fisica_a_escribir);
			break;

		case SOLICITUD_ESCRIBIR_ARCHIVO_FS:
			char **nombre_escribir;
			int *puntero_archivo_a_escribir;
			int *direccion_fisica_a_leer;
			leer_paquete_solicitud_escribir_archivo_fs(logger, conexion_con_kernel, nombre_escribir, puntero_archivo_a_escribir, direccion_fisica_a_leer);

			break;

		default:
			log_trace(logger, "Se recibio una orden desconocida de %s en %s.", NOMBRE_MODULO_KERNEL, NOMBRE_MODULO_FILESYSTEM);
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

	// Inicializar tabla FAT con 0s
	uint32_t byte_en_cero = 0;
	for (uint32_t i = 0; i < cantidad_de_entradas_tabla_fat; i++)
	{
		fwrite(&byte_en_cero, sizeof(uint32_t), 1, archivo_tabla_fat);
	}

	fclose(archivo_tabla_fat);
	dar_full_permisos_a_archivo(configuracion_filesystem->path_fat);
}

void inicializar_archivo_de_bloques()
{
	pthread_mutex_lock(&mutex_archivo_bloques);

	log_debug(logger, "Inicializando archivo de bloques");

	// Creo archivo para bloques con su correspondiente tamaño
	FILE *archivo_bloques = fopen(configuracion_filesystem->path_bloques, "wb+");
	int archivo_bloques_file_descriptor = fileno(archivo_bloques);
	int tamanio_bloques = configuracion_filesystem->cant_bloques_total * configuracion_filesystem->tam_bloques;
	ftruncate(archivo_bloques_file_descriptor, tamanio_bloques);
	fclose(archivo_bloques);

	dar_full_permisos_a_archivo(configuracion_filesystem->path_bloques);

	bitmap_bloques_libres_swap = malloc(sizeof(int) * configuracion_filesystem->cant_bloques_swap);
	for (int i = 0; i < configuracion_filesystem->cant_bloques_swap; i++)
	{
		bitmap_bloques_libres_swap[i] = true;
	}

	pthread_mutex_unlock(&mutex_archivo_bloques);
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
	crear_archivo_fcb(fcb_archivo_nuevo);
}

void crear_archivo_fcb(FCB *fcb)
{
	char path_fcb[200];
	sprintf(path_fcb, "%s/%s.fcb", configuracion_filesystem->path_fcb, fcb->nombre_archivo);

	FILE *archivo_fcb = fopen(path_fcb, "w+");
	fprintf(archivo_fcb, "NOMBRE_ARCHIVO=%s\n", fcb->nombre_archivo);
	fprintf(archivo_fcb, "TAMANIO_ARCHIVO=%d\n", fcb->tamanio_archivo);
	fprintf(archivo_fcb, "BLOQUE_INICIAL=%d\n", fcb->bloque_inicial);
	fclose(archivo_fcb);
	dar_full_permisos_a_archivo(path_fcb);
}

void dar_full_permisos_a_archivo(char *path_archivo)
{
	log_debug(logger, "Intentado otorgar todos los permisos al archivo %s", path_archivo);
	chmod(path_archivo, strtol("0777", 0, 8));
	log_debug(logger, "Otorgados todos los permisos al archivo %s", path_archivo);
}

void *leer_bloque_swap(int numero_de_bloque)
{
	pthread_mutex_lock(&mutex_archivo_bloques);
	log_info(logger, "Acceso SWAP: %d", numero_de_bloque);
	usleep((configuracion_filesystem->retardo_acceso_bloques) * 1000);
	FILE *archivo_bloques = fopen(configuracion_filesystem->path_bloques, "rb");
	fseek(archivo_bloques, numero_de_bloque * configuracion_filesystem->tam_bloques, SEEK_SET);
	void *bloque = malloc(configuracion_filesystem->tam_bloques);
	fread(bloque, configuracion_filesystem->tam_bloques, 1, archivo_bloques);
	fclose(archivo_bloques);
	pthread_mutex_unlock(&mutex_archivo_bloques);
	return bloque;
}

void escribir_bloque_swap(int numero_de_bloque, void *bloque)
{
	pthread_mutex_lock(&mutex_archivo_bloques);
	log_info(logger, "Acceso SWAP: %d", numero_de_bloque);
	usleep((configuracion_filesystem->retardo_acceso_bloques) * 1000);
	FILE *archivo_bloques = fopen(configuracion_filesystem->path_bloques, "rb+");
	fseek(archivo_bloques, numero_de_bloque * configuracion_filesystem->tam_bloques, SEEK_SET);
	fwrite(bloque, 1, configuracion_filesystem->tam_bloques, archivo_bloques);
	fclose(archivo_bloques);
	pthread_mutex_unlock(&mutex_archivo_bloques);
}

t_list *buscar_bloques_libres_en_swap(int cantidad_de_bloques)
{
	t_list *bloques_libres = list_create();

	int bloques_libres_encontrados = 0;

	for (int i = 0; i < configuracion_filesystem->cant_bloques_swap && bloques_libres_encontrados < cantidad_de_bloques; i++)
	{
		if (bitmap_bloques_libres_swap[i])
		{
			int *bloque_libre = malloc(sizeof(int));
			*bloque_libre = i;
			list_add(bloques_libres, bloque_libre);
			bloques_libres_encontrados++;
		}
	}

	if (cantidad_de_bloques != bloques_libres_encontrados)
	{
		list_destroy(bloques_libres);
		return NULL;
	}

	return bloques_libres;
}

void liberar_bloques_en_swap(t_list *numeros_de_bloques_a_liberar)
{
	t_list_iterator *iterador = list_iterator_create(numeros_de_bloques_a_liberar);

	while (list_iterator_has_next(iterador))
	{
		int *numero_de_bloque_a_liberar = list_iterator_next(iterador);
		bitmap_bloques_libres_swap[*numero_de_bloque_a_liberar] = true;
	}
}

t_list *reservar_bloques_en_swap(int cantidad_de_bloques)
{
	t_list *bloques_libres = buscar_bloques_libres_en_swap(cantidad_de_bloques);
	if (bloques_libres == NULL)
	{
		log_debug(logger, "No se encontraron %d bloques libres en SWAP", cantidad_de_bloques);
		return NULL;
	}

	// Marcar como ocupados y llenar con 0s cada bloque reservado
	t_list_iterator *iterador = list_iterator_create(bloques_libres);
	while (list_iterator_has_next(iterador))
	{
		int *numero_de_bloque_reservado = list_iterator_next(iterador);
		bitmap_bloques_libres_swap[*numero_de_bloque_reservado] = false;

		char *bloque_con_ceros = malloc(configuracion_filesystem->tam_bloques);
		for (int i = 0; i < configuracion_filesystem->tam_bloques; i++)
		{
			bloque_con_ceros[i] = '\0';
		}

		escribir_bloque_swap(*numero_de_bloque_reservado, bloque_con_ceros);
	}

	return bloques_libres;
}

void truncar_archivo_fs(char *nombre_archivo, int nuevo_tamanio)
{
	bool _filtro_fcb_por_nombre_archivo(FCB * fcb)
	{
		return strcmp(fcb->nombre_archivo, nombre_archivo) == 0;
	};

	FCB *fcb = list_find(fcbs, (void *)_filtro_fcb_por_nombre_archivo);

	if (nuevo_tamanio > fcb->tamanio_archivo)
	{
		fcb->bloque_inicial = ampliar_tamanio_archivo(fcb, nuevo_tamanio);
		fcb->tamanio_archivo = nuevo_tamanio;
		crear_archivo_fcb(fcb);
		return;
	}
	else if (nuevo_tamanio < fcb->tamanio_archivo)
	{
		fcb->bloque_inicial = reducir_tamanio_archivo(fcb, nuevo_tamanio);
		fcb->tamanio_archivo = nuevo_tamanio;
		crear_archivo_fcb(fcb);
		return;
	}

	// Si el nuevo tamaño es igual al actual, no se requiere acción.
	return;
}

int ampliar_tamanio_archivo(FCB *fcb, int nuevo_tamanio)
{
	uint32_t indice_primer_bloque = fcb->bloque_inicial;
	int cantidad_de_bloques_actual = ceil(fcb->tamanio_archivo / (double)(configuracion_filesystem->tam_bloques));
	int nueva_cantidad_de_bloques = ceil(nuevo_tamanio / (double)(configuracion_filesystem->tam_bloques));
	int bloques_a_agregar = nueva_cantidad_de_bloques - cantidad_de_bloques_actual;
	uint32_t *puntero_memoria_tabla_fat;
	FILE *puntero_archivo_tabla_fat;
	abrir_tabla_fat(&puntero_memoria_tabla_fat, &puntero_archivo_tabla_fat);

	if (indice_primer_bloque == -1)
	{
		indice_primer_bloque = buscar_bloque_libre_en_fat(puntero_memoria_tabla_fat);
		escribir_entrada_fat_por_indice(puntero_memoria_tabla_fat, EOF_FS, indice_primer_bloque);
		bloques_a_agregar--;
	}

	uint32_t indice_bloque_actual = indice_primer_bloque;

	while (leer_entrada_fat_por_indice(puntero_memoria_tabla_fat, indice_bloque_actual) != EOF_FS)
	{
		indice_bloque_actual = leer_entrada_fat_por_indice(puntero_memoria_tabla_fat, indice_bloque_actual);
	}

	while (bloques_a_agregar - 1 > 0)
	{
		uint32_t indice_bloque_libre = buscar_bloque_libre_en_fat(puntero_memoria_tabla_fat);
		escribir_entrada_fat_por_indice(puntero_memoria_tabla_fat, indice_bloque_libre, indice_bloque_actual);
		indice_bloque_actual = indice_bloque_libre;
		escribir_entrada_fat_por_indice(puntero_memoria_tabla_fat, EOF_FS, indice_bloque_actual);
		bloques_a_agregar--;
	}

	cerrar_tabla_fat(puntero_memoria_tabla_fat, puntero_archivo_tabla_fat);
	return indice_primer_bloque;
}

int reducir_tamanio_archivo(FCB *fcb, int nuevo_tamanio)
{
	uint32_t indice_primer_bloque = fcb->bloque_inicial;
	int cantidad_de_bloques_actual = ceil(fcb->tamanio_archivo / (double)(configuracion_filesystem->tam_bloques));
	int nueva_cantidad_de_bloques = ceil(nuevo_tamanio / (double)(configuracion_filesystem->tam_bloques));
	int bloques_a_quitar = cantidad_de_bloques_actual - nueva_cantidad_de_bloques;
	uint32_t *puntero_memoria_tabla_fat;
	FILE *puntero_archivo_tabla_fat;
	abrir_tabla_fat(&puntero_memoria_tabla_fat, &puntero_archivo_tabla_fat);

	uint32_t indice_bloque_actual = indice_primer_bloque;

	for (int i = 0; i < nueva_cantidad_de_bloques - 1; i++)
	{
		indice_bloque_actual = leer_entrada_fat_por_indice(puntero_memoria_tabla_fat, indice_bloque_actual);
	}

	for (int i = 0; i < bloques_a_quitar + 1; i++)
	{
		uint32_t indice_proximo_bloque = leer_entrada_fat_por_indice(puntero_memoria_tabla_fat, indice_bloque_actual);
		if (i == 0)
		{
			escribir_entrada_fat_por_indice(puntero_memoria_tabla_fat, EOF_FS, indice_bloque_actual);
		}
		else
		{
			escribir_entrada_fat_por_indice(puntero_memoria_tabla_fat, 0, indice_bloque_actual);
		}

		indice_bloque_actual = indice_proximo_bloque;
	}

	cerrar_tabla_fat(puntero_memoria_tabla_fat, puntero_archivo_tabla_fat);
	return indice_primer_bloque;
}

void abrir_tabla_fat(uint32_t **puntero_memoria_tabla_fat, FILE **puntero_archivo_tabla_fat)
{
	*puntero_archivo_tabla_fat = fopen(configuracion_filesystem->path_fat, "rb+");
	int archivo_tabla_fat_file_descriptor = fileno(*puntero_archivo_tabla_fat);
	*puntero_memoria_tabla_fat = mmap(NULL, tamanio_archivo_tabla_fat, PROT_READ | PROT_WRITE, MAP_SHARED, archivo_tabla_fat_file_descriptor, 0);
}

void cerrar_tabla_fat(uint32_t *puntero_tabla_fat, FILE *puntero_archivo_tabla_fat)
{
	munmap(puntero_tabla_fat, tamanio_archivo_tabla_fat);
	fclose(puntero_archivo_tabla_fat);
}

uint32_t buscar_bloque_libre_en_fat(uint32_t *puntero_tabla_fat)
{
	int bloque_libre = -1;

	// Arranco en i = 1 porque el bloque 0 NUNCA se puede usar (boot)
	for (int i = 1; i < cantidad_de_entradas_tabla_fat && bloque_libre == -1; i++)
	{
		if (leer_entrada_fat_por_indice(puntero_tabla_fat, i) == 0)
		{
			bloque_libre = i;
		}
	}

	return bloque_libre;
}

uint32_t leer_entrada_fat_por_indice(uint32_t *puntero_tabla_fat, uint32_t indice_fat)
{
	usleep((configuracion_filesystem->retardo_acceso_fat) * 1000);
	uint32_t entrada_fat = puntero_tabla_fat[indice_fat];
	log_info(logger, "Acceso FAT - Entrada: %d - Valor: %d (LECTURA)", indice_fat, entrada_fat);
	return entrada_fat;
}

void escribir_entrada_fat_por_indice(uint32_t *puntero_tabla_fat, uint32_t indice_a_escribir, uint32_t indice_donde_escribir)
{
	usleep((configuracion_filesystem->retardo_acceso_fat) * 1000);
	log_info(logger, "Acceso FAT - Entrada: %d - Valor: %d (ESCRITURA)", indice_donde_escribir, indice_a_escribir);
	puntero_tabla_fat[indice_donde_escribir] = indice_a_escribir;
}

FCB *iniciar_fcb(char *nombre_archivo, uint32_t tamanio_archivo, uint32_t bloque_inicial)
{
	FCB *fcb = malloc(sizeof(FCB));
	fcb->nombre_archivo = strdup(nombre_archivo); // Revisar
	fcb->tamanio_archivo = tamanio_archivo;
	fcb->bloque_inicial = bloque_inicial;
	return fcb;
}

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
	// solicitar_escribir_memoria(informacion);

	/*int fdab = open(configuracion_filesystem->path_bloques, O_RDWR);
	char *bloque = mmap(NULL,tamanio_archivo,PROT_READ | PROT_WRITE, MAP_SHARED ,fdab, 0);
	char *informacion = bloque[bloque_real*configuracion_filesystem->tam_bloques];
	*/
	// ENVIAR A MEMORIA DESPUES
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