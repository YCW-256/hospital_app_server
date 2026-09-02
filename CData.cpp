#include "CData.h"
map<string, int>CData::online_users = {};

map<int, ENVIR_RESP>CData::now_envir = {};

pthread_mutex_t CData::mtx_envir = PTHREAD_MUTEX_INITIALIZER;