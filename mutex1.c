#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define P 5 // productores
#define C 4 // consumidores
#define N 12 // Tamaño del buffer
#define T_SIZE (110 * (P + C)) // Tamaño del array T: se hara la suma

// Estructura que funcionará como pila LIFO:
struct pilaLIFO{
    int buffer[N];
    int buffer_count;
};

// Creamos buffer LIFO:
struct pilaLIFO pila;

// Creamos array T (sumatorio):
int T[T_SIZE];

// Variables para la suma de pares e impares:
int ind_pares = 0;  //Índices
int ind_impares = 1;
int sum_pares = 0;  //Sumas
int sum_impares = 0;

// Duración de los sleep:
int duracion_sleep_prod;
int duracion_sleep_cons;

// Mútexes y variables de condición:
pthread_mutex_t mutex;
pthread_cond_t can_produce;
pthread_cond_t can_consume;

// Mútexes:
pthread_mutex_t mutexProductor;
pthread_mutex_t mutexConsumidor;

// Hilos
pthread_t productores[P];
pthread_t consumidores[C];
int tid[P + C]; // Para almacenar TIDs

// Contador de productores terminados:
int prod_terminados = 0;

// Contador de elementos consumidos:
int elementos_consumidos = 18*P;

// Prototipado de funciones:
void* productor(void *arg);
void* consumidor(void *arg);

// Main
int main(int argc, char** argv) {

    // Procesamos argumentos:
    if (argc != 3){
        printf("ERROR: Número de argumentos incorrecto. | Usage: %s t_prod t_cons.", argv[0]);
        exit(EXIT_FAILURE);
    }
    duracion_sleep_prod = atoi(argv[1]);
    duracion_sleep_cons = atoi(argv[2]);

    // Inicializamos buffer LIFO:
    pila.buffer_count = 0;

    //TODO: REVISAR
    // Inicializamos T
    int i = 0;
    for (i = 0; i < T_SIZE; i++) {
        T[i] = i / 2;
    }

    // Inicialización de mutexes
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Error al inicializar el mutex");
        return 1;
    }

    if (pthread_mutex_init(&mutexProductor, NULL) != 0) {
        perror("Error al inicializar el mutex del productor");
        return 1;
    }

    if (pthread_mutex_init(&mutexConsumidor, NULL) != 0) {
        perror("Error al inicializar el mutex del consumidor");
        return 1;
    }
    
    // Inicialización de la variable de condición para productores
    if (pthread_cond_init(&can_produce, NULL) != 0) {
        perror("Error al inicializar la variable de condición para productores");
        pthread_mutex_destroy(&mutex);
        return 1;
    }
    
    // Inicialización de la variable de condición para consumidores
    if (pthread_cond_init(&can_consume, NULL) != 0) {
        perror("Error al inicializar la variable de condición para consumidores");
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&can_produce);
        return 1;
    }

    // Creación de hilos:
    // Creamos los productores:
    for (i = 0; i < P; i++) {
        tid[i] = i;
        pthread_create(&productores[i], NULL, productor, &tid[i]);
    }
        
    // Creamos los consumidores:
    for (i; i < P + C; i++) {
        tid[i] = i;
        pthread_create(&consumidores[i - P], NULL, consumidor, &tid[i]);
    }    

    // Esperamos a que termine cada hilo productor:
    for (i = 0; i < P; i++) {pthread_join(productores[i], NULL);}
    
    // Marcamos que los productores ya han terminado:
    prod_terminados = 1;

    // Esperamos a que termine cada hilo consumidor:
    for (i = 0; i < C; i++) {pthread_join(consumidores[i], NULL);}

    // Imprimimos resultados:
    printf("Suma total de pares: %d\n", sum_pares);
    printf("Suma total de impares: %d\n", sum_impares);

    // Destruimos el mútex y las variables de condición:
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexProductor);
    pthread_mutex_destroy(&mutexConsumidor);
    pthread_cond_destroy(&can_produce);
    pthread_cond_destroy(&can_consume);

}

// Funciones del productor y del consumidor:
void *productor(void *arg) {

    // Casteamos el argumento a tipo int
    int id = *((int *)arg);

    // Contador de items producidos:
    int items_produced = 0;

    // Bucle de producción:
    while (items_produced < 18) {

        if (items_produced < 18){
            // Producción de items
            int item = rand() % 10;
            sleep(duracion_sleep_prod); // Tiempo de producción
            printf("Productor %d: Produciendo item %d\n", id, item);

            // Bloqueamos el mútex:
            pthread_mutex_lock(&mutex);
            while (pila.buffer_count >= N) {
                pthread_cond_wait(&can_produce, &mutex);
            }

            // Agregar item al buffer
            pila.buffer[pila.buffer_count] = item;
            pila.buffer_count++;
            sleep(duracion_sleep_prod); //Tiempo de agregación
            printf("Productor %d: Agregando item %d al buffer\n", id, item);

            // Aumentamos el número de items producidos
            items_produced++;

            // Cambiamos variable de condición y desbloqueamos mútex:
            pthread_cond_signal(&can_consume);
            pthread_mutex_unlock(&mutex);

            // Imprimimos que la tarea obligatoria está completa:
            printf("Productor %d: Tarea obligatoria realizada. Desbloqueamos mútex. Sumatorio de valores pares.\n", id);    
        }

        
        // Intentamos bloquear al mútex:
        if (pthread_mutex_trylock(&mutexProductor) == 0){

            // Calculamos número de elementos de T a sumar (aleatorio entre 2 y 4):
            int num_suma = 2 + (rand()%2);

            // Bucle del sumatorio:
            int i = 0;
            while(i < num_suma){

                // Contribuir al sumatorio de los valores de las posiciones pares de T
                sum_pares += T[ind_pares];

                // Aumentamos el índice:
                ind_pares += 2;
                i++;
            }

            // Liberamos el mútex:
            pthread_mutex_unlock(&mutexProductor);    

            // Mensaje de éxito:
            sleep(duracion_sleep_prod);
            printf("Productor %d: TERMINADO. Suma de pares completada: %d\n", id, sum_pares);
            }
        }

    pthread_exit(NULL);
}

void *consumidor(void *arg) {

    // Cast
    int id = *((int *)arg);

    // Bucle para consumir indefinidamente.
    while (1) {

        // Bloqueamos el mútex:
        pthread_mutex_lock(&mutex); 

        // Si los productores han terminado, salimos:
        if (prod_terminados == 1 && pila.buffer_count <= 0){
            pthread_mutex_unlock(&mutex);
            break;
        } 
        
        // Esperamos a la variable de condición:
        while (pila.buffer_count <= 0) {
            pthread_cond_wait(&can_consume, &mutex);
        }

        // Extraemos un item del buffer
        int item = pila.buffer[pila.buffer_count - 1];
        sleep(duracion_sleep_cons);
        printf("Consumidor %d: Extrayendo item %d del buffer\n", id, item);

        // Consumimos ítem (bajamos count):
        pila.buffer_count = pila.buffer_count - 1;
        sleep(duracion_sleep_cons);
        printf("Consumidor: %d Consumiendo item %d del buffer\n", id, item);
        elementos_consumidos--;

        // Imprimimos que la tarea obligatoria está completa:
        printf("Consumidor %d: Tarea obligatoria realizada. Desbloqueamos mútex. Sumatorio de valores impares.\n", id);

        // Desbloqueamos mútex y variable de condición:
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&can_produce);

        // Intentamos bloquear mútex:
        if (pthread_mutex_trylock(&mutexConsumidor) == 0){

            // Calculamos número de elementos de T a sumar (aleatorio entre 2 y 4):
            int num_suma = 2 + (rand()%2);
            // Bucle del sumatorio:
            int i = 0;
            while(i < num_suma){
                // Contribuir al sumatorio de los valores de las posiciones impares de T
                sum_impares += T[ind_impares];
                // Aumentamos el índice:
                ind_impares += 2;
                i++;
            }

            // Desbloqueamos mútex:
            pthread_mutex_unlock(&mutexConsumidor);

            // Mensaje de éxito:
            sleep(duracion_sleep_cons);
            printf("Consumidor %d: Suma de impares completada: %d\n", id, sum_impares);
            }
        }

        //TODO: SI ES EL ÚLTIMO CONSUMIDOR HAY Q HACER UN BROADCAST AL RESTO PARA QUE SE DESPIERTEN: 18 * P
        //! ???? ESTO SIGUE SIN IR POR ALGUN MOTIVO, LO VAMOS A DEJAR PERO EL ERROR ES OTRO
        // Si es el último consumidor se despiertan al resto:
        if (elementos_consumidos == 0){
            pthread_cond_broadcast(&can_consume);
        }
    pthread_exit(NULL);
}