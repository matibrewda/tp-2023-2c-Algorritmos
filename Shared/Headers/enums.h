#ifndef ENUMS_H_
#define ENUMS_H_

typedef enum
{
	// Kernel a CPU
	SOLICITUD_EJECUTAR_PROCESO,
	SOLICITUD_INTERRUMPIR_PROCESO,
	RESPUESTA_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO,
	RESPUESTA_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION,
	RESPUESTA_DEVOLVER_PROCESO_POR_SLEEP,
	RESPUESTA_DEVOLVER_PROCESO_POR_WAIT,
	RESPUESTA_DEVOLVER_PROCESO_POR_SIGNAL,
	RESPUESTA_DEVOLVER_PROCESO_POR_ERROR,
	RESPUESTA_DEVOLVER_PROCESO_POR_PAGEFAULT,
	RESPUESTA_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM,

	// Kernel a Memoria
	SOLICITUD_INICIAR_PROCESO_MEMORIA,
	SOLICITUD_FINALIZAR_PROCESO_MEMORIA,
	SOLICITUD_CARGAR_PAGINA_EN_MEMORIA,

	// Kernel a Filesystem
	SOLICITUD_ABRIR_ARCHIVO_FS,
	SOLICITUD_CREAR_ARCHIVO_FS,
	SOLICITUD_TRUNCAR_ARCHIVO_FS,
	SOLICITUD_LEER_ARCHIVO_FS,
	SOLICITUD_ESCRIBIR_ARCHIVO_FS,

	// CPU a Kernel
	RESPUESTA_EJECUTAR_PROCESO,
	RESPUESTA_INTERRUMPIR_PROCESO,
	SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO,
	SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION,
	SOLICITUD_DEVOLVER_PROCESO_POR_SLEEP,
	SOLICITUD_DEVOLVER_PROCESO_POR_WAIT,
	SOLICITUD_DEVOLVER_PROCESO_POR_SIGNAL,
	SOLICITUD_DEVOLVER_PROCESO_POR_ERROR,
	SOLICITUD_DEVOLVER_PROCESO_POR_PAGEFAULT,
	SOLICITUD_DEVOLVER_PROCESO_POR_OPERACION_FILESYSTEM,

	// CPU a Memoria
	SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA,
	SOLICITUD_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU,
	SOLICITUD_PEDIR_NUMERO_DE_MARCO_A_MEMORIA,
	SOLICITUD_LEER_VALOR_EN_MEMORIA,
	SOLICITUD_ESCRIBIR_VALOR_EN_MEMORIA,

	// Memoria a Kernel
	RESPUESTA_INICIAR_PROCESO_MEMORIA,
	RESPUESTA_FINALIZAR_PROCESO_MEMORIA,
	RESPUESTA_CARGAR_PAGINA_EN_MEMORIA,

	// Memoria a CPU
	RESPUESTA_PEDIR_INSTRUCCION_A_MEMORIA,
	RESPUESTA_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU,
	RESPUESTA_NUMERO_DE_MARCO_A_CPU,
	RESPUESTA_LEER_VALOR_EN_MEMORIA,
	RESPUESTA_ESCRIBIR_VALOR_EN_MEMORIA,

	// Memoria a Filesystem
	SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM,
	SOLICITUD_LIBERAR_BLOQUES_EN_FILESYSTEM,
	SOLICITUD_CONTENIDO_BLOQUE_EN_FILESYSTEM,
	SOLICITUD_ESCRIBIR_PAGINA_EN_SWAP,
	RESPUESTA_LEER_ARCHIVO_MEMORIA,
	RESPUESTA_ESCRIBIR_ARCHIVO_MEMORIA,
	RESPUESTA_LEER_VALOR_EN_MEMORIA_DESDE_FILESYSTEM,

	// Fileystem a Kernel
	RESPUESTA_ABRIR_ARCHIVO_FS,
	RESPUESTA_CREAR_ARCHIVO_FS,
	RESPUESTA_TRUNCAR_ARCHIVO_FS,
	RESPUESTA_LEER_ARCHIVO_FS,
	RESPUESTA_ESCRIBIR_ARCHIVO_FS,

	// Filesystem a Memoria
	SOLICITUD_LEER_ARCHIVO_MEMORIA,
	SOLICITUD_ESCRIBIR_ARCHIVO_MEMORIA,
	SOLICITUD_LEER_VALOR_EN_MEMORIA_DESDE_FILESYSTEM,
	RESPUESTA_PEDIR_BLOQUES_A_FILESYSTEM,
	RESPUESTA_CONTENIDO_BLOQUE_EN_FILESYSTEM,
	RESPUESTA_LIBERAR_BLOQUES_EN_FILESYSTEM
} op_code;

typedef enum
{
	FOPEN_OPCODE,
	FCLOSE_OPCODE,
	FSEEK_OPCODE,
	FREAD_OPCODE,
	FWRITE_OPCODE,
	FTRUNCATE_OPCODE
} fs_op_code;

const char *nombre_opcode(op_code opcode);

#endif /* ENUMS_H_ */
