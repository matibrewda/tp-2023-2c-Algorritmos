#include "Headers/utilidades_conexion.h"

// Crea un socket cliente e intenta establecer una conexion con el servidor: NO BLOQUEA
// Si no pudo establecer una conexion, devuelve -1 y destruye el socket.
// Posteriormente se debe destruir el socket con un close()!
// El int retornado se utilizara para enviar o recibir paquetes con el servidor al cual se conecto (CONEXION BIDIRECCIONAL).
int crear_socket_cliente(t_log *logger, char *ip_proceso_servidor, char *puerto_proceso_servidor, const char *nombre_modulo_cliente, const char *nombre_modulo_servidor)
{
	log_debug(logger, "Intentando conectar %s con %s ...", nombre_modulo_cliente, nombre_modulo_servidor);

	struct addrinfo hints;
	struct addrinfo *server_info;

	// Init de hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// "Cargamos" la variable server_info (getaddrinfo es una syscall que devuelve informacion de red de la IP "ip_proceso servidor" y el puerto "puerto_proceso_servidor")
	getaddrinfo(ip_proceso_servidor, puerto_proceso_servidor, &hints, &server_info);

	// Crea un socket (syscall)
	int socket_cliente = socket(
		server_info->ai_family,
		server_info->ai_socktype,
		server_info->ai_protocol);

	// Fallo al crear socket
	if (socket_cliente == -1)
	{
		log_error(logger, "Fallo la syscall socket() al intentar crear el socket cliente, para conectar %s con %s en el puerto %s.", nombre_modulo_cliente, nombre_modulo_servidor, puerto_proceso_servidor);
		freeaddrinfo(server_info);
		return -1;
	}

	int resultado_connect = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	// Error de conexion
	if (resultado_connect == -1)
	{
		log_error(logger, "No se pudo conectar %s con %s en %s:%s", nombre_modulo_cliente, nombre_modulo_servidor, ip_proceso_servidor, puerto_proceso_servidor);

		freeaddrinfo(server_info);
		close(socket_cliente);
		return -1;
	}

	log_debug(logger, "Se realizo una conexion entre el cliente %s y el servidor %s.", nombre_modulo_cliente, nombre_modulo_servidor);

	freeaddrinfo(server_info);

	return socket_cliente;
}

// Crea un socket para un servidor: NO BLOQUEA
// Posteriormente se debe destruir el socket con un close()!
// El int retornado se utilizara para esperar conexiones de clientes.
int crear_socket_servidor(t_log *logger, char *puerto_proceso_servidor, const char *nombre_modulo_servidor, const char *nombre_cliente_a_escuchar)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	// Init de hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// "Cargamos" la variable server_info (getaddrinfo es una syscall que devuelve informacion de red de la IP "ip_proceso servidor" y el puerto "puerto_proceso_servidor")
	getaddrinfo(NULL, puerto_proceso_servidor, &hints, &server_info);

	// Crea un socket (syscall)
	int socket_servidor = socket(
		server_info->ai_family,
		server_info->ai_socktype,
		server_info->ai_protocol);

	// Fallo al crear socket
	if (socket_servidor == -1)
	{
		log_error(logger, "Fallo la syscall socket() al intentar crear el socket servidor, para que %s reciba conexiones de %s en el puerto %s.", nombre_modulo_servidor, nombre_cliente_a_escuchar, puerto_proceso_servidor);
		freeaddrinfo(server_info);
		return -1;
	}

	// Asocia el socket anteriormente creado al puerto "puerto_proceso_servidor"
	int resultado_bind = bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);

	// Fallo en bind
	if (resultado_bind == -1)
	{
		log_error(logger, "Fallo la syscall bind() al intentar crear el socket servidor, para que %s reciba conexiones de %s en el puerto %s.", nombre_modulo_servidor, nombre_cliente_a_escuchar, puerto_proceso_servidor);
		freeaddrinfo(server_info);
		close(socket_servidor);
		return -1;
	}

	// Prepara al socket para que escuche conexiones (hasta SOMAXCONN cantidad de conexiones)
	int resultado_listen = listen(socket_servidor, SOMAXCONN);

	// Falo en listen
	if (resultado_bind == -1)
	{
		log_error(logger, "Fallo la syscall listen() al intentar crear el socket servidor, para que %s reciba conexiones de %s en el puerto %s.", nombre_modulo_servidor, nombre_cliente_a_escuchar, puerto_proceso_servidor);
		freeaddrinfo(server_info);
		close(socket_servidor);
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_servidor;
}

// Espera una conexion de un cliente en un socket servidor ya creado: BLOQUEA!
// Devuelve un socket representando la conexion entre el cliente y el servidor.
// Posteriormente se debe destruir el socket con un close()!
// El int retornado se utilizara para enviar o recibir paquetes con el servidor al cual se conecto (CONEXION BIDIRECCIONAL).
int esperar_conexion_de_cliente(t_log *logger, int socket_servidor, const char *nombre_modulo_servidor, const char *nombre_cliente_a_escuchar)
{
	log_trace(logger, "%s esta a la espera de conexiones de %s.", nombre_modulo_servidor, nombre_cliente_a_escuchar);

	int socket_conexion_con_cliente = accept(socket_servidor, (void *)NULL, NULL);

	if (socket_conexion_con_cliente == -1)
	{
		log_error(logger, "Fallo la syscall accept() al esperar conexiones entre el cliente %s y el servidor %s.", nombre_cliente_a_escuchar, nombre_modulo_servidor);
	}
	else
	{
		log_debug(logger, "Se realizo una conexion entre el cliente %s y el servidor %s.", nombre_cliente_a_escuchar, nombre_modulo_servidor);
	}

	return socket_conexion_con_cliente;
}

// NO BLOQUEA
t_paquete *crear_paquete(t_log *logger, op_code codigo_operacion)
{
	log_trace(logger, "Se creara un paquete (inicialmente vacio) con el codigo de operacion %d", (int)codigo_operacion);

	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_operacion;
	crear_buffer(paquete);

	log_trace(logger, "Se creo un paquete (inicialmente vacio) con el codigo de operacion %d", (int)codigo_operacion);

	return paquete;
}

// NO BLOQUEA
void destruir_paquete(t_log *logger, t_paquete *paquete)
{
	log_trace(logger, "Comenzando la destruccion de paquete.");

	if (paquete == NULL)
	{
		return;
	}

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	log_trace(logger, "Paquete destruido con exito.");
}

// NO BLOQUEA
bool enviar_paquete(t_log *logger, int conexion, t_paquete *paquete, const char *nombre_proceso_origen, const char *nombre_proceso_destino)
{
	log_trace(logger, "Comenzando el envio de paquete desde %s a %s.", nombre_proceso_origen, nombre_proceso_destino);

	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializar_paquete(paquete, bytes);

	ssize_t resultado_send = send(conexion, a_enviar, bytes, 0);

	if (resultado_send == -1)
	{
		log_error(logger, "Error al enviar paquete desde %s a %s.", nombre_proceso_origen, nombre_proceso_destino);
	}
	else
	{
		log_trace(logger, "Paquete enviado con exito desde %s a %s.", nombre_proceso_origen, nombre_proceso_destino);
	}

	free(a_enviar);

	return resultado_send != -1;
}

// NO BLOQUEA
bool enviar_operacion_sin_paquete(t_log *logger, int conexion, op_code codigo_operacion, const char *nombre_proceso_origen, const char *nombre_proceso_destino)
{
	log_trace(logger, "Comenzando el envio de operacion %d (sin paquete) desde %s a %s.", codigo_operacion, nombre_proceso_origen, nombre_proceso_destino);

	ssize_t resultado_send = send(conexion, &codigo_operacion, sizeof(op_code), 0);

	if (resultado_send == -1)
	{
		log_error(logger, "Error al enviar operacion %d (sin paquete) desde %s a %s.", codigo_operacion, nombre_proceso_origen, nombre_proceso_destino);
	}
	else
	{
		log_trace(logger, "Operacion %d (sin paquete) enviado con exito desde %s a %s.", codigo_operacion, nombre_proceso_origen, nombre_proceso_destino);
	}

	return resultado_send != -1;
}

// NO BLOQUEA
void agregar_int_a_paquete(t_log *logger, t_paquete *paquete, int entero, const char *nombre_proceso_origen, const char *nombre_proceso_destino, op_code codigo_operacion)
{
	log_trace(logger, "Se agregara el entero %d al paquete de origen %s, destino %s, y codigo de operacion %d.", entero, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
	memcpy((paquete->buffer->stream) + (paquete->buffer->size), &entero, sizeof(int));
	paquete->buffer->size += sizeof(int);

	log_trace(logger, "Se agrego el entero %d al paquete de origen %s, destino %s, y codigo de operacion %d.", entero, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
}

// NO BLOQUEA
void agregar_int32_a_paquete(t_log *logger, t_paquete *paquete, uint32_t entero, const char *nombre_proceso_origen, const char *nombre_proceso_destino, op_code codigo_operacion)
{
	log_trace(logger, "Se agregara el entero %d al paquete de origen %s, destino %s, y codigo de operacion %d.", entero, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t));
	memcpy((paquete->buffer->stream) + (paquete->buffer->size), &entero, sizeof(uint32_t));
	paquete->buffer->size += sizeof(uint32_t);

	log_trace(logger, "Se agrego el entero %d al paquete de origen %s, destino %s, y codigo de operacion %d.", entero, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
}

// NO BLOQUEA
void agregar_string_a_paquete(t_log *logger, t_paquete *paquete, char *string, const char *nombre_proceso_origen, const char *nombre_proceso_destino, op_code codigo_operacion)
{
	if (string == NULL)
	{
		log_trace(logger, "Se esta intentado agregar un string NULL al paquete de origen %s, destino %s, y codigo de operacion %d.", nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
		log_trace(logger, "Se agregara el largo del string (0) para poder luego deserializarlo al paquete de origen %s, destino %s, y codigo de operacion %d.", nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
		agregar_int_a_paquete(logger, paquete, 0, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
		return;
	}

	log_trace(logger, "Se agregara el string (%s) al paquete de origen %s, destino %s, y codigo de operacion %d.", string, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);

	int string_length = strlen(string) + 1;
	log_trace(logger, "Se agregara el largo del string (%d) para poder luego deserializarlo al paquete de origen %s, destino %s, y codigo de operacion %d.", string_length, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
	agregar_int_a_paquete(logger, paquete, string_length, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + string_length);
	memcpy((paquete->buffer->stream) + (paquete->buffer->size), string, string_length);
	paquete->buffer->size += string_length;

	log_trace(logger, "Se agrego el string (%s) al paquete de origen %s, destino %s, y codigo de operacion %d.", string, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
}

// NO BLOQUEA
void agregar_lista_de_enteros_a_paquete(t_log *logger, t_list *lista, t_paquete *paquete, const char *nombre_proceso_origen, const char *nombre_proceso_destino, op_code codigo_operacion)
{
	int cantidad_de_elementos = list_size(lista);
	log_trace(logger, "Se agregara una lista de %d enteros al paquete de origen %s, destino %s, y codigo de operacion %d.", cantidad_de_elementos, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
	agregar_int_a_paquete(logger, paquete, cantidad_de_elementos, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);

	t_list_iterator *iterador = list_iterator_create(lista);

	while (list_iterator_has_next(iterador))
	{
		int *puntero_a_elemento = list_iterator_next(iterador);
		int elemento = *puntero_a_elemento;

		agregar_int_a_paquete(logger, paquete, elemento, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
	}

	list_iterator_destroy(iterador);

	log_trace(logger, "Se agrego una lista de %d enteros al paquete de origen %s, destino %s, y codigo de operacion %d.", cantidad_de_elementos, nombre_proceso_origen, nombre_proceso_destino, codigo_operacion);
}

// NO BLOQUEA
void crear_buffer(t_paquete *paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

// NO BLOQUEA
void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

// Espera hasta recibir una operacion a traves de la conexion: BLOQUEA!
op_code esperar_operacion(t_log *logger, const char *nombre_proceso_que_espera, const char *nombre_proceso_que_manda, int conexion)
{
	int cod_op;

	log_trace(logger, "%s esta esperando un codigo de operacion proveniente de %s.", nombre_proceso_que_espera, nombre_proceso_que_manda);

	if (recv(conexion, &cod_op, sizeof(int), MSG_WAITALL) > 0)
	{
		log_trace(logger, "%s recibio un codigo de operacion proveniente de %s y es %d.", nombre_proceso_que_espera, nombre_proceso_que_manda, cod_op);
		return cod_op;
	}

	log_error(logger, "%s no pudo recibir un codigo de operacion proveniente de %s.", nombre_proceso_que_espera, nombre_proceso_que_manda);
	return -1;
}

// Espera hasta recibir un paquete de la conexion: BLOQUEA!
// Devuelve un puntero al buffer de los contenidos del paquete recibido
// Se usa JUSTO DESPUES de recibir un codigo de operacion desde la funcion esperar_operacion
void *recibir_paquete(t_log *logger, const char *nombre_proceso_que_espera, const char *nombre_proceso_que_manda, int *tamanio_buffer, int conexion, op_code codigo_operacion)
{
	void *buffer;

	log_trace(logger, "%s este esperando un paquete de operacion %d proveniente de %s.", nombre_proceso_que_espera, codigo_operacion, nombre_proceso_que_manda);

	recv(conexion, tamanio_buffer, sizeof(int), MSG_WAITALL);
	buffer = malloc(*tamanio_buffer);
	recv(conexion, buffer, *tamanio_buffer, MSG_WAITALL);

	log_trace(logger, "%s recibio un paquete de operacion %d proveniente de %s y creo un buffer de %d bytes para su posterior lectura.", nombre_proceso_que_espera, codigo_operacion, nombre_proceso_que_manda, *tamanio_buffer);

	return buffer;
}

// NO BLOQUEA
t_list *leer_lista_de_enteros_desde_buffer_de_paquete(t_log *logger, const char *nombre_proceso_que_lee, const char *nombre_proceso_mando, void **buffer_de_paquete_con_offset, op_code codigo_operacion)
{
	t_list *lista = list_create();

	log_trace(logger, "%s intentera leer una lista de enteros del paquete ya recibido de operacion %d proveniente de %s.", nombre_proceso_que_lee, codigo_operacion, nombre_proceso_mando);

	int cantidad_de_elementos;
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_que_lee, nombre_proceso_mando, buffer_de_paquete_con_offset, &cantidad_de_elementos, codigo_operacion);

	log_trace(logger, "%s leyo el tamanio de la lista de enteros del paquete ya recibido de operacion %d proveniente de %s y es %d.", nombre_proceso_que_lee, codigo_operacion, nombre_proceso_mando, cantidad_de_elementos);

	for (int i = 0; i < cantidad_de_elementos; i++)
	{
		int *elemento = malloc(sizeof(int));
		leer_int_desde_buffer_de_paquete(logger, nombre_proceso_que_lee, nombre_proceso_mando, buffer_de_paquete_con_offset, elemento, codigo_operacion);
		list_add(lista, elemento);
	}

	log_trace(logger, "%s leyo con exito toda la lista de enteros del paquete ya recibido de operacion %d proveniente de %s.", nombre_proceso_que_lee, codigo_operacion, nombre_proceso_mando);

	return lista;
}

// NO BLOQUEA
void leer_int_desde_buffer_de_paquete(t_log *logger, const char *nombre_proceso_que_lee, const char *nombre_proceso_mando, void **buffer_de_paquete_con_offset, int *entero, op_code codigo_operacion)
{
	log_trace(logger, "%s intentara leer un entero del paquete ya recibido de operacion %d proveniente de %s.", nombre_proceso_que_lee, codigo_operacion, nombre_proceso_mando);

	memcpy(entero, *buffer_de_paquete_con_offset, sizeof(int));
	*buffer_de_paquete_con_offset += sizeof(int);

	log_trace(logger, "%s leyo un entero del paquete ya recibido de operacion %d proveniente de %s y es %d.", nombre_proceso_que_lee, codigo_operacion, nombre_proceso_mando, *entero);
}

// NO BLOQUEA
void leer_int32_desde_buffer_de_paquete(t_log *logger, const char *nombre_proceso_que_lee, const char *nombre_proceso_mando, void **buffer_de_paquete_con_offset, uint32_t *entero, op_code codigo_operacion)
{
	log_trace(logger, "%s intentara leer un entero del paquete ya recibido de operacion %d proveniente de %s.", nombre_proceso_mando, codigo_operacion, nombre_proceso_mando);

	memcpy(entero, *buffer_de_paquete_con_offset, sizeof(uint32_t));
	*buffer_de_paquete_con_offset += sizeof(uint32_t);

	log_trace(logger, "%s leyo un entero del paquete ya recibido de operacion %d proveniente de %s y es %d.", nombre_proceso_mando, codigo_operacion, nombre_proceso_mando, *entero);
}

// NO BLOQUEA
void leer_string_desde_buffer_de_paquete(t_log *logger, const char *nombre_proceso_que_lee, const char *nombre_proceso_mando, void **buffer_de_paquete_con_offset, char **string, op_code codigo_operacion)
{
	int string_length;

	log_trace(logger, "%s intentara leer un string del paquete ya recibido de operacion %d proveniente de %s.", nombre_proceso_que_lee, codigo_operacion, nombre_proceso_mando);

	log_trace(logger, "%s intentara leer el tamanio del string del paquete ya recibido de operacion %d proveniente de %s.", nombre_proceso_que_lee, codigo_operacion, nombre_proceso_mando);
	leer_int_desde_buffer_de_paquete(logger, nombre_proceso_que_lee, nombre_proceso_mando, buffer_de_paquete_con_offset, &string_length, codigo_operacion);
	log_trace(logger, "%s leyo que el tamanio del string a leer del paquete ya recibido de operacion %d proveniente de %s es de %d.", nombre_proceso_que_lee, codigo_operacion, nombre_proceso_mando, string_length);

	if (string_length == 0)
	{
		*string = NULL;
		return;
	}

	*string = malloc(string_length);
	memcpy(*string, *buffer_de_paquete_con_offset, string_length);
	*buffer_de_paquete_con_offset += string_length;

	log_trace(logger, "%s leyo un string del paquete ya recibido de operacion %d proveniente de %s y es '%s'.", nombre_proceso_que_lee, codigo_operacion, nombre_proceso_mando, *string);
}