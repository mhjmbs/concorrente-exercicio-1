/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Codigo: Comunicação entre threads usando variável compartilhada e exclusao mutua com bloqueio */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

#define MAX_SOMA 100000
#define DIV 1000

const long int cnt_multiples = MAX_SOMA/DIV + 1;
long int soma = 0; //variavel compartilhada entre as threads
int finished = 0;
pthread_mutex_t mutex; //variavel de lock para exclusao mutua
pthread_cond_t ready_to_print, ready_to_sum;

//funcao executada pelas threads
void *ExecutaTarefa (void *arg) {
    long int id = (long int) arg;
    printf("Thread : %ld esta executando...\n", id);

    for (int i=0; i<=MAX_SOMA; i++) {
        //--entrada na SC
        pthread_mutex_lock(&mutex);
        //--SC (seção critica)
        while(!finished && soma <= MAX_SOMA && soma % DIV == 0) { //verifica se a soma atual deve ser impressa
            pthread_cond_broadcast(&ready_to_print);
            pthread_cond_wait(&ready_to_sum, &mutex);
        }
        if(finished) { //finaliza caso o último valor desejado já tenha sido impresso
            pthread_mutex_unlock(&mutex);
            break;
        }
        soma++; //incrementa a variavel compartilhada 
        //--saida da SC
        pthread_mutex_unlock(&mutex);
    }

    printf("Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra (void *args) {
    printf("Extra : esta executando...\n");

    pthread_mutex_lock(&mutex);
    for (int printed = 0; printed < cnt_multiples; printed++) {
        while(!(soma <= MAX_SOMA && soma % DIV == 0)) { //verifica se a soma atual deve ser impressa
            pthread_cond_wait(&ready_to_print, &mutex);
        }
        printf("soma = %ld \n", soma);
        soma++; //torna a soma atual inválida para impressão e, portanto, válida para incrementação
        pthread_cond_broadcast(&ready_to_sum);
    }
    finished = 1;
    pthread_mutex_unlock(&mutex);

    printf("Extra : terminou!\n");
    pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid; //identificadores das threads no sistema
    int nthreads; //qtde de threads (passada linha de comando)

    //--le e avalia os parametros de entrada
    if(argc<2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    //--aloca as estruturas
    tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
    if(tid==NULL) {puts("ERRO--malloc"); return 2;}

    //--inicilaiza o mutex e a condição(lock de exclusao mutua)
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&ready_to_sum, NULL);
    pthread_cond_init(&ready_to_print, NULL);

    //--cria as threads
    for(long int t=0; t<nthreads; t++) {
        if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
            printf("--ERRO: pthread_create()\n"); exit(-1);
        }
    }

    //--cria thread de log
    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create()\n"); exit(-1);
    }

    //--espera todas as threads terminarem
    for (int t=0; t<nthreads+1; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n"); exit(-1); 
        } 
    } 

    //--finaliza o mutex
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&ready_to_sum);
    pthread_cond_destroy(&ready_to_print);
    printf("Valor de 'soma' = %ld\n", soma);

    return 0;
}