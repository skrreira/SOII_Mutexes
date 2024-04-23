/*INCLUDES NECESARIOS*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>


//Constantes
#define P 5 // productores //cuando cambie a linea de comando usar reserva dinamica
#define C 2// consumidores
#define N 12 // tamano del buffer
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

/*CRACION MUTEX*/
pthread_mutex_t mutex;
pthread_mutex_t mutexSumaProd;
pthread_mutex_t mutexSumaCons;

int argumentos[P];

//--------------------FUNCIONES--------------------

//Funcion que inicializa el buffer a -1
void inicializar();

//Funcion que produce un item aleatorio
int produce_item(int i);
//Funcion que inserta un item en el buffer
void insert_item(int item);
//Funcion que consume un item del buffer
int consume_item();

//Funcion que suma los valores del array A
void sumaProductor(int id);
//Funcion que suma los valores del array B
void sumaConsumidor(int id);

//Funcion que duerme un proceso durante un tiempo
void dormirProceso(struct timespec *req);

//Funcion que define el comportamiento del productor
void *productor(void *arg);
//Funcion que define el comportamiento del consumidor
void *consumidor(void *argC);

//--------------------MAIN--------------------

int main(int argc, char ** argv) {
    //asignacion de los sleeps
    if(argc<6){
        printf("ERROR: Número de argumentos incorrecto. | Usage: %s sleep_produce sleep_insert sleep_sum_prod sleep_extract sleep_consume sleep_sum_cons\n.", argv[0]);
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


    /*ELIMINAR MUTEX */
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexSumaProd);
    pthread_mutex_destroy(&mutexSumaCons);
    return 0;
}

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

//TODO: Revisar el rand del productor y consumidor, prod = 18 cons hasta que acabe el prod

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
}

/*FUNCION PARA REALIZAR LA SUMA DE LOS CONSUMIDORES*/
void sumaConsumidor(int id) {
    int i;
    sleep(sleepSumaC);
    int numIt = rand();//numero de iteraciones que se van a realizar
    int it;
    if(posB+numIt >TAM_B)it = TAM_B;
    else it = posB+numIt;
    for (i = posB; i < it; i++) {
        sumaCons += B[i];
    }
    posB = i;
}

/*FUNCION PARA CODIGO OPTATIVO*/
/*FUNCION QUE DUERME UN PROCESO DURANTE UN TIEMPO*/
void dormirProceso(struct timespec *req) {
    nanosleep(req, NULL);
}

/*FUNCIÓN QUE DEFINE EL COMPORTAMIENTO DEL PRODUCTOR*/
void *productor(void *arg) {
    int i;
    int item = -1;
    int id = *(int *) arg;
    
    for (i = 0; i <ITEMS_BY_P || posA<TAM_A; i++) {
        if(i<ITEMS_BY_P){

            printf("Productor %d: Produciendo item %d\n", id, item);

            if (item == -1)item = produce_item(i);

            pthread_mutex_lock(&mutex); 
            
            while (posBuffer == N - 1){
                pthread_mutex_unlock(&mutex); 
                
                struct timespec sleep_time;
                sleep_time.tv_sec = 0;
                sleep_time.tv_nsec = 1000000; 

                dormirProceso(&sleep_time);
                
                pthread_mutex_lock(&mutex);
            }

            insert_item(item);
            printf("Productor %d: Agregando item %d al buffer\n", id, item);

            item = -1;

            pthread_mutex_unlock(&mutex);   

            printf("Productor %d: Tarea obligatoria realizada. Desbloqueamos mútex. Sumatorio de valores pares.\n", id);
        }

        if (!pthread_mutex_trylock(&mutexSumaProd)) {
           sumaProductor(id);
           pthread_mutex_unlock(&mutexSumaProd);
        }
    }

    printf("\n######## Productor %d: TERMINADO. Suma de pares completada: %d ########\n\n", id, sumaProd);
    pthread_exit(NULL);
}

/*FUNCIÓN QUE DEFINE EL COMPORTAMIENTO DEL CONSUMIDOR*/
void *consumidor(void *argC) {
    int i;
    int item;
    int idC = *(int *) argC;

    for (i = 0; i <ITERACIONES_CONSUMER || posB<TAM_B ; i++) {
        if(i<ITERACIONES_CONSUMER){
            printf("Consumidor %d: Extrayendo item %d del buffer\n", idC, item);

            pthread_mutex_lock(&mutex); 
            
            while (posBuffer == -1){
                pthread_mutex_unlock(&mutex); 

                struct timespec sleep_time;
                sleep_time.tv_sec = 0;
                sleep_time.tv_nsec = 1000000;  

                dormirProceso(&sleep_time);
                pthread_mutex_lock(&mutex);
            }
            
            item = consume_item();
            printf("Consumidor: %d Consumiendo item %d del buffer\n", idC, item);
            
            pthread_mutex_unlock(&mutex);
            printf("Consumidor %d: Tarea obligatoria realizada. Desbloqueamos mútex. Sumatorio de valores impares.\n", idC);
        }
        
        if (!pthread_mutex_trylock(&mutexSumaCons)) {
            sumaConsumidor(idC);
            pthread_mutex_unlock(&mutexSumaCons);
        }
    }

    printf("\n######## Consumidor %d: Suma de impares completada: %d ########\n\n", idC, sumaCons);
    pthread_exit(NULL);//Finalizamos la ejecución del hilo productor
}










