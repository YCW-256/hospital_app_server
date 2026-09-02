#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>//修改1
#include <stdio.h>
#include <errno.h>


#include "protecol.h"
#include "DbManager.h"

#include "EPollServer.h"
#include "TcpServer.h"

#include "DetectorModel.h"


#include <ctime>
int main() {

	/*DetectorModel inser;
	inser.db_save(1001, 60, 40);*/

	std::srand((unsigned)std::time(nullptr));
	cout << "ddddddddddddddddddddddddd" << endl;
	EPollServer epoll_server(10086);
	epoll_server.start();
	cout << "success" << endl;
	return 0;
	
	
	//TcpServer tcp_server(10086);
	//tcp_server.start();
}