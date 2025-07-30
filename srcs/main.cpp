#include <webserv.hpp>

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

	int		epoll_fd = epoll_create1(0);

	if (epoll_fd == -1) {
		std::cout << RED"Error create"RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	
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

	int opt = 1;	//A ne pas garder, aide juste a refermer aussitot le port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
	    perror("setsockopt");
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

	struct epoll_event	event;
	memset(&event, 0, sizeof(event));
	event.data.fd = server_fd;
	event.events = EPOLLIN;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}

	#define MAX_EVENTS 10
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");

		struct epoll_event events[MAX_EVENTS];
		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		std::cout << RED"Epoll wait triggered"RESET << std::endl;
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