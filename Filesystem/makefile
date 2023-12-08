COMPILADOR=gcc
LIBRERIAS= -lcommons -lpthread -lreadline -lm
NOMBRE_MODULO=filesystem
EJECUTABLE=Ejecutable/$(NOMBRE_MODULO).o
CONFIGURACION="Configuracion/$(NOMBRE_MODULO).config"
LOGS=Logs/$(NOMBRE_MODULO).log
FUENTES_MODULO=$(shell find *.c) $(shell find Source/*.c)
FUENTES_SHARED=$(shell find ../Shared/*.c)

.PHONY: ejecutar limpiar compilar
ejecutar: limpiar compilar
	./$(EJECUTABLE) $(CONFIGURACION)
limpiar:
	rm -rf $(EJECUTABLE)
	rm -rf $(LOGS)
	rm -rf fs/bloques.dat
	rm -rf fs/fat.dat
	rm -rf fs/fcbs/*
compilar:
	$(COMPILADOR) $(FUENTES_MODULO) $(FUENTES_SHARED) -o $(EJECUTABLE) $(LIBRERIAS) -Werror
detener:
	pkill -f $(NOMBRE_MODULO).o
detener:
	pkill -f $(NOMBRE_MODULO).o