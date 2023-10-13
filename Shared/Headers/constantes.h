#ifndef CONSTANTES_H_
#define CONSTANTES_H_

// Opcodes de instrucciones
#define SET_OPCODE_INSTRUCCION 1
#define SUM_OPCODE_INSTRUCCION 2
#define SUB_OPCODE_INSTRUCCION 3
#define JNZ_OPCODE_INSTRUCCION 4
#define SLEEP_OPCODE_INSTRUCCION 5
#define WAIT_OPCODE_INSTRUCCION 6
#define SIGNAL_OPCODE_INSTRUCCION 7
#define MOV_IN_OPCODE_INSTRUCCION 8
#define MOV_OUT_OPCODE_INSTRUCCION 9
#define FOPEN_OPCODE_INSTRUCCION 10
#define FCLOSE_OPCODE_INSTRUCCION 11
#define FSEEK_OPCODE_INSTRUCCION 12
#define FREAD_OPCODE_INSTRUCCION 13
#define FWRITE_OPCODE_INSTRUCCION 14
#define FTRUNCATE_OPCODE_INSTRUCCION 15
#define EXIT_OPCODE_INSTRUCCION 16

// Nombres de instrucciones
#define SET_NOMBRE_INSTRUCCION "SET"
#define SUM_NOMBRE_INSTRUCCION "SUM"
#define SUB_NOMBRE_INSTRUCCION "SUB"
#define JNZ_NOMBRE_INSTRUCCION "JNZ"
#define SLEEP_NOMBRE_INSTRUCCION "SLEEP"
#define WAIT_NOMBRE_INSTRUCCION "WAIT"
#define SIGNAL_NOMBRE_INSTRUCCION "SIGNAL"
#define MOV_IN_NOMBRE_INSTRUCCION "MOV_IN"
#define MOV_OUT_NOMBRE_INSTRUCCION "MOV_OUT"
#define FOPEN_NOMBRE_INSTRUCCION "F_OPEN"
#define FCLOSE_NOMBRE_INSTRUCCION "F_CLOSE"
#define FSEEK_NOMBRE_INSTRUCCION "F_SEEK"
#define FREAD_NOMBRE_INSTRUCCION "F_READ"
#define FWRITE_NOMBRE_INSTRUCCION "F_WRITE"
#define FTRUNCATE_NOMBRE_INSTRUCCION "F_TRUNCATE"
#define EXIT_NOMBRE_INSTRUCCION "EXIT"

// Nombres de registros
#define AX_NOMBRE_REGISTRO "AX"
#define BX_NOMBRE_REGISTRO "BX"
#define CX_NOMBRE_REGISTRO "CX"
#define DX_NOMBRE_REGISTRO "DX"
#define PC_NOMBRE_REGISTRO "PC"

// Nombres de modulos
#define NOMBRE_MODULO_KERNEL "Kernel"
#define NOMBRE_MODULO_CPU "CPU"
#define NOMBRE_MODULO_CPU_INTERRUPT "CPU (INTERRUPT)"
#define NOMBRE_MODULO_CPU_DISPATCH "CPU (DISPATCH)"
#define NOMBRE_MODULO_FILESYSTEM "Filesystem"
#define NOMBRE_MODULO_MEMORIA "Memoria"

// Estado procesos (codigos)
#define CODIGO_ESTADO_PROCESO_NEW 'N'
#define CODIGO_ESTADO_PROCESO_READY 'R'
#define CODIGO_ESTADO_PROCESO_EXECUTING 'X'
#define CODIGO_ESTADO_PROCESO_EXIT 'E'
#define CODIGO_ESTADO_PROCESO_BLOCKED 'B'
#define CODIGO_ESTADO_PROCESO_DESCONOCIDO '?'

// Estado procesos (nombres completos)
#define NOMBRE_ESTADO_PROCESO_NEW "NEW"
#define NOMBRE_ESTADO_PROCESO_READY "READY"
#define NOMBRE_ESTADO_PROCESO_EXECUTING "EXEC"
#define NOMBRE_ESTADO_PROCESO_EXIT "EXIT"
#define NOMBRE_ESTADO_PROCESO_BLOCKED "BLOCKED"

// Algoritmos de planificacion
#define ALGORITMO_PLANIFICACION_FIFO "FIFO"
#define ALGORITMO_PLANIFICACION_ROUND_ROBIN "RR"
#define ALGORITMO_PLANIFICACION_PRIORIDADES "PRIORIDADES"

#endif /* CONSTANTES_H_ */