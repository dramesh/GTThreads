#include <stdio.h>
#include <stdint.h>
#include "gtthread.h"

gtthread_t philosopher[5];
gtthread_mutex_t chopstick[5];

void think (int id) {
	long int i = 0, think_time = rand()%10000000;
	printf("Philosopher #%d is thinking...\n", id);
	for (i=0; i<think_time; i++);
}

void acquire_chopsticks (int id) {
	if (id%2 == 0) {
	gtthread_mutex_lock(&chopstick[id]);
	printf("Philosopher #%d acquired left chopstick.\n", id);
	gtthread_mutex_lock(&chopstick[(id+1)%5]);
	printf("Philosopher #%d acquired right chopstick.\n", id);
	} else {
	gtthread_mutex_lock(&chopstick[(id+1)%5]);
	printf("Philosopher #%d acquired right chopstick.\n", id);
	gtthread_mutex_lock(&chopstick[id]);
	printf("Philosopher #%d acquired left chopstick.\n", id);
	}
}

void release_chopsticks (int id) {	
	if (id%2 == 0) {
	gtthread_mutex_unlock(&chopstick[(id+1)%5]);
	printf("Philosopher #%d released right chopstick.\n", id);
	gtthread_mutex_unlock(&chopstick[id]);
        printf("Philosopher #%d released left chopstick.\n", id);
	} else {
	gtthread_mutex_unlock(&chopstick[id]);
        printf("Philosopher #%d released left chopstick.\n", id);
	gtthread_mutex_unlock(&chopstick[(id+1)%5]);
	printf("Philosopher #%d released right chopstick.\n", id);
	}
}

void eat (int id) {
	long int i = 0, eat_time = rand()%1000000000;
	acquire_chopsticks(id);
	printf("Philosopher #%d is eating...\n", id);
	for (i=0; i<eat_time; i++);
	release_chopsticks(id);
}

void *dining (int n) {
	while(1) {
		think(n);
		printf("Philosopher #%d is hungry...\n", n);
		eat(n);
		printf("Philosopher #%d finished eating\n", n);
	}
}

void main () {
	int i;
	gtthread_init(100000);
	for (i=0; i<5; i++) {
		gtthread_mutex_init(&chopstick[i]);
		gtthread_create(&philosopher[i], (void *)dining, (int *)(i+1));
	}
	gtthread_exit(NULL);
}
