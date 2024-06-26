/*INCLUDES NECESARIOS*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


//Constantes
#define P 2 // productores //cuando cambie a linea de comando usar reserva dinamica
#define C 2// consumidores
#define N 10 // tama~no del buffer
#define TAM_A (P*100) //tamaño array A
#define TAM_B (C*100) //tamaño array B
#define ITEMS_BY_P 20 // items por cada productor
#define ITERACIONES_CONSUMER (P*ITEMS_BY_P/C) //numero de iteraciones de cada consumidor, para cumplir que todos los elementos sean consumidos
#define SLEEP_MAX_TIME 4// m´aximo tiempo para bloquear al productor

int buffer[N];//Definimos un buffer de enteros de tamaño 10
int posBuffer = -1; //Variable global que indica la posición del último elemento del buffer
/*DECLARACION DE LOS ARRAY*/
int A[TAM_A], B[TAM_B];

//posicion del ultimo elemento sumado del array A
int posA = 0;
//suma de los productores
int sumaProd = 0;

//posicion del ultimo elemento sumado del array B
int posB = 0;
//suma de los productores
int sumaCons = 0;

/*SLEEPS*/
int sleepProducir = 1;
int sleepInsertar = 2;
int sleepSumarP = 2;
int sleepExtraer = 1;
int sleepConsumir = 3;
int sleepSumaC = 1;

/*CRACION MUTEX Y VARIABLES DE CONDICION*/
pthread_mutex_t mutex;
pthread_mutex_t mutexSumaProd;
pthread_mutex_t mutexSumaCons;
pthread_cond_t condProd;
pthread_cond_t condCons;

/*FUNCIÓN PARA INICIALIZAR EL BUFFER A -1*/
void inicializar() {
    int i;
    for (i = 0; i < N; i++) {
        buffer[i] = -1;//Se rellenan todos los huecos con -1
    }
}

/*FUNCIÓN PARA PRODUCIR UN ITEM ALEATORIO*/
int produce_item(int i) {
    sleep(sleepProducir);
    return i;//Devolvemos i para que sea más fácil discernir si hay una carrera crítica o no

}

/*FUNCIÓN PARA INSERTAR UN ELEMENTO EN EL BUFFER*/
void insert_item(int item) {//La función recibe el item que se va a insertar en el buffer como parámetro
    sleep(sleepInsertar);
    posBuffer = posBuffer + 1;//Actualizamos la posición del último elemento de la pila
    buffer[posBuffer] = item;//Insertamos el elemento que se quiere meter en el buffer en la nueva posición pos
}

/*FUNCIÓN PARA RETIRAR UN ENTERO DEL BUFFER*/
int consume_item() {
    int item = -1;
    if (posBuffer >=0) {//Comprobamos que la posición del último elemento en el buffer sea mayor/igual a cero.
        sleep(sleepExtraer);
        item = buffer[posBuffer];//Asignamos a la variable item el valor del elemento que está en la posición pos del buffer
        sleep(sleepConsumir);
        buffer[posBuffer] = -1;//Al eliminar el elemento del buffer, este se sustituye por -1 y, por tanto, se indica que esa posición estará libre
        posBuffer = posBuffer - 1;//Se actualiza la posición del último elemento de la pila
    }
    return item;//Devolvemos el elemento que ha sido consumido
}

/*FUNCION PARA REALIZAR LA SUMA DE LOS PRODUCTORES*/
void sumaProductor(int id) {
    int i;
    sleep(sleepSumarP);
    int numIt = rand() % 5 + 1;//numero de iteraciones que se van a realizar
    int it;
    if(posA+numIt >TAM_A)it = TAM_A;
    else it = posA+numIt;
    for (i = posA; i < TAM_A && i < it; i++) {
        sumaProd += A[i];
    }
    posA = i;
    //printf("Productor[ID:%d]: suma actual = %d\n",id,sumaProd);
}

/*FUNCION PARA REALIZAR LA SUMA DE LOS CONSUMIDORES*/
void sumaConsumidor(int id) {
    int i;
    sleep(sleepSumaC);
    int numIt = rand() % 5 + 1;//numero de iteraciones que se van a realizar
    int it;
    if(posB+numIt >TAM_B)it = TAM_B;
    else it = posB+numIt;
    for (i = posB; i < it; i++) {
        sumaCons += B[i];
    }
    posB = i;
    //printf("Consumidor[ID:%d]: suma actual = %d\n",id,sumaCons);
}

/*FUNCIÓN QUE DEFINE EL COMPORTAMIENTO DEL PRODUCTOR*/
void *productor(void *arg) {
    int i;
    int item = -1;
    int id = *(int *) arg;
    int mostrarSuma = 1;
    printf("Productor %d creado\n",id);
    for (i = 0; i <ITEMS_BY_P || posA<TAM_A; i++) {
        if(i<ITEMS_BY_P){
            if (item == -1)item = produce_item(i);
            pthread_mutex_lock(&mutex); // entramos a la region critica
            while (posBuffer == N - 1)pthread_cond_wait(&condProd, &mutex);
            insert_item(item);
            printf("El productor %d ha insertado el elemento %d en la posición %d\n", id, item,posBuffer);
            item = -1;
            pthread_cond_signal(&condCons);
            pthread_mutex_unlock(&mutex);
        }

        if (!pthread_mutex_trylock(&mutexSumaProd)) {
            sumaProductor(id);
            pthread_mutex_unlock(&mutexSumaProd);
        }
        if(mostrarSuma) {
            if(posA == TAM_A) {
                printf("Productor %d --- suma contribuida: %d, posicion final %d\n", id, sumaProd, posA);
                mostrarSuma = 0;
            }
        }
    }
    printf("Productor %d --- TERMINADO\n", id);
    pthread_exit(NULL);//Finalizamos la ejecución del hilo productor
}

/*FUNCIÓN QUE DEFINE EL COMPORTAMIENTO DEL CONSUMIDOR*/
void *consumidor(void *argC) {
    int i;
    int item;
    int idC = *(int *) argC;
    int mostrarSuma = 1;
    printf("Consumidor %d creado\n",idC);
    for (i = 0; i <ITERACIONES_CONSUMER || posB<TAM_B ; i++) {
        if(i<ITERACIONES_CONSUMER){
            pthread_mutex_lock(&mutex); // entramos a la region critica
            while (posBuffer == -1)pthread_cond_wait(&condCons, &mutex);
            item = consume_item();
            printf("El consumidor %d ha consumido el elemento %d en la posición %d\n", idC, item, posBuffer + 1);
            pthread_cond_signal(&condProd);
            pthread_mutex_unlock(&mutex);
        }
        if (!pthread_mutex_trylock(&mutexSumaCons)) {
            sumaConsumidor(idC);
            pthread_mutex_unlock(&mutexSumaCons);
        }
        if(mostrarSuma) {
            if (posB == TAM_B) {
                printf("Consumidor %d --- suma contribuida: %d,posicion final %d\n", idC, sumaCons, posB);
                mostrarSuma = 0;
            }
        }
    }
    printf("Consumidor %d --- TERMINADO\n", idC);
    pthread_exit(NULL);//Finalizamos la ejecución del hilo productor
}

int main(int argc, char ** argv) {
    //asignacion de los sleeps
    if(argc<6){
        printf("Argumentos incorrectors. Insertar el tiempo de los 6 sleeps\n");
        exit(EXIT_FAILURE);
    }
    else{
        sleepProducir = atoi(argv[1]);
        sleepInsertar = atoi(argv[2]);
        sleepSumarP = atoi(argv[3]);
        sleepExtraer = atoi(argv[4]);
        sleepConsumir = atoi(argv[5]);
        sleepSumaC = atoi(argv[6]);
    }
    /*INICIALIZACION DE LOS ARRAYS*/
    for (int i = 0; i < TAM_A; i++) {
        A[i] = i;
    }
    for (int j = 0; j < TAM_B; j++) {
        B[j] = j;
    }
    //inicializamos el buffer
    inicializar();
    /*DECLARACIÓN DE IDENTIFICADORES DE HILOS*/
    pthread_t arrayProductor[P];//Declaramos el identificador del hilo productor
    pthread_t arrayConsumidor[C];//Declaramos el identificador del hilo consumidor

    /*CREACION MUTEX*/
    if (pthread_mutex_init(&mutex, 0) != 0) {
        printf("Error al crear mutex\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&mutexSumaProd, 0) != 0) {
        printf("Error al crear mutex\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&mutexSumaCons, 0) != 0) {
        printf("Error al crear mutex\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&condProd, 0) != 0) {
        printf("Error al crear variable de conducion productor.\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&condCons, 0) != 0) {
        printf("Error al crear variable de conducion consumidor.\n");
        exit(EXIT_FAILURE);
    }
    int argumentos[P];
    //Creacion productores
    for (int i = 0; i < P; i++) {
        argumentos[i] = i;
        pthread_create(&arrayProductor[i], NULL, productor, &argumentos[i]);
    }
    int argumentosC[C];
    //Creacion consumidores
    for (int j = 0; j < C; j++) {
        argumentosC[j] = j;
        pthread_create(&arrayConsumidor[j], NULL, consumidor, &argumentosC[j]);
    }

    /*ESPERAR A QUE LOS HILOS SE EJECUTEN*/
    for (int i = 0; i < P; i++) {
        pthread_join(arrayProductor[i], NULL);//Esperamos al productor
    }
    for (int i = 0; i < C; i++) {
        pthread_join(arrayConsumidor[i], NULL);//Esperamos al consumidor
    }


    /*ELIMINAR MUTEX Y VARIABLES DE CONDICION*/
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexSumaProd);
    pthread_mutex_destroy(&mutexSumaCons);
    pthread_cond_destroy(&condProd);
    pthread_cond_destroy(&condCons);
    return 0;
}









