#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "list.h"

int init_list(List *l)
{
	*l = NULL;
	return (0);
}

int set(List *l, char *v1, int key, int N, double *v2)
{

    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    if (newNode == NULL)
        return -1;
    strcpy(newNode->v1, v1);
    newNode->key = key;
    newNode->N = N;
    for (int i = 0; i < N; i++)
        newNode->v2[i] = v2[i];
    newNode->next = *l;
    *l = newNode;
    return 0;
}

int exist_element(List l, int key)
{
    while (l != NULL)
	{
        if (l->key == key)
            return (0);
        l = l->next;
    }
    return (-1);
}

int get_element(List l, int key, char *value1, int *N_value2, double *V_value2)
{
    while (l != NULL)
	{
        if (l->key == key)
		{
            strcpy(value1, l->v1);
            *N_value2 = l->N;
            memcpy(V_value2, l->v2, l->N * sizeof(double));
            return 0;
        }
        l = l->next;
    }
    return -1;
}

int modify_element(List l, int key, char *new_v1, int new_N, double *new_v2)
{
    while (l != NULL)
	{
        if (l->key == key)
		{
            strcpy(l->v1, new_v1);
            l->N = new_N;
            for (int i = 0; i < new_N; i++)
                l->v2[i] = new_v2[i];
            return 0;
        }
        l = l->next;
    }
    return -1;
}

int delete_element(List *l, int key)
{
    struct Node *prev = NULL;
    struct Node *current = *l;

    while (current != NULL && current->key != key)
	{
        prev = current;
        current = current->next;
    }
    if (current == NULL)
        return -1;
    if (prev == NULL)
        *l = current->next;
    else
        prev->next = current->next;
    free(current);
    return 0;
}

int destroy(List *l)
{
	struct Node *temp;
    while (*l != NULL)
	{
        temp = *l;
        *l = (*l)->next;
        free(temp);
    }
	return 0;
}
