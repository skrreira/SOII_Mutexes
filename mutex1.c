#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define P 3 // productores
#define C 3 // consumidores
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
int sleep_prod;
int sleep_insert;
int sleep_sum_prod;
int sleep_extract;
int sleep_cons;
int sleep_sum_cons;

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
    if (argc != 7){
        printf("ERROR: Número de argumentos incorrecto. | Usage: %s sleep_produce sleep_insert sleep_sum_prod sleep_extract sleep_consume sleep_sum_cons\n.", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    sleep_prod = atoi(argv[1]);
    sleep_insert = atoi(argv[2]);
    sleep_sum_prod = atoi(argv[3]);
    sleep_extract = atoi(argv[4]);
    sleep_cons = atoi(argv[5]);
    sleep_sum_cons = atoi(argv[6]);

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
    for (; i < P + C; i++) {
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
            sleep(sleep_prod); // Tiempo de producción
            printf("Productor %d: Produciendo item %d\n", id, item);

            // Bloqueamos el mútex:
            pthread_mutex_lock(&mutex);
            while (pila.buffer_count >= N) {
                pthread_cond_wait(&can_produce, &mutex);
            }

            // Agregar item al buffer
            pila.buffer[pila.buffer_count] = item;
            pila.buffer_count++;
            sleep(sleep_insert); //Tiempo de agregación
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
            sleep(sleep_sum_prod);
            printf("Productor %d: TERMINADO. Suma de pares completada: %d\n", id, sum_pares);
            }
        }

    pthread_exit(NULL);
}

void *consumidor(void *arg) {

    // Ponemos -1 para ajustar el indice a 0 y 1
    int id = *((int *)arg) - 1;

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
        sleep(sleep_extract);
        printf("Consumidor %d: Extrayendo item %d del buffer\n", id, item);

        // Consumimos ítem (bajamos count):
        pila.buffer_count = pila.buffer_count - 1;
        sleep(sleep_cons);
        printf("Consumidor: %d Consumiendo item %d del buffer\n", id, item);
        elementos_consumidos--;

        // Imprimimos que la tarea obligatoria está completa:
        printf("Consumidor %d: Tarea obligatoria realizada. Desbloqueamos mútex. Sumatorio de valores impares.\n", id );

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
            sleep(sleep_sum_cons);
            printf("Consumidor %d: Suma de impares completada: %d\n", id, sum_impares);
            }
        }
        
        // Si es el último consumidor se despiertan al resto:
        if (elementos_consumidos == 0){
            pthread_cond_broadcast(&can_consume);
        }
    pthread_exit(NULL);
}