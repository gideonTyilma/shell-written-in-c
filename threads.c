#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/*week 13 examples pthread_create GUYS use an editor called anyfile-notepad if
you're editing in drive its got some basic syntax highlighting which is better
than the default bullshit */

/* Charlie - From my tests, sum and max are both working correctly. I still need
   to comment most of the thread code, though. All we need now is timing code
   and then gather some data, write the report, and present in discussion
   tomorrow */

void *get_max_thread(void *ptr);
void *get_sum_thread(void *ptr);
int get_max(int *arr, int size);
int get_sum(int *arr, int size);

/* struct for passing int array to thread functions */
struct int_arr {
    int size;
    int *nums;
};

void *get_max_thread(void *ptr) {
    struct int_arr arr = *(struct int_arr *)ptr;
    int *max = (int *)malloc(sizeof(int));
    *max = get_max(arr.nums, arr.size);

    return max;
}

void *get_sum_thread(void *ptr) {
    struct int_arr arr = *(struct int_arr *)ptr;
    int *sum = (int *)malloc(sizeof(int));

    *sum = get_sum(arr.nums, arr.size);

    return sum;
}
int get_max(int *arr, int size) {
    int i = 0, j;

    for (j = 0; j < size; j++) {
        if (arr[j] > i) {
            i = arr[j];
        }
    }
    return i;
}
int get_sum(int *arr, int size) {
    int sum = 0, i;

    for (i = 0; i < size; i++) {
        sum += arr[i];
        sum = sum % 1000000;
    }
    return sum;
}
struct timeval tv_delta(struct timeval start, struct timeval end) {
    struct timeval delta = end;

    delta.tv_sec -= start.tv_sec;
    delta.tv_usec -= start.tv_usec;
    if (delta.tv_usec < 0) {
        delta.tv_usec += 1000000;
        delta.tv_sec--;
    }

    return delta;
}

int main(int argc, char **argv) {
    int num_elems, num_threads, seed, task, elems_per_thread, total;
    pthread_t *ids;
    struct int_arr *args;
    char *print;
    int *arr, *results, *result_ptr;
    int i, j, max, sum;
    clock_t start, end;
    double time_elapsed;

    if (argc < 6) {
        printf("Incorrect argument format\n");
        return 1;
    }

    num_elems = atoi(argv[1]);
    num_threads = atoi(argv[2]);
    elems_per_thread =
        (num_elems / num_threads) + (num_elems % num_threads ? 1 : 0);
    seed = atoi(argv[3]);
    task = atoi(argv[4]);
    print = argv[5];
    arr = (int *)malloc(num_elems *
                        sizeof(int)); /*makes an array based on num_elems*/
    results = (int *)malloc(
        num_threads *
        sizeof(int)); /* array to store the results of each thread */
    args = (struct int_arr *)malloc(
        num_threads *
        sizeof(struct int_arr)); /* array to store arguments to each thread */
    ids = (pthread_t *)malloc(num_threads *
                              sizeof(pthread_t)); /* array for thread ids */

    srand(seed); /*creates seed for random # generation*/
    for (i = 0; i < num_elems; i++) {
        arr[i] = rand(); /*generates a random # for each element in the array*/
    }

    total = 0;
    /* split the array of random numbers into a set of structs for each thread
     */
    struct rusage start_ru, end_ru;
    struct timeval start_wall, end_wall;
    struct timeval diff_ru_utime, diff_wall, diff_ru_stime;
    int n = 50000, i1, j1, *p;

    /* Collecting information */
    getrusage(RUSAGE_SELF, &start_ru);
    gettimeofday(&start_wall, NULL);

    for (i = 0; i < num_threads; i++) {
        args[i].nums = (int *)malloc(elems_per_thread * sizeof(int));
        args[i].size = 0;
        for (j = 0; j < elems_per_thread && total < num_elems; j++) {
            args[i].nums[j] = arr[total++];
            args[i].size++;
        }
    }
    start = clock(); /*set the timer here*/
    if (task == 1) {
        for (i = 0; i < num_threads; i++) {
            if (pthread_create(&(ids[i]), NULL, get_max_thread, &(args[i])) !=
                0) {
                fprintf(stderr, "thread creation failed\n");
                exit(1);
            }
        }

        for (i = 0; i < num_threads; i++) {
            if (pthread_join(ids[i], (void **)&result_ptr) != 0) {
                fprintf(stderr, "thread join failed\n");
                exit(1);
            }
            results[i] = *result_ptr;
        }
        max = get_max(results, num_threads);
        if (*print == 'y' || *print == 'Y') {
            printf("Max is: %d\n", max);
        }
    }
    if (task == 2) {
        for (i = 0; i < num_threads; i++) {
            if (pthread_create(&(ids[i]), NULL, get_sum_thread, &(args[i])) !=
                0) {
                fprintf(stderr, "thread creation failed\n");
                exit(1);
            }
        }

        for (i = 0; i < num_threads; i++) {
            if (pthread_join(ids[i], (void **)&result_ptr) != 0) {
                fprintf(stderr, "thread join failed\n");
                exit(1);
            }
            results[i] = *result_ptr;
        }
        sum = get_sum(results, num_threads);
        if (*print == 'y' || *print == 'Y') {
            printf("Sum is %d\n", sum);
        }
    }
    end = clock();
    time_elapsed =
        ((double)(end - start) / CLOCKS_PER_SEC); /*get the time elapsed here
                                   and print that information */

    /* Computing difference */
    diff_ru_utime = tv_delta(start_ru.ru_utime, end_ru.ru_utime);
    diff_ru_stime = tv_delta(start_ru.ru_stime, end_ru.ru_stime);
    diff_wall = tv_delta(start_wall, end_wall);

    printf("User time: %ld.%06ld\n", diff_ru_utime.tv_sec,
           diff_ru_utime.tv_usec);
    printf("System time: %ld.%06ld\n", diff_ru_stime.tv_sec,
           diff_ru_stime.tv_usec);
    printf("Wall time: %ld.%06ld\n", diff_wall.tv_sec, diff_wall.tv_usec);

    return 0;
}
