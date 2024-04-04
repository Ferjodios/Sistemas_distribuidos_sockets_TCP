
#include "claves.h"
#include "list.h"
#include <stdio.h>
#include <pthread.h>
List messages;

int inited_list = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int init()
{
	pthread_mutex_lock(&mutex);
	if (destroy(&messages) == -1)
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
	if (init_list(&messages) == -1)
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
	inited_list = 1;
	pthread_mutex_unlock(&mutex);
	return (0);
}

int set_value(int key, char *value1, int N_value2, double *V_value2)
{
	pthread_mutex_lock(&mutex);
	if (inited_list == 0)
	{
		if (init_list(&messages) == -1)
		{
			pthread_mutex_unlock(&mutex);
			return (-1);
		}
		inited_list = 1;
	}
	if (exist_element(messages, key) == 0)
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
	if (N_value2 > 32 || N_value2 < 1)
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
	if (set(&messages, value1, key, N_value2, V_value2) == 0)
	{
		pthread_mutex_unlock(&mutex);
		return (0);
	}
	else
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
}

int get_value(int key, char *value1, int *N_value2, double *V_value2)
{
	pthread_mutex_lock(&mutex);
	if (inited_list == 0)
	{
		if (init_list(&messages) == -1)
		{
			pthread_mutex_unlock(&mutex);
			return (-1);
		}
		inited_list = 1;
	}
	if (get_element(messages, key, value1, N_value2, V_value2) == 0)
	{
		pthread_mutex_unlock(&mutex);
		return (0);
	}
	else
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
}

int modify_value(int key, char *value1, int N_value2, double *V_value2)
{
	pthread_mutex_lock(&mutex);
	if (inited_list == 0)
	{
		if (init_list(&messages) == -1)
		{
			pthread_mutex_unlock(&mutex);
			return (-1);
		}
		inited_list = 1;
	}
	if (exist_element(messages, key) == -1)
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
	if (N_value2 > 32 || N_value2 < 1)
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
	if (modify_element(messages, key, value1, N_value2, V_value2) == 0)
	{
		pthread_mutex_unlock(&mutex);
		return (0);
	}
	else
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
}

int delete_key(int key)
{
	pthread_mutex_lock(&mutex);
	if (inited_list == 0)
	{
		if (init_list(&messages) == -1)
		{
			pthread_mutex_unlock(&mutex);
			return (-1);
		}
		inited_list = 1;
	}
	if (exist_element(messages, key) == -1)
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}
	if (delete_element(&messages, key) == 0)
	{
		pthread_mutex_unlock(&mutex);
		return (0);
	}
	else
	{
		pthread_mutex_unlock(&mutex);
		return (-1);
	}	
}

int exist(int key)
{
	pthread_mutex_lock(&mutex);
	if (inited_list == 0)
	{
		if (init_list(&messages) == -1)
		{
			pthread_mutex_unlock(&mutex);
			return (-1);
		}
		inited_list = 1;
	}
	if (exist_element(messages, key) == 0)
	{
		pthread_mutex_unlock(&mutex);
		return (1);
	}
	else
	{
		pthread_mutex_unlock(&mutex);
		return (0);
	}
}

//Por si alguien quisiera ver la lista, no te olvide de incluir en el .h
/*void print_list()
{
    printf("Quiero imprimir la lista\n");
    if (messages == NULL) {
        printf("La lista está vacía\n");
        return;
    }

    List current = messages;
    // Iterar sobre todos los nodos de la lista
    while (current != NULL) {
        printf("v1: %s\n", current->v1);
        printf("key: %d\n", current->key);
        printf("N: %d\n", current->N);
        printf("v2: ");
        for (int i = 0; i < current->N; i++) {
            printf("%f ", current->v2[i]);
        }
        printf("\n\n");

        current = current->next; // Avanzar al siguiente nodo
    }
}*/

