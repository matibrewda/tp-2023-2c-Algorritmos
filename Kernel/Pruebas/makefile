COMPILADOR=gcc
LIBRERIAS= -lcommons -lpthread -lreadline
NOMBRE_MODULO=pruebas
EJECUTABLE=$(NOMBRE_MODULO).o
LOGS=$(NOMBRE_MODULO).log
FUENTES_MODULO=$(shell find *.c) $(shell find Source/*.c)
FUENTES_SHARED=$(shell find ../../Shared/*.c)

.PHONY: ejecutar limpiar compilar
ejecutar: limpiar compilar
	./$(EJECUTABLE)
limpiar:
	rm -rf $(EJECUTABLE)
	rm -rf $(LOGS)
compilar:
	$(COMPILADOR) $(FUENTES_MODULO) $(FUENTES_SHARED) -o $(EJECUTABLE) $(LIBRERIAS) -Werror