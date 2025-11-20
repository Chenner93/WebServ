#include <CGI.hpp>

CGI::CGI () {
	std::cout << "Constructor CGI Called" << std::endl;
	_cgi_path = 0;
	_script_path = 0;	
}

CGI::CGI(const std::string& cgi_path, const std::string& script_path) {
	std::cout << "Constructor CGI Called + Data" << std::endl;
	_cgi_path = cgi_path;
	_script_path = script_path;
}

CGI::~CGI() {
	std::cout << "Destructor CGI Called" << std::endl;
}

