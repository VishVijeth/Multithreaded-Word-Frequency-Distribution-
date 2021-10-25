#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "linked_queue.h"

#define lock(X) \
do { \
    int err = pthread_mutex_lock(X); \
    if (err) { \
        errno = err; \
        perror("lock"); \
        abort(); \
        } \
}while(0) \

#define unlock(X) \
do { \
    int err = pthread_mutex_unlock(X); \
    if (err) { \
        errno = err; \
        perror("unlock"); \
        abort(); \
        } \
}while(0) \

int init(queue_t *Q){
	Q->front = NULL;
    Q->rear = NULL;
	Q->count = 0;
	Q->active = 0;
	pthread_mutex_init(&Q->lock, NULL);
	pthread_cond_init(&Q->read_ready, NULL);
	return 0;
}

int destroy(queue_t *Q){
	pthread_mutex_destroy(&Q->lock);
	pthread_cond_destroy(&Q->read_ready);
	return 0;
}

int enqueue(queue_t *Q, char* word){
	
	lock(&Q->lock);

	QNode* temp = (QNode*)malloc(sizeof(QNode));
	temp->next = NULL;

    int length = strlen(word)+1;
    temp->path = (char*)malloc(sizeof(char)*length);
	memmove(temp->path, word, strlen(word));
    temp->path[length-1] = '\0';

	// If queue is empty
    if(Q->rear == NULL){
        Q->front = Q->rear = temp;
    }
	else{
		Q->rear->next = temp;
    	Q->rear = temp;
	}
	
    ++Q->count;

	unlock(&Q->lock);
	pthread_cond_signal(&Q->read_ready);
	return 0;
}

char* dequeue(queue_t* Q){

	lock(&Q->lock);
	
	if(Q->count == 0) {
		
		--Q->active;

		if(Q->active == 0){
			unlock(&Q->lock);
			pthread_cond_broadcast(&Q->read_ready);
			return NULL;
		}
		while(Q->count == 0 && Q->active > 0){
			pthread_cond_wait(&Q->read_ready, &Q->lock);
		}
		if(Q->count == 0){
			unlock(&Q->lock);
			return NULL;
		}
		++Q->active;
	}

    if(Q->front == NULL){
		unlock(&Q->lock);
        return NULL;
    }

	//Perform the dequeue
    QNode* temp = Q->front;
    Q->front = Q->front->next;

    if(Q->front == NULL){
        Q->rear = NULL;
    }

	int length = strlen(temp->path)+1;
	char* deqRes = malloc(length*sizeof(char));
	memmove(deqRes,temp->path,strlen(temp->path));
	deqRes[length-1] = '\0';
	
	--Q->count;
	free(temp->path);
    free(temp);

	/*
	if(Q->count == 0){
		Q->front = NULL;
		Q->rear = NULL;
	}
	*/
	pthread_mutex_unlock(&Q->lock);
	return deqRes;
}

int qclose(queue_t *Q){
	pthread_mutex_lock(&Q->lock);
	pthread_cond_broadcast(&Q->read_ready);
	pthread_mutex_unlock(&Q->lock);	
	return 0;
}

int take(len_t* num){
    lock(&num->lock);
    int holder = num->length;
    num->length--;
    unlock(&num->lock);
    return holder;
}

int len_init(len_t *Q, int length){
	pthread_mutex_init(&Q->lock, NULL);
    Q->length = length;
	return 0;
}

int destroylen(len_t *Q){
	pthread_mutex_destroy(&Q->lock);
	return 0;
}