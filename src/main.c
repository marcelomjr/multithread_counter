#include <stdio.h>
#include <pthread.h>
#include <stdint.h>     // uint64_t
#include <unistd.h>     // sleep

#define MAX_THREAD_NUMBER 4

int is_prime_number(uint64_t number);
void get_inputs(uint64_t inputs[], int* number_of_inputs);
void* thread_routine(void *arg);

// Protects the work distribution
pthread_mutex_t input_lock; 

// Protects the primes counter
pthread_mutex_t prime_lock; 

// Array of the ids of threads
pthread_t threads_id[MAX_THREAD_NUMBER];

// Counter of the number of primes
int number_of_primes = 0;

// Input management
uint64_t inputs[30];
int number_of_inputs;
int input_index = 0;

void* thread_routine(void *arg) {
    
    int work_is_ended = 0;
    uint64_t number;

    // Repeats while there are inputs to be tested
    while (!work_is_ended) {
        // Wait until no thread is using the inputs array
        pthread_mutex_lock(&input_lock);

        // If there are still inputs to be tested
        if (input_index < number_of_inputs) {

            // Get the number and move the index
            number = inputs[input_index];
            input_index++;
            
            // Free the input_lock
            pthread_mutex_unlock(&input_lock);
            
            // Verify whether the number is prime
            if (is_prime_number(number)) {
                
                // Update the counter avoinding the race condition
                pthread_mutex_lock(&prime_lock);
                number_of_primes++;
                pthread_mutex_unlock(&prime_lock);
            }
        } else {

            work_is_ended = 1;
            // Free the lock of inputs
            pthread_mutex_unlock(&input_lock);
        }
    }
}

int main() {

    // Get the numbers and save them in inputs array
    get_inputs(inputs, &number_of_inputs);
    
    // Create all the threads
    for (int i = 0; i < MAX_THREAD_NUMBER; i++) {
        pthread_create(&threads_id[i], NULL, thread_routine, NULL);
    }

    // Wait until all the threads end
    for (int i = 0; i < MAX_THREAD_NUMBER; i++) {
        pthread_join(threads_id[i], NULL);
    }
    
    // Print the result
    printf("%d\n", number_of_primes);

    return 0;
}

/* Verify if a received number is prime, 
   if true returns 1, otherwise returns 0.
*/
int is_prime_number(uint64_t number) {
    int is_prime = 1;
    
    // If number is smaller than 2 it isn't a prime number
    if (number < 2) {
        is_prime = 0;
    }
    /* If the number is two, it is a prime number, but if it is bigger
     than two we need test if there is a division with remainder equals 
     zero, in this case the number isn't prime.*/
    else if (number > 2) {
        for (uint64_t div = 2; (div < number && is_prime == 1); div++) {
           
            if (number % div == 0) {
                is_prime = 0;
            }
        }
    }
    return is_prime;
}

/* It gets the numbers from the standard input, one by one, 
   until the end of file character is found */
void get_inputs(uint64_t inputs[], int* number_of_inputs) {
    
    uint64_t input;
    int index = 0;
    
    while (scanf("%lu", &input) != EOF) {
        inputs[index] = input;
        index++;
    }
    *number_of_inputs = index;
}

