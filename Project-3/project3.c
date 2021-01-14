#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

pthread_mutex_t *bufManip; // array to hold mutex for each publisherType

typedef struct thread  // struct for publisher thread
{
    pthread_t tid;  // thread id
    int num_of_created_books;  // counter for how many books have been created
    int max_num_of_books;  // goal of book count for thread
}thread;

typedef struct publisher_type
{
    /*
     * type 
     * num of thread || array of threads
     * buffer of books -> in string
    */
    int type;
    int bookCount;
    int buffer_size;  // indiates the size of books buffer
    thread **threads_array;  // publisher threads for current publisher type
    pthread_mutex_t *mutex;  // mutex for this publisher type
    char **buffer;  // holds the pubilshed books for packagers
}publisher_type;

typedef struct packager{
    /**
     * buffer of books
    */
    pthread_t tid;  // thread id 
    int number;  // id of packager
    char **package;  // array of books in current package
}packager;

// to pass publish book function as an arg
typedef struct book{
    int ptype;  // publisher type
    int pno;  // publisher number
    publisher_type *pblshr_strct;  // publisher struct 
    char **book_name; // book string booki_j
}book;

void *publish_book( void *arg){  // function for publisher threads
    /**here the lock or semaphore will start. we need publisher type thread no and buffer
     * First check whehter the publisher_type buffer is intiliazed if not create it then put in it
     * if publisher buffer is full double the size
     * for loop burada olacak max number of books to create
    */
    book *my_arg = (book *) arg;  // get the arguments for the books to be created
    pthread_mutex_lock((bufManip + my_arg->ptype));
    if (my_arg->pblshr_strct->buffer == NULL){
        my_arg->pblshr_strct->buffer = malloc(sizeof(char *) * my_arg->pblshr_strct->buffer_size);
        printf("succesfully created buffer\n");
    }


    char book[20];
    sprintf(book,"Book%d_%d", (int) my_arg->pno, (int) my_arg->ptype);
    int index, exist_space = 0;
    for(index=0; index<sizeof(my_arg->pblshr_strct->buffer); index++){
        if (my_arg->pblshr_strct->buffer[index] == NULL){
            //my_arg->pblshr_strct->buffer[index] = book;
            exist_space = 1;
            break;
        }
    }
    if (!exist_space){
        //double the size of buffer
        double_buffer_size(my_arg);
    }
    my_arg->pblshr_strct->buffer[index] = book;
    printf("Publisher %d of type %d\t%s is published and put into the buffer %d\n", my_arg->pno, my_arg->ptype, book, my_arg->ptype);

    //if ()
    /*
    * if publishing done
    * Publisher 2 of type 1 Finished publishing 5 books. Exiting the system
    */
    pthread_mutex_unlock((bufManip + my_arg->ptype));

}

void *package_book(void *arg){  // function for the packager threads
    printf("Packager 1\tPut Book2_1 into the package.\n");
    /*
    *   if packager is done -> Packager 3 There are no publishers left in the system. Only 1 of 6
    *   number of books could be packaged. The package
    *   contains Book2_15. Exiting the system.
    */
}


void double_buffer_size(book *arg){  // double buffer size in case packagers are not fast enough 
    printf("Publisher %d of type %d\tBuffer is full. Resizing the buffer.\n", arg->pno, arg->ptype);
    arg->pblshr_strct->buffer_size = arg->pblshr_strct->buffer_size * 2;
    arg->pblshr_strct->buffer=realloc(arg->pblshr_strct->buffer, arg->pblshr_strct->buffer_size);
}

int main(int argc, char* argv[]) {
    if ( argc != 10 ){return 1;}
    
    int pubTypeCount = atoi(argv[2]), pubCount = atoi(argv[3]), packCount = atoi(argv[4]);
    bufManip = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t *) * pubTypeCount);
    publisher_type *publishers[pubTypeCount];
    /** May need to design for loop for publisher. Additional for loop to call publishbook func with the  given number */
    int i, j , k;
    for (i = 1; i<=pubTypeCount; i++){  // create required number of publisherTypes
        publisher_type *publisher = (publisher_type *)malloc(sizeof(publisher_type));
        pthread_mutex_t mutex;  // initialize a mutex for current publisherType
        bufManip[i-1] = mutex;  // set mutex to global buffer according to publisherType number
        publisher->mutex = &mutex;
        publishers[i-1] = publisher;
        publisher->threads_array = (long *)malloc(sizeof(long) * pubCount);  // set publisher threads array size of publisherType
        publisher->bookCount = 0;
        publisher->threads_array = (thread *) malloc(sizeof(thread) * pubCount);
        publisher->buffer_size = atoi(argv[7]);  // set initial size of publisherType buffer
        for (j = 1; j <= pubCount; j++){  // create publisher threads
            book *my_arg = (book *)malloc(sizeof(book));
            my_arg->ptype = i;my_arg->pno = j;my_arg->pblshr_strct = publisher;  // set args for books of this publisherType and publisher thread
            pthread_t thread_no;  // initialize new publisher thread
            thread *thread_strc = (thread *) malloc(sizeof(thread));
            thread_strc->tid = thread_no;
            thread_strc->num_of_created_books = 0;
            thread_strc->max_num_of_books = atoi(argv[5]);  // set how many books the publisher needs to publish
            publisher->threads_array[j-1] = thread_strc;  // add publisher thread to publisherType threads array
            pthread_create(&thread_no, NULL, publish_book, my_arg);  // start the newly created publisher thread
        }
    }
    pthread_exit(0);
    return 0;

}
