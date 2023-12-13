#include "Headers/utilidades_thread_safe.h"

bool queue_is_empty_thread_safe(t_queue *cola, pthread_mutex_t *mutex)
{
    bool resultado;

    pthread_mutex_lock(mutex);
    resultado = queue_is_empty(cola);
    pthread_mutex_unlock(mutex);

    return resultado;
}

void *queue_pop_thread_safe(t_queue *cola, pthread_mutex_t *mutex)
{
    void *elemento;

    pthread_mutex_lock(mutex);
    if (queue_is_empty(cola))
    {
        elemento = NULL;
    }
    else
    {
        elemento = queue_pop(cola);
    }
    
    pthread_mutex_unlock(mutex);

    return elemento;
}

void queue_push_thread_safe(t_queue *cola, void *elemento, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
    queue_push(cola, elemento);
    pthread_mutex_unlock(mutex);
}

void list_add_thread_safe(t_list *lista, void *elemento, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
    list_add(lista, elemento);
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

void list_iterate_thread_safe(t_list *lista, void(*funcion_elemento)(void*), pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	t_list_iterator *iterador = list_iterator_create(lista);

	while (list_iterator_has_next(iterador))
	{
		void *elemento = list_iterator_next(iterador);
        funcion_elemento(elemento);
	}

	list_iterator_destroy(iterador);
	pthread_mutex_unlock(mutex);
}

void* list_find_thread_safe(t_list *lista, bool(*filtro)(void*), pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	void* resultado = list_find(lista, filtro);
	pthread_mutex_unlock(mutex);
    return resultado;
}

void list_remove_and_destroy_by_condition_thread_safe(t_list *lista, bool(*filtro)(void*), void(*destructor)(void*), pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	list_remove_and_destroy_by_condition(lista, filtro, destructor);
	pthread_mutex_unlock(mutex);
}

void list_destroy_and_destroy_elements_thread_safe(t_list *lista, void(*destructor)(void*), pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	list_destroy_and_destroy_elements(lista, destructor);
	pthread_mutex_unlock(mutex);
}

void list_remove_by_condition_thread_safe(t_list *lista, bool(*filtro)(void*), pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	list_remove_by_condition(lista, filtro);
	pthread_mutex_unlock(mutex);
}

void list_sort_thread_safe(t_list *lista, bool (*comparador)(void *, void *), pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	list_sort(lista, comparador);
	pthread_mutex_unlock(mutex);
}

void* list_get_thread_safe(t_list *lista, int indice, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	void* resultado = list_get(lista, indice);
	pthread_mutex_unlock(mutex);
    return resultado;
}

int list_size_thread_safe(t_list *lista, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	int resultado = list_size(lista);
	pthread_mutex_unlock(mutex);
    return resultado;
}

void bitarray_set_bit_thread_safe(t_bitarray *bitarray, int indice, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	bitarray_set_bit(bitarray, indice);
	pthread_mutex_unlock(mutex);
}

void bitarray_clean_bit_thread_safe(t_bitarray *bitarray, int indice, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	bitarray_clean_bit(bitarray, indice);
	pthread_mutex_unlock(mutex);
}

bool bitarray_test_bit_thread_safe(t_bitarray *bitarray, int indice, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
	bool resultado = bitarray_test_bit(bitarray, indice);
	pthread_mutex_unlock(mutex);
    return resultado;
}