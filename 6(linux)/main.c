#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#define LEFT (i + N - 1) % N
#define RIGHT (i + 1) % N
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define MAX_TIME 5

int N;
sem_t *mutex = NULL;
sem_t *eaters = NULL;
int* state = NULL;


void eat(int i) {
    int time = rand() % MAX_TIME;
    printf("Philosopher #%d is eating...\n", i + 1);
    sleep(time);
    printf("Philosopher #%d stopped eating...\n", i + 1);
}


void think(int i) {
    int time = rand() % MAX_TIME;
    printf("Philosopher #%d is thinking...\n", i + 1);
    sleep(time);
}


void test(int i) {
    if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[i] = EATING;
        sem_post(&eaters[i]);
    }
}


void take_sticks(int i) {
    sem_wait(mutex);
    state[i] = HUNGRY;
    test(i);
    sem_post(mutex);
    sem_wait(&eaters[i]);
}

void put_sticks(int i) {
    sem_wait(mutex);
    state[i] = THINKING;
    test(LEFT);
    test(RIGHT);
    sem_post(mutex);
}


void* philosopher(void* arg) {
    int i = *((int*)arg);
    while (1 == 1) { //бесконечный цикл
        think(i); //сначала философ думает
        take_sticks(i); //затем берёт палочки (или, если быть точнее, пытается взять)
        eat(i); //ест
        put_sticks(i); //кладёт палочки обратно
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <NUMBER_OF_PHILOSOPHERS>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);
    if (N <= 1) {
        fprintf(stderr, "Error by transformation of the argument...\n");
        return 2;
    }

    mutex = (sem_t*)malloc(sizeof(sem_t));
    if (sem_init(mutex, 0, 1) != 0) {
        fprintf(stderr, "Error by creating semaphore...\n");
        return 3;
    }

    eaters = (sem_t*)calloc(N, sizeof(sem_t));
    state = (int*)calloc(N, sizeof(int));

    memset(state, 0, N);

    srand(time(NULL));
    pthread_t *philosophers = (pthread_t*)malloc(N * sizeof(pthread_t));

    int i;
    for (i = 0; i < N; i++) {
        if (sem_init(&eaters[i], 0, 0) != 0) {
            fprintf(stderr, "Error by creating semaphore...\n");
            return 3;
        }
    }

    int *t = (int *)malloc(sizeof(int));
    for (i = 0; i < N; i++) {
        *t = i;
        if (pthread_create(&philosophers[i], NULL, philosopher, t) != 0) {
            fprintf(stderr, "Error by creating thread\n");
            return 2;
        }
        usleep(100000);
    }

    void* result;
    for (i = 0; i < N; i++) {
        if (philosophers[i] != -1) {
            if (pthread_join(philosophers[i], &result) != 0) {
                fprintf(stderr, "Error by joining thread\n");
                return 3;
            }
        }
    }
    return 0;
}
