#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "WFD_JSD.h"
#include "WFD_rep.h"

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

int rep_init(rep_t* R){
	R->head = NULL;
    R->length = 0;
	pthread_mutex_init(&R->lock, NULL);
	return 0;
}

Allf* createR(){
    Allf *node = (Allf*)malloc(sizeof(Allf));
    node->wordcount = 0;
    node->ll = NULL;
    node->name = NULL;
    node->next = NULL;
    return node;
}

int lengthRep(Allf* front){
    Allf* ptr = front;
    int count = 0;
    while(ptr != NULL) {
        ptr = ptr->next;
        count++;
    }
    return count;
}

//THIS IS FOR TESTING ONLY!!!!!!!!!!!!!!!!!!!
void printRep(rep_t* rep){

    Allf *ptr = rep->head;
    int length = rep->length;
    //int length = lengthRep(front);
    printf("Length of Rep = %d \n", length);

    while(ptr != NULL){
       printf("Name of the file = %s, ",ptr->name);
       printf("Wordcount = %d, ",ptr->wordcount);
       printList(ptr->ll);
	   ptr = ptr->next;
    }

    printf("\n");
}

void addRep(rep_t* rep, Node* llhead, char* name){
    
    lock(&rep->lock);

    Allf* temp = createR();
    temp->wordcount = howMany(llhead);
    temp->ll = llhead;

    int namelen = strlen(name)+1;
    temp->name = (char*)malloc(sizeof(char)*namelen); 
    memmove(temp->name, name, strlen(name));
    temp->name[namelen-1] = '\0';
   
    Allf* first = rep->head;
    Allf* follow = NULL;
    
    if(rep->head == NULL){ 
        rep->head = temp;
    }
    else{
        while(first != NULL && (temp->wordcount > first->wordcount)){
            follow = first;
            first = first->next;       
        }
        // If the current insertion has the lowest wordcount
        if(follow == NULL){
            rep->head = temp;
            temp->next = first;
        }
        else{
            follow->next = temp;
            temp->next = first;
        }
    }

    rep->length++;
    unlock(&rep->lock);
    return;
}

int destroyRep(rep_t *Q){
	pthread_mutex_destroy(&Q->lock);
	return 0;
}

// Single thread solution: (Just in case!)
/*
void computeJSD( Allf* head){
Allf* traverse = head;
Allf* traverse2 = NULL;
targs * holder = NULL;
while(traverse != NULL){
   traverse2 = traverse->next;
   while (traverse2 != NULL){
       {
           holder->first = traverse->ll;
           holder->second = traverse2->ll;
            printf("%lf ", JSD(holder));
            printf("%s %s\n", traverse->name,traverse2->name);
            traverse2 = traverse2->next;
       }
       traverse = traverse->next;
    }
}
}
*/

void clearRep(Allf* head){
    Allf* traverse = head;
    while (traverse != NULL){
        Allf* holder = traverse;
        traverse = traverse->next;
        holder->next = NULL;
        freeList(holder->ll);
        free(holder->name);
        free(holder);
    }
}