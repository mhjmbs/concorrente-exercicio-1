#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include "timer.h"

typedef struct {
    long n;
    float *a, *b;
    double *result;
} t_args;

double dot(float* a, float* b, long n);
void* mt_dot(void* arg);

int main(int argc, char* argv[]) {
    double start;
    GET_TIME(start);

    if(argc != 3) {
        printf("Usage: %s <n_threads> <input_file>\n", argv[0]);
        return 1;
    }
    
    int n_threads = atoi(argv[1]);
    const char* file_name = argv[2];

    
    if(n_threads <= 0) {
        printf("n_threads must be an integer greater or equal to 1\n");
        return 2;
    }
    
    FILE *input_file = fopen(file_name, "rb");
    if(input_file == NULL) {
        printf("Failed opening file %s\n", file_name);
        return 3;
    }
    
    long n;
    if(fread(&n, sizeof(long), 1, input_file) != 1) {
        printf("Failed reading file %s\n", file_name);
        return 4;
    }
    
    float* v[2] = {NULL, NULL};
    for(int i = 0; i < 2; i++) {
        v[i] = calloc(n, sizeof(float));
        if(v[i] == NULL) {
            printf("Failed allocating memory\n");
            free(v[0]), free(v[1]);
            fclose(input_file);
            return 5;
        }else if(fread(v[i], sizeof(float), n, input_file) != n) {
            printf("Failed reading file %s\n", file_name);
            free(v[0]), free(v[1]);
            fclose(input_file);
            return 6;
        }
    }
    
    double file_ans;
    if(fread(&file_ans, sizeof(double), 1, input_file) != 1) {
        printf("Failed reading file %s\n", file_name);
        free(v[0]), free(v[1]);
        return 7;
    }
    
    fclose(input_file);
    
    long seg_sz = n / n_threads;
    long remainder = n % n_threads;
    
    pthread_t* tid = calloc(n_threads, sizeof(pthread_t));
    if(tid == NULL) {
        printf("Failed allocating memory\n");
        free(v[0]), free(v[1]);
        return 8;
    }
    
    double *results = calloc(n_threads, sizeof(double));
    if(results == NULL) {
        printf("Failed allocating memory\n");
        free(tid);
        free(v[0]), free(v[1]);
        return 9;
    }
    
    t_args* args = calloc(n_threads, sizeof(t_args));
    if(args == NULL) {
        printf("Failed allocating memory\n");
        free(tid);
        free(v[0]), free(v[1]);
        free(results);
        return 9;
    }
    
    for(int thread = 0, i = 0; thread < n_threads; thread++) {
        long sz = seg_sz;
        if(remainder > 0) {
            sz++;
            remainder--;
        }
        
        args[thread].n = sz;
        args[thread].a = &v[0][i];
        args[thread].b = &v[1][i];
        args[thread].result = &results[thread];

        if(pthread_create(&tid[thread], NULL, mt_dot, (void*) &args[thread])) {
            printf("Failed to create thread\n");
            free(v[0]), free(v[1]);
            free(tid);
            free(results);
            return 10;
        }
        
        i += sz;
    }

    v[0] = v[1] = NULL;
    
    double ans = 0;
    
    for(int i = 0; i < n_threads; i++) {
        if(pthread_join(tid[i], NULL)) {
            printf("Failed joining thread\n");
            free(tid);
            free(results);
            return 11;
        }
        ans += results[i];
    }
    
    free(v[0]), free(v[1]);
    free(tid);
    free(results);
    free(args);
    v[0] = v[1] = NULL;
    tid = NULL;
    results = NULL;
    args = NULL;
    
    printf("Sequential dot product answer: %f\n", file_ans);
    printf("Multithreaded dot product answer: %f\n", ans);
    
    double e;
    if(file_ans != 0) {
        e = fabs((file_ans-ans)/file_ans);
    }else {
        e = fabs(ans);
    }
    printf("e = %f\n", e);
    
    double finish;
    GET_TIME(finish);
    printf("Time elapsed: %lf\n", finish-start);

    return 0;
}

double dot(float* a, float* b, long n) {
    double ans = 0;
    for(long i = 0; i < n; i++) {
        ans += a[i]*b[i];
    }
    return ans;
}

void* mt_dot(void* arg) {
    t_args* args = (t_args*) arg;
    *(args->result) = dot(args->a, args->b, args->n);
    return 0;
}