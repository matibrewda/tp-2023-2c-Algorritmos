#ifndef UTILIDADES_THREAD_SAFE_H_
#define UTILIDADES_THREAD_SAFE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

#include <commons/collections/list.h>
#include <commons/collections/queue.h>

bool queue_is_empty_thread_safe(t_queue *cola, pthread_mutex_t *mutex);
void *queue_pop_thread_safe(t_queue *cola, pthread_mutex_t *mutex);
void queue_push_thread_safe(t_queue *cola, void *elemento, pthread_mutex_t *mutex);
void queue_iterate_thread_safe(t_queue *cola, void(*funcion_elemento)(void*), pthread_mutex_t *mutex);
void* list_find_thread_safe(t_list *lista, bool(*filtro)(void*), pthread_mutex_t *mutex);
void list_remove_and_destroy_by_condition_thread_safe(t_list *lista, bool(*filtro)(void*), void(*destructor)(void*), pthread_mutex_t *mutex);
void list_sort_thread_safe(t_list *lista, bool (*comparador)(void *, void *), pthread_mutex_t *mutex);

#endif /* UTILIDADES_THREAD_SAFE_H_ */
