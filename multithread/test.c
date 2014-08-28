#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUMTHRDS 6

int count=0;
int value;

pthread_mutex_t wrt;
pthread_mutex_t mutex;

void *write(void *arg){
	int i;
	int j = (int)arg;
	for(i=1;i<11;i++){
		pthread_mutex_lock(&wrt);
		value = i*j;
		printf("Thread %d writing, value is %d.\n",j,value);
		pthread_mutex_unlock(&wrt);
		sleep(1);
	}

	pthread_exit((void*) 0);
}

void *read(void *arg){
	int i;
	int j = (int)arg;
	for(i=1;i<6;i++){
		pthread_mutex_lock(&mutex);
		count++;
		if(count==1)
			pthread_mutex_lock(&wrt);
		pthread_mutex_unlock(&mutex);
		printf("Thread %d reading, value is %d.\n",j,value);
		pthread_mutex_lock(&mutex);
		count--;
		if(count==0)
			pthread_mutex_unlock(&wrt);
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	pthread_exit((void*) 0);
}

int main(){
	int i;
	void *status;
	pthread_t callThd[NUMTHRDS];
	pthread_attr_t attr;

	pthread_mutex_init(&wrt, NULL);
	pthread_mutex_init(&mutex,NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for(i=1;i<=NUMTHRDS;i++){
		if(i<=2){
			pthread_create(&callThd[i-1],&attr,write,(void *)i);
		}else{
			pthread_create(&callThd[i-1],&attr,read,(void *)i);
		}
	}

	pthread_attr_destroy(&attr);

	for(i=1;i<=NUMTHRDS;i++){
		pthread_join(callThd[i], &status);
	}

	printf("Program ternimating...value is %d",value);
	pthread_mutex_destroy(&wrt);
	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);
}