#include <webserv.hpp>

int main(int ac, char **av) {

	if (ac != 2) {
		std::cout << RED"Not cool"RESET << std::endl;
		return 1;
	}

	(void)av;
	
	return 0;
}
