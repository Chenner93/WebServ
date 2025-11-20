#pragma once

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

class CGI {
	private:
		std::string _cgi_path;      // /usr/bin/php-cgi
		std::string _script_path;   // /var/www/cgi-bin/script.php
		pid_t _pid;
		int _pipe_in[2];
		int _pipe_out[2];


	public:
		CGI();
		CGI(const std::string& cgi_path, const std::string& script_path);
		~CGI();

		// bool execute(const HTTPRequest& request);
		std::string getOutput();
		void setTimeout(int seconds);
};