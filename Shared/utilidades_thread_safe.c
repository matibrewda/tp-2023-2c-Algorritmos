#include "Headers/utilidades_thread_safe.h"

void *queue_pop_thread_safe(t_queue *cola, pthread_mutex_t *mutex)
{
    void *elemento;

    pthread_mutex_lock(mutex);
    elemento = queue_pop(cola);
    pthread_mutex_unlock(mutex);

    return elemento;
}

void queue_push_thread_safe(t_queue *cola, void *elemento, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
    queue_push(cola, elemento);
    pthread_mutex_unlock(mutex);
}

void queue_iterate_thread_safe(t_queue *cola, void(*funcion_elemento)(void*), pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	t_list_iterator *iterador = list_iterator_create(cola->elements);

	while (list_iterator_has_next(iterador))
	{
		void *elemento = list_iterator_next(iterador);
        funcion_elemento(elemento);
	}

	list_iterator_destroy(iterador);
	pthread_mutex_unlock(mutex);
}