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

void *queue_pop_thread_safe(t_queue *cola, pthread_mutex_t *mutex);
void queue_push_thread_safe(t_queue *cola, void *elemento, pthread_mutex_t *mutex);
void queue_iterate_thread_safe(t_queue *cola, void(*funcion_elemento)(void*), pthread_mutex_t *mutex);

#endif /* UTILIDADES_THREAD_SAFE_H_ */
