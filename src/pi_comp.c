#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#include "my_rand.h"
#include "timer.h"


//Global variables.
long long int throw_count;
int num_of_threads;
_Atomic long long int circle_hits = 0;
double avg_thread_time = 0;

void* pi_thread_estimation(void* thread_id){
    double start;
    double finish;
    
    long tid = (long) thread_id;
    long local_throw_count = (throw_count / num_of_threads);

    if(tid == 0){
        local_throw_count += throw_count % num_of_threads;
    }
    long long int local_circle_hits = 0;

    GET_TIME(start);
    for(long long i = 0; i < local_throw_count; i++){
        unsigned int seed = time(NULL) + i + tid;
        double x_rand = get_rand(&seed);
        double y_rand = get_rand(&seed);
        double dist_square = x_rand * x_rand + y_rand * y_rand;
        if(dist_square <= 1){
            local_circle_hits++;
        }
    }
    GET_TIME(finish);
    avg_thread_time += finish - start;


    //Using atoimic operations to write in the critical segment.


    circle_hits += local_circle_hits;

    
}


int main(int argc, char** argv){

    num_of_threads = atoi(argv[1]);
    long double pi_estimation_serial;
    long double pi_estimation_parallel;

    // Serial algorithm for the estimation of pi.
    throw_count = atoi(argv[2]);
    double start_serial;
    double stop_serial;
    GET_TIME(start_serial);
    for(long long int throw = 0; throw < throw_count; throw++){
        unsigned int seed = time(NULL) + throw;
        double x_rand = get_rand(&seed);
        double y_rand = get_rand(&seed);
        double dist_square = x_rand * x_rand + y_rand * y_rand;
        if(dist_square <= 1) circle_hits++; 
    }

        pi_estimation_serial = 4 * circle_hits / ((double) throw_count);
        GET_TIME(stop_serial);
        double serial_time = stop_serial - start_serial;
        printf("Time for serial   estimation: %f | Pi serial estimation: %Lf\n",serial_time, pi_estimation_serial);
        circle_hits = 0; // Setting the global variable to 0 before the threads execution takes place.

        // Now using pthreads
        //Initialization of pthread_mutex

    
        pthread_t* threads = malloc(num_of_threads * sizeof(pthread_t));
        double start_parallel;
        double stop_parallel;
        double parallel_time;

        GET_TIME(start_parallel)
        for(long i = 0; i < num_of_threads; i++){
            
            if(pthread_create(&threads[i], NULL, pi_thread_estimation, (void*) i)){
                printf("Error! Function pthread_create failed.\n");
                exit(-1);
            }
        }
        for(long i = 0; i < num_of_threads; i++){
            if(pthread_join(threads[i], NULL)){
                fprintf(stderr, "Pthreads join operetion returned exit error.\n");
                    exit(-1);
                
            }
        }
        GET_TIME(stop_parallel)

        parallel_time = stop_parallel - start_parallel;
        avg_thread_time = avg_thread_time / num_of_threads;

        free(threads);

        pi_estimation_parallel = 4 * circle_hits / ((double) throw_count);
        printf("Time for parallel estimation: %f | Pi parallel estimation: %Lf\n",parallel_time, pi_estimation_parallel);
        printf("Average thread time: %f\n", avg_thread_time);



}