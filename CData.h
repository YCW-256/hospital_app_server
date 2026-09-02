#pragma once
#include <map>
#include <string>
#include "protecol.h"
using namespace std;
class CData
{
public:
	static map<string, int>online_users;

	static map<int,ENVIR_RESP> now_envir;

	static pthread_mutex_t mtx_envir;

	

};

