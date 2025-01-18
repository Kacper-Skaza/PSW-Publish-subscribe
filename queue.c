#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"

TQueue* createQueue(int size)
{
	if (size < 0)
	{
		return NULL;
	}

	TQueue *queue = (TQueue *)malloc(sizeof(TQueue));
	if (queue == NULL)
	{
		return NULL;
	}

	queue->subscribers = NULL;
	queue->messages = (void **)malloc(sizeof(void *) * size);
	queue->message_register = NULL;

	queue->subscriber_count = 0;
	queue->message_count = 0;
	queue->size = size;

	pthread_mutex_init(&queue->lock, NULL);
	pthread_cond_init(&queue->cond_not_full, NULL);
	pthread_cond_init(&queue->cond_not_empty, NULL);

	return queue;
}

void destroyQueue(TQueue *queue)
{
	pthread_mutex_lock(&queue->lock);

	for (int i=0; i<queue->message_count; i++)
	{
		free(queue->messages[i]);
	}

	free(queue->subscribers);
	free(queue->messages);
	free(queue->message_register);

	pthread_cond_destroy(&queue->cond_not_full);
	pthread_cond_destroy(&queue->cond_not_empty);

	pthread_mutex_unlock(&queue->lock);
	pthread_mutex_destroy(&queue->lock);
}

void subscribe(TQueue *queue, pthread_t thread)
{
	pthread_mutex_lock(&queue->lock);

	// Realokacja pamieci
	queue->subscriber_count++;
	queue->subscribers = (pthread_t *)realloc(queue->subscribers, sizeof(pthread_t) * queue->subscriber_count);
	queue->message_register = (int *)realloc(queue->message_register, sizeof(int) * queue->subscriber_count);
	if (!queue->subscribers || !queue->message_register)
	{
		pthread_mutex_unlock(&queue->lock);
		return;
	}

	queue->subscribers[queue->subscriber_count - 1] = thread; // Dodanie do listy subskrybentow
	queue->message_register[queue->subscriber_count - 1] = queue->message_count; // Uznanie wczesniejszych wiadomosci za przeczytane

	pthread_mutex_unlock(&queue->lock);
}

void unsubscribe(TQueue *queue, pthread_t thread)
{
	pthread_mutex_lock(&queue->lock);

	for (int i=0; i<queue->subscriber_count; i++)
	{
		if (pthread_equal(queue->subscribers[i], thread))
		{
			// Przesuniecie watkow
			for (int j=i; j<queue->subscriber_count-1; j++)
			{
				queue->subscribers[j] = queue->subscribers[j+1];
				queue->message_register[j] = queue->message_register[j+1];
			}

			// Realokacja pamieci
			queue->subscriber_count--;
			queue->subscribers = (pthread_t *)realloc(queue->subscribers, sizeof(pthread_t) * queue->subscriber_count);
			queue->message_register = (int *)realloc(queue->message_register, sizeof(int) * queue->subscriber_count);

			// Jesli najstarsza wiadomosc zostala odczytana przez wszystkie watki
			int all_read = 1;
			while (queue->message_count > 0 && all_read == 1)
			{
				for (int i=0; i<queue->subscriber_count; i++)
				{
					if (queue->message_register[i] == 0)
					{
						all_read = 0;
						break;
					}
				}
				if (all_read == 1)
				{
					removeMsg(queue, queue->messages[0]);
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
	if (queue->subscriber_count <= 0)
	{
		pthread_mutex_unlock(&queue->lock);
		return;
	}

	// Czekaj jesli kolejka jest pelna
	while (queue->message_count >= queue->size)
	{
		pthread_cond_wait(&queue->cond_not_full, &queue->lock);
	}

	// Dodaj wiadomosc
	queue->messages[queue->message_count] = msg;
	queue->message_count++;

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
	for (int i=0; i<queue->subscriber_count; i++)
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
	while (queue->message_register[subscriber_index] >= queue->message_count)
	{
		pthread_cond_wait(&queue->cond_not_empty, &queue->lock);
	}

	// Pobierz wiadomosc
	void *msg = queue->messages[queue->message_register[subscriber_index]];
	queue->message_register[subscriber_index]++;

	// Jesli najstarsza wiadomosc zostala odczytana przez wszystkie watki
	int all_read = 1;
	while (queue->message_count > 0 && all_read == 1)
	{
		for (int i=0; i<queue->subscriber_count; i++)
		{
			if (queue->message_register[i] == 0)
			{
				all_read = 0;
				break;
			}
		}
		if (all_read == 1)
		{
			removeMsg(queue, queue->messages[0]);
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
	for (int i=0; i<queue->subscriber_count; i++)
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
	int available = queue->message_count - queue->message_register[subscriber_index];

	// Sukces
	pthread_mutex_unlock(&queue->lock);
	return available;
}

void removeMsg(TQueue *queue, void *msg)
{
	// Brak blokady
	//pthread_mutex_lock(&queue->lock);

	// Znajdz wiadomosc w kolejce
	int found = -1;
	for (int i=0; i<queue->message_count; i++)
	{
		if (queue->messages[i] == msg)
		{
			found = i;
			// Interfejs nie odpowiada za zwalnianie pamieci
			//free(queue->messages[i]);
			break;
		}
	}
	if (found == -1) // Wiadomosci nie ma w kolejce
	{
		pthread_mutex_unlock(&queue->lock);
		return;
	}

	// Przesuniece wiadomosci
	for (int i=found; i<queue->message_count-1; i++)
	{
		queue->messages[i] = queue->messages[i+1];
	}

	// Dostosuj liczniki wiadomosci dla subskrybentow
	queue->message_count--;
	for (int i=0; i<queue->subscriber_count; i++)
	{
		if (queue->message_register[i] > found)
		{
			queue->message_register[i]--;
		}
	}

	// Obudz czekajacych na dodanie nowych wiadomosci
	pthread_cond_signal(&queue->cond_not_full);

	// Brak blokady
	//pthread_mutex_unlock(&queue->lock);
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
	if (size < queue->message_count)
	{
		int diff = queue->message_count - size;

		// Usun nadmiarowe wiadomosci
		for (int i=0; i<diff; i++)
		{
			free(queue->messages[i]);
		}

		// Przesun pozostale wiadomosci
		for (int i=diff; i<queue->message_count; i++)
		{
			queue->messages[i - diff] = queue->messages[i];
		}

		// Dostosuj liczniki odczytanych wiadomosci dla watkow
		for (int i=0; i<queue->subscriber_count; i++)
		{
			if (queue->message_register[i] - diff > 0)
				queue->message_register[i] -= diff;
			else
				queue->message_register[i] = 0;
		}

		queue->message_count = size;
	}

	// Realokacja pamieci
	queue->messages = (void **)realloc(queue->messages, sizeof(void *) * size);
	if (queue->messages == NULL)
	{
		pthread_mutex_unlock(&queue->lock);
		return;
	}

	// Ustaw rozmiar
	queue->size = size;

	// Jesli w kolejce sa wolne miejsca na wiadomosci
	if (queue->size > queue->message_count)
		pthread_cond_broadcast(&queue->cond_not_full);

	// Sukces
	pthread_mutex_unlock(&queue->lock);
}

