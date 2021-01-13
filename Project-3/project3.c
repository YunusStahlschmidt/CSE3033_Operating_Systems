#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

pthread_mutex_t *bufManip; 

typedef struct publisher_type
{
    /*
     * type 
     * num of thread || array of threads
     * buffer of books -> in string
    */
    int type;
    int bookCount;
    long *threads_array;
    pthread_mutex_t *mutex;
    char **buffer;
}publisher_type;

typedef struct packager{
    /**
     * buffer of books
    */
    int number;
    char **package;
}packager;
// to pass publish book function as an arg
typedef struct book{
    int ptype;
    int pno;
    struct publisher_type *pblshr_strct;
    char **book_name; // book string booki_j
}book;

void *publish_book( void *arg){
    /**here the lock or semaphore will start. we need publisher type thread no and buffer
     * First check whehter the publisher_type buffer is intiliazed if not create it then put in it
     * if publisher buffer is full double the size
    */

    
    char book[20];
    //sprintf(book,"Book%d_%d", (int) arg->pno, (int) arg->ptype);
    printf("Publisher 1 of type 1\tBook_1 is published and put into the buffer 1\n");
    /*
    * if publishing done
    * Publisher 2 of type 1 Finished publishing 5 books. Exiting the system
    */
}

void *package_book(void *arg){
    printf("Packager 1\tPut Book2_1 into the package.\n");
    /*
    *   if packager is done -> Packager 3 There are no publishers left in the system. Only 1 of 6
    *   number of books could be packaged. The package
    *   contains Book2_15. Exiting the system.
    */
}


void *resize_buffer(void *arg){
    printf("Publisher 3 of type 1\tBuffer is full. Resizing the buffer.\n");
}

int main(int argc, char* argv[]) {
    if ( argc != 10 ){return 1;}
    
    int pubTypeCount = atoi(argv[2]), pubCount = atoi(argv[3]), packCount = atoi(argv[4]);
    bufManip = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t *) * pubTypeCount);
    publisher_type *publishers[pubTypeCount];

    int i, j , k;
    for (i = 1; i<=pubTypeCount; i++){
        publisher_type *publisher = (publisher_type *)malloc(sizeof(publisher_type));
        pthread_mutex_t mutex;
        bufManip[i-1] = mutex;
        publisher->mutex = &mutex;
        publishers[i-1] = publisher;
        publisher->threads_array = (long *)malloc(sizeof(long) * pubCount);
        publisher->bookCount = 0;
        for (j = 1; j <= pubCount; j++){
            book *my_arg = (book *)malloc(sizeof(book));
            my_arg->ptype = i;my_arg->pno = j;my_arg->pblshr_strct = publisher;
            pthread_t thread_no;
            pthread_create(&thread_no, NULL, publish_book, my_arg);
            publisher->threads_array[j-1] = thread_no;
        }
    }
    pthread_exit(0);
    return 0;

}
