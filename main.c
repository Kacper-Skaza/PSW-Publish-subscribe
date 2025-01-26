#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "queue.h"

void* timer(void *arg)
{
	usleep(500000); // 0.5 sekundy

	for (int i=1; i<=30; i++)
	{
		printf("\n======== %d ========\n", i);
		sleep(1);
	}

	return NULL;
}

void* thread_func_1(void *arg)
{
	pthread_t thread_ID = pthread_self(); // ID watku
	TQueue *queue = (TQueue*)arg; // Wskaznik do kolejki
	void *msg1, *msg2, *msg3, *msg4, *msg5, *msg6, *msg7; // Wskazniki do wiadomosci

	sleep(1); // 0 -> 1
	msg1 = strdup("1. Hello world!");
	printf("Expected time: 1; Thread [%lu] is attempting to add message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg1);
	addMsg(queue, msg1);
	printf("Expected time: 1; Thread [%lu] successfully added message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg1);

	sleep(2); // 1 -> 3
	msg2 = strdup("2. Hello world!");
	printf("Expected time: 3; Thread [%lu] is attempting to add message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg2);
	addMsg(queue, msg2);
	printf("Expected time: 3; Thread [%lu] successfully added message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg2);

	sleep(5); // 3 -> 8
	msg3 = strdup("3. Hello world!");
	printf("Expected time: 8; Thread [%lu] is attempting to add message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg3);
	addMsg(queue, msg3);
	printf("Expected time: 8; Thread [%lu] successfully added message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg3);

	sleep(2); // 8 -> 10
	setSize(queue, 2);
	printf("Expected time: 10; Thread [%lu] changed size of queue to '2'.\n", (unsigned long)thread_ID);

	sleep(1); // 10 -> 11
	msg4 = strdup("4. Hello world!");
	printf("Expected time: 11; Thread [%lu] is attempting to add message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg4);
	addMsg(queue, msg4);
	printf("Expected time: 11; Thread [%lu] successfully added message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg4);

	sleep(2); // 11 -> 13
	msg5 = strdup("5. Hello world!");
	printf("Expected time: 13; Thread [%lu] is attempting to add message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg5);
	addMsg(queue, msg5);
	printf("Expected time: 13; Thread [%lu] successfully added message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg5);

	sleep(1); // 13 -> 14 && 14 -> 17
	msg6 = strdup("6. Hello world!");
	printf("Expected time: 14; Thread [%lu] is attempting to add message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg6);
	addMsg(queue, msg6);
	printf("Expected time: 17; Thread [%lu] successfully added message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg6);

	sleep(2); // 17 -> 19 && 19 -> 21
	msg7 = strdup("7. Hello world!");
	printf("Expected time: 19; Thread [%lu] is attempting to add message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg7);
	addMsg(queue, msg7);
	printf("Expected time: 21; Thread [%lu] successfully added message '%s' to the queue.\n", (unsigned long)thread_ID, (char*)msg7);

	usleep(2250000); // 21 -> 23.25
	free(msg1); free(msg2); free(msg3); free(msg4); free(msg5); free(msg6); free(msg7);

	return NULL;
}

void* thread_func_2(void *arg)
{
	pthread_t thread_ID = pthread_self(); // ID watku
	TQueue *queue = (TQueue*)arg; // Wskaznik do kolejki

	sleep(2); // 0 -> 2
	subscribe(queue, thread_ID);
	printf(">> Expected time: 2; Thread [%lu] subscribed.\n", (unsigned long)thread_ID);

	sleep(2); // 2 -> 4
	printf("Expected time: 4; Thread [%lu] has '%d' messages available [Expected result: 1].\n", (unsigned long)thread_ID, getAvailable(queue, thread_ID));

	sleep(2); // 4 -> 6
	printf("Expected time: 6; Thread [%lu] attempting to receive message.\n", (unsigned long)thread_ID);
	printf("Expected time: 6; Thread [%lu] received message: '%s'.\n", (unsigned long)thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(1); // 6 -> 7 && 7 -> 8
	printf("Expected time: 7; Thread [%lu] attempting to receive message.\n", (unsigned long)thread_ID);
	printf("Expected time: 8; Thread [%lu] received message: '%s'.\n", (unsigned long)thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(4); // 8 -> 12
	printf("Expected time: 12; Thread [%lu] has '%d' messages available [Expected result: 1].\n", (unsigned long)thread_ID, getAvailable(queue, thread_ID));

	sleep(2); // 12 -> 14
	printf("Expected time: 14; Thread [%lu] has '%d' messages available [Expected result: 2].\n", (unsigned long)thread_ID, getAvailable(queue, thread_ID));

	sleep(3); // 14 -> 17
	printf("Expected time: 17; Thread [%lu] attempting to receive message.\n", (unsigned long)thread_ID);
	printf("Expected time: 17; Thread [%lu] received message: '%s'.\n", (unsigned long)thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(3); // 17 -> 20
	printf("Expected time: 20; Thread [%lu] has '%d' messages available [Expected result: 2].\n", (unsigned long)thread_ID, getAvailable(queue, thread_ID));

	sleep(1); // 20 -> 21
	printf("Expected time: 21; Thread [%lu] attempting to receive message.\n", (unsigned long)thread_ID);
	printf("Expected time: 21; Thread [%lu] received message: '%s'.\n", (unsigned long)thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(1); // 21 -> 22
	printf("Expected time: 22; Thread [%lu] attempting to receive message.\n", (unsigned long)thread_ID);
	printf("Expected time: 22; Thread [%lu] received message: '%s'.\n", (unsigned long)thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(1); // 22 -> 23
	printf("Expected time: 23; Thread [%lu] attempting to receive message.\n", (unsigned long)thread_ID);
	printf("Expected time: 23; Thread [%lu] received message: '%s'.\n", (unsigned long)thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(0); // 23
	unsubscribe(queue, thread_ID);
	printf(">> Expected time: 23; Thread [%lu] unsubscribed and ended.\n", (unsigned long)thread_ID);

	return NULL;
}

void* thread_func_3(void *arg)
{
	pthread_t thread_ID = pthread_self(); // ID watku
	TQueue *queue = (TQueue*)arg; // Wskaznik do kolejki

	sleep(4); // 0 -> 4
	subscribe(queue, thread_ID);
	printf(">> Expected time: 4; Thread [%lu] subscribed.\n", (unsigned long)thread_ID);

	sleep(1); // 4 -> 5 && 5 -> 8
	printf("Expected time: 5; Thread [%lu] attempting to receive message.\n", (unsigned long)thread_ID);
	printf("Expected time: 8; Thread [%lu] received message: '%s'.\n", (unsigned long)thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(4); // 8 -> 12
	printf("Expected time: 12; Thread [%lu] has '%d' messages available [Expected result: 1].\n", (unsigned long)thread_ID, getAvailable(queue, thread_ID));

	sleep(2); // 12 -> 14
	printf("Expected time: 14; Thread [%lu] has '%d' messages available [Expected result: 2].\n", (unsigned long)thread_ID, getAvailable(queue, thread_ID));

	sleep(2); // 14 -> 16
	printf("Expected time: 16; Thread [%lu] attempting to receive message.\n", (unsigned long)thread_ID);
	printf("Expected time: 16; Thread [%lu] received message: '%s'.\n", (unsigned long)thread_ID, (char*)getMsg(queue, thread_ID));

	sleep(2); // 16 -> 18
	unsubscribe(queue, thread_ID);
	printf(">> Expected time: 18; Thread [%lu] unsubscribed and ended.\n", (unsigned long)thread_ID);

	return NULL;
}

void* thread_func_4(void *arg)
{
	pthread_t thread_ID = pthread_self(); // ID watku
	TQueue *queue = (TQueue*)arg; // Wskaznik do kolejki

	sleep(12); // 0 -> 12
	subscribe(queue, thread_ID);
	printf(">> Expected time: 12; Thread [%lu] subscribed.\n", (unsigned long)thread_ID);

	sleep(2); // 12 -> 14
	printf("Expected time: 14; Thread [%lu] has '%d' messages available [Expected result: 1].\n", (unsigned long)thread_ID, getAvailable(queue, thread_ID));

	sleep(1); // 14 -> 15
	unsubscribe(queue, thread_ID);
	printf(">> Expected time: 15; Thread [%lu] unsubscribed and ended.\n", (unsigned long)thread_ID);

	return NULL;
}

void single_thread_test()
{
	pthread_t thread_ID=1000;
	const int QUEUE_SIZE=6;

	// Tworzenie kolejki
	TQueue *queue;
	queue = createQueue(QUEUE_SIZE);
	printf("Queue created with size '%d'\n", QUEUE_SIZE);

	// Deklaracja zmiennych
	int *m1, *m2, *m3, *p, num;
	m1 = malloc(sizeof(int)); m2 = malloc(sizeof(int)); m3 = malloc(sizeof(int));
	*m1 = 10; *m2 = 20; *m3 = 30;

	// Test wskaznikow
	subscribe(queue, thread_ID);
	addMsg(queue, m1);
	p = getMsg(queue, thread_ID);
	printf("[Result: %d] Test pointers [Details: %d=%d && %p=%p]\n", (*m1==*p && m1==p), *m1, *p, m1, p);

	// Test usuwania wiadomosci
	num = getAvailable(queue, thread_ID);
	printf("[Result: %d] Test message removal [Details: %d=%d]\n", (num == 0), num, 0);
	unsubscribe(queue, thread_ID);

	// Test NULL w 'getMsg' i x<0 w 'getAvailable'
	p = getMsg(queue, thread_ID);
	printf("[Result: %d] Test NULL in 'getMsg' [Details: %p=%p]\n", (p==NULL), p, NULL);
	num = getAvailable(queue, thread_ID);
	printf("[Result: %d] Test x<0 in 'getAvailable' [Details: %d<%d]\n", (num < 0), num, 0);

	// Test 'subscribe'
	for (int i=100; i>=0; i--)
	{
		subscribe(queue, thread_ID+i);
	}
	addMsg(queue, m1);
	addMsg(queue, m2);
	p = getMsg(queue, thread_ID);
	num = getAvailable(queue, thread_ID);
	printf("[Result: %d] Test 'subscribe' [Details: %d=%d && %d=%d && %p=%p]\n", (num==1 && *m1==*p && m1==p), num, 1, *m1, *p, m1, p);
	for (int i=0; i<=100; i++)
	{
		unsubscribe(queue, thread_ID+i);
	}

	// Test 'unsubscribe'
	subscribe(queue, thread_ID);
	addMsg(queue, m1);
	addMsg(queue, m2);
	addMsg(queue, m3);
	addMsg(queue, m1);
	unsubscribe(queue, thread_ID);
	subscribe(queue, thread_ID);
	addMsg(queue, m2);
	addMsg(queue, m3);
	num = getAvailable(queue, thread_ID);
	printf("[Result: %d] Test 'unsubscribe' [Details: %d=%d]\n", (num == 2), num, 2);
	unsubscribe(queue, thread_ID);

	// Test 'removeMsg'
	subscribe(queue, thread_ID);
	addMsg(queue, m1);
	addMsg(queue, m2);
	removeMsg(queue, m1);
	p = getMsg(queue, thread_ID);
	printf("[Result: %d] Test 'removeMsg' [Details: %d=%d && %p=%p]\n", (*m2==*p && m2==p), *m2, *p, m2, p);
	unsubscribe(queue, thread_ID);

	// Test 'setSize'
	subscribe(queue, thread_ID);
	addMsg(queue, m1);
	addMsg(queue, m2);
	addMsg(queue, m3);
	addMsg(queue, m1);
	addMsg(queue, m1);
	addMsg(queue, m1);
	setSize(queue, 4);
	p = getMsg(queue, thread_ID);
	printf("[Result: %d] Test 'setSize' [Details: %d=%d && %p=%p]\n", (*m3==*p && m3==p), *m3, *p, m3, p);
	unsubscribe(queue, thread_ID);

	// Test 'destroyQueue'
	destroyQueue(queue);
	printf("[Result: 1] Test 'destroyQueue'\n\n");

	// Zwolnienie pamieci
	free(m1); free(m2); free(m3);
}



int main()
{
	// Testy na pojedynczym watku
	printf("========== SINGLE THREAD TEST ==========\n");
	single_thread_test();

	// Testy na wielu watkach
	printf("========== MULTI THREAD TEST ==========\n");
	const int THREADS_NUM=5, QUEUE_SIZE=1;
	pthread_t threads[THREADS_NUM];

	// Tworzenie kolejki
	TQueue *queue;
	queue = createQueue(QUEUE_SIZE);
	printf("Queue created with size '%d'\n", QUEUE_SIZE);

	// Tworzenie watkow
	pthread_create(&threads[0], NULL, thread_func_1, queue);
	pthread_create(&threads[1], NULL, thread_func_2, queue);
	pthread_create(&threads[2], NULL, thread_func_3, queue);
	pthread_create(&threads[3], NULL, thread_func_4, queue);

	// Tworzenie timera
	pthread_create(&threads[4], NULL, timer, NULL);

	// Czekanie na zakonczenie watkow
	for (int i=0; i<THREADS_NUM-1; i++)
	{
		pthread_join(threads[i], NULL);
	}

	// Zakonczenie timera
	pthread_cancel(threads[4]);

	// Zniszczenie kolejki
	destroyQueue(queue);

	return 0;
}

