#pragma once

  /***********/
 /*	COLOR	*/
/***********/
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"



#define PORT 8080


  /*******************/
 /*	BASIC INCLUDE	*/
/*******************/
#include <iomanip>
#include <iostream>
#include <exception>

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>