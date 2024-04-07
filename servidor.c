#define MAX_VALUE_LENGTH 256
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "claves.h"
//a?
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "lines.h"

//añadir protocolo de comunicacion metodo a metodo mencionar si se pasa todo a texto o no

pthread_mutex_t mutex_mensaje;
pthread_cond_t cond_mensaje;
int not_finished = true;
int sd;

void treat_request(void *sc_request)
{
	pthread_mutex_lock(&mutex_mensaje);
	int sc = *((int *) sc_request);
	not_finished = false;
	pthread_cond_signal(&cond_mensaje);
	pthread_mutex_unlock(&mutex_mensaje);

	char op;
	int key;
	char v1[MAX_VALUE_LENGTH];
	int N;
	double	v2[20];
	int error;
	recvMessage(sc, (char *) &op, sizeof(char));

	switch (op)
	{
		case 0:
			error = init();
			break;
		case 1:

			recvMessage(sc, (char *) &key, sizeof(int));
			key = ntohl(key);

			readLine(sc, v1, MAX_VALUE_LENGTH);

			recvMessage(sc, (char *) &N, sizeof(int));
			N = ntohl(N);

			for (int i = 0; i < N; i++) {
				char v2_str[20];
				readLine(sc, v2_str, 20);
				v2[i] = strtod(v2_str, NULL);
			}

			error = set_value(key, v1, N, v2);
			break;
		case 2:

			recvMessage(sc, (char *) &key, sizeof(int));
			key = ntohl(key);

			error = get_value(key, v1, &N, v2);

			sendMessage(sc, v1, strlen(v1) + 1);

			int N_network = htonl(N);
			sendMessage(sc, (char *)&N_network, sizeof(int));

			for (int i = 0; i < N; i++)
			{
				char v2_str[20]; 
				snprintf(v2_str, sizeof(v2_str), "%lf", v2[i]);
				sendMessage(sc, v2_str, strlen(v2_str) + 1); // Incluye el carácter nulo en la longitud
			}

			sendMessage(sc, (char *) &error, sizeof(int));
			break;
		case 3:

			recvMessage(sc, (char *) &key, sizeof(int));
			key = ntohl(key);

			readLine(sc, v1, MAX_VALUE_LENGTH);

			recvMessage(sc, (char *) &N, sizeof(int));
			N = ntohl(N);

			for (int i = 0; i < N; i++) {
				char v2_str[20];
				readLine(sc, v2_str, 20);
				v2[i] = strtod(v2_str, NULL);
			}

			error = modify_value(key, v1, N, v2);
			break;
		case 4:
			recvMessage(sc, (char *) &key, sizeof(int));
			key = ntohl(key);

			error = delete_key(key);
			break;
		case 5:
			recvMessage(sc, (char *) &key, sizeof(int));
			key = ntohl(key);

			error = exist(key);
			break;
	}

	error = htonl(error);
	sendMessage(sc, (char *) &error, sizeof(int));

	close(sc);
	pthread_exit(0);
}

int main(int argc, char *argv[])
	{
	struct sockaddr_in server_addr,  client_addr;
	socklen_t size;
	pthread_attr_t t_attr;
	pthread_t thid;
	int sc;
	int val = 1;

	if (argc != 2)
	{
		perror("Faltan el argumento correspondiente al puerto\n"); //Comprobar que el tipo de argumento sea valido
		return (-1);
	}

	pthread_mutex_init(&mutex_mensaje, NULL);
	pthread_cond_init(&cond_mensaje, NULL);
	pthread_attr_init(&t_attr);
	pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

	if ((sd = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		printf ("SERVER: Error en el socket\n");
		return (-1);
	}
	//val = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

	bzero((char *)&server_addr, sizeof(server_addr));//ponemos a cero server_addr
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(argv[1]));

	//enlaza el socket con la dirección IP y el número del puerto del servidor
	if (bind(sd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("Bind\n");
		return (-1);
	}

	//el servidor se quede esperando una conexión entrante
	if (listen(sd, SOMAXCONN) < 0) {
		perror("Listen\n");
		return (-1);
	}

	size = sizeof(client_addr);

	while(1)
	{
		printf("esperando conexion\n");
		sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);
		if (sc == -1) {
			printf("Error en accept\n");
			return (-1);
		}
		printf("conexión aceptada de IP: %s   Puerto: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		if (pthread_create(&thid, &t_attr, (void *)treat_request, (void *)&sd)== 0)
		{
			pthread_mutex_lock(&mutex_mensaje);
			while (not_finished)
				pthread_cond_wait(&cond_mensaje, &mutex_mensaje);
			not_finished = true;
			pthread_mutex_unlock(&mutex_mensaje);
		}
	}
	//cerrar socket 
}