#ifndef LCL_QUEUE_H
#define LCL_QUEUE_H

// ==============================================
//
//	Version 1.1, 2025-01-16
//
// ==============================================

#include <pthread.h>

struct TQueue {
	int messages_size;				// Rozmiar tablicy przechowywanych wiadomosci
	int messages_count;				// Aktualna liczba przechowywanych wiadomosci
	int subscribers_size;			// Rozmiar tablicy watkow subskrybujacych
	int subscribers_count;			// Aktualna liczba watkow subskrybujacych

	void **messages;				// Tablica przechowywanych wiadomosci
	int *messages_register;			// Tablica licznikow odczytanych wiadomosci dla watkow
	pthread_t *subscribers;			// Tablica watkow subskrybujacych

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

