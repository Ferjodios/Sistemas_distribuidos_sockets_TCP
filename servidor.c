#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "claves.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "lines.h"

#define MAX_VALUE_LENGTH 256

int sd; //descriptor del socket global

// Función para manejar las solicitudes de los clientes
void treat_request(void *sc_request)
{
	int sc = *((int *) sc_request); // Descriptor de socket del cliente

	int op;
	int key;
	char v1[MAX_VALUE_LENGTH];
	int N;
	double	v2[20];
	int error;

	// Recepción de la operación solicitada desde el cliente
	if (recvMessage(sc, (char *) &op, sizeof(int32_t)) < 0)
	{
		printf("Error recibiendo del socket\n");
	}
	op = ntohl(op);
	// Switch para manejar las diferentes operaciones
	switch (op)
	{
		case 0: // Reseteo de la lista
			error = init();
			break;
		case 1: // Establecer un valor
			if (recvMessage(sc, (char *) &key, sizeof(int)) < 0)
			{
				printf("Error recibiendo del socket\n");
			}
			key = ntohl(key);
			
			readLine(sc, v1, MAX_VALUE_LENGTH);

			if (recvMessage(sc, (char *) &N, sizeof(int)) < 0)
			{
				printf("Error recibiendo del socket\n");
			}
			N = ntohl(N);

			for (int i = 0; i < N; i++) {
				char v2_str[20];
				readLine(sc, v2_str, 20);
				v2[i] = strtod(v2_str, NULL);
			}

			error = set_value(key, v1, N, v2);
			break;
		case 2: // Obtener un valor

			if (recvMessage(sc, (char *) &key, sizeof(int)) < 0)
			{
				printf("Error recibiendo del socket\n");
			}
			key = ntohl(key);

			error = get_value(key, v1, &N, v2);

			if (sendMessage(sc, v1, strlen(v1) + 1) < 0)
			{
				printf("Error recibiendo del socket\n");
			}

			int N_network = htonl(N);
			if (sendMessage(sc, (char *)&N_network, sizeof(int)) < 0)
			{
				printf("Error enviando al socket\n");
			}

			for (int i = 0; i < N; i++)
			{
				char v2_str[20]; 
				snprintf(v2_str, sizeof(v2_str), "%lf", v2[i]);
				if (sendMessage(sc, v2_str, strlen(v2_str) + 1) < 0)
				{
					printf("Error enviando al socket\n");
				}
			}

			if (sendMessage(sc, (char *) &error, sizeof(int)) < 0)
			{
				printf("Error enviando al socket\n");
			}
			break;
		case 3: // Modificar un valor
			if (recvMessage(sc, (char *) &key, sizeof(int)) < 0)
			{
				printf("Error recibiendo del socket\n");
			}
			key = ntohl(key);

			readLine(sc, v1, MAX_VALUE_LENGTH);

			if (recvMessage(sc, (char *) &N, sizeof(int)) < 0)
			{
				printf("Error recibiendo del socket\n");
			}
			N = ntohl(N);

			for (int i = 0; i < N; i++) {
				char v2_str[20];
				readLine(sc, v2_str, 20);
				v2[i] = strtod(v2_str, NULL);
			}

			error = modify_value(key, v1, N, v2);
			break;
		case 4: // Eliminar una clave
			if (recvMessage(sc, (char *) &key, sizeof(int)) < 0)
			{
				printf("Error recibiendo del socket\n");
			}
			key = ntohl(key);

			error = delete_key(key);
			break;
		case 5: // Verificar si existe una clave
			if (recvMessage(sc, (char *) &key, sizeof(int)) < 0)
			{
				printf("Error recibiendo del socket\n");
			}
			key = ntohl(key);

			error = exist(key);
			break;
	}
	// Enviar el código de error de vuelta al cliente en formato red
	error = htonl(error);
	if (sendMessage(sc, (char *) &error, sizeof(int)) < 0)
    {
        printf("Error enviando al socket\n");
    }
	// Cerrar el socket del cliente
	close(sc);
	// Salir del hilo
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
		perror("Faltan el argumento correspondiente al puerto\n");
		return (-1);
	}

	pthread_attr_init(&t_attr);
	pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

	if ((sd = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		printf ("SERVER: Error en el socket\n");
		return (-1);
	}
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

	bzero((char *)&server_addr, sizeof(server_addr));//ponemos a cero server_addr
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(argv[1]));

	//enlaza el socket con la dirección IP y el número del puerto del servidor
	if (bind(sd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("Error en Bind\n");
		return (-1);
	}

	//el servidor se quede esperando una conexión entrante
	if (listen(sd, SOMAXCONN) < 0) {
		printf("Error en Listen\n");
		return (-1);
	}

	size = sizeof(client_addr);

	// Bucle principal para aceptar conexiones y manejarlas en hilos separados
	while(1)
	{
		printf("esperando conexion\n");
		// Aceptar la conexión entrante
		sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);
		if (sc == -1) {
			printf("Error en accept\n");
			return (-1);
		}
		printf("conexión aceptada de IP: %s   Puerto: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		// Crear un hilo para manejar la solicitud del cliente
		pthread_create(&thid, &t_attr, (void *)treat_request, (void *)&sc);
	}
}