#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdatomic.h>

const int64_t SIZES[3] = { 256 * 1024, 1024 * 1024, 64 * 1024 * 1024};

static uint64_t getCurTime() {
    struct timespec tms;
    if (clock_gettime(CLOCK_REALTIME, &tms)) {
        return -1;
    }
    return tms.tv_sec * 1000000 + tms.tv_nsec / 1000;
}

// Передбачаємо частину функції RunBenchmark
float RunBenchmark_AtomicMemory(const int64_t SIZE) {
    atomic_int* atomic_array = malloc(sizeof(atomic_int) * SIZE);

    // Ініціалізація
    for (int64_t i = 0; i < SIZE; i++) {
        atomic_init(&atomic_array[i], 0);
    }

    uint64_t startTime = getCurTime();
    for (int64_t i = 0; i < SIZE; i++) {
        atomic_fetch_add(&atomic_array[i], 1);
    }
    uint64_t finishTime = getCurTime();

    float time_atomic_memory = (finishTime - startTime) / 1000000.0;
    printf("time_atomic_memory: %f \n", time_atomic_memory);
    free(atomic_array);
    return time_atomic_memory;
}

// Частина функції RunBenchmark
float RunBenchmark_CacheDelays(const int64_t SIZE) {
    volatile int* volatile_array = malloc(sizeof(volatile int) * SIZE);

    // Ініціалізація
    for (int64_t i = 0; i < SIZE; i++) {
        volatile_array[i] = 0;
    }

    uint64_t startTime = getCurTime();
    for (int64_t i = 0; i < SIZE; ++i) {
        volatile_array[i]++;
    }
    uint64_t finishTime = getCurTime();

    float time_cache_delays = (finishTime - startTime) / 1000000.0;
    printf("time_cache_delays: %f \n", time_cache_delays);
    free((void*)volatile_array);
    return time_cache_delays;
}

float* RunBenchmark_SequentialAndRandom(const int64_t SIZE, int number_of_experiment)
{
    unsigned char** sequential_pointers = (unsigned char**)malloc(SIZE * sizeof(unsigned char*));
    unsigned char** random_pointers = (unsigned char**)malloc(SIZE * sizeof(unsigned char*));
    unsigned char *static_array = (unsigned char*)malloc(SIZE);

    if (sequential_pointers == NULL || random_pointers == NULL || static_array == NULL) {
        printf("Out of Memory\n");
        return 0;
    }

    //printf("Number of experiment: %d \n", number_of_experiment);

    for(int64_t i = 0; i < SIZE; i++)
    {
        static_array[i] = i%255;
        sequential_pointers[i] = &static_array[i];

        unsigned char *val = (unsigned char *)malloc(sizeof(unsigned char));
        if (val == NULL)
            fprintf(stderr, "Не вдалося виділити пам'ять \n");

        *val = i%255;
        random_pointers[i] = val;
    }

    uint64_t startTime = getCurTime();
    for (int i = 0; i < SIZE; ++i) {
        (*sequential_pointers[i])++;
    }
    uint64_t finishTime = getCurTime();
    double time_sequential_pointers = (finishTime - startTime) / 1000000.0;
    //printf("time_sequential_pointers: %f \n", time_sequential_pointers);

    startTime = getCurTime();
    for(int64_t i = 0; i < SIZE; i++)
    {
        (*random_pointers[i])++;
    }
    finishTime = getCurTime();
    double time_random_pointers = (finishTime - startTime) / 1000000.0;
    //printf("time_random_pointers: %f \n", time_random_pointers);

    for(int64_t i = 0; i < SIZE; i++)
        free(random_pointers[i]);
    free(sequential_pointers);
    free(random_pointers);
    free(static_array);

    float *time_results = (float *) malloc(3);
    time_results[0] = time_sequential_pointers;
    time_results[1] = time_random_pointers;
    return time_results;
}


void print_table_of_experiments(double cache_delays_result_time,double atomic_memory_result_time, double sequential_pointers_result_time, double random_pointers_result_time, int64_t array_size, int number_of_experiments) {
    printf("Number of experiments: %d\n", number_of_experiments);
    printf("+---------------+--------------+------------+\n");
    printf("|     Mode      |   Time (s)   | Array Size |\n");
    printf("+---------------+--------------+------------+\n");
    printf("|  Cache Delay  |   %lf   |  %lld   |\n", cache_delays_result_time, array_size);
    printf("+---------------+--------------+------------+\n");
    printf("|     Atomic    |   %lf   |  %lld   |\n", atomic_memory_result_time, array_size);
    printf("+---------------+--------------+------------+\n");
    printf("|  Sequential   |   %lf   |  %lld   |\n", sequential_pointers_result_time, array_size);
    printf("+---------------+--------------+------------+\n");
    printf("|    Random     |   %lf   |  %lld   |\n", random_pointers_result_time, array_size);
    printf("+---------------+--------------+------------+\n");
}

int main(int argc, char* argv[]) {
    int number_of_experiments = 10;
    for (int i = 0; i < sizeof(SIZES)/sizeof(SIZES[0]); ++i)
    {
        float cache_delays_result_time = 0.0,
              atomic_memory_result_time = 0.0,
              sequential_pointers_result_time = 0.0,
              random_pointers_result_time = 0.0;
        for (int number_of_experiment = 1; number_of_experiment <= number_of_experiments; ++number_of_experiment) {
            cache_delays_result_time += RunBenchmark_CacheDelays(SIZES[i]);
            atomic_memory_result_time += RunBenchmark_AtomicMemory(SIZES[i]);
            float *time_results = RunBenchmark_SequentialAndRandom(SIZES[i], number_of_experiment);
            sequential_pointers_result_time += time_results[0];
            random_pointers_result_time     += time_results[1];
        }
        cache_delays_result_time /=number_of_experiments;
        atomic_memory_result_time /=number_of_experiments;
        sequential_pointers_result_time /=number_of_experiments;
        random_pointers_result_time /=number_of_experiments;
        print_table_of_experiments(cache_delays_result_time, atomic_memory_result_time, sequential_pointers_result_time, random_pointers_result_time, SIZES[i], number_of_experiments);
    }
    return 0;
}