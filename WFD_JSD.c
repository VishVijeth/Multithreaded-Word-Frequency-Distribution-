#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h> 
#include <limits.h>
#include <assert.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "WFD_JSD.h"

int lengthOflist(Node *front){
    Node *ptr = front ;
    int count = 0;
    while(ptr != NULL) {
        ptr = ptr->next;
        count++;
    }
    return count;
}
//THIS IS FOR TESTING ONLY!!!!!!!!!!!!!!!!!!!
void printList(Node *front){
    
    Node *ptr = front ;
    
    int length = lengthOflist(front); 
    printf("Length of List = %d , List ->", length);
    while(ptr != NULL){
          printf(" %s" , ptr->word);
          printf(" %lf", ptr->freq);
	   ptr = ptr->next;
    }
    printf("\n");
    
}

int present(Node *front , char* word){
    
    Node *ptr = front;
    if (front == NULL) return 0;
     while(ptr != NULL){
        if(strcmp(ptr->word, word) == 0){
            ptr->count++;
            return 1;
        }
        ptr = ptr->next;
    }
    
    return 0;
}

double howMany(Node *head){
    Node* ptr = head;
    double counter = 0;
    while(ptr != NULL){
        counter += ptr->count;
        ptr = ptr->next;
    }
    return counter;
}

void populateFreq(Node * head){
    Node* ptr = head;
    double holder = howMany(head);
    while (ptr != NULL){
        ptr->freq = (double)(ptr->count/holder);
        ptr = ptr->next;
    }
}

Node* create(){
    Node *node = (Node*)malloc(sizeof(Node));
    node->count = 1;
    node->freq = 0.0;
    node->next = NULL;
    return node;
}
void freeList(Node * front){
    Node * temp;
    while(front != NULL){
        temp = front;
        front = front->next;
        temp->next = NULL;
        if (temp->word != NULL) free(temp->word);
        free(temp);
    }
}

Node* add(Node * head, int length, char * word){
        Node* t = create();
        t->word = (char*)malloc(sizeof(char)*length);
        for(int i = 0; i < length-1; i++){
                t->word[i] = word[i];
        }
        
        t->word[length-1] = '\0';
        Node* temp = head;
        Node* follow = NULL;
        if (head == NULL){
            return t;
        }
        else{
            while(temp != NULL && strcmp(temp->word,t->word) < 0){
                follow = temp;
                temp = temp->next;
        }
        if(follow == NULL) {
            t->next = temp;
            return t;
        }
        else{
            follow->next = t;
            t->next = temp;
            return head;
        }  
    }
}

Node* WFD(char* name){
    
    int input_fd = open(name, O_RDONLY);
    if(input_fd == -1){ 
        perror(name);
        return NULL;
    }

    Node* head = NULL;
    char buffer[64];
    int bytes_read = 0;
    int ranonce = 0;
    int width = 64;
    char* word = (char*) malloc(width*sizeof(char));
    int wordlen = 0;
    int fword = 0;
    //read in file
    while((bytes_read = read(input_fd, buffer, 62)) > 0){
        for(int i = 0; i < bytes_read; i++){ //go through the buffer
            if(isspace(buffer[i])){
                if(ranonce > 0 && wordlen > 0){
                    word[wordlen] = '\0';
                    if(present(head,word) == 0){
                        wordlen+=1;
                        head = add(head,wordlen, word);
                    }
                    wordlen = 0;   
                    fword++;
                }
            }
            else{
                if(isdigit(buffer[i]) != 0 || isalpha(buffer[i]) != 0 || buffer[i] == '-'){
                     if (wordlen+1 > width){
                        char *h = realloc(word, sizeof(char) * 2 * width);
                        word = h;
                        width *=2;
                    }
                    if (isalpha(buffer[i]) != 0) {
                            word[wordlen] = tolower(buffer[i]);
                    }
                    else word[wordlen] = buffer[i];
                    wordlen++;
                    ranonce++;
                    
                }
            }
        }
    }
    if(ranonce > 0 && wordlen > 0){
                    if(present(head,word) == 0){
                        word[wordlen] = '\0';
                        wordlen+=1;
                        head = add(head,wordlen, word);
                    }
                    wordlen = 0;   
                    fword++;
                }
    free(word);
    close(input_fd);
    populateFreq(head);
    return head;
}

double getFreq(Node* head, char* word){

    Node* ptr = head;

    while(ptr){
        if(strcmp(ptr->word, word) == 0){
            return ptr->freq;
        }
        ptr = ptr->next;
    }
    return 0;
}

double KLD(Node* first, Node* second){

    Node* ptr = first;
    double KLD = 0;

    while(ptr){
        double f1 = ptr->freq;
        double f2 = getFreq(second, ptr->word);
        double avgf = (f1 + f2)/2;
        double temp = f1*log(f1/avgf)/log(2);
        KLD += temp;
        ptr = ptr->next;
    }

    return KLD;
}

double JSD(Node* first, Node* second){

    double KLD1 = KLD(first, second);
    double KLD2 = KLD(second, first);
    double add = (KLD1+KLD2);
    add = add/2;
    double result = sqrt(add);
    return result;
}