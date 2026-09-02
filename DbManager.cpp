#include "DbManager.h"
DbManager::DbManager()
{
	cout << "DbManager被创建" << endl;
}
DbManager& DbManager::getInstance()
{
	// TODO: 在此处插入 return 语句
	static DbManager instance;
	return instance;
}

Connection* DbManager::get_connection()
{
	Connection* conn = nullptr;
	bool result;
	try  {
		Driver* driver = get_driver_instance();
		conn = driver->connect(DB_HOST, DB_USER, DB_PASS);
		if (conn) {
			conn->setSchema("test_db");
			cout << "连接数据库成功" << endl;
			return conn;
		}
		else {
			cerr << "driver->connect 返回了空指针 " << endl;
			return nullptr;
		}
	}
	catch (SQLException& e) {
		cerr << "数据库连接失败: " << e.what() << endl;
		return nullptr;
	}
}

void DbManager::close_connection(Connection* conn, PreparedStatement* pstmt, ResultSet* res)
{
	if (res != nullptr)
		delete res;
	if (pstmt != nullptr)
		delete pstmt;
	if (conn != nullptr)
		delete conn;
}


