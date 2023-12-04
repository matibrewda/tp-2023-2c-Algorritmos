#!/bin/bash

cd Memoria
make limpiar
make compilar
nohup ./Ejecutable/memoria.o Configuracion/memoria_prueba_base.config &
cd ..

cd Filesystem
make limpiar
make compilar
nohup ./Ejecutable/filesystem.o Configuracion/filesystem_prueba_base.config &
cd ..

cd CPU
make limpiar
make compilar
nohup ./Ejecutable/cpu.o Configuracion/cpu_prueba_base.config &
cd ..

cd Kernel
make limpiar
make compilar
./Ejecutable/kernel.o Configuracion/kernel_prueba_base.config &