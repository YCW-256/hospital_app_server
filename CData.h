#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include "protecol.h"
#include <vector>
typedef struct {
	int now_size = 0;

	// ↓↓↓ 新增这几个字段
	int total_frags = 0;
	int width = 0;
	int height = 0;
	int channels = 3;
	std::map<int, std::vector<uint8_t>> frags;

	char data[1024 * 1024 * 10];   // 你原来就有的，保留（也可以删掉，改用 frags 拼接）
} MY_FILE;

struct FileReceiver {
	int now_size = 0;                                 // 已收分片数
	int total_frags = 0;
	int width = 0;
	int height = 0;
	int channels = 3;
	std::map<int, std::vector<uint8_t>> frags;          // index -> 该片数据
};
using namespace std;
class CData
{
public:
	static map<string, int>online_users;

	static map<int,ENVIR_RESP> now_envir;

	static pthread_mutex_t mtx_envir;

	static unordered_map<string, MY_FILE> my_files;

	static pthread_mutex_t mtx_my_files;   // ★ 新增：保护 my_files

};

