#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdatomic.h>
#define SUCCESFULL 0
#define PROBELM -1






//------------------------------
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
    char dir_name [PATH_MAX];
} Node;

/*A function that initializes a node in the queue with dir_name value*/
Node* init_Node (char* dir_name){
    Node* node = calloc(1 , sizeof(Node));
    strcpy(&(node->dir_name[0]),dir_name);
    node -> next = NULL;
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
            printf("%s",node->dir_name);
        }
        node = node->next;
    }
    printf("]\n");
}

/*Given a message to print the function prints it and then frees the message*/
void print_message (char* message){
    printf("%s\n",message);
    free(message);
}


// ------------------------------
//         Static Variables

//The queue that holds all the directories need to be search 
static Queue directory_queue;

//make it atomic to assure that incrementing the counter would be atomic (will assure that will be incremented properly)
static atomic_int counter = 0;


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

char* extend_path (char* original_path,char* extention){
    char* exteneded_path = calloc(1,PATH_MAX);
    strcat(exteneded_path,original_path);
    strcat(exteneded_path,"/");
    strcat(exteneded_path,extention);
    return exteneded_path;
}


int search_directory(char* dir_name,char* term){
    struct stat info; // A struct to hold some data about file (linux fs api)
    char* exteneded_path; // A string to hold the concatenated path
    struct dirent* x; //could be either a file or a folder
    if (access(dir_name,R_OK) != 0 || access(dir_name,X_OK) != 0){
        printf("Directory %s: Permission denied.\n", dir_name);
        return 1;
    }
    DIR* dir = opendir(dir_name);
    if (dir==NULL){
        printf("Directory %s: Permission denied.\n", dir_name);
        return 1;
    }
    
    x = readdir(dir);
    while (x!=NULL){
        
        //todo: remove .git from here
        if (strcmp(x->d_name,".")==0 || strcmp(x->d_name,"..")==0 || strcmp(x->d_name ,".git")==0){
            //case it is . or .. ignore it
            x = readdir(dir);
            continue;
        }
        
        exteneded_path = extend_path(dir_name, x->d_name);

        if (name_contains_term(x->d_name,term)){
            //if the filename contains the term -> print it and increment the counter
            printf("%s\n",exteneded_path);
            //todo: make sure it is done atomicly
            counter ++;
        }
        if (stat(exteneded_path,&info) < 0){
            fprintf(stderr,"Error: can't get stat of directory entry for %s\n",exteneded_path);
            return PROBELM;
        }
        if (S_ISDIR(info.st_mode)){
            //If it is a directory add it to the queue
            insert(&directory_queue , exteneded_path);
        }

        free(exteneded_path);
        x = readdir(dir);
    }
    closedir(dir);
    return 0;
}

void work(char* term){
    char* dir;
    while (!is_empty(&directory_queue)){
        dir = pull(&directory_queue);
        search_directory(dir,term);
    }
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
        fprintf(stderr, "Error: number of threads entered (i.e third argument) is invalid.\n");
        return 1;
    }
    
    directory_queue = init_Queue();
    insert(&directory_queue, root_directory);
    work (term);
    printf("Done searching, found %d files\n", counter);
    //todo: assert that root directory can be searched
    return SUCCESFULL;


}