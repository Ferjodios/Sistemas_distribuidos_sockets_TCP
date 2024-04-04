#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "proxy.h"
#define NUM_THREADS 50

/*Prueba que el programa funciona correctamente, hace uso de la funcion
imprimir lista simplemente para saber que funciona correcto. No se presupone
que esta comunicacion del cliente sea realista*/
int check_program()
{
	// Obtener un valor
    char value1_get[50];
    int N_value2_get = 3;
    double V_value2_get[N_value2_get];

    double v343[] = {3, 4, 3};
    double v123456789[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    double v42[] = {42.424242, 42.424242};

    // Establecer un valores para la lista
    if (set_value(343, "Mi numero favorito", 3, v343) == -1)
        fprintf(stderr, "Error setting value for key\n");
    if (set_value(1, "Numeros del 1 al 9", 9, v123456789) == -1)
        fprintf(stderr, "Error setting value for key\n");
    if (set_value(42, "Viva 42 telefonica", 2, v42) == -1)
        fprintf(stderr, "Error setting value for key\n");
	
    // Establecer el mismo valor nuevamente (Tiene que dar error)
    if (set_value(343, "Mi numero favorito", 3, v343) == -1)
        fprintf(stderr, "Error setting value for key\n");

	//Obtener valor
    if (get_value(343, value1_get, &N_value2_get, V_value2_get) == -1)
        fprintf(stderr, "Error getting value for key.\n");
    else
    {
		printf("Your values: v1: %s, N2: %d\n", value1_get, N_value2_get);
		printf("Contenido de v2:\n");
		for(int i = 0; i < N_value2_get; i++)
		{
			printf("%f ", V_value2_get[i]);
		}
		printf("\n");
    }

    // Modificar un valor
    if (modify_value(343, "Me encanta el numero este", 3, v343) == -1)
        fprintf(stderr, "Error modifying value for key.\n");
    if (modify_value(1, "Invasion de 42", 2, v42) == -1)
        fprintf(stderr, "Error modifying value for key.\n");

    // Verificar la existencia de una clave
    if (exist(343) <= 0)
        fprintf(stderr, "Key does not exist.\n");

    // Eliminar una clave
    if (delete_key(343) == -1)
        fprintf(stderr, "Error deleting key\n");

	//Obtener el valor de una key que no deberia existir
    if (get_value(343, value1_get, &N_value2_get, V_value2_get) == -1)
        fprintf(stderr, "Error getting value for key.\n");
    else
    {
		printf("Your values: v1: %s, N2: %d\n", value1_get, N_value2_get);
		printf("Contenido de v2:\n");
		for(int i = 0; i < N_value2_get; i++)
		{
			printf("%f ", V_value2_get[i]);
		}
		printf("\n");
    }

    // Verificar la existencia de una clave (No deberia existir)
    if (exist(343) <= 0)
        fprintf(stderr, "Key does not exist.\n");

    return 0;
}

/*Probara enviandole cosas que la cola no deberia enviar*/
int check_program_error()
{
    char v1[300];
    char character = 'A';
	double v343[] = {3, 4, 3};

    for (int i = 0; i < 300; ++i)
	{
        v1[i] = character;
        ++character;
        if (character > 'Z')
            character = 'A';
    }

	//Me deberia decir que imposible v1
	if (set_value(2, v1, 3, v343) == -1)
        fprintf(stderr, "Error setting value for key\n");
	//Me deberia decir que imposible N
	if (set_value(2, "Mi numero favorito", -4, v343) == -1)
        fprintf(stderr, "Error setting value for key\n");
	//Me deberia decir que imposible N
	if (set_value(2, "Mi numero favorito", 53, v343) == -1)
        fprintf(stderr, "Error setting value for key\n");
	return (0);
}

void *hilo_cliente(void *arg)
{
    int key = *((int *)arg);
    char value1[] = "example_value";
    int N_value2 = 3;
    double V_value2[] = {1.1, 2.2, 3.3};

	//Inicializa
	if (init() == -1)
	{
		printf("Error in init\n");
		pthread_exit(NULL);
	}

    // Establecer un valor
    if (set_value(key, value1, N_value2, V_value2) == -1)
	{
        fprintf(stderr, "Error setting value for key %d.\n", key);
        pthread_exit(NULL);
    }

    // Obtener un valor
    char value1_get[50];
    int N_value2_get = 3;
    double V_value2_get[N_value2_get];

    if (get_value(key, value1_get, &N_value2_get, V_value2_get) == -1)
	{
        fprintf(stderr, "Error getting value for key %d.\n", key);
        pthread_exit(NULL);
    }

    // Modificar un valor
    double new_V_value2[] = {4.4, 5.5, 6.6};
    if (modify_value(key, value1, N_value2, new_V_value2) == -1)
	{
        fprintf(stderr, "Error modifying value for key %d.\n", key);
        pthread_exit(NULL);
    }

    // Verificar la existencia de una clave
    if (exist(key) <= 0)
	{
        fprintf(stderr, "Key does not exist %d.\n", key);
        pthread_exit(NULL);
    }
    // Eliminar una clave
    if (delete_key(key) == -1)
	{
        fprintf(stderr, "Error deleting key %d.\n", key);
        pthread_exit(NULL);
    }

    // Verificar la existencia de una clave
    if (exist(key) <= 0)
	{
        fprintf(stderr, "Key does not exist %d.\n", key);
        pthread_exit(NULL);
    }
    pthread_exit(NULL);
}

/*Genera un monton de hilos con rutinas normales de clientes*/
int check_program_threads()
{
    pthread_t threads[NUM_THREADS];
    int keys[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
	{
        keys[i] = i + 1;
        if (pthread_create(&threads[i], NULL, hilo_cliente, (void *)&keys[i]) != 0)
		{
            fprintf(stderr, "Error creating thread %d\n", i);
            exit(EXIT_FAILURE);
        }
        usleep(500000);
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    return 0;
}

int main(int argc, char *argv[])
{
	int arg;

    if (argc > 2)
	{
        printf("No mas de un argumento\n");
        return (-1);
    }
	else if(argc == 2)
	{
		arg = atoi(argv[1]);
		switch (arg)
		{
			case 1:
				check_program();
				break;
			case 2:
				check_program_error();
				break;
			case 3:
				check_program_threads();
				break;
			default:
				printf("El argumento debe estar entre 1 y 3.\n");
				return (-1); // Salir del programa con código de error
		}
	}
	else
		check_program_threads();
    return (0);
}

