#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXN 100000000

int main() {
    srand(time(NULL));

    long n = 100000000;
    
    float* v[2] = {NULL, NULL};
    for(int i = 0; i < 2; i++) {
        v[i] = calloc(n, sizeof(float));
        if(v[i] == NULL) {
            printf("Failed allocating memory\n");
            free(v[0]), free(v[1]);
            return 1;
        }
        for(int j = 0; j < n; j++) {
            int sign = (rand()%2 == 0) ? -1 : 1;
            v[i][j] = sign * (double)rand()/RAND_MAX * MAXN;
        }
    }

    double ans = 0;

    for(int i = 0; i < n; i++) {
        ans += (double)v[0][i] * v[1][i];
    }
    
    FILE *input_file = fopen("input_file.bin", "wb");

    if(input_file == NULL) {
        free(v[0]), free(v[1]);
        printf("Failed to generate file\n");
        return 1;
    }

    if(fwrite(&n, sizeof(long), 1, input_file) != 1 || 
       fwrite(v[0], sizeof(float), n, input_file) != n || 
       fwrite(v[1], sizeof(float), n, input_file) != n || 
       fwrite(&ans, sizeof(double), 1, input_file) != 1)
    {
        free(v[0]), free(v[1]);
        printf("Failed to write file\n");
        return 2;
    }
    
    fclose(input_file);
    free(v[0]), free(v[1]);

    printf("Input_file generated successfully.\n");
    printf("n = %ld, dot = %lf\n", n, ans);
}