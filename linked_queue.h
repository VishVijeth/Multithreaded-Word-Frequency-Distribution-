#pragma once

typedef struct QNode{
    char* path;
    struct QNode* next;
}QNode;

typedef struct {
    QNode* front;
    QNode* rear;
	unsigned count;
	unsigned active;
	pthread_mutex_t lock;
	pthread_cond_t read_ready;
} queue_t;

typedef struct len_t{
    int length;
    pthread_mutex_t lock;
}len_t;

int init(queue_t *Q);
int destroy(queue_t *Q);
int enqueue(queue_t *Q,char* word);
char* dequeue(queue_t *Q);
int qclose(queue_t *Q);
int take(len_t* num);
int len_init(len_t *Q, int length);
int destroylen(len_t *Q);