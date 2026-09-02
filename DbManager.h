#pragma once
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include "protecol.h"
#include <string>
using namespace std;
using namespace sql;
class DbManager
{
public:
	//禁止拷贝和构造
	DbManager(const DbManager&) = delete;
	DbManager& operator=(const DbManager&) = delete;
	static DbManager& getInstance();
	Connection* get_connection();
	void close_connection(Connection* conn, PreparedStatement* pstmt, ResultSet* res);
private:
	DbManager();
	//~DbManager()=delete;//防止外部delete
	//mysql 连接参数
	const string DB_HOST = "tcp://127.0.0.1:3306";
	const string DB_USER = "root";
	const string DB_PASS = "123456";
	const string DB_NAME = "test_db";
};

