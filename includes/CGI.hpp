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
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

class CGI {
	private:
		std::string _cgi_path;      // /usr/bin/php-cgi
		std::string _script_path;   // /var/www/cgi-bin/script.php
		pid_t 		_pid;
		int 		_socketIn[2];
		int 		_socketOut[2];
		bool		child;


	public:
		CGI();
		CGI(const std::string& cgi_path, const std::string& script_path);
		~CGI();

		// bool execute(const HTTPRequest& request);
		// std::string getOutput();
		// void setTimeout(int seconds);
		void	setSocketVector();
		void	execCGI();

		//UTILS
		std::string	getScriptPath();
		std::string	getCgiPath();
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