#include "DetectorModel.h"

bool DetectorModel::db_save(int id,int wet, int tempture)
{

	Connection* conn = nullptr;
	PreparedStatement* pstmt = nullptr;
	//ResultSet* res = nullptr;
	int result;
	try {
		conn = DbManager::getInstance().get_connection();
		conn->setSchema("test_db");
		cout << "连接数据库成功" << endl;
		string sql="INSERT INTO device_data(device_id,wet, tempture) VALUES(?,?, ?)";

		pstmt = conn->prepareStatement(sql);
		pstmt->setInt(1, id);
		pstmt->setInt(2, wet);
		pstmt->setInt(3, tempture);
		result = pstmt->executeUpdate();
	}
	catch (SQLException& e) {
		cerr << "数据库连接失败: " << e.getErrorCode() << endl;
	}
	DbManager::getInstance().close_connection(conn, pstmt, nullptr);
	return result>0?result:0;
	
}


