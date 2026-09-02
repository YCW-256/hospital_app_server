#include "TcpServer.h"
#include "protecol.h"
#include "CData.h"
#include "UserModel.h"
#include "TaskController.h"
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>//修改1
#include <stdio.h>
#include <errno.h>

TcpServer::TcpServer(int port)
{
	client_fs.resize(MAX_SIZE);
	int result = 0;
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	set_nonblocking(server_fd);
	if (server_fd < 0) {
		perror("socket error");
		return ;
	}
	this->port = port;
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	result = bind(server_fd, (struct sockaddr*)&address, sizeof(address));
	if (result < 0) {
		perror("bind error");
		return ;
	}
	result = listen(server_fd, 5);
	if (result < 0) {
		perror("listen error");
		return ;
	}
	cout << "服务器启动成功" << endl;

}
void TcpServer::set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1) {
		perror("fcnl error");
		return;
	}
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		perror("fcanl sefl error");
	}

}
void TcpServer::rule_client_f(CLIENT_F& client_f)
{
	char tem_buf[MAX_SIZE] = {0};
	char* buf = client_f.buf;
	int* use_len = &client_f.use_len;
	int* read_len = &client_f.read_len;
	//拷贝
	memcpy(tem_buf, buf + (*use_len), *(read_len)-*(use_len));
	memcpy(buf, tem_buf, *(read_len)-*(use_len));
	*read_len = *(read_len)-*(use_len);
	*(use_len) = 0;
	
}
int TcpServer::getFd()
{
	return this->server_fd;
}
void TcpServer::start()
{

	int epoll_fd = epoll_create(1);
	if (epoll_fd == -1) {

		perror("epoll error");
		close(server_fd);
		return ;
	}

	//2向epoll实例添加服务器fd，设置监听事件为输入
	struct epoll_event ev, events[2048];//ev 输入参数，用于配置当前事件的参数    events 用于保存已就绪数据的节点
	ev.data.fd = server_fd;  //需要epoll监听的事件描述符
	//修改3 设置为边缘触发
	ev.events = EPOLLIN | EPOLLET;    //监听可读事件

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
		perror("spoll_ctl error");
		close(server_fd);
		close(epoll_fd);

	}
	char buf[1024];
	while (1) {
		int ret = epoll_wait(epoll_fd, events, MAX_SIZE - 1, -1);
		if (ret == -1) {
			perror("epoll wait error");
			break;
		}
		for (int i = 0; i < ret; i++) {
			int fd = events[i].data.fd;
			if (fd == server_fd) {
				cout << "asd"<<endl;
				//4 等待/接收连接
				struct sockaddr_in client_address;
				int addrlen = sizeof(client_address);
				int client_fd = accept(server_fd, (sockaddr*)&client_address, (socklen_t*)&addrlen);
				if (client_fd < 0) {
					perror("accept fail");
					return ;
				}
				//修改4 设置描述符为非阻塞模式
				set_nonblocking(client_fd);

				ev.data.fd = client_fd;
				//修改5 客户端事件设置为边缘触发
				ev.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == 1) {
					perror("epoll_ctl error");
					continue;
				}
				cout << "新客户端已连接，IP" << inet_ntoa(client_address.sin_addr) << "端口是： " << ntohs(client_address.sin_port) << endl;

			}
			else {
				char* buf = client_fs[fd].buf;
				int* read_len = &client_fs[fd].read_len;
				int *use_len= &client_fs[fd].use_len;
				char data_buf[1024];
				while (1) {
					int now_size = *read_len;//方便使用 recv_len值
					int len = recv(fd, buf + now_size, MAX_SIZE - 1 - now_size, 0);
					//cout << "len是：" << len<<endl;
					cout << "asdasdasdasdasdasdasdas" << endl;
					if (len > 0) {
						*read_len = now_size + len;
						while ((*read_len)-(*use_len) >= sizeof(HEAD))  {//如果数据足够包   先取出包，暂且不偏移数据指针
							cout << "【服务端】收到头了" << endl;
							HEAD head;
							memset(&head, 0, sizeof(head));
							memcpy(&head, buf+(*use_len), sizeof(head));
							if (head.len + sizeof(head) <= (*read_len) - (*use_len)) {//如果数据足够  包+体 ，这时才偏移数据指针
								cout << "【服务端】收到包了" << endl;
								*use_len = *use_len + sizeof(head);
								memset(data_buf, 0, sizeof(data_buf));
								memcpy(data_buf, buf + (*use_len), head.len);
								*use_len = *use_len + head.len;
								TaskController::create_task(head.type,data_buf, head.len, fd);
								
								/*if (head.type == SERVICE_TYPE::LOGIN) {
									LoginTask task(data_buf, head.len, fd);
									task.execute();
								}
								else if (head.type == SERVICE_TYPE::CHAT) {
									ChatTask task(data_buf, head.len, fd);
									task.execute();
								}*/



							}
							else {
								break;
							}
	
						}
					}
					else if (len == 0) {
						printf("客户端断开%d\n", fd);
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
						close(fd);
						break;
					}
					else {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							//走到这里说明，缓冲区数据被读空 
							//此时client_fs[fd]中的缓冲区buf，最多还有0.999999个包（不关心他是什么包），
							// 但我们要将缓冲区中use过的去掉，没有用的放到开头
							rule_client_f(client_fs[fd]);
							cout<<"规范"<<endl;
							//------------------------------------------------------------------------
							break;
						}
						else {
							//其他错误
							perror("recv error");
							epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
							close(fd);
							break;
						}
					}







				}

			}




		}
	}
	//int client_fd = -1;
	//while (1) {
	//	struct sockaddr_in client_address;
	//	int addlen = sizeof(client_address);
	//	client_fd = accept(server_fd, (struct sockaddr*)&client_address, (socklen_t*)&addlen);
	//	if (client_fd < 0) {
	//		perror("accept error");
	//	}
	//	else {
	//		cout << "【服务器】有一个新客户端连接，客户端IP:" << inet_ntoa(client_address.sin_addr)
	//			<< "端口号：" << ntohs(client_address.sin_port) << endl;
	//		pthread_t tid = -1;
	//		pthread_create(&tid, NULL, recv_hander, &client_fd);
	//	}
	//}
	//close(server_fd);
	//cout << "【服务器】已退出" << endl;




}







void* TcpServer::recv_hander(void* arg)
{
	int fd = *(int*)arg;
	pthread_t tid = pthread_self();
	pthread_detach(tid);
	char buffer[1024] = { 0 };
	char send_buffer[1024] = { 0 };
	while (1) {
		memset(buffer, 0, sizeof(buffer));
		HEAD head;

		ssize_t len = recv(fd, &head, sizeof(HEAD), 0);
		cout << "接收" << endl;
		if (len == 0) {
			break;
		}
		len = recv(fd, buffer, head.len, 0);
		if (len == 0) {
			break;
		}
		/*BusinessTask *task=TaskController::create_task(head.type, buffer, len, fd)；
		if (task != nullptr) {
			task->execute();
			delete task;
		}*/



		/*if (head.type == SERVICE_TYPE::LOGIN) {
			LoginTask task(buffer, head.len, fd);
			task.execute();
			
		}
		else if (head.type == SERVICE_TYPE::CHAT) {
			ChatTask chat_task(buffer, head.len, fd);
			chat_task.execute();
		}*/




	}

}
