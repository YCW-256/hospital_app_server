#pragma once
#include "TcpServer.h"
#include <sys/epoll.h>
#include <fcntl.h>//修改1
#include <stdio.h>
#include <errno.h>
class EPollServer
{
public:
	EPollServer(int port);

	~EPollServer();

	void start();
	
	static void set_nonblocking(int fd);

	static void rule_client_f(CLIENT_F& client_f);

private:
	TcpServer* tcp_server;
	
	struct epoll_event ev, events[2048];//ev 输入参数，用于配置当前事件的参数    events 用于保存已就绪数据的节点
	
	int server_fd;

	int epoll_fd;

	vector<CLIENT_F> client_fs;
};

