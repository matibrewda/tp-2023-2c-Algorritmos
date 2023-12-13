COMPILADOR=gcc
LIBRERIAS= -lcommons -lpthread -lreadline
NOMBRE_MODULO=memoria
EJECUTABLE=Ejecutable/$(NOMBRE_MODULO).o
CONFIGURACION="Configuracion/$(NOMBRE_MODULO).config"
LOGS=Logs/$(NOMBRE_MODULO).log
FUENTES_MODULO=$(shell find *.c) $(shell find Source/*.c)
FUENTES_SHARED=$(shell find ../Shared/*.c)

.PHONY: valgrind ejecutar limpiar compilar
valgrind: limpiar compilar
	valgrind --leak-check=full --track-origins=yes --log-file=Valgrind/valgrind.txt ./$(EJECUTABLE) $(CONFIGURACION)
ejecutar: limpiar compilar
	./$(EJECUTABLE) $(CONFIGURACION)
limpiar:
	rm -rf $(EJECUTABLE)
	rm -rf $(LOGS)
compilar:
	$(COMPILADOR) $(FUENTES_MODULO) $(FUENTES_SHARED) -o $(EJECUTABLE) $(LIBRERIAS) -Werror
detener:
	pkill -f $(NOMBRE_MODULO).o