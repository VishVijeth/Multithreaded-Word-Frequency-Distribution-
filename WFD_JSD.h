#pragma once

typedef struct Node{
    char * word;
    int count;
    double freq;
	struct Node* next;
}Node;

typedef struct targs{
    Node* first;
    Node* second;
}targs;

int lengthOflist(Node *front);
void printList(Node *front);
int present(Node *front , char* word);
double howMany(Node *head);
void populateFreq(Node * head);
Node* create();
void freeList(Node * front);
Node* add(Node * head, int length, char * word);
Node* WFD(char* name);
double getFreq(Node* head, char* word);
double KLD(Node* first, Node* second);
double JSD(Node* first, Node* second);

