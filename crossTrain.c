#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


#define NUM_TRAINS 5

//Direcoes dos trens
#define DA1 0
#define DB1 1
#define DA2 2
#define DB2 3

// Definição de prioridades
#define ALTA 1
#define MEDIA 2
#define BAIXA 3

//Semaforo que controla o cruzamento
sem_t semCrossing;

//Varivaris para o auxilio de prioridades
int Hpriority = 0;
int Mpriority = 0;

// Representação dos trens
struct Train{
    int id;
    int priority; // Alta 1, Média 2 , Baixa 3;
    int direction; // A1 0 B1 1
};

void *train_thread(void *train_ptr){
    struct Train *train = (struct Train *)train_ptr;
    int nextDir;
    
    // Trem se aproximando
    printf("----------------- \n Trem %d (Prioridade %d) se aproximando do cruzamento vindo da direção %s.\n",
            train->id, train->priority, (train->direction == DA1) ? "A1" : "B1");
    
    sleep(rand() % 3);

    if (train->priority == ALTA) {
        sem_wait(&semCrossing);
        printf(" ----------------- \n Trem %d passando com prioridade alta.\n", train->id);
        Hpriority++;
    }
    else if (train->priority == MEDIA) {
        sem_wait(&semCrossing);
        while (Hpriority > 0) {
            sem_post(&semCrossing); // Libera e espera na fila até não haver trens de alta prioridade
            sem_wait(&semCrossing);
        }
        printf(" ----------------- \n Trem %d passando com prioridade média.\n", train->id);
        Mpriority++;
    }
    else if (train->priority == BAIXA) {
        sem_wait(&semCrossing);
        while (Hpriority > 0 || Mpriority > 0) {
            sem_post(&semCrossing); // Libera e espera na fila até não haver trens de alta ou média prioridade
            sem_wait(&semCrossing);
        }
        printf(" ----------------- \n Trem %d passando com prioridade baixa.\n", train->id);
    }

    // Direção que o trem vai seguir 
    nextDir = ((rand() % 2 == 0)? DA2 : DB2);

    // Atravessando o cruzamento 
    printf("------------------ \n Trem %d atravessando o cruzamento indo para %s. \n",
            train->id, (nextDir == DA2)? "A2": "B2");

    // Tempo para atravessar
    sleep(2);    
    
    // Libera o acesso
    printf("------------------ \n Trem %d liberou o cruzamento. \n", train->id);
    
    if (train->priority == ALTA) {
        Hpriority--;
    }
    else if (train->priority == MEDIA) {
        Mpriority--;
    }
    
    sem_post(&semCrossing);

    pthread_exit(NULL);
}

int main(){
    pthread_t trains[NUM_TRAINS]; // Array de IDs das threads
    struct Train train[NUM_TRAINS];

    srand(time(NULL));

    // Inicializa o semáforo - Disponível
    sem_init(&semCrossing, 0, 1);
    
    // Criando Trens - Threads
    for (int i = 0; i < NUM_TRAINS; i++){
        train[i].id = i + 1;
        train[i].priority = rand() % 3 + 1; // Prioridade entre 1 e 3
        train[i].direction = rand() % 2; // Direção A1 0, B1 1
        
        // Cria a thread
        pthread_create(&trains[i], NULL, train_thread, (void *)&train[i]);
        sleep(rand() % 3); // Intervalo aleatório entre a criação de trens
    }

    // Esperando as threads terminarem
    for (int i = 0; i < NUM_TRAINS; i++) {
        pthread_join(trains[i], NULL);
    }

    // Destruindo o semáforo para controlar o acesso ao cruzamento
    sem_destroy(&semCrossing);

    return 0;
}

