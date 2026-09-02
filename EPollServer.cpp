#include "EPollServer.h"
#include "TaskController.h"
#include <time.h>
EPollServer::EPollServer(int port)
{
	
	this->client_fs.resize(1024);
	this->tcp_server = new TcpServer(port);
	server_fd = this->tcp_server->getFd();
	epoll_fd = epoll_create(1);
	if (epoll_fd == -1) {

		perror("epoll error");
		close(server_fd);
		return;
	}

	//2向epoll实例添加服务器fd，设置监听事件为输入
	
	ev.data.fd = server_fd;  //需要epoll监听的事件描述符
	//修改3 设置为边缘触发
	ev.events = EPOLLIN | EPOLLET;    //监听可读事件

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
		perror("spoll_ctl error");
		close(server_fd);
		close(epoll_fd);
	}


}

EPollServer::~EPollServer()
{
	if (this->tcp_server != nullptr) {
		delete this->tcp_server;
	}
}

void EPollServer::start()
{


	while (1) {
		int ret = epoll_wait(epoll_fd, events, MAX_SIZE - 1, -1);
		if (ret == -1) {
			perror("epoll wait error");
			break;
		}
		for (int i = 0; i < ret; i++) {
			int fd = events[i].data.fd;
			if (fd == server_fd) {
				/*
				cout << "asd" << endl;
				//4 等待/接收连接
				struct sockaddr_in client_address;
				int addrlen = sizeof(client_address);
				//int client_fd = accept(server_fd, (sockaddr*)&client_address, (socklen_t*)&addrlen);
				int client_fd;
				while (1) {
					client_fd = accept(server_fd, (sockaddr*)&client_address, (socklen_t*)&addrlen);
					if (client_fd < 0) {
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break;  // 所有连接已处理完毕
						perror("accept fail");
						break;
					}
					usleep(100);
				}
				if (client_fd < 0) {
					perror("accept fail");
					return;
				}
				//修改4 设置描述符为非阻塞模式
				set_nonblocking(client_fd);

				ev.data.fd = client_fd;
				//修改5 客户端事件设置为边缘触发
				ev.events = EPOLLIN | EPOLLET;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
					perror("epoll_ctl error");
					continue;
				}
				cout << "新客户端已连接，IP" << inet_ntoa(client_address.sin_addr) << "端口是： " << ntohs(client_address.sin_port) << endl;
				*/
				while (1) {
					struct sockaddr_in client_address;
					socklen_t addrlen = sizeof(client_address);
					int client_fd = accept(server_fd, (sockaddr*)&client_address, &addrlen);

					if (client_fd < 0) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
							// 队列已空，没有更多连接了 —— 这才是正常退出
							break;
						}
						// 真正的错误（比如 EMFILE 文件描述符耗尽），记录并继续
						perror("accept fail");
						break;
					}

					// 新连接 setup
					set_nonblocking(client_fd);

					ev.data.fd = client_fd;
					ev.events = EPOLLIN | EPOLLET;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
						perror("epoll_ctl error");
						close(client_fd);       // 加不进去就关掉，避免 fd 泄漏
						continue;               // 继续处理下一个连接
					}

					cout << "新客户端已连接，IP" << inet_ntoa(client_address.sin_addr)
						<< " 端口：" << ntohs(client_address.sin_port) << endl;
				}






			}
			else {
				char* buf = client_fs[fd].buf;
				int* read_len = &client_fs[fd].read_len;
				int* use_len = &client_fs[fd].use_len;
				char data_buf[1024];
				while (1) {
					int now_size = *read_len;//方便使用 recv_len值
					int len = recv(fd, buf + now_size, MAX_SIZE - 1 - now_size, 0);


					cout << "len是：" << len << endl;
					if (len > 0) {
						*read_len = now_size + len;
						cout << "当前read_len: " << (*read_len) << endl;
						if (*read_len >= MAX_SIZE - 1) {
							rule_client_f(client_fs[fd]);
							cout << "【！！！缓冲区满，进行规范！！！】"<<endl;
						}

						while ((*read_len) - (*use_len) >= sizeof(HEAD)) {//如果数据足够包   先取出包，暂且不偏移数据指针
							cout << "【服务端】收到头了" << endl;
							HEAD head;
							memset(&head, 0, sizeof(head));
							memcpy(&head, buf + (*use_len), sizeof(head));
							cout << "头的长度" << head.len << endl;
							if (head.len + sizeof(head) <= (*read_len) - (*use_len)) {//如果数据足够  包+体 ，这时才偏移数据指针
								cout << "【服务端】收到包了" << endl;
								*use_len = *use_len + sizeof(head);
								memset(data_buf, 0, sizeof(data_buf));
								memcpy(data_buf, buf + (*use_len), head.len);
								*use_len = *use_len + head.len;
								TaskController::create_task(head.type,data_buf,head.len,fd);
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
							/*rule_client_f(client_fs[fd]);
							cout << "规范" << endl;*/
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



}

void EPollServer::set_nonblocking(int fd)
{
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

void EPollServer::rule_client_f(CLIENT_F& client_f)
{
	char tem_buf[MAX_SIZE] = { 0 };
	char* buf = client_f.buf;
	int* use_len = &client_f.use_len;
	int* read_len = &client_f.read_len;
	//拷贝
	memcpy(tem_buf, buf + (*use_len), *(read_len)-*(use_len));
	memcpy(buf, tem_buf, *(read_len)-*(use_len));
	*read_len = *(read_len)-*(use_len);
	*(use_len) = 0;
}
