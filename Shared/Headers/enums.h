#ifndef ENUMS_H_
#define ENUMS_H_

typedef enum
{
	MENSAJE_DE_KERNEL,
	MENSAJE_DE_MEMORIA,
	MENSAJE_DE_FILESYSTEM,
	MENSAJE_DE_CPU_INTERRUPT,
	MENSAJE_DE_CPU_DISPATCH,
	MENSAJE_DE_CPU
} op_code;

const char* nombre_opcode(op_code opcode);

#endif /* ENUMS_H_ */
