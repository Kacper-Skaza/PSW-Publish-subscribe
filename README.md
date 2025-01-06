<h1 align="center">Publish-subscribe (20 pkt)</h1>
<h3 align="center">
Programowanie systemowe i współbieżne<br>
Kacper Skaza 160 174<br>
&lt;kacper.skaza@student.put.poznan.pl&gt;<br>
v1.0, 2025-01-06
</h3>



Projekt jest dostępny w repozytorium pod adresem:<br>
<https://github.com/Kacper-Skaza/Publish-subscribe-PSiW>



# Struktury danych

Cała kolejka jest definiowana strukturą `TQueue`. pozwala to na łatwy dostęp do
potrzebnych zasobów, gdyż przy wywoływaniu funkcji wystarczy podać wskaźnik.

- `TQueue`:

```C
typedef struct TQueue
{
	pthread_t *subscribers;		// Tablica watkow subskrybujacych
	void **messages;		// Tablica wskaznikow na wiadomosci
	int *message_register;		// Tablica licznikow odczytanych wiadomosci dla watkow

	void *last_message;		// Wskaznik na ostatnia wyslana wiadomosc
	int subscriber_count;		// Liczba subskrybentow
	int message_count;		// Liczba przechowywanych wiadomosci
	int size;			// Rozmiar kolejki

	pthread_mutex_t lock;		// Mutex do synchronizacji
	pthread_cond_t cond_not_full;	// Zmienna warunkowa dla niepelnej kolejki
	pthread_cond_t cond_not_empty;	// Zmienna warunkowa dla niepustej kolejki
} TQueue;
```



# Funkcje

- `createQueue(TQueue *queue, int *size)` —
inicjuje strukturę `TQueue` reprezentującą nową kolejkę o początkowym,
maksymalnym rozmiarze `size`.

- `destroyQueue(TQueue *queue)` —
usuwa kolejkę `queue` i zwalnia pamięć przez nią zajmowaną. Próba dostarczania
lub odbioru nowych wiadomości z takiej kolejki będzie kończyła się błędem.

- `subscribe(TQueue *queue, pthread_t *thread)` —
rejestruje wątek `thread` jako kolejnego odbiorcę wiadomości z kolejki `queue`.

- `unsubscribe(TQueue *queue, pthread_t *thread)` —
wyrejestrowuje wątek `thread` z kolejki `queue`. Nieodebrane przez wątek
wiadomości są traktowane jako odebrane.

- `addMsg(TQueue *queue, void *msg)` —
wstawia do kolejki `queue` nową wiadomość reprezentowaną wskaźnikiem `msg`.

- `void* getMsg(TQueue *queue, pthread_t *thread)` —
odbiera pojedynczą wiadomość z kolejki `queue` dla wątku `thread`. Jeżeli nie ma
nowych wiadomości, funkcja jest blokująca. Jeżeli wątek `thread` nie jest
zasubskrybowany – zwracany jest pusty wskaźnik `NULL`.

- `getAvailable(TQueue *queue, pthread_t *thread)` —
zwraca liczbę wiadomości z kolejki `queue` dostępnych dla wątku `thread`.

- `removeMsg(TQueue *queue, void *msg)` —
usuwa wiadomość `msg` z kolejki.

- `setSize(TQueue *queue, int *size)` —
ustala nowy, maksymalny rozmiar kolejki. Jeżeli nowy rozmiar jest mniejszy od
aktualnej liczby wiadomości w kolejce, to nadmiarowe wiadomości są usuwane
z kolejki, począwszy od najstarszych



# Przykład użycia

* Dane wejściowe:

```C
─────────────────────────────────────────────────────────────────────
Faza	Wątek T₁	Wątek T₂	Wątek T₃	Wątek T₄
─────────────────────────────────────────────────────────────────────
1: 	put(m₁)
2: 			subscribe()
3: 	put(m₂)
4: 			1←getAvail() 	subscribe()
5: 					get()
6: 			m₂←get() 	│
7: 			get() 		│
8: 	put(m₃) 	│ 		│
9: 			m₃← 		m₃←
10: 	setSize(2)
11: 	put(m₄)
12: 			1←getAvail() 	1←getAvail() 	subscribe()
13: 	put(m₅)
14: 	put(m₆) 	2←getAvail() 	2←getAvail()	1←getAvail()
15: 	│ 						unsubscribe()
16: 	│ 				m₄←get()
17: 	┴ 		m₄←get()
18: 					unsubscribe()
19: 	put(m₇)
20: 	│ 		2←getAvail()
21: 	┴ 		m₅←get()
22: 			m₆←get()
23: 			m₇←get()
─────────────────────────────────────────────────────────────────────
```

* Dane wyjściowe:

```C
=== Queue created with size 1 ===

========== 1 ==========
Expected time: 1; Thread [1] is attempting to add message '1. Hello world!' to the queue.
Expected time: 1; Thread [1] successfully added message '1. Hello world!' to the queue.

========== 2 ==========
>> Expected time: 2; Thread [2] subscribed.

========== 3 ==========
Expected time: 3; Thread [1] is attempting to add message '2. Hello world!' to the queue.
Expected time: 3; Thread [1] successfully added message '2. Hello world!' to the queue.

========== 4 ==========
>> Expected time: 4; Thread [3] subscribed.
Expected time: 4; Thread [2] has 1 messages available [Expected result: 1].

========== 5 ==========
Expected time: 5; Thread [3] attempting to receive message.

========== 6 ==========
Expected time: 6; Thread [2] attempting to receive message.
Expected time: 6; Thread [2] received message: '2. Hello world!'

========== 7 ==========
Expected time: 7; Thread [2] attempting to receive message.

========== 8 ==========
Expected time: 8; Thread [1] is attempting to add message '3. Hello world!' to the queue.
Expected time: 8; Thread [1] successfully added message '`mż \☺' to the queue.
Expected time: 8; Thread [2] received message: '3. Hello world!'
Expected time: 8; Thread [3] received message: '3. Hello world!'

========== 9 ==========

========== 10 ==========
Expected time: 10; Thread [1] is changed size of queue to '2'.

========== 11 ==========
Expected time: 11; Thread [1] is attempting to add message '4. Hello world!' to the queue.
Expected time: 11; Thread [1] successfully added message '4. Hello world!' to the queue.

========== 12 ==========
>> Expected time: 12; Thread [4] subscribed.
Expected time: 12; Thread [3] has 1 messages available [Expected result: 1].
Expected time: 12; Thread [2] has 1 messages available [Expected result: 1].

========== 13 ==========
Expected time: 13; Thread [1] is attempting to add message '5. Hello world!' to the queue.
Expected time: 13; Thread [1] successfully added message '5. Hello world!' to the queue.

========== 14 ==========
Expected time: 14; Thread [4] has 1 messages available [Expected result: 1].
Expected time: 14; Thread [3] has 2 messages available [Expected result: 2].
Expected time: 14; Thread [2] has 2 messages available [Expected result: 2].
Expected time: 14; Thread [1] is attempting to add message '6. Hello world!' to the queue.

========== 15 ==========
>> Expected time: 15; Thread [4] unsubscribed.
>> Expected time: 15; Thread [4] ended.

========== 16 ==========
Expected time: 16; Thread [3] attempting to receive message.
Expected time: 16; Thread [3] received message: '4. Hello world!'

========== 17 ==========
Expected time: 17; Thread [2] attempting to receive message.
Expected time: 17; Thread [2] received message: '4. Hello world!'
Expected time: 17; Thread [1] successfully added message '6. Hello world!' to the queue.

========== 18 ==========
>> Expected time: 18; Thread [3] unsubscribed.
>> Expected time: 18; Thread [3] ended.

========== 19 ==========
Expected time: 19; Thread [1] is attempting to add message '7. Hello world!' to the queue.

========== 20 ==========
Expected time: 20; Thread [2] has 2 messages available [Expected result: 2].

========== 21 ==========
Expected time: 21; Thread [2] attempting to receive message.
Expected time: 21; Thread [2] received message: '5. Hello world!'
Expected time: 21; Thread [1] successfully added message '7. Hello world!' to the queue.
>> Expected time: 21; Thread [1] ended.

========== 22 ==========
Expected time: 22; Thread [2] attempting to receive message.
Expected time: 22; Thread [2] received message: '6. Hello world!'

========== 23 ==========
Expected time: 23; Thread [2] attempting to receive message.
Expected time: 23; Thread [2] received message: '7. Hello world!'
>> Expected time: 23; Thread [2] unsubscribed.
>> Expected time: 23; Thread [2] ended.
```
