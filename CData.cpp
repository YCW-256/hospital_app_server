#include "CData.h"
map<string, int>CData::online_users = {};

map<int, ENVIR_RESP>CData::now_envir = {};

pthread_mutex_t CData::mtx_envir = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t CData::mtx_my_files = PTHREAD_MUTEX_INITIALIZER;   // ★ 新增：保护 my_files

unordered_map<string, MY_FILE> CData::my_files = {};
int CData::total = 0;