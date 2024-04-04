#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "claves.h"
#include "structs_handler.h"
//a?
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

//añadir protocolo de comunicacion metodo a metodo mencionar si se pasa todo a texto o no

pthread_mutex_t mutex_mensaje;
pthread_cond_t cond_mensaje;
int not_finished = true;
int sd, sc;

void treat_request(void *mess)
{
	struct request message;	/* mensaje local */
	struct response res;

	pthread_mutex_lock(&mutex_mensaje);
	message = (*(struct request *) mess);
	not_finished = false;
	pthread_cond_signal(&cond_mensaje);
	pthread_mutex_unlock(&mutex_mensaje);
	switch (message.op)
	{
		case 0:
			res.error = init();
			break;
		case 1:
			res.error = set_value(message.key, message.v1, message.N, message.v2);
			break;
		case 2:
			res.error = get_value(message.key, message.v1, &message.N, message.v2);
			res.key = message.key;
			res.N = message.N;
			strcpy(res.v1, message.v1);
			for (int i = 0; i < res.N; i++)
    			res.v2[i] = message.v2[i];
			break;
		case 3:
			res.error = modify_value(message.key, message.v1, message.N, message.v2);
			break;
		case 4:
			res.error = delete_key(message.key);
			break;
		case 5:
			res.error = exist(message.key);
			break;
	}
	//Casteos a network falta
	res.error = htonl(res.error);

	//Puede pasar a otra funcion o algo asi
	if (write(sc, (char *) &res.op, sizeof(int)) == -1)
	{ 
		perror("Error al escribir en el socket sc (op)\n");
		close (sc);
	}

	if (write(sc, res.v1, MAX_VALUE_LENGTH) == -1)
	{ 
		perror("Error al escribir en el socket sc (v1)\n");
		close (sc);
	}

	if (write(sc, (char *) &res.key, sizeof(int)) == -1)
	{ 
		perror("Error al escribir en el socket sc (key)\n");
		close (sc);
	}

	if (write(sc, (char *) &res.N, sizeof(int)) == -1)
	{ 
		perror("Error al escribir en el socket sc (N)\n");
		close (sc);
	}

	if (write(sc, (char *) &res.v2, sizeof(double) * res.N) == -1)
	{ 
		perror("Error al escribir en el socket sc (v2)\n");
		close (sc);
	}

	if (write(sc, (char *) &res.error, sizeof(int)) == -1)
	{ 
		perror("Error al escribir en el socket sc (error)\n");
		close (sc);
	}
	close(sc);
	pthread_exit(0);

}

/*void print_message(struct request message) {
    printf("Operation: %d\n", message.op);
    printf("Node: key=%d, v1=%s, N=%d\n", message.key, message.v1, message.N);
	printf("Contenido de v2:\n");
    for (int i = 0; i < message.N; i++) {
        printf("%f ", message.v2[i]);
    }
    printf("\n");
}*/

int main(int argc, char *argv[])
	{
	struct request message;
	struct sockaddr_in server_addr,  client_addr;
	socklen_t size;
	pthread_attr_t t_attr;
	pthread_t thid;
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

		//Recibir mensaje que se puede pasar a otra funcion o crear una funcion que lo haga
		// Leer el atributo 'op'
		if (read(sc, (char *) &message.op, sizeof(int)) == -1) { 
			perror("Error al leer el socket sc (op)\n");
			close(sc);
		}

		// Leer el atributo 'v1'
		// Creo que si conoce max_value_length
		if (read(sc, message.v1, MAX_VALUE_LENGTH) == -1) { 
			perror("Error al leer el socket sc (v1)\n");
			close(sc);
		}

		// Leer el atributo 'key'
		if (read(sc, (char *) &message.key, sizeof(int)) == -1) { 
			perror("Error al leer el socket sc (key)\n");
			close(sc);
		}

		// Leer el atributo 'N'
		if (read(sc, (char *) &message.N, sizeof(int)) == -1) { 
			perror("Error al leer el socket sc (N)\n");
			close(sc);
		}

		// Leer el atributo 'v2'
		if (read(sc, (char *) &message.v2, sizeof(double) * message.N) == -1) { 
			perror("Error al leer el socket sc (v2)\n");
			close(sc);
		}

		if (pthread_create(&thid, &t_attr, (void *)treat_request, (void *)&message)== 0)
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