#ifndef LIST_H
#define LIST_H

#include <pthread.h>

typedef struct TQueue
{
	pthread_t *subscribers;			// Tablica watkow subskrybujacych
	void **messages;				// Tablica wskaznikow na wiadomosci
	int *message_register;			// Tablica licznikow odczytanych wiadomosci dla watkow

	void *last_message;				// Wskaznik na ostatnia wyslana wiadomosc
	int subscriber_count;			// Liczba subskrybentow
	int message_count;				// Liczba przechowywanych wiadomosci
	int size;						// Rozmiar kolejki

	pthread_mutex_t lock;			// Mutex do synchronizacji
	pthread_cond_t cond_not_full;	// Zmienna warunkowa dla niepelnej kolejki
	pthread_cond_t cond_not_empty;	// Zmienna warunkowa dla niepustej kolejki
} TQueue;

int createQueue(TQueue *queue, int size);
int destroyQueue(TQueue *queue);

int subscribe(TQueue *queue, pthread_t *thread);
int unsubscribe(TQueue *queue, pthread_t *thread);

int addMsg(TQueue *queue, void *msg);
void* getMsg(TQueue *queue, pthread_t *thread);
int getAvailable(TQueue *queue, pthread_t *thread);
int removeMsg(TQueue *queue, void *msg);

int setSize(TQueue *queue, int size);

#endif

