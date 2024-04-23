#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define P 1 // Productores
#define C 2 // Consumidores
#define N 12 // Tamaño del buffer
#define T_SIZE (110 * (P + C)) // Tamaño del array T

/**
 * Estructura que funcionará como pila LIFO
 * @param buffer: Buffer de la pila
 * @param buffer_count: Número de elementos en el buffer
 */
struct pilaLIFO {
    int buffer[N];
    int buffer_count;
} pila;


// Inicializacion

int T[T_SIZE];
int ind_pares = 0;
int ind_impares = 1;
int sum_pares = 0;
int sum_impares = 0;

int duracion_sleep_prod;
int duracion_sleep_cons;

pthread_mutex_t mutex;
pthread_mutex_t mutexProductor;
pthread_mutex_t mutexConsumidor;

pthread_t productores[P];
pthread_t consumidores[C];
int tid[P + C];

int prod_terminados = 0;

void* productor(void *arg);
void* consumidor(void *arg);

void inicializar_T() {
    for (int i = 0; i < T_SIZE; i++) {
        T[i] = i / 2;
    }
}

void *productor(void *arg) {
    int id = *((int *)arg);
    int items_produced = 0;
    while (items_produced < 18) {
        int item = rand() % 100;
        sleep(duracion_sleep_prod);

        pthread_mutex_lock(&mutex);
        while (pila.buffer_count == N) {
            pthread_mutex_unlock(&mutex);
            sleep(1);
            pthread_mutex_lock(&mutex);
        }

        pila.buffer[pila.buffer_count++] = item;
        printf("Productor %d: Produciendo item %d, buffer_count %d\n", id, item, pila.buffer_count);
        pthread_mutex_unlock(&mutex);
        sleep(duracion_sleep_prod);
        items_produced++;

        if (pthread_mutex_trylock(&mutexProductor) == 0) {
            int sum_count = rand() % 3 + 2; // Sumar entre 2 y 4 elementos
            while (sum_count-- > 0 && ind_pares < T_SIZE) {
                sum_pares += T[ind_pares];
                ind_pares += 2;
            }
            pthread_mutex_unlock(&mutexProductor);
        }
    }
    pthread_exit(NULL);
}

void *consumidor(void *arg) {
    int id = *((int *)arg);
    while (1) {
        pthread_mutex_lock(&mutex);
        if (prod_terminados == 1 && pila.buffer_count == 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        while (pila.buffer_count == 0) {
            pthread_mutex_unlock(&mutex);
            sleep(1);
            pthread_mutex_lock(&mutex);
        }

        int item = pila.buffer[--pila.buffer_count];
        printf("Consumidor %d: Consumiendo item %d, buffer_count %d\n", id, item, pila.buffer_count);
        pthread_mutex_unlock(&mutex);
        sleep(duracion_sleep_cons);

        if (pthread_mutex_trylock(&mutexConsumidor) == 0) {
            int sum_count = rand() % 3 + 2; // Sumar entre 2 y 4 elementos
            while (sum_count-- > 0 && ind_impares < T_SIZE) {
                sum_impares += T[ind_impares];
                ind_impares += 2;
            }
            pthread_mutex_unlock(&mutexConsumidor);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("ERROR: Número de argumentos incorrecto. Usage: %s t_prod t_cons\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    duracion_sleep_prod = atoi(argv[1]);
    duracion_sleep_cons = atoi(argv[2]);

    pila.buffer_count = 0;
    inicializar_T();

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexProductor, NULL);
    pthread_mutex_init(&mutexConsumidor, NULL);

    for (int i = 0; i < P; i++) {
        tid[i] = i;
        pthread_create(&productores[i], NULL, productor, &tid[i]);
    }

    for (int i = 0; i < C; i++) {
        tid[P + i] = P + i;
        pthread_create(&consumidores[i], NULL, consumidor, &tid[P + i]);
    }

    for (int i = 0; i < P; i++) {
        pthread_join(productores[i], NULL);
    }
    prod_terminados = 1;

    for (int i = 0; i < C; i++) {
        pthread_join(consumidores[i], NULL);
    }

    printf("Suma total de pares: %d\n", sum_pares);
    printf("Suma total de impares: %d\n", sum_impares);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexProductor);
    pthread_mutex_destroy(&mutexConsumidor);
    return 0;
}
