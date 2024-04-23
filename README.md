# Sincronización de Procesos en Sistemas Operativos II

Este repositorio contiene los códigos fuente para la práctica 3 de la asignatura de Sistemas Operativos II del Grado en Ingeniería Informática. La práctica se centra en la implementación y manejo de mecanismos para solucionar el problema de las carreras críticas mediante el uso de mutexes y variables de condición.

## Descripción

La práctica está dividida en dos partes principales:

### Ejercicio 1: Productor-Consumidor Modificado

Este ejercicio implementa una versión modificada del clásico problema del productor-consumidor usando hilos, mutexes y variables de condición. Los detalles son los siguientes:

- **Productores**: Producen 18 items cada uno y contribuyen al sumatorio de valores en posiciones pares de un array.
- **Consumidores**: Consumen todos los items producidos y contribuyen al sumatorio de valores en posiciones impares del mismo array.

### Ejercicio 2: Práctica Voluntaria

Este ejercicio es una extensión voluntaria que busca explorar otras soluciones al problema del productor-consumidor, específicamente evitando el uso de variables de condición y utilizando solo mutexes para la sincronización.

## Compilación y Ejecución

Para compilar y ejecutar los programas, asegúrese de tener `gcc` y `make` instalados en su sistema. Puede compilar los programas utilizando el siguiente comando:

```
gcc -pthread -o productor_consumidor productor_consumidor.c
```

Para ejecutar el programa