#pragma once

/*-----------  INCLUDES -----------*/

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#include "response.hpp"
#include "request.hpp"
#include "colors.hpp"

/*-----------  DEFINE -----------*/

#define	BUFFER_SIZE 8192
using	namespace std;

string int_to_string(int n);