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
#include <errno.h>
#include <pthread.h>
#include "WFD_JSD.h"
#include "WFD_rep.h"
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

typedef struct Dargs{
    queue_t* dir_q;
    queue_t* file_q;
    char* suffix;
}Dargs;

typedef struct Fargs{
    queue_t* file_q;
    queue_t* dir_q;
    rep_t* rep;
}Fargs;

typedef struct Aargs{
    rep_t * rep;
    len_t * num;
    double** storage;
}Aargs;

Allf* findinrep(Allf * head, int num){
        int trav1 = num;
        Allf* trav11 = head;

        while(trav1 > 0){
            trav1--;
            trav11 = trav11->next;
        }
    return trav11;
}

int isdir(char *name) {
    struct stat data;

    int err = stat(name, &data);
    
    // should confirm err == 0
    if (err) {
        perror(name);  // print error message
        return 0;
    }
    if (S_ISDIR(data.st_mode)) {// S_ISDIR macro is true if the st_mode says the file is a directory
        return 1;
    } 
    return 0;
}

int isreg(char *name){
    struct stat data;
    
    int err = stat(name, &data);
    
    // should confirm err == 0
    if (err) {
        perror(name);  // print error message
        return 0;
    }
    
    if (S_ISREG(data.st_mode)) { // S_ISREG macro is true if the st_mode says the file is a regular file
        return 1;
    } 
    return 0;
}

// Traversing through the directory queue and to update the file and directory queues
void* traverseDir(void* A){
    Dargs* args = A;
    while(args->dir_q->count > 0 || args->dir_q->active > 0){
        char* name = dequeue(args->dir_q);
        DIR* dirptr = opendir(name);
        struct dirent *de;
        if (dirptr == NULL){
            perror("cannot open directory");
            continue;
        }
        while((de = readdir(dirptr))){ 
            if(de->d_name[0] == '.'){  //If the filename starts with a .
                continue;
            }
            int length = strlen(name)+strlen(de->d_name)+2;
            char* path = malloc(length*sizeof(char));
            
            memmove(path, name, strlen(name)); // concat current Direcotry
            memmove(&path[strlen(name)], "/", 1); // concat "/"
            memmove(&path[strlen(name)+1],de->d_name,strlen(de->d_name)); // concat new directory
            path[length-1] = '\0'; // null terminator
    
            if(isdir(path)){
                enqueue(args->dir_q, path);
            }
            else if(isreg(path)){
                
                if(strlen(de->d_name) >= strlen(args->suffix)){

                    int i = strlen(args->suffix)-1;
                    int j = strlen(de->d_name)-1;

                    while(i >= 0){
                        if(args->suffix[i] != de->d_name[j]){
                            break;
                        }
                        i--;
                        j--;
                    }

                    if(i == -1){
                        enqueue(args->file_q, path);
                    }
                }
            }
            free(path);
        }
        int a =closedir(dirptr);
        if (a == -1 ) {
            perror("failure to close directory");
        } 
        free(name); 
    } 
    return NULL;
}

void* traverseFile(void* A){

    Fargs* args = A;
    while(args->file_q->count > 0 || args->dir_q->count > 0 || args->dir_q->active > 0){
        char* name = dequeue(args->file_q);
        Node* WFDres = WFD(name);
        addRep(args->rep, WFDres, name);
        free(name);
    }
    return NULL;
}

void* largeCompute(void* A){
    Aargs* args = A;
    while (args->num->length != -1){

        int spot = take(args->num);
        int wordspot1 = (int)args->storage[spot][1];
        int wordspot2 = (int)args->storage[spot][2];

        Allf* trav1 = findinrep(args->rep->head, wordspot1);
        Allf* trav2 = findinrep(args->rep->head, wordspot2);

        int tot = trav1->wordcount + trav2->wordcount; 
        args->storage[spot][0] = tot;
        args->storage[spot][3] = JSD(trav1->ll,trav2->ll);
    }
    return NULL;
}

// Compare function that coudlve been used for qsort
/*
int cmpfunc(const void *pa, const void *pb) {
   double* a = *(double**)pa;
   double* b = *(double**)pa;
   return (a[0] > b[0]) - (a[0] < b[0]);
}
*/

int main(int argc, char**argv){
    
    queue_t file_q;
    queue_t dir_q;
    rep_t rep_1;

    init(&file_q);
    init(&dir_q);
    rep_init(&rep_1);

    int d_threads = 1;
    int f_threads = 1;
    int a_threads = 1;

    char* suffix = (char*)malloc(sizeof(char)*5);
    strcpy(suffix, ".txt");
    suffix[4] = '\0';
    int error = 0;
    
    for(int i = 1; i < argc; i++){
        if((strlen(argv[i]) >=2) && argv[i][0] == '-'){
            if(argv[i][1] == 'd'){
                int holder = (strlen(argv[i]));
                holder = holder-2;
                char * num = (char*)malloc(sizeof(char)*holder+1);
                if(holder == 0) {
                    perror("Error: Invalid/Missing arguments");
                    return EXIT_FAILURE;
                }
                else{
                    for(int j = 0; j < holder; j++){
                        num[j] = argv[i][j+2];
                    }
                }
                num[holder] = '\0';
                    d_threads = atoi(num);
                    if (d_threads <= 0) {
                        perror("Error: Invalid/Missing arguments");
                        return EXIT_FAILURE;
                    };
                    free(num);
            }
            else if(argv[i][1] == 'f'){
                int holder = (strlen(argv[i]));
                holder = holder-2;
                char* num = (char*)malloc(sizeof(char)*holder+1);
                if(holder == 0){
                    free(suffix);
                    perror("Error: Invalid/Missing arguments");
                    return EXIT_FAILURE;
                }
                else{
                    for(int j = 0; j < holder; j++){
                        num[j] = argv[i][j+2];
                    }
                }
                num[holder] = '\0';
                f_threads = atoi(num);
                if(f_threads <= 0){
                    perror("Error: Invalid/Missing arguments");
                    return EXIT_FAILURE;
                }
                free(num);
            }
            else if(argv[i][1] == 'a'){
                int holder = (strlen(argv[i]));
                holder = holder-2;
                char * num = (char*)malloc(sizeof(char)*holder+1);
                if(holder == 0) {
                    perror("Error: Invalid/Missing arguments");
                    free(num);
                    return EXIT_FAILURE;
                }
                else{
                    for(int j = 0; j < holder; j++){
                        num[j] = argv[i][j+2];
                    }
                }
                num[holder] = '\0';
                a_threads = atoi(num);
                if(a_threads <= 0){
                    perror("Error: arg too small");
                    return EXIT_FAILURE;
                }
                free(num);
            }
            else if(argv[i][1] == 's'){
                int holder = strlen(argv[i]);
                holder = holder - 2;
                if(holder == 0){ 
                    char *h = realloc(suffix, sizeof(char)*1);
                    suffix = h;
                    suffix[0] = '\0';
                }
                else{
                    char *h = realloc(suffix,sizeof(char)*holder+1);
                    suffix = h;
                    for(int j = 0; j < holder; j++){
                        suffix[j] = argv[i][j+2];
                    }
                    suffix[holder] = '\0';
                }
            }
            else{
                error++;
            }
        }
        else if(isdir(argv[i])){
            enqueue(&dir_q, argv[i]);
        }
        else if(isreg(argv[i])){
            enqueue(&file_q, argv[i]);
        }
    }
    
    // Threading Directory Queue traversing
    Dargs* d_args = malloc(d_threads*sizeof(Dargs));
    pthread_t* d_tids = malloc(d_threads * sizeof(pthread_t));

    for(int i = 0; i < d_threads; i++){
        d_args[i].file_q = &file_q;
        d_args[i].dir_q = &dir_q;
        d_args[i].suffix = suffix;
        int e = pthread_create(&d_tids[i], NULL, traverseDir, &d_args[i]);
        if(e != 0){
            errno = e;
            perror("Cannot create Directory Thread");
            abort();
        }
    }
    for(int i = 0; i < d_threads; i++){
        pthread_join(d_tids[i], NULL);
    }

    // Threading File Queue traversing
    Fargs* f_args = malloc(f_threads*sizeof(Fargs));
    pthread_t* f_tids = malloc(f_threads * sizeof(pthread_t));
    
    for(int i = 0; i < f_threads; i++){
        f_args[i].file_q = &file_q;
        f_args[i].dir_q = &dir_q;
        f_args[i].rep = &rep_1;
        int e = pthread_create(&f_tids[i], NULL, traverseFile, &f_args[i]);
        if(e != 0){
            errno = e;
            perror("Cannot create File Thread");
            abort();
        }
    }

    for(int i = 0; i < f_threads; i++){
        pthread_join(f_tids[i], NULL);
    }

    if(rep_1.length < 2){
        perror("Not enough items in repo");
        destroy(&dir_q);
        destroy(&file_q);
        free(suffix);
        free(d_args);
	    free(d_tids);
        free(f_args);
	    free(f_tids);
        clearRep(rep_1.head);
        destroyRep(&rep_1);
        return EXIT_FAILURE;
    }

    int numcomp = rep_1.length;
    int arrsize = (numcomp*(numcomp-1))/2; // will always be even so we shillin
    double** wordstore = (double**)malloc(sizeof(double*) * arrsize);
    for (int i = 0; i < arrsize; i++){
        wordstore[i] = (double*)calloc(4,sizeof(double));
    }

    int total = 0;
    for(int i = 0; i < numcomp; i++){
        for(int j = i + 1; j < numcomp; j++){
            wordstore[total][1] = (double)i;
            wordstore[total][2] = (double)j;
            total++;
        }
    }

    len_t num_1;
    len_init(&num_1, arrsize-1);

    // Threading computing JSD
    Aargs* a_args = malloc(a_threads*sizeof(Aargs));
    pthread_t* a_tids = malloc(a_threads * sizeof(pthread_t));

    for(int i = 0; i < a_threads; i++){
        a_args[i].rep = &rep_1;
        a_args[i].num = &num_1;
        a_args[i].storage = wordstore;
        int e = pthread_create(&a_tids[i], NULL, largeCompute, &a_args[i]);
        if(e != 0){
            errno = e;
            perror("Cannot create Analysis Thread");
            abort();
        }
    }

    for (int i = 0; i < a_threads;i++){
        pthread_join(a_tids[i],NULL);
    }

    // Sorting the results based on descending word count
    for (int i = 0; i < arrsize; i++){
        for (int k =i+1; k < arrsize; k++){
            if (wordstore[i][0]<wordstore[k][0]){
                for (int j = 0; j < 4; j++){
                    double h1 = wordstore[i][j];
                    wordstore[i][j] = wordstore[k][j];
                    wordstore[k][j] = h1;
                }
            }
        }
    }

    // Could have also used qsort
    // qsort(wordstore, arrsize, sizeof(wordstore[0]), cmpfunc);
    
    // Printing the result
    for (int i = 0; i < arrsize; i++){
        Allf* Node1 = findinrep(rep_1.head,wordstore[i][1]); 
        Allf* Node2 = findinrep(rep_1.head,wordstore[i][2]); 
        printf("%lf %s %s\n", wordstore[i][3], Node1->name, Node2->name);
    }

    // Freeing memory for all the structs
    for(int i = 0; i < arrsize; i++){
        free(wordstore[i]);
    }

    destroy(&dir_q);
    destroy(&file_q);
    free(suffix);
    free(d_args);
	free(d_tids);
    free(f_args);
	free(f_tids);
    free(a_args);
    free(a_tids);
    destroylen(&num_1);
    clearRep(rep_1.head);
    destroyRep(&rep_1);
    free(wordstore);

    if(error != 0){
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}