#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#define SUCCESFULL 0
#define PROBELM -1

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

void basic_queue_test(){
    Queue q = init_Queue();
    printf("Is empty : %s (should be false)\n",is_empty(&q)? "true" : "false" );
    insert(&q, "Tomer");
    insert(&q, "Efrat");
    insert(&q, "Niv");
    insert(&q, "Hava");
    insert(&q, "David");
    print_queue(&q);
    printf("Is empty : %s (should be true)\n",is_empty(&q)? "true" : "false" );
    printf("Len is: %d (should be 5)\n",q.len);
    print_message(pull(&q));
    print_message(pull(&q));
    print_message(pull(&q));
    printf("Len is: %d (should be 2)\n",q.len);
    print_message(pull(&q));
    print_message(pull(&q));
    printf("Len is: %d (should be 0)\n",q.len);
}

int main(){
    basic_queue_test();
}