#include "proxy.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

//Métodos proporcionados por los profesores
static int sendMessage(int socket, char * buffer, int len)
{
	int r;
	int l = len;

	do {	
		r = write(socket, buffer, l);
		l = l -r;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (-1);
	else
		return(0);
}

static int recvMessage(int socket, char *buffer, int len)
{
	int r;
	int l = len;
		

	do {	
		r = read(socket, buffer, l);
		l = l -r ;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (-1);
	else
		return(0);
}

static ssize_t readLine(int fd, void *buffer, size_t n)
{
	ssize_t numRead;
	size_t totRead;
	char *buf;
	char ch;


	if (n <= 0 || buffer == NULL) { 
		errno = EINVAL;
		return -1; 
	}

	buf = buffer;
	totRead = 0;
	
	for (;;) {
			numRead = read(fd, &ch, 1);

			if (numRead == -1) {	
					if (errno == EINTR)
						continue;
				else
			return -1;
			} else if (numRead == 0) {
					if (totRead == 0)
						return 0;
			else
						break;
			} else {
					if (ch == '\n')
						break;
					if (ch == '\0')
						break;
					if (totRead < n - 1) {
				totRead++;
				*buf++ = ch; 
			}
		} 
	}
	
	*buf = '\0';
		return totRead;
}


static int init_socket()
{
	struct sockaddr_in server_addr;
	struct hostent *hp;
	int sd;
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
		return (-1);
	}
	//Devuelve el valor del servidor si no ha habido ningun problema
	return (sd);
}

//Comprueba que los valores sean los correctos
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
	//Inicializa el socket
	int sd = init_socket();
	if (sd == -1)
	{
		printf("Error inicializando el socket\n");
		return (-1);
	}

	//Envía el mensaje por el socket
	int op = 0;
	op = htonl(op);
	if (sendMessage(sd, (char *) &op, sizeof(int32_t)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	//Recibe la respuesta del servidor
	int error;
	if (recvMessage(sd, (char *) &error, sizeof(int)) < 0) {
		printf("Error recibiendo\n");
		return (-1);
	}
	error = ntohl(error);
	// Gestión del resultado
	if (error != 0)
	{
		printf("Error en init\n");
		return (-1);
	}		
	else
	{
		printf("Lista enlazada vaciada\n");
		return (0);
	}
}

int set_value(int key, char *value1, int N_value2, double *V_value2)
{
	//Inicializa el socket
	int sd = init_socket();
	if (sd == -1)
	{
		printf("Error inicializando el socket\n");
		return (-1);
	}
	if (check_value(value1, N_value2, V_value2) == -1)
	{
		printf("No son los valores correctos\n");
		return (-1);
	}
	//Envía el mensaje por el socket
	int op = 1;
	op = htonl(op);
	if (sendMessage(sd, (char *) &op, sizeof(int32_t)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	key = htonl(key);
	sendMessage(sd, (char *) &key, sizeof(int));

	sendMessage(sd, value1, strlen(value1) + 1);

	int N_value2_network = htonl(N_value2);
	sendMessage(sd, (char *)&N_value2_network, sizeof(int));

	for (int i = 0; i < N_value2; i++)
	{
		char V_value2_str[20]; 
		snprintf(V_value2_str, sizeof(V_value2_str), "%lf", V_value2[i]);
		if (sendMessage(sd, V_value2_str, strlen(V_value2_str) + 1) < 0)
		{
			printf("Error enviando\n");
			return (-1);
		}
	}

	//Recibe la respuesta del servidor
	int error;
	if (recvMessage(sd, (char *) &error, sizeof(int)) < 0) {
		printf("Error recibiendo\n");
		return (-1);
	}
	error = ntohl(error);
	// Gestión del resultado
	if (error != 0)
	{
		printf("Error en set_value\n");
		return (-1);
	}
	else
	{
		printf("Valor establecido\n");
		return (0);
	}
}


int get_value(int key, char *value1, int *N_value2, double *V_value2)
{
	//Inicializa el socket
	int sd = init_socket();
	if (sd == -1)
	{
		printf("Error inicializando el socket\n");
		return (-1);
	}
	//Envía el mensaje por el socket
	int op = 2;
	op = htonl(op);
	if (sendMessage(sd, (char *) &op, sizeof(int32_t)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	key = htonl(key);
	sendMessage(sd, (char *) &key, sizeof(int));

	//Recibe la respuesta del servidor
	readLine(sd, value1, 256);

	int N_network;
	if (recvMessage(sd, (char *)&N_network, sizeof(int)) < 0) {
		printf("Error recibiendo\n");
		return (-1);
	}
	*N_value2 = ntohl(N_network);

	for (int i = 0; i < *N_value2; i++) {
		char v2_str[20];
		readLine(sd, v2_str, sizeof(v2_str));
		V_value2[i] = strtod(v2_str, NULL);
	}

	int error;
	if (recvMessage(sd, (char *) &error, sizeof(int)) < 0) {
		printf("Error recibiendo\n");
		return (-1);
	}
	error = ntohl(error);
	// Gestión del resultado
	if (error != 0)
	{
		printf("Error en get_value\n");
		return (-1);
	}
	else
	{
		printf("Tus valores: v1: %s, N2: %d\n", value1, *N_value2);
		printf("Contenido de v2:\n");
		for(int i = 0; i < *N_value2; i++)
		{
			printf("%f ", V_value2[i]);
		}
		printf("\n");
		return (0);
	}
}

int modify_value(int key, char *value1, int N_value2, double *V_value2)
{
	//Inicializa el socket
	int sd = init_socket();
	if (sd == -1)
	{
		printf("Error inicializando el socket\n");
		return (-1);
	}
	if (check_value(value1, N_value2, V_value2) == -1)
	{
		printf("No son los valores correctos\n");
		return (-1);
	}
	
	//Envía el mensaje por el socket
	int op = 3;
	op = htonl(op);
	if (sendMessage(sd, (char *) &op, sizeof(int32_t)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	key = htonl(key);
	if (sendMessage(sd, (char *) &key, sizeof(int)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	if (sendMessage(sd, value1, strlen(value1) + 1) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	int N_value2_network = htonl(N_value2);
	if (sendMessage(sd, (char *)&N_value2_network, sizeof(int)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	for (int i = 0; i < N_value2; i++)
	{
		char V_value2_str[20]; 
		snprintf(V_value2_str, sizeof(V_value2_str), "%lf", V_value2[i]);
		if (sendMessage(sd, V_value2_str, strlen(V_value2_str) + 1) < 0)
		{
			printf("Error enviando\n");
			return (-1);
		}
	}

	//Recibe la respuesta del servidor
	int error;
	if (recvMessage(sd, (char *) &error, sizeof(int)) < 0) {
		printf("Error recibiendo\n");
		return (-1);
	}
	error = ntohl(error);
	// Gestión del resultado
	if (error != 0)
	{
		printf("Error en modify_value\n");
		return (-1);
	}
	else
	{
		printf("Modificación exitosa\n");
		return (0);
	}
}

int delete_key(int key)
{
	//Inicializa el socket
	int sd = init_socket();
	if (sd == -1)
	{
		printf("Error inicializando el socket\n");
		return (-1);
	}
	//Envía el mensaje por el socket
	int op = 4;
	op = htonl(op);
	if (sendMessage(sd, (char *) &op, sizeof(int32_t)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	key = htonl(key);
	if (sendMessage(sd, (char *) &key, sizeof(int)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	//Recibe la respuesta del servidor
	int error;
	if (recvMessage(sd, (char *) &error, sizeof(int)) < 0) {
		printf("Error recibiendo\n");
		return (-1);
	}
	error = ntohl(error);
	// Gestión del resultado
	if (error != 0)
	{
		printf("Error en delete\n");
		return (-1);
	}
	else
	{
		printf("Eliminado con éxito\n");
		return (0);
	}
}


int exist(int key)
{
	//Inicializa el socket
	int sd = init_socket();
	if (sd == -1)
	{
		printf("Error inicializando el socket\n");
		return (-1);
	}
	//Envía el mensaje por el socket
	int op = 5;
	op = htonl(op);
	if (sendMessage(sd, (char *) &op, sizeof(int32_t)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}
	key = htonl(key);
	if (sendMessage(sd, (char *) &key, sizeof(int)) < 0) {
		printf("Error enviando\n");
		return (-1);
	}

	//Recibe la respuesta del servidor
	int error;
	if (recvMessage(sd, (char *) &error, sizeof(int)) < 0) {
		printf("Error recibiendo\n");
		return (-1);
	}
	error = ntohl(error);

	// Gestión del resultado
	if (error == 0)
	{
		printf("No existe\n");
		return (0);
	}
	else if (error == 1)
	{
		printf("Existe\n");
		return (1);
	}	
	else 
	{
		printf("Error en la función\n");
		return (-1);
	}
}