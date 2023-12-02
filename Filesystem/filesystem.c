#include "filesystem.h"
#include "commons/config.h"
#include "fcb.h"
#include "fat.h"
#include "swap.h"
#include "bloque.h"
#include "directorio.h"

// Retardo de respuesta! TODO:
// usleep((configuracion_filesystem->retardo_acceso_bloques) * 1000);
// usleep((configuracion_filesystem->retardo_acceso_fat) * 1000);

pthread_t hilo_memoria;
pthread_t hilo_kernel;

t_log *logger = NULL;
t_argumentos_filesystem *argumentos_filesystem = NULL;
t_config_filesystem *configuracion_filesystem = NULL;

int socket_kernel = -1;
int conexion_con_kernel = -1;
int conexion_con_memoria = -1;

int32_t abrirArchivo(char *pathFCBFolder, char *nombreDeArchivo);

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

int main(int cantidad_argumentos_recibidos, char **argumentos)
{
	atexit(terminar_filesystem);

	// Inicializacion
	logger = crear_logger(RUTA_ARCHIVO_DE_LOGS, NOMBRE_MODULO_FILESYSTEM, LOG_LEVEL);
	if (logger == NULL)
	{
		terminar_filesystem();
		return EXIT_FAILURE;
	}

	log_debug(logger, "Inicializando %s", NOMBRE_MODULO_FILESYSTEM);

	argumentos_filesystem = leer_argumentos(logger, cantidad_argumentos_recibidos, argumentos);
	if (argumentos_filesystem == NULL)
	{
		terminar_filesystem();
		return EXIT_FAILURE;
	}

	configuracion_filesystem = leer_configuracion(logger, argumentos_filesystem->ruta_archivo_configuracion);

	if (configuracion_filesystem == NULL)
	{
		terminar_filesystem();
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
		terminar_filesystem();
		return EXIT_FAILURE;
	}

	crear_archivo(configuracion_filesystem->path_fcb, "EstoEsUnNuevoArchivo");

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

	int fat_checker = iniciarFAT(logger, configuracion_filesystem->path_fat, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->cant_bloques_swap, configuracion_filesystem->tam_bloques);
	
	/*
	if (fat_checker == 0)
	{
		log_debug(logger, "FS FAT: Tabla FAT iniciada correctamente.\n");
	}
	else
		log_debug(logger, "FS FAT: No fue posible iniciar la tabla FAT.\n");

	// DIRECTORIO: Es una tabla con una entrada por archivo. Cada entrada tiene:
	// Nombre: El nombre de archivo, utilizado como ID de archivo.
	// Bloque inicial: El bloque logico inicial, se utilizará en la tabla FAT para buscar su bloque siguiente.

	DirectorioArray directorio = inicializarDirectorioArray(logger);
	procesarArchivosEnDirectorio(&directorio, configuracion_filesystem->path_fcb);

	// Utilizar el array de directorios como desees
	for (size_t i = 0; i < directorio.cantidadDirectorios; i++)
	{
		printf("Nombre: %s, Bloque Inicial: %u\n", directorio.directorios[i].nombreArchivo, directorio.directorios[i].numBloqueInicial);
	}

	// Liberar la memoria utilizada por el array de directorios, al final, al terminar filesystem.

	// prueba asignar bloques
	FATEntry *fat = abrirFAT(configuracion_filesystem->path_fat, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->cant_bloques_swap);

	// Prueba del contenido de cada ENTRADA FAT

	printf("Mostrando tabla FAT inicial\n"); // Luego borrar esta linea!
	for (size_t i = 0; i < (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap); ++i)
	{
		printf("Entrada FAT %zu: %d\n", i, fat[i].block_value);
	}

	char *ruta = concatenarRutas(configuracion_filesystem->path_fcb, "Notas1erParcialK9999.fcb");
	BLOQUE **bloques = leerBloquesDesdeArchivo(configuracion_filesystem->path_bloques, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->tam_bloques);

	printf("Mostrando tabla BLOQUES inicial\n"); // Luego borrar esta linea!
	for (size_t i = 0; i < configuracion_filesystem->cant_bloques_total; ++i)
	{
		printf("Entrada BLOQUES %zu: %s\n", i, bloques[i]->valorDeBloque);
	}

	// Prueba de CREAR ARCHIVO
	// Caso 1) No existe:
	abrirArchivo(configuracion_filesystem->path_fcb, "ArchivoPepito");
	// Caso 2) Existe:
	abrirArchivo(configuracion_filesystem->path_fcb, "estadisticas");

	// modificarBloque (configuracion_filesystem->path_bloques,configuracion_filesystem->tam_bloques);
	asignarBloques(configuracion_filesystem->path_fat, "/home/utnso/tp-2023-2c-Algorritmos/Filesystem/BlocksFile/ARCHIVO_BLOQUES.bin", ruta, bloques, fat, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->cant_bloques_swap, configuracion_filesystem->tam_bloques);

	printf("Mostrando tabla FAT post modificacion\n"); // Luego borrar esta linea!

	for (size_t i = 0; i < (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap); ++i)
	{
		printf("Entrada FAT %zu: %d\n", i, fat[i].block_value);
	}

	printf("Mostrando tabla BLOQUES post modificacion\n"); // Luego borrar esta linea!
	for (size_t i = 0; i < configuracion_filesystem->cant_bloques_total; ++i)
	{
		printf("Entrada BLOQUES %zu: %s\n", i, bloques[i]->valorDeBloque);
	}

	eliminarBloques(configuracion_filesystem->path_fat, ruta, configuracion_filesystem->path_bloques, fat, bloques, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->cant_bloques_swap, configuracion_filesystem->tam_bloques);

	printf("Mostrando tabla FAT post eliminacion de bloques\n");
	for (size_t i = 0; i < (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap); ++i)
	{
		printf("Entrada FAT %zu: %d\n", i, fat[i].block_value);
	}

	printf("Mostrando tabla BLOQUES post eliminacion de bloques\n");
	for (size_t i = 0; i < configuracion_filesystem->cant_bloques_total; ++i)
	{
		printf("Entrada BLOQUES %zu: %s\n", i, bloques[i]->valorDeBloque);
	}

	// liberar memoria!
	// PRUEBA DE TRUNCAR truncar_archivo

	asignarBloques(configuracion_filesystem->path_fat, "/home/utnso/tp-2023-2c-Algorritmos/Filesystem/BlocksFile/ARCHIVO_BLOQUES.bin", "/home/utnso/tp-2023-2c-Algorritmos/Filesystem/Fcbs/estadisticas.fcb", bloques, fat, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->cant_bloques_swap, configuracion_filesystem->tam_bloques);

	truncar_archivo("/home/utnso/tp-2023-2c-Algorritmos/Filesystem/Fcbs/estadisticas.fcb", 2, configuracion_filesystem, fat, bloques);

	printf("Mostrando tabla FAT post truncamiento\n");
	for (size_t i = 0; i < (configuracion_filesystem->cant_bloques_total - configuracion_filesystem->cant_bloques_swap); ++i)
	{
		printf("Entrada FAT %zu: %d\n", i, fat[i].block_value);
	}

	printf("Mostrando tabla BLOQUES post truncamiento\n");
	for (size_t i = 0; i < configuracion_filesystem->cant_bloques_total; ++i)
	{
		printf("Entrada BLOQUES %zu: %s\n", i, bloques[i]->valorDeBloque);
	}
*/
	socket_kernel = crear_socket_servidor(logger, configuracion_filesystem->puerto_escucha_kernel, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
	if (socket_kernel == -1)
	{
		terminar_filesystem();
		return EXIT_FAILURE;
	}

	conexion_con_memoria = crear_socket_cliente(logger, configuracion_filesystem->ip_memoria, configuracion_filesystem->puerto_memoria, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);
	if (conexion_con_memoria == -1)
	{
		terminar_filesystem();
		return EXIT_FAILURE;
	}

	conexion_con_kernel = esperar_conexion_de_cliente(logger, socket_kernel, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
	if (conexion_con_kernel == -1)
	{
		terminar_filesystem();
		return EXIT_FAILURE;
	}

	pthread_create(&hilo_kernel, NULL, comunicacion_kernel, NULL);
	pthread_create(&hilo_memoria, NULL, comunicacion_memoria, NULL);

	pthread_join(hilo_kernel, NULL);
	pthread_join(hilo_memoria, NULL);

	// Finalizacion
	terminar_filesystem(logger, argumentos_filesystem, configuracion_filesystem, socket_kernel, conexion_con_kernel, conexion_con_memoria);
	//liberarDirectorioArray(&directorio);
	//cerrarFAT(fat);
	return EXIT_SUCCESS;
}

void terminar_filesystem()
{
	if (logger != NULL)
	{
		log_warning(logger, "Algo salio mal!");
		log_warning(logger, "Finalizando %s", NOMBRE_MODULO_FILESYSTEM);
	}

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

		if (operacion_recibida_memoria == SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM)
		{
			log_trace(logger, "Se recibio una orden de %s.", NOMBRE_MODULO_MEMORIA);

			int cantidad_de_bloques_a_reservar = leer_paquete_solicitud_pedir_bloques_a_fs(logger, conexion_con_memoria);

			// TO DO (LUCIANO)
			// t_list* lista_bloques_libres
			// SI NO ENCUENTRO LA CANTIDAD DE BLOQUES LIBRES QUE ME PIDIERON, devuelvo lista vacia

			// t_paquete *paquete_respuesta_pedir_bloques_a_fs = crear_paquete_respuesta_pedir_bloques_a_fs(logger, lista_bloques); // TODO: (JULI)
			//  enviar_paquete(logger, conexion_con_memoria, paquete_respuesta_pedir_bloques_a_fs, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA); // TODO: (JULI)
		}
		else if (operacion_recibida_memoria == SOLICITUD_LIBERAR_BLOQUES_EN_FILESYSTEM)
		{
			log_trace(logger, "Se recibio una orden de %s", NOMBRE_MODULO_MEMORIA);

			// t_list* bloques_a_liberar = leer_paquete_solicitud_liberar_bloques_de_fs(logger, conexion_con_memoria); // TODO: (JULI)

			// TO DO (LUCIANO)
			// Iterar lista y marcar cada bloque como libre

			t_paquete *paquete = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_LIBERAR_BLOQUES_EN_FILESYSTEM, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);
			enviar_paquete(logger, conexion_con_memoria, paquete, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);
		}
		else if (operacion_recibida_memoria == RESPUESTA_ESCRIBIR_VALOR_EN_MEMORIA)
		{
			t_paquete *paquete_respuesta_leer_archivo = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_LEER_ARCHIVO_FS, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
			enviar_paquete(logger, conexion_con_kernel, paquete_respuesta_leer_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
		}
		else if (operacion_recibida_memoria == RESPUESTA_LEER_VALOR_EN_MEMORIA_DESDE_FILESYSTEM)
		{
			char *nombre_archivo_a_escribir;
			int puntero_archivo_a_escribir;
			u_int32_t valor_a_escribir;

			leer_paquete_solicitud_escribir_archivo_fs(logger, conexion_con_kernel, &nombre_archivo_a_escribir, &puntero_archivo_a_escribir, &valor_a_escribir);

			// TO DO: escribir byte en filesystem donde apunte "puntero_archivo_a_escribir" (LUCIANO)

			t_paquete *paquete_respuesta_escribir_archivo = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_ESCRIBIR_ARCHIVO_FS, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
			enviar_paquete(logger, conexion_con_kernel, paquete_respuesta_escribir_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
		}
		else
		{
			log_trace(logger, "Se recibio una orden desconocida de %s.", NOMBRE_MODULO_KERNEL);
		}
	}
}

void *comunicacion_kernel()
{
	while (true)
	{
		int operacion_recibida_kernel = esperar_operacion(logger, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL, conexion_con_kernel);
		log_debug(logger, "se recibio la operacion %s de %s", nombre_opcode(operacion_recibida_kernel), NOMBRE_MODULO_KERNEL);

		if (operacion_recibida_kernel == SOLICITUD_ABRIR_ARCHIVO_FS)
		{
			log_trace(logger, "Se recibio una orden de %s para abrir un archivo.", NOMBRE_MODULO_KERNEL);

			char *nombre_archivo_a_abrir = leer_paquete_solicitud_abrir_archivo_fs(logger, conexion_con_kernel);

			bool existe_archivo = false; // TODO: que contenga info posta (LUCIANO)
			int tamanio_archivo = 32;	 // TODO: que contenga info posta: si existe, tamanio, si no existe, mandame un -1 (LUCIANO)

			t_paquete *paquete_respuesta_abrir_archivo_fs = crear_paquete_respuesta_abrir_archivo_fs(logger, existe_archivo, tamanio_archivo);
			enviar_paquete(logger, conexion_con_kernel, paquete_respuesta_abrir_archivo_fs, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
		}
		else if (operacion_recibida_kernel == SOLICITUD_CREAR_ARCHIVO_FS)
		{
			log_trace(logger, "Se recibio una orden de %s para crear un archivo.", NOMBRE_MODULO_KERNEL);

			char *nombre_archivo_a_crear = leer_paquete_solicitud_crear_archivo_fs(logger, conexion_con_kernel);

			// TO DO: crear FCB para archivo de "nombre_archivo" (LUCIANO)

			t_paquete *paquete_respuesta_crear_archivo = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_CREAR_ARCHIVO_FS, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
			enviar_paquete(logger, conexion_con_kernel, paquete_respuesta_crear_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
		}
		else if (operacion_recibida_kernel == SOLICITUD_TRUNCAR_ARCHIVO_FS)
		{
			log_trace(logger, "Se recibio una orden de %s para truncar un archivo.", NOMBRE_MODULO_KERNEL);

			char *nombre_archivo_a_crear;
			int nuevo_tamanio_archivo;

			leer_paquete_solicitud_truncar_archivo_fs(logger, conexion_con_kernel, &nombre_archivo_a_crear, &nuevo_tamanio_archivo);

			// TO DO: crear FCB para archivo de "nombre_archivo" (LUCIANO)

			t_paquete *paquete_respuesta_truncar_archivo = crear_paquete_con_opcode_y_sin_contenido(logger, RESPUESTA_TRUNCAR_ARCHIVO_FS, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
			enviar_paquete(logger, conexion_con_kernel, paquete_respuesta_truncar_archivo, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_KERNEL);
		}
		else if (operacion_recibida_kernel == SOLICITUD_LEER_ARCHIVO_FS)
		{
			log_trace(logger, "Se recibio una orden de %s para leer un archivo.", NOMBRE_MODULO_KERNEL);

			char *nombre_archivo_a_leer;
			int puntero_archivo_a_leer;
			int direccion_fisica_a_escribir;

			leer_paquete_solicitud_leer_archivo_fs(logger, conexion_con_kernel, &nombre_archivo_a_leer, &puntero_archivo_a_leer, &direccion_fisica_a_escribir);

			// TO DO: leer byte de filesystem donde apunta "puntero_archivo_a_leer" (LUCIANO)

			t_pedido_escribir_valor_en_memoria *pedido_escribir_valor_en_memoria = malloc(sizeof(t_pedido_escribir_valor_en_memoria));
			pedido_escribir_valor_en_memoria->valor_a_escribir = 3; // TODO LUCIANO
			pedido_escribir_valor_en_memoria->direccion_fisica = direccion_fisica_a_escribir;
			t_paquete *paquete_solicitud_escribir_valor_en_memoria = crear_paquete_solicitud_escribir_valor_en_memoria(logger, pedido_escribir_valor_en_memoria, NOMBRE_MODULO_FILESYSTEM);
			enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_escribir_valor_en_memoria, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);
		}
		else if (operacion_recibida_kernel == SOLICITUD_ESCRIBIR_ARCHIVO_FS)
		{
			log_trace(logger, "Se recibio una orden de %s para escribir un archivo.", NOMBRE_MODULO_KERNEL);

			char *nombre_archivo_a_escribir;
			int puntero_archivo_a_escribir;
			int direccion_fisica_a_leer;

			leer_paquete_solicitud_escribir_archivo_fs(logger, conexion_con_kernel, &nombre_archivo_a_escribir, &puntero_archivo_a_escribir, &direccion_fisica_a_leer);

			t_paquete *paquete_solicitud_leer_valor_en_memoria_desde_filesystem = crear_paquete_solicitud_leer_valor_en_memoria_desde_filesystem(logger, nombre_archivo_a_escribir, puntero_archivo_a_escribir, direccion_fisica_a_leer);
			enviar_paquete(logger, conexion_con_memoria, paquete_solicitud_leer_valor_en_memoria_desde_filesystem, NOMBRE_MODULO_FILESYSTEM, NOMBRE_MODULO_MEMORIA);
		}
		else
		{
			log_trace(logger, "Se recibio una orden desconocida de %s.", NOMBRE_MODULO_KERNEL);
		}
	}
}

int32_t crear_archivo(char *path, char *nombreNuevoArchivo)
{
	char nombreFormateado[256]; // Asegúrate de que este array es lo suficientemente grande
	sprintf(nombreFormateado, "%s.fcb", nombreNuevoArchivo);
	// Crear un archivo FCB (en ejecucion) con tamaño 0 y sin bloque inicial. Coloco bloque inicial -1 para indicar que inicia SIN bloque inicial.
	FCB *fcbArchivoNuevo = iniciar_fcb(nombreNuevoArchivo, 0, UINT32_MAX);

	if (fcbArchivoNuevo != NULL)
	{
		log_debug(logger, "FS: EStructura FCB de nuevo archivo creado correctamente.");
		// Guardar el fcb creado en un archivo .fcb en el disco.

		char rutaCompleta[256]; // Tamaño suficientemente grande para contener la ruta completa

		// Concatenar la ruta base con el nombre del nuevo archivo
		snprintf(rutaCompleta, sizeof(rutaCompleta), "%s%s", path, nombreFormateado);

		guardar_fcb_en_archivo(fcbArchivoNuevo, rutaCompleta);
		// Liberar la memoria no utilizada.
		log_debug(logger, "FS: Archivo FCB, del nuevo archivo, creado correctamente.");

		// LOG MINIMO PEDIDO POR TP:
		log_info(logger, "Crear Archivo: Crear Archivo: %s", nombreNuevoArchivo);
		free(fcbArchivoNuevo);
	}

	// Siempre será posible crear un archivo y por lo tanto esta operación deberá devolver OK.
	return 0;
}

int32_t abrirArchivo(char *pathFCBFolder, char *nombreDeArchivo)
{
	// Concatenar pathFCBFolder, nombreDeArchivo y la extensión ".fcb"
	char rutaCompleta[256]; // Asegúrate de que este array sea lo suficientemente grande
	snprintf(rutaCompleta, sizeof(rutaCompleta), "%s%s.fcb", pathFCBFolder, nombreDeArchivo);
	int32_t checker = verificarSiExisteFCBdeArchivo(rutaCompleta);

	if (checker == 0)
	{
		FCB *fcb = crear_fcb(rutaCompleta);
		printf("tamanio_archivo: %i", fcb->tamanio_archivo);
		return fcb->tamanio_archivo;

		log_info(logger, "Abrir Archivo: <%s>", fcb->nombre_archivo);
	}
	else
	{
		log_info(logger, "Abrir Archivo: NO EXISTE <%s>", nombreDeArchivo);
		printf("No existe el archivo.");
		return -1;
	}
}

int32_t truncar_archivo(char *path, uint32_t nuevo_tamano, t_config_filesystem *configuracion_filesystem, FATEntry fat[], BLOQUE *bloques[])
{

	// validar que haya espacio en FAT
	FCB *fcb = crear_fcb(path);

	printf("%d", fcb->tamanio_archivo);

	if (nuevo_tamano > fcb->tamanio_archivo)
	{
		uint32_t aSumar = nuevo_tamano - fcb->tamanio_archivo;
		sumarBloques(configuracion_filesystem->path_fat, configuracion_filesystem->path_bloques, path, fat, bloques, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->cant_bloques_swap, configuracion_filesystem->tam_bloques, aSumar);
		fcb->tamanio_archivo = nuevo_tamano;
	}

	if (fcb->tamanio_archivo > nuevo_tamano)
	{
		uint32_t aRestar = fcb->tamanio_archivo - nuevo_tamano;
		restarBloques(configuracion_filesystem->path_fat, configuracion_filesystem->path_bloques, path, fat, bloques, configuracion_filesystem->cant_bloques_total, configuracion_filesystem->cant_bloques_swap, configuracion_filesystem->tam_bloques, aRestar);
		fcb->tamanio_archivo = nuevo_tamano;
	}

	guardar_fcb_en_archivo(fcb, path);
	log_info(logger, "Truncar Archivo: <%s> - Tamaño: <%d>", fcb->nombre_archivo, nuevo_tamano);
}
