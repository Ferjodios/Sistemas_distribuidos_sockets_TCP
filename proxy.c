
#include "claves.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
int sd;


static int init_socket()
{
	struct sockaddr_in server_addr;
	struct hostent *hp;

	// Accedemos a las variables de entorno que deben ser definidas antes
	char* port = getenv("PORT_TUPLAS");
	if (port == NULL){//Si no existe salta error
		printf("No se ha declarado la variable de entorno PORT_TUPLAS\n");
		return (-1);
	}
	char* server_ip = getenv("IP_TUPLAS");
	if (server_ip == NULL){//Si no existe salta error
		printf("No se ha declarado la variable de entorno IP_TUPLAS\n");
		return (-1);
	}
	// Creamos el socket
	//falta comprobar si en caso de error es 1 como dice en el ejemplo
	if ((sd = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		printf ("SERVER: Error en el socket\n");
		return (-1);
	}

	bzero((char *)&server_addr, sizeof(server_addr));
	hp = gethostbyname(server_ip);
	if (hp == NULL) {
		printf("Error en gethostbyname\n");
		return (-1);
	}
	memcpy (&(server_addr.sin_addr), hp->h_addr, hp->h_length);
	server_addr.sin_family  = AF_INET;
	server_addr.sin_port    = htons(atoi(port));
	
	if (connect(sd, (struct sockaddr *) &server_addr,  sizeof(server_addr)) < 0) {
		printf("Error en connect\n");
		//close(sd);
		return (-1);
	}
	return (0);
}


//falta preguntar a david si pasar las variables globales esta hecho a posta para beneficiar la modularidad
int communication(struct request message, struct response *res)
{
	// printf("Tamano de struct: %ld, Tamano de mensaje: %ld\n", sizeof(struct request), sizeof(message));
	// printf("Tamaño por separado: %ld \n", sizeof(message.key) + sizeof(message.N) + sizeof(message.op) +
	// sizeof(message.queue) + sizeof(message.v1) + sizeof(message.v2));

	// Escribo los datos a enviar de la estructura en el socket
	// Escribir el atributo 'op'
	if (write(sd, (char *) &message.op, sizeof(int)) == -1)
	{
		perror("Error al escribir en el socket sd (op)\n");
		close (sd);
		return (-1);
	}

	// Escribir el atributo 'v1'
	if (write(sd, message.v1, MAX_VALUE_LENGTH) ==-1){ 
		perror("Error al escribir en el socket sd (v1)\n");
		close (sd);
		return (-1);
	}

	// Escribir el atributo 'key'
	if (write(sd, (char *) &message.key, sizeof(int)) == -1) { 
		perror("Error al escribir en el socket sd (key)\n");
		close(sd);
		return (-1);
	}

	// Escribir el atributo 'N'
	if (write(sd, (char *) &message.N, sizeof(int)) == -1) { 
		perror("Error al escribir en el socket sd (N)\n");
		close(sd);
		return (-1);
	}

	// Escribir el atributo 'v2'
	if (write(sd, (char *) &message.v2, sizeof(double) * message.N) == -1) { 
		perror("Error al escribir en el socket sd (v2)\n");
		close(sd);
		return (-1);
	}

	// Leo la respuesta del socket

	if (read(sd, (char *) &res->op, sizeof(int)) == -1)
	{ 
		perror("Error al leer del socket sd (op)\n");
		close (sd);
	}

	if (read(sd, res->v1, MAX_VALUE_LENGTH) == -1)
	{ 
		perror("Error al leer del socket sd (v1)\n");
		close (sd);
	}

	if (read(sd, (char *) &res->key, sizeof(int)) == -1)
	{ 
		perror("Error al leer del socket sd (key)\n");
		close (sd);
	}

	if (read(sd, (char *) &res->N, sizeof(int)) == -1)
	{ 
		perror("Error al leer del socket sd (N)\n");
		close (sd);
	}

	if (read(sd, (char *) &res->v2, sizeof(double) * res->N) == -1)
	{ 
		perror("Error al leer del socket sd (v2)\n");
		close (sd);
	}

	if (read(sd, (char *) &res->error, sizeof(int)) == -1)
	{ 
		perror("Error al leer del socket sd (error)\n");
		close (sd);
	}
	close(sd);

	return (0);
}

//falta comprobar que n_value coincida con v_value
static int check_value(char *value1, int N_value2, double *V_value2)
{
	if (strlen(value1) >= 256)
		return (-1);
	if (N_value2 < 1 || N_value2 > 32)
		return (-1);
	return (0);
}

int init()
{
	struct request message;	
	struct response res;
	if (init_socket() == -1)
	{
		printf("Error initialiting socket\n");
		return (-1);
	}
	//falta casteos a network
	message.op = htonl(0);
	if (communication(message, &res) == -1)
	{
		printf("Error in coms with server\n");
		return (-1);
	}
	if (res.error != 0)
	{
		printf("Error in init\n");
		return (-1);
	}		
	else
	{
		printf("Linked list emptied\n");
		return (0);
	}
}

int set_value(int key, char *value1, int N_value2, double *V_value2)
{
	struct request message;	
	struct response res;
	if (init_socket() == -1)
	{
		printf("Error initialiting socket\n");
		return (-1);
	}
	if (check_value(value1, N_value2, V_value2) == -1)
	{
		printf("Not the right values\n");
		return (-1);
	}
	message.op = 1;
	message.key = key;
	message.N = N_value2;
	strcpy(message.v1, value1);
	for (int i = 0; i < N_value2; i++)
    	message.v2[i] = V_value2[i];
	if (communication(message, &res) == -1)
	{
		printf("Error in coms with server\n");
		return (-1);
	}
	if (res.error != 0)
	{
		printf("Error in set_value\n");
		return (-1);
	}
	else
	{
		printf("Value set\n");
		return (0);
	}
}

int get_value(int key, char *value1, int *N_value2, double *V_value2)
{
	struct request message;	
	struct response res;
	if (init_socket() == -1)
	{
		printf("Error initialiting socket\n");
		return (-1);
	}
	message.op = 2;
	message.key = key;
	message.N = *N_value2;
	strcpy(message.v1, value1);
	memcpy(message.v2, V_value2, *N_value2 * sizeof(double));
	if (communication(message, &res) == -1)
	{
		printf("Error in coms with server\n");
		return (-1);
	}
	if (res.error != 0)
	{
		printf("Error in get_value\n");
		return (-1);
	}
	else
	{
		printf("Your values: v1: %s, N2: %d\n", res.v1, res.N);
		strcpy(value1, res.v1);
		*N_value2 = res.N;
		printf("Contenido de v2:\n");
    	for(int i = 0; i < res.N; i++)
		{
        	printf("%f ", res.v2[i]);
			V_value2[i] = res.v2[i];
		}
		printf("\n");
		return (0);
	}
}

int modify_value(int key, char *value1, int N_value2, double *V_value2)
{
	struct request message;	
	struct response res;
	if (init_socket() == -1)
	{
		printf("Error initialiting socket\n");
		return (-1);
	}
	if (check_value(value1, N_value2, V_value2) == -1)
	{
		printf("Not the right values\n");
		return (-1);
	}
	message.op = 3;
	message.key = key;
	message.N = N_value2;
	strcpy(message.v1, value1);
	memcpy(message.v2, V_value2, N_value2 * sizeof(double));
	if (communication(message, &res) == -1)
	{
		printf("Error in coms with server\n");
		return (-1);
	}
	if (res.error != 0)
	{
		printf("Error in modify_value\n");
		return (-1);
	}
	else
	{
		printf("Modify succesful\n");
		return (0);
	}
}

int delete_key(int key)
{
	struct request message;	
	struct response res;
	if (init_socket() == -1)
	{
		printf("Error initialiting socket\n");
		return (-1);
	}
	message.op = 4;
	message.key = key;
	if (communication(message, &res) == -1)
	{
		printf("Error in coms with server\n");
		return (-1);
	}
	if (res.error != 0)
	{
		printf("Error in delete\n");
		return (-1);
	}
	else
	{
		printf("deleted sucessful\n");
		return (0);
	}
}

int exist(int key)
{
	struct request message;	
	struct response res;
	if (init_socket() == -1)
	{
		printf("Error initialiting socket\n");
		return (-1);
	}
	message.op = 5;
	message.key = key;
	if (communication(message, &res) == -1)
	{
		printf("Error in coms with server\n");
		return (-1);
	}
	if (res.error == 0)
	{
		printf("Does not exist\n");
		return (0);
	}
	else if (res.error == 1)
	{
		printf("Exists\n");
		return (1);
	}	
	else 
	{
		printf("Error in function\n");
		return (-1);
	}
}