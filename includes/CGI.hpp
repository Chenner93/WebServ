#pragma once

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#include <sys/types.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <Request/Request.hpp>

enum CGIState {
	CGI_WRITING_BODY,    // only POST: J'envoie le body au CGI -> EPOLLOUT sur stdin
	CGI_READING_OUTPUT,  // Je recupere la reponse du CGI -> EPOLLIN sur stdout
	CGI_DONE             // Peut renvoyer la reponse
};

class CGI {
	private:

		CGIState state;
		std::string _cgi_path;      // /usr/bin/php-cgi
		std::string _script_path;   // /var/www/cgi-bin/script.php
		pid_t 		_pid;
		int 		_socket[2];


	public:
		CGI();
		CGI(const std::string& cgi_path, const std::string& script_path);
		~CGI();

		// bool execute(const HTTPRequest& request);
		// std::string getOutput();
		// void setTimeout(int seconds);
		bool	checkSocket(int fd);

		void	CGIEvent(int &epoll_fd, std::vector<Client> &clients, struct epoll_event &event);
		void	execCGI(Request *httpRequest);

		//SETTER
		void	setSocketVector();
		void	setState(std::string method);
		void	setFork();
		void	setDup2();
		void	setEpoll(int epoll_fd, std::vector<Client> &clients, struct epoll_event &event);

		//GETTER
		std::string	getScriptPath();
		std::string	getCgiPath();
		int			getPid() const;
		int			getSocketParent() const;
		int			getSocketChild() const;
};


// socketpair(stdin_pipe)
// socketpair(stdout_pipe)
// fork()

// dans le fils == 0 :
//     dup2(stdin_pipe[1], STDIN_FILENO)
//     dup2(stdout_pipe[1], STDOUT_FILENO)
//     close tout
//     execve(script)

// dans le parent > 0 :
//     close(stdin_pipe[1])        // on écrit sur stdin_pipe[0]
//     close(stdout_pipe[1])       // on lit sur stdout_pipe[0]