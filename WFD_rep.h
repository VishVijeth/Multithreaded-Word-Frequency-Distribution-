#pragma once

typedef struct Allf{
    struct Node* ll;
    char* name;
    int wordcount;
    struct Allf* next;
}Allf;

typedef struct rep_t{
    struct Allf* head;
    unsigned length;
    pthread_mutex_t lock;
}rep_t;

int rep_init(rep_t* R);
Allf* createR();
void addRep(rep_t* rep, Node* llhead, char* name);
int destroyRep(rep_t *Q);
//void computeJSD(Allf* head);
void clearRep(Allf * head);