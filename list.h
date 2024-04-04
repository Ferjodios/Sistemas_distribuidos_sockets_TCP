#ifndef _LISTA_H
#define _LISTA_H        1

#define MAX_N 32
#define MAX_VALUE_LENGTH	256
#include <stddef.h>

struct Node{ 
	char 	v1[MAX_VALUE_LENGTH];
	int 	key;
	int 	N;
	double	v2[MAX_N];
	struct 	Node *next; 
};


typedef struct Node * List;
int init_list(List *l);
int set(List *l, char *v1,  int key, int N, double *v2);
int exist_element(List l, int key);
int get_element(List l, int key, char *value1, int *N_value2, double *V_value2);
int modify_element(List l, int key, char *new_v1, int new_N, double *new_v2);
int delete_element(List *l, int key);
int destroy(List *l);

#endif

