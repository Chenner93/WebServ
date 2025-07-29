// #include <webserv.hpp>
// #include <sys/socket.h>
// #include <errno.h>
// #include <stdio.h>
// #include <netinet/in.h> // define struct sockaddr_in
// #include <string.h>
// #include <stdlib.h>

// int main(int ac, char **av) {

// 	// (void)av;
// 	// if (ac != 2) {
// 	// 	std::cout << RED"Error: only require config file"RESET << std::endl;
// 	// 	return 1;
// 	// }
	
// 	int	server_fd;

// 	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
// 	{
// 		perror("cannot create socket");
// 		return 0;
// 	}

// 	struct sockaddr_in address;

// 	const int	PORT = 8080; //Where the clients can reach at

// 	/* htonl converts a long integer (e.g. address) to a network representation */ 
// 	/* htons converts a short integer (e.g. port) to a network representation */ 
// 	memset((char *)&address, 0, sizeof(address));
// 	address.sin_family = AF_INET;
// 	address.sin_addr.s_addr = htonl(INADDR_ANY);
// 	address.sin_port = htons(PORT);
// 	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
// 	{
// 	    perror("bind failed");
// 	    return 0;
// 	}
// 	// -------------------------------------------------------------------------------
// 	if (listen(server_fd, 3) < 0) 
// 	{
// 	    perror("In listen"); 
// 	    exit(1); 
// 	}
// 	int new_socket;
// 	socklen_t addrlen = sizeof(address);
// 	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
// 	{
// 	    perror("In accept");            
// 	    exit(1);        
// 	}
// 	return 0;
// }

#define PORT 8080

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>

int	server_fd;

void handle_sigint(int sig) {
    printf("\n[INFO] Fermeture du serveur...\n");
    close(server_fd);  // Libère le port
    printf("[INFO] Port libéré, serveur arrêté.\n");
    exit(0);
}

int main(int argc, char const *argv[])
{

	signal(SIGINT, handle_sigint);

    int new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // char *hello = "Hello from server";
	char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
    // std::string	files = ;
	// int			fd = open()

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        char buffer[30000] = {0};
        valread = read(new_socket , buffer, 30000);
        printf("%s\n",buffer );
		write(new_socket , hello , strlen(hello));
        printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
    return 0;
}

// #include <stdio.h>
// #include <sys/socket.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <netinet/in.h>
// #include <string.h>

// #define PORT 8080
// int main(int argc, char const *argv[])
// {
//     int server_fd, new_socket; long valread;
//     struct sockaddr_in address;
//     int addrlen = sizeof(address);
    
//     // Only this line has been changed. Everything is same.
//     char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
    
//     // Creating socket file descriptor
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
//     {
//         perror("In socket");
//         exit(EXIT_FAILURE);
//     }
    

//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons( PORT );
    
//     memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
//     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
//     {
//         perror("In bind");
//         exit(EXIT_FAILURE);
//     }
//     if (listen(server_fd, 10) < 0)
//     {
//         perror("In listen");
//         exit(EXIT_FAILURE);
//     }
//     while(1)
//     {
//         printf("\n+++++++ Waiting for new connection ++++++++\n\n");
//         if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
//         {
//             perror("In accept");
//             exit(EXIT_FAILURE);
//         }
        
//         char buffer[30000] = {0};
//         valread = read( new_socket , buffer, 30000);
//         printf("%s\n",buffer );
//         write(new_socket , hello , strlen(hello));
//         printf("------------------Hello message sent-------------------");
//         close(new_socket);
//     }
//     return 0;
// }