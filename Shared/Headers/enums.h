#ifndef ENUMS_H_
#define ENUMS_H_

typedef enum
{	
	// Kernel a CPU
	SOLICITUD_EJECUTAR_PROCESO,
	SOLICITUD_INTERRUMPIR_PROCESO,
	RESPUESTA_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO,
	RESPUESTA_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION,
	
	// Kernel a Memoria
	SOLICITUD_INICIAR_PROCESO_MEMORIA,
	SOLICITUD_FINALIZAR_PROCESO_MEMORIA,
	
	// CPU a Kernel
	RESPUESTA_EJECUTAR_PROCESO,
	RESPUESTA_INTERRUMPIR_PROCESO,
	SOLICITUD_DEVOLVER_PROCESO_POR_SER_INTERRUMPIDO,
	SOLICITUD_DEVOLVER_PROCESO_POR_CORRECTA_FINALIZACION,

	// CPU a Memoria
	SOLICITUD_PEDIR_INSTRUCCION_A_MEMORIA,
	SOLICITUD_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU,

	// Memoria a Kernel
	RESPUESTA_INICIAR_PROCESO_MEMORIA,
	RESPUESTA_FINALIZAR_PROCESO_MEMORIA,
	
	// Memoria a CPU
	RESPUESTA_PEDIR_INSTRUCCION_A_MEMORIA,
	RESPUESTA_PEDIR_INFO_DE_MEMORIA_INICIAL_PARA_CPU,

	// Memoria a Filesystem
	SOLICITUD_PEDIR_BLOQUES_A_FILESYSTEM,
	SOLICITUD_LIBERAR_BLOQUES_EN_FILESYSTEM,
	RESPUESTA_LEER_ARCHIVO_MEMORIA,
	RESPUESTA_ESCRIBIR_ARCHIVO_MEMORIA,

	// Filesystem a Memoria
	SOLICITUD_LEER_ARCHIVO_MEMORIA,
	SOLICITUD_ESCRIBIR_ARCHIVO_MEMORIA,

	// DEFECTO
	OPCODE_DEFECTO
} op_code;

const char* nombre_opcode(op_code opcode);

#endif /* ENUMS_H_ */
