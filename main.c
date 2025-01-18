#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"

void* timer(void *arg)
{
	usleep(500000); // 0.5 sekundy

	for (int i=1; i<=30; i++)
	{
		printf("\n========== %d ==========\n", i);
		sleep(1);
	}

	return NULL;
}

void* thread_func_1(void *arg)
{
	pthread_t thread_ID = pthread_self()-1; // ID watku
	TQueue *queue = (TQueue*)arg; // Wskaznik do kolejki
	void *msg = NULL; // Wiadomosc

	sleep(1); // 0 -> 1
	msg = strdup("1. Hello world!");
	printf("Expected time: 1; Thread [%llu] is attempting to add message '%s' to the queue.\n", thread_ID, (char*)msg);
	addMsg(queue, msg);
	printf("Expected time: 1; Thread [%llu] successfully added message '%s' to the queue.\n", thread_ID, (char*)msg);

	sleep(2); // 1 -> 3
	msg = strdup("2. Hello world!");
	printf("Expected time: 3; Thread [%llu] is attempting to add message '%s' to the queue.\n", thread_ID, (char*)msg);
	addMsg(queue, msg);
	printf("Expected time: 3; Thread [%llu] successfully added message '%s' to the queue.\n", thread_ID, (char*)msg);

	sleep(5); // 3 -> 8
	msg = strdup("3. Hello world!");
	printf("Expected time: 8; Thread [%llu] is attempting to add message '%s' to the queue.\n", thread_ID, (char*)msg);
	addMsg(queue, msg);
	printf("Expected time: 8; Thread [%llu] successfully added message '%s' to the queue.\n", thread_ID, (char*)msg);

	sleep(2); // 8 -> 10
	setSize(queue, 2);
	printf("Expected time: 10; Thread [%llu] is changed size of queue to '%d'.\n", thread_ID, 2);

	sleep(1); // 10 -> 11
	msg = strdup("4. Hello world!");
	printf("Expected time: 11; Thread [%llu] is attempting to add message '%s' to the queue.\n", thread_ID, (char*)msg);
	addMsg(queue, msg);
	printf("Expected time: 11; Thread [%llu] successfully added message '%s' to the queue.\n", thread_ID, (char*)msg);

	sleep(2); // 11 -> 13
	msg = strdup("5. Hello world!");
	printf("Expected time: 13; Thread [%llu] is attempting to add message '%s' to the queue.\n", thread_ID, (char*)msg);
	addMsg(queue, msg);
	printf("Expected time: 13; Thread [%llu] successfully added message '%s' to the queue.\n", thread_ID, (char*)msg);

	sleep(1); // 13 -> 14 && 14 -> 17
	msg = strdup("6. Hello world!");
	printf("Expected time: 14; Thread [%llu] is attempting to add message '%s' to the queue.\n", thread_ID, (char*)msg);
	addMsg(queue, msg);
	printf("Expected time: 17; Thread [%llu] successfully added message '%s' to the queue.\n", thread_ID, (char*)msg);

	sleep(2); // 17 -> 19 && 19 -> 21
	msg = strdup("7. Hello world!");
	printf("Expected time: 19; Thread [%llu] is attempting to add message '%s' to the queue.\n", thread_ID, (char*)msg);
	addMsg(queue, msg);
	printf("Expected time: 21; Thread [%llu] successfully added message '%s' to the queue.\n", thread_ID, (char*)msg);
	printf(">> Expected time: 21; Thread [%llu] ended.\n", thread_ID);

	return NULL;
}

void* thread_func_2(void *arg)
{
	pthread_t thread_ID = pthread_self()-1; // ID watku
	TQueue *queue = (TQueue*)arg; // Wskaznik do kolejki

	sleep(2); // 0 -> 2
	subscribe(queue, thread_ID);
	printf(">> Expected time: 2; Thread [%llu] subscribed.\n", thread_ID);

	sleep(2); // 2 -> 4
	printf("Expected time: 4; Thread [%llu] has %d messages available [Expected result: 1].\n", thread_ID, getAvailable(queue, thread_ID));

	sleep(2); // 4 -> 6
	printf("Expected time: 6; Thread [%llu] attempting to receive message.\n", thread_ID);
	printf("Expected time: 6; Thread [%llu] received message: '%s'\n", thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(1); // 6 -> 7 && 7 -> 8
	printf("Expected time: 7; Thread [%llu] attempting to receive message.\n", thread_ID);
	printf("Expected time: 8; Thread [%llu] received message: '%s'\n", thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(4); // 8 -> 12
	printf("Expected time: 12; Thread [%llu] has %d messages available [Expected result: 1].\n", thread_ID, getAvailable(queue, thread_ID));

	sleep(2); // 12 -> 14
	printf("Expected time: 14; Thread [%llu] has %d messages available [Expected result: 2].\n", thread_ID, getAvailable(queue, thread_ID));

	sleep(3); // 14 -> 17
	printf("Expected time: 17; Thread [%llu] attempting to receive message.\n", thread_ID);
	printf("Expected time: 17; Thread [%llu] received message: '%s'\n", thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(3); // 17 -> 20
	printf("Expected time: 20; Thread [%llu] has %d messages available [Expected result: 2].\n", thread_ID, getAvailable(queue, thread_ID));

	sleep(1); // 20 -> 21
	printf("Expected time: 21; Thread [%llu] attempting to receive message.\n", thread_ID);
	printf("Expected time: 21; Thread [%llu] received message: '%s'\n", thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(1); // 21 -> 22
	printf("Expected time: 22; Thread [%llu] attempting to receive message.\n", thread_ID);
	printf("Expected time: 22; Thread [%llu] received message: '%s'\n", thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(1); // 22 -> 23
	printf("Expected time: 23; Thread [%llu] attempting to receive message.\n", thread_ID);
	printf("Expected time: 23; Thread [%llu] received message: '%s'\n", thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(0); // 23
	unsubscribe(queue, thread_ID);
	printf(">> Expected time: 23; Thread [%llu] unsubscribed.\n", thread_ID);
	printf(">> Expected time: 23; Thread [%llu] ended.\n", thread_ID);

	return NULL;
}

void* thread_func_3(void *arg)
{
	pthread_t thread_ID = pthread_self()-1; // ID watku
	TQueue *queue = (TQueue*)arg; // Wskaznik do kolejki

	sleep(4); // 0 -> 4
	subscribe(queue, thread_ID);
	printf(">> Expected time: 4; Thread [%llu] subscribed.\n", thread_ID);

	sleep(1); // 4 -> 5 && 5 -> 8
	printf("Expected time: 5; Thread [%llu] attempting to receive message.\n", thread_ID);
	printf("Expected time: 8; Thread [%llu] received message: '%s'\n", thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(4); // 8 -> 12
	printf("Expected time: 12; Thread [%llu] has %d messages available [Expected result: 1].\n", thread_ID, getAvailable(queue, thread_ID));

	sleep(2); // 12 -> 14
	printf("Expected time: 14; Thread [%llu] has %d messages available [Expected result: 2].\n", thread_ID, getAvailable(queue, thread_ID));

	sleep(2); // 14 -> 16
	printf("Expected time: 16; Thread [%llu] attempting to receive message.\n", thread_ID);
	printf("Expected time: 16; Thread [%llu] received message: '%s'\n", thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(2); // 16 -> 18
	unsubscribe(queue, thread_ID);
	printf(">> Expected time: 18; Thread [%llu] unsubscribed.\n", thread_ID);
	printf(">> Expected time: 18; Thread [%llu] ended.\n", thread_ID);

	return NULL;
}

void* thread_func_4(void *arg)
{
	pthread_t thread_ID = pthread_self()-1; // ID watku
	TQueue *queue = (TQueue*)arg; // Wskaznik do kolejki

	sleep(12); // 0 -> 12
	subscribe(queue, thread_ID);
	printf(">> Expected time: 12; Thread [%llu] subscribed.\n", thread_ID);

	sleep(2); // 12 -> 14
	printf("Expected time: 14; Thread [%llu] has %d messages available [Expected result: 1].\n", thread_ID, getAvailable(queue, thread_ID));

	sleep(1); // 14 -> 15
	unsubscribe(queue, thread_ID);
	printf(">> Expected time: 15; Thread [%llu] unsubscribed.\n", thread_ID);
	printf(">> Expected time: 15; Thread [%llu] ended.\n", thread_ID);

	return NULL;
}



int main()
{
	const int THREADS_NUM=5, QUEUE_SIZE=1;
	pthread_t threads[THREADS_NUM];

	// Tworzenie kolejki
	TQueue *queue;
	queue = createQueue(QUEUE_SIZE);
	printf("=== Queue created with size %d ===\n", QUEUE_SIZE);

	// Tworzenie timera
	pthread_create(&threads[0], NULL, timer, NULL);

	// Tworzenie watkow
	pthread_create(&threads[1], NULL, thread_func_1, queue);
	pthread_create(&threads[2], NULL, thread_func_2, queue);
	pthread_create(&threads[3], NULL, thread_func_3, queue);
	pthread_create(&threads[4], NULL, thread_func_4, queue);

	// Czekanie na zakonczenie watkow
	for (int i=1; i<THREADS_NUM; i++)
	{
		pthread_join(threads[i], NULL);
	}

	// Zakonczenie timera
	pthread_cancel(threads[0]);

	// Zniszczenie kolejki
	destroyQueue(queue);

	return 0;
}

