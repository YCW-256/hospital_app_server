#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <map>
#include <vector>
#define MAX_SIZE 4048
using namespace std;


typedef struct {

	char buf[MAX_SIZE] = {};
	int read_len=0;
	int use_len = 0;

}CLIENT_F;
class TcpServer
{
public:
	TcpServer(int port);

	void start();

	static void* recv_hander(void* arg);

	static void set_nonblocking(int fd);

	static void rule_client_f(CLIENT_F & client_f);
	
	int getFd();
private:
	map<string, int>online_users;
	vector<CLIENT_F> client_fs;
	int port;
	string ip;
	int server_fd;
};

