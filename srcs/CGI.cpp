#include <CGI.hpp>

CGI::CGI () {
	std::cout << "Constructor CGI Called" << std::endl;
	_pid = -1;	
}

CGI::CGI(const std::string& cgi_path, const std::string& script_path) {
	std::cout << "Constructor CGI Called + Data" << std::endl;
	_cgi_path = cgi_path;
	_script_path = script_path;
	_pid = -1;
}

CGI::~CGI() {
	std::cout << "Destructor CGI Called" << std::endl;
}

void	CGI::setSocketVector() {
//Mettre un catch a ce throw

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _socketIn) == -1)
	{
		throw "Error: socketpair 'IN' CGI";
	}
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _socketOut) == -1)
	{
		close(_socketIn[0]), close(_socketIn[1]);
		throw "Error: socketpair 'OUT' CGI";
	}
}