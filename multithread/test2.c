#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMTHRDS 4

typedef struct{
	int a;
	int b;
} data_str;

data_str data;
pthread_mutex_t mutex;
pthread_cond_t cond;

void *rand_a(void *arg){
	int i;
	int j = (int)arg;
	for(i=0;i<20;i++){
		pthread_mutex_lock(&mutex);
		data.a = rand()%6;
		printf("Thread %d: a = %d.\n",j,data.a);
		if(data.a == data.b){
			printf("a = %d; b = %d; signal sent.\n",data.a,data.b);
			pthread_cond_signal(&cond);
		}
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	pthread_exit(NULL);
}

void *rand_b(void *arg){
	int i;
	int j = (int)arg;
	for(i=0;i<20;i++){
		pthread_mutex_lock(&mutex);
		data.b = rand()%5;
		printf("Thread %d: b = %d.\n",j,data.b);
		if(data.a == data.b){
			printf("a = %d; b = %d; signal sent.\n",data.a,data.b);
			pthread_cond_signal(&cond);
		}
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	pthread_exit(NULL);
}

void *wait(void *arg){
	int i;
	int j = (int)arg;
	pthread_mutex_lock(&mutex);
	while(data.a != data.b){
		printf("Thread %d: a!=b, blocking...\n",j);
		pthread_cond_wait(&cond,&mutex);
	}
	printf("Thread %d: be signaled...\n",j);
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}

int main(){
	int i;
	pthread_t threads[NUMTHRDS];
	pthread_attr_t attr;

	data.a = 6;
	data.b = 7;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init(&mutex,NULL);
	pthread_cond_init(&cond,NULL);

	pthread_create(&threads[0],&attr, rand_a, (void*)1);
	pthread_create(&threads[1],&attr, rand_b, (void*)2);
	pthread_create(&threads[2],&attr, wait, (void*)3);
	pthread_create(&threads[3],&attr, wait, (void*)4);

	for(i=0;i<4;i++){
		pthread_join(threads[i],NULL);
	}

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	pthread_exit(NULL);
}