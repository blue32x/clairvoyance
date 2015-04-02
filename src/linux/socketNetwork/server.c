#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

int main(void)
{
	int server_socket;
	int client_socket;
	int client_addr_size;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	char buff_rcv[BUF_SIZE];
	char buff_snd[BUF_SIZE];
	   
	// Init
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(server_socket == -1)
	{
		printf("server socket create failed.\n");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(6061); 			// server port
	//server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// server ip
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");	// server ip

	// Bind
	if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 )
	{
		printf("between server and client socket bind failed.\n");
		exit(1);
	}

	while(1)
	{
		// Listen for client socket
		if(listen(server_socket, 5) == -1)
		{
			printf("server listen failed.\n");
			exit(1);
		}

		// Accept client socket
		int client_addr_size = sizeof(client_addr);
		client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

		if(client_socket == -1)
		{
			printf("client socket accept failed.\n");
			exit(1);
		}

		// Read
		read(client_socket, buff_rcv, BUF_SIZE);
		printf("receive: %s\n", buff_rcv);
	      
		// Write
		sprintf(buff_snd, "%d : %s", strlen(buff_rcv), buff_rcv);
		write(client_socket, buff_snd, strlen(buff_snd) + 1);

		close(client_socket);
		return 0;
	}
}
