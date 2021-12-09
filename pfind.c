#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <dirent.h>

#define SUCCESFULL 0
#define PROBELM -1

/* -----------------------------------------------------------------------------------------------
----------------------------------    Queue Related Functions    ---------------------------------
-------------------------------------------------------------------------------------------------*/

/*A struct to represent a node in the queue
Fields:
1) char* dir_name
2) Node* next
*/
typedef struct Node {
    struct Node* next;
    char* dir_name;
} Node;

/*A function that initializes a node in the queue with dir_name value*/
Node* init_Node (char* dir_name){
    Node* node = calloc(1 , sizeof(Node));
    node -> next = NULL;
    node ->dir_name = dir_name;
    return node;
}

/*A struct to represent a Queue
Fields:
1) int len
2) Node* head
3) Node* tail
*/
typedef struct Queue {
    int len;
    Node* head;
    Node* tail;
} Queue;

/*A function that initializes a empty queue*/
Queue init_Queue(){
    Queue* queue = calloc(1,sizeof(Queue));
    queue->len = 0;
    queue->head = NULL;
    queue->tail = NULL;
    return *queue;
}

/*Returns 1 if and only if the queue is empty (e.i. it's length is 0) */  
int is_empty(Queue* q){
    return ((q->len)==0);
    //todo: Add locks to avoid collisions (Changes in queue in the middle of is_empty)

}

/*given a dir_name to enter to queue, the function inserts it as the last element and changes pointers accordingly*/
void insert (Queue* q ,char* dir_name){
    Node* node = init_Node(dir_name);
    if (q->len == 0){
        q->head = node;
        q->tail = node;
    }
    else{
        q->tail->next = node;
        q->tail = node;
    }
    q->len ++;
    //todo: Lock neccessary parts to avoid collisions
}

/*Returns the first dir_name in the queue (by FIFO order) and removes it from queue*/
char* pull(Queue* q ){
    
    //assert it is not empty for debugging purposes
    assert(!is_empty(q));

    //git the first one
    Node* node = q->head;
    
    //copy the dir_name content
    char* dir_name = malloc(strlen(node->dir_name));
    strcpy(dir_name,node->dir_name);

    //remove the first element and free it
    q->head = node->next;
    q->len--;
    free(node);
    return dir_name;
}

/*Iterate throgh the queue and print it element by element*/
void print_queue(Queue* q){
    int l = q->len;
    Node* node = q->head;
    printf ("[");
    for (int i=0;i<l;i++){
        if (i!= l-1){
            printf("%s,",node->dir_name);
        }
        else{
            printf("%s]\n",node->dir_name);
        }
        
        node = node->next;
    }
}

/*Given a message to print the function prints it and then frees the message*/
void print_message (char* message){
    printf("%s\n",message);
    free(message);
}

/* -----------------------------------------------------------------------------------------------
-----------------------------    Main Related Functions    ---------------------------------------
-------------------------------------------------------------------------------------------------*/

/*Given a string s checks if contains only digits and then converts it to integer
    if the string contains non-digit charachters , the function returns -1.
    otherwise, the function returns an integer representing the converted number.
    */
int string_to_num(char* s){
    
    int i;
    char c;
    int l = strlen(s);
    for (i=0; i<l ;i++){
        c = s[i];
        if (c<'0' || c >'9'){
            return -1;
        }
    }
    return atoi(s);
}


/* -----------------------------------------------------------------------------------------------
-----------------------------    Directory search functions   ------------------------------------
-------------------------------------------------------------------------------------------------*/
/*The function returns 1 if and only if term is contained in name and 0 otherwise*/
int name_contains_term(char* name, char* term){
    return strstr(name,term)!=NULL;
}


int search_directory(char* dir_name,char* term){
    DIR* dir = opendir(dir_name);
    if (dir==NULL){
        return 1;
    }
    
    struct dirent* x; //could be either a file or a folder
    x = readdir(dir);
    while (x!=NULL){
        if (name_contains_term(x->d_name,term)){
            printf("%s\n",x->d_name);
        }
        x = readdir(dir);
    }

    closedir(dir);
    return 0;
}

/* -----------------------------------------------------------------------------------------------
----------------------------------------    MAIN    ----------------------------------------------
-------------------------------------------------------------------------------------------------*/

int main (int argc, char* argv[]){

    // Firstly: we will parse the arguments given a check wheter they are valid
    if (argc != 4) {
        fprintf(stderr,"Error: Too many or not enough arguments given to program. please check and retry.\n");
        return 1;
    }
    char* root_directory = argv[1];
    char* term = argv[2];
    int n = string_to_num(argv[3]); //num of threads to create

    if (n <= 0){
        fprintf(stderr, "Error: number of threds entered (i.e third argument) is invalid.\n");
        return 1;
    }
    
    //todo: assert that root directory can be searched
    return search_directory(root_directory,term);
}