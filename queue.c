#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

TQueue* createQueue(int size)
{
	if (size < 0)
		return NULL;

	// Tworzenie
	TQueue *queue = (TQueue *)malloc(sizeof(TQueue));

	queue->messages_size = size;
	queue->messages_count = 0;
	queue->subscribers_size = 16;
	queue->subscribers_count = 0;

	queue->messages = (void **)malloc(sizeof(void *) * queue->messages_size);
	queue->messages_register = (int *)malloc(sizeof(int) * queue->subscribers_size);
	queue->subscribers = (pthread_t *)malloc(sizeof(pthread_t) * queue->subscribers_size);

	pthread_mutex_init(&queue->lock, NULL);
	pthread_cond_init(&queue->cond_not_full, NULL);
	pthread_cond_init(&queue->cond_not_empty, NULL);

	// Sukces
	return queue;
}

void destroyQueue(TQueue *queue)
{
	pthread_mutex_lock(&queue->lock);

	// Niszczenie
	for (int i=0; i<queue->messages_count; i++)
	{
		free(queue->messages[i]);
	}

	free(queue->messages);
	free(queue->messages_register);
	free(queue->subscribers);

	pthread_cond_destroy(&queue->cond_not_full);
	pthread_cond_destroy(&queue->cond_not_empty);

	pthread_mutex_unlock(&queue->lock);
	pthread_mutex_destroy(&queue->lock);

	// Sukces
	free(queue);
}

void subscribe(TQueue *queue, pthread_t thread)
{
	pthread_mutex_lock(&queue->lock);

	// Realokacja pamieci, jesli konieczna
	queue->subscribers_count++;
	if (queue->subscribers_size < queue->subscribers_count)
	{
		queue->subscribers_size *= 2;
		queue->messages_register = (int *)realloc(queue->messages_register, sizeof(int) * queue->subscribers_size);
		queue->subscribers = (pthread_t *)realloc(queue->subscribers, sizeof(pthread_t) * queue->subscribers_size);
	}

	// Dodanie do listy subskrybentow
	queue->subscribers[queue->subscribers_count - 1] = thread;

	// Uznanie wczesniejszych wiadomosci za przeczytane
	queue->messages_register[queue->subscribers_count - 1] = queue->messages_count;

	// Sukces
	pthread_mutex_unlock(&queue->lock);
}

void unsubscribe(TQueue *queue, pthread_t thread)
{
	pthread_mutex_lock(&queue->lock);

	for (int i=0; i<queue->subscribers_count; i++)
	{
		if (pthread_equal(queue->subscribers[i], thread))
		{
			// Przesuniecie watkow
			for (int j=i; j<queue->subscribers_count-1; j++)
			{
				queue->subscribers[j] = queue->subscribers[j+1];
				queue->messages_register[j] = queue->messages_register[j+1];
			}

			// Realokacja pamieci, jesli konieczna
			queue->subscribers_count--;
			if (queue->subscribers_size > 16 && queue->subscribers_size/10 > queue->subscribers_count)
			{
				queue->subscribers_size /= 2;
				queue->messages_register = (int *)realloc(queue->messages_register, sizeof(int) * queue->subscribers_size);
				queue->subscribers = (pthread_t *)realloc(queue->subscribers, sizeof(pthread_t) * queue->subscribers_size);
			}

			// Jesli najstarsza wiadomosc zostala odczytana przez wszystkie watki
			int all_read = 1;
			while (queue->messages_count > 0 && all_read == 1)
			{
				for (int i=0; i<queue->subscribers_count; i++)
				{
					if (queue->messages_register[i] <= 0)
					{
						all_read = 0;
						break;
					}
				}
				if (all_read == 1) // Usun najstarsza wiadomosc
				{
					// Przesuniece wiadomosci
					for (int i=0; i<queue->messages_count-1; i++)
					{
						queue->messages[i] = queue->messages[i+1];
					}

					// Dostosuj liczniki wiadomosci dla subskrybentow
					queue->messages_count--;
					for (int i=0; i<queue->subscribers_count; i++)
					{
						queue->messages_register[i]--;
					}

					// Obudz czekajacych na dodanie nowych wiadomosci
					pthread_cond_signal(&queue->cond_not_full);
				}
			}

			// Sukces
			pthread_mutex_unlock(&queue->lock);
			return;
		}
	}

	// Ten watek nie byl subskrybentem
	pthread_mutex_unlock(&queue->lock);
}

void addMsg(TQueue *queue, void *msg)
{
	pthread_mutex_lock(&queue->lock);

	// Jesli nie ma subskrybentow
	if (queue->subscribers_count <= 0)
	{
		pthread_mutex_unlock(&queue->lock);
		return;
	}

	// Czekaj jesli kolejka jest pelna
	while (queue->messages_count >= queue->messages_size)
	{
		pthread_cond_wait(&queue->cond_not_full, &queue->lock);
	}

	// Dodaj wiadomosc
	queue->messages[queue->messages_count] = msg;
	queue->messages_count++;

	// Obudz czekajacych na odczytanie nowych wiadomosci
	pthread_cond_broadcast(&queue->cond_not_empty);

	// Sukces
	pthread_mutex_unlock(&queue->lock);
}

void* getMsg(TQueue *queue, pthread_t thread)
{
	pthread_mutex_lock(&queue->lock);

	// Znajdz subskrybenta
	int subscriber_index = -1;
	for (int i=0; i<queue->subscribers_count; i++)
	{
		if (pthread_equal(queue->subscribers[i], thread))
		{
			subscriber_index = i;
			break;
		}
	}
	if (subscriber_index == -1) // Watek nie jest subskrybentem
	{
		pthread_mutex_unlock(&queue->lock);
		return NULL;
	}

	// Czekaj jesli nie ma nowych wiadomosci
	while (queue->messages_register[subscriber_index] >= queue->messages_count)
	{
		pthread_cond_wait(&queue->cond_not_empty, &queue->lock);
	}

	// Znajdz subskrybenta (index mogl ulec zmianie badz watek mogl odsubskrybowac)
	subscriber_index = -1;
	for (int i=0; i<queue->subscribers_count; i++)
	{
		if (pthread_equal(queue->subscribers[i], thread))
		{
			subscriber_index = i;
			break;
		}
	}
	if (subscriber_index == -1) // Watek nie jest subskrybentem
	{
		pthread_mutex_unlock(&queue->lock);
		return NULL;
	}

	// Pobierz wiadomosc
	void *msg = queue->messages[queue->messages_register[subscriber_index]];
	queue->messages_register[subscriber_index]++;

	// Jesli najstarsza wiadomosc zostala odczytana przez wszystkie watki
	int all_read = 1;
	while (queue->messages_count > 0 && all_read == 1)
	{
		for (int i=0; i<queue->subscribers_count; i++)
		{
			if (queue->messages_register[i] <= 0)
			{
				all_read = 0;
				break;
			}
		}
		if (all_read == 1) // Usun najstarsza wiadomosc
		{
			// Przesuniece wiadomosci
			for (int i=0; i<queue->messages_count-1; i++)
			{
				queue->messages[i] = queue->messages[i+1];
			}

			// Dostosuj liczniki wiadomosci dla subskrybentow
			queue->messages_count--;
			for (int i=0; i<queue->subscribers_count; i++)
			{
				queue->messages_register[i]--;
			}

			// Obudz czekajacych na dodanie nowych wiadomosci
			pthread_cond_signal(&queue->cond_not_full);
		}
	}

	// Sukces
	pthread_mutex_unlock(&queue->lock);
	return msg;
}

int getAvailable(TQueue *queue, pthread_t thread)
{
	pthread_mutex_lock(&queue->lock);

	// Znajdz subskrybenta
	int subscriber_index = -1;
	for (int i=0; i<queue->subscribers_count; i++)
	{
		if (pthread_equal(queue->subscribers[i], thread))
		{
			subscriber_index = i;
			break;
		}
	}
	if (subscriber_index == -1) // Watek nie jest subskrybentem
	{
		pthread_mutex_unlock(&queue->lock);
		return -1;
	}

	// Pobierz wartosc
	int result = queue->messages_count - queue->messages_register[subscriber_index];

	// Sukces
	pthread_mutex_unlock(&queue->lock);
	return result;
}

void removeMsg(TQueue *queue, void *msg)
{
	pthread_mutex_lock(&queue->lock);

	// Znajdz wiadomosc w kolejce
	int found = -1;
	for (int i=0; i<queue->messages_count; i++)
	{
		if (queue->messages[i] == msg)
		{
			found = i;
			break;
		}
	}
	if (found == -1) // Wiadomosci nie ma w kolejce
	{
		pthread_mutex_unlock(&queue->lock);
		return;
	}

	// Przesuniece wiadomosci
	for (int i=found; i<queue->messages_count-1; i++)
	{
		queue->messages[i] = queue->messages[i+1];
	}

	// Dostosuj liczniki wiadomosci dla subskrybentow
	queue->messages_count--;
	for (int i=0; i<queue->subscribers_count; i++)
	{
		if (queue->messages_register[i] > found)
		{
			queue->messages_register[i]--;
		}
	}

	// Obudz czekajacych na dodanie nowych wiadomosci
	pthread_cond_signal(&queue->cond_not_full);

	// Sukces
	pthread_mutex_unlock(&queue->lock);
}

void setSize(TQueue *queue, int size)
{
	pthread_mutex_lock(&queue->lock);

	if (size < 0)
	{
		pthread_mutex_unlock(&queue->lock);
		return;
	}

	// Jesli nowy rozmiar jest mniejszy niz ilosc wiadomosci
	if (size < queue->messages_count)
	{
		int diff = queue->messages_count - size;

		// Przesun wiadomosci pozostajace w kolejce
		for (int i=diff; i<queue->messages_count; i++)
		{
			queue->messages[i-diff] = queue->messages[i];
		}

		// Dostosuj liczniki odczytanych wiadomosci dla watkow
		for (int i=0; i<queue->subscribers_count; i++)
		{
			if (queue->messages_register[i] - diff > 0)
				queue->messages_register[i] -= diff;
			else
				queue->messages_register[i] = 0;
		}

		queue->messages_count = size;
	}

	// Realokacja pamieci
	queue->messages = (void **)realloc(queue->messages, sizeof(void *) * size);
	if (queue->messages == NULL)
	{
		pthread_mutex_unlock(&queue->lock);
		return;
	}

	// Ustaw rozmiar
	queue->messages_size = size;

	// Jesli w kolejce sa wolne miejsca na wiadomosci
	if (queue->messages_size > queue->messages_count)
		pthread_cond_broadcast(&queue->cond_not_full);

	// Sukces
	pthread_mutex_unlock(&queue->lock);
}

