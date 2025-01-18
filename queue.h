#ifndef LCL_QUEUE_H
#define LCL_QUEUE_H

// ==============================================
//
// Version 1.1, 2025-01-16
//
// ==============================================

#include <pthread.h>

struct TQueue {
	pthread_t *subscribers;			// Tablica watkow subskrybujacych
	void **messages;				// Tablica wskaznikow na wiadomosci
	int *message_register;			// Tablica licznikow odczytanych wiadomosci dla watkow

	int subscriber_count;			// Liczba subskrybentow
	int message_count;				// Liczba przechowywanych wiadomosci
	int size;						// Rozmiar kolejki

	pthread_mutex_t lock;			// Mutex do synchronizacji
	pthread_cond_t cond_not_full;	// Zmienna warunkowa dla niepelnej kolejki
	pthread_cond_t cond_not_empty;	// Zmienna warunkowa dla niepustej kolejki
};
typedef struct TQueue TQueue;

TQueue* createQueue(int size);
void destroyQueue(TQueue *queue);

void subscribe(TQueue *queue, pthread_t thread);
void unsubscribe(TQueue *queue, pthread_t thread);

void addMsg(TQueue *queue, void *msg);
void* getMsg(TQueue *queue, pthread_t thread);
int getAvailable(TQueue *queue, pthread_t thread);
void removeMsg(TQueue *queue, void *msg);

void setSize(TQueue *queue, int size);

#endif //LCL_QUEUE_H

