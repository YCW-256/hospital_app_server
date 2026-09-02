#include "UserModel.h"
#include <string.h>

bool UserModel::db_login(char* nike, char* password)
{
	Connection* conn=nullptr;
	PreparedStatement *pstmt=nullptr;
	ResultSet* res = nullptr;
	bool result;
	try {
		conn = DbManager::getInstance().get_connection();
		conn->setSchema("test_db");
		cout << "连接数据库成功" << endl;
		string sql = "select * from USER where name = ? and password = ?";
		pstmt = conn->prepareStatement(sql);
		pstmt->setString(1, nike);
		pstmt->setString(2, password);
		res = pstmt->executeQuery();
		result = res->next();
	}
	catch (SQLException& e) {
		cerr << "数据库连接失败: " << e.getErrorCode() << endl;
	}
	DbManager::getInstance().close_connection(conn,pstmt,res);
	return result;
}

bool UserModel::doctor_login(const string& account, const string& pwd,int & doctor_id)
{

	Connection* conn = nullptr;
	PreparedStatement* pstmt = nullptr;
	ResultSet* res = nullptr;
	bool result=false;
	try {
		conn = DbManager::getInstance().get_connection();
		conn->setSchema("hospital_db");
		cout << "连接数据库成功" << endl;
		string sql = "select * from doctors where  account= ? and pwd = ?";
		pstmt = conn->prepareStatement(sql);
		pstmt->setString(1, account);
		pstmt->setString(2, pwd);
		res = pstmt->executeQuery();
		if (res->next())
		{
			result = true;
			doctor_id = res->getInt("doctor_id");
			cout << "查询到医生id = " << doctor_id << endl;
		}
		
	}
	catch (SQLException& e) {
		std::cerr << "数据库异常 code:" << e.getErrorCode()
			<< " msg:" << e.what() << std::endl;
	}
	DbManager::getInstance().close_connection(conn, pstmt, res);
	return result;

	
}

bool UserModel::doctor_get_meet(int id, int style, char patient_name[][15], char time[][15], int state[], int& count)
{
    Connection* conn = nullptr;
    PreparedStatement* pstmt = nullptr;
    ResultSet* res = nullptr;
    bool result = false;

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");
        cout << "连接数据库成功" << endl;

        string sql;
        // 0: 今天, 1: 前七天(含今天), 2: 前一个月(含今天)
        if (style == 0) {
            sql = "SELECT p.patient_name AS patient_name, m.meet_date, m.meet_time, m.meet_state "
                "FROM meet_record m "
                "JOIN patients p ON m.patient_id = p.patient_id "
                "WHERE m.doctor_id = ? AND m.meet_date = CURDATE() "
                "ORDER BY m.meet_time DESC";
        }
        else if (style == 1) {
            sql = "SELECT p.patient_name AS patient_name, m.meet_date, m.meet_time, m.meet_state "
                "FROM meet_record m "
                "JOIN patients p ON m.patient_id = p.patient_id "
                "WHERE m.doctor_id = ? AND m.meet_date >= DATE_SUB(CURDATE(), INTERVAL 6 DAY) "
                "ORDER BY m.meet_date DESC, m.meet_time DESC";
        }
        else if (style == 2) {
            sql = "SELECT p.patient_name AS patient_name, m.meet_date, m.meet_time, m.meet_state "
                "FROM meet_record m "
                "JOIN patients p ON m.patient_id = p.patient_id "
                "WHERE m.doctor_id = ? AND m.meet_date >= DATE_SUB(CURDATE(), INTERVAL 1 MONTH) "
                "ORDER BY m.meet_date DESC, m.meet_time DESC";
        }
        else {
            // 如果 style 不在 0,1,2 范围内，可按需处理（比如返回全量，或直接返回 false）
            // 这里暂定为返回全量并倒序，防止调用处报错
            sql = "SELECT p.patient_name AS patient_name, m.meet_date, m.meet_time, m.meet_state "
                "FROM meet_record m "
                "JOIN patients p ON m.patient_id = p.patient_id "
                "WHERE m.doctor_id = ? "
                "ORDER BY m.meet_date DESC, m.meet_time DESC";
        }

        pstmt = conn->prepareStatement(sql);
        pstmt->setInt(1, id);
        // 注意：因为现在 style 不再是数据库过滤状态的条件，所以不需要 setInt(2, style) 了

        res = pstmt->executeQuery();

        int index = 0;
        // 假设传入的数组足够大
        while (res->next() && index < 100) {
            // 获取病人姓名
            string name = res->getString("patient_name");
            strncpy(patient_name[index], name.c_str(), 14);
            patient_name[index][14] = '\0';

            // 组合会诊时间：日期 + "上午/下午"
            string meet_date = res->getString("meet_date");
            int meet_time = res->getInt("meet_time");
            string time_str = meet_date + " " + (meet_time == 0 ? "上午" : "下午");
            strncpy(time[index], time_str.c_str(), 14);
            time[index][14] = '\0';

            // 会诊状态
            state[index] = res->getInt("meet_state");

            cout << "【记录】:" << patient_name[index] << " " << time[index] << endl;
            index++;
        }

        result = true;
        cout << "查询到 " << index << " 条会诊记录" << endl;
        count = index;

    }
    catch (SQLException& e) {
        std::cerr << "数据库异常 code:" << e.getErrorCode()
            << " msg:" << e.what() << std::endl;
        result = false;
    }

    // 统一释放资源
    DbManager::getInstance().close_connection(conn, pstmt, res);
    return result;
}

bool UserModel::get_depart_info(const string& department, std::vector<DOCCTOR_INFO>& res)
{
    Connection* conn = nullptr;
    PreparedStatement* pstmt = nullptr;
    ResultSet* rs = nullptr;
    bool result = false;

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");
        cout << "连接数据库成功" << endl;

        // 查询指定科室的所有医生ID和姓名
        string sql = "SELECT doctor_id, name FROM doctors WHERE department = ? ORDER BY doctor_id";
        pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, department);
        rs = pstmt->executeQuery();

        res.clear(); // 清空输出向量

        while (rs->next()) {
            DOCCTOR_INFO info;
            info.id = rs->getInt("doctor_id");
            string name_str = rs->getString("name");

            // 安全复制到字符数组（假设数组大小足够）
            strncpy(info.name, name_str.c_str(), sizeof(info.name) - 1);
            info.name[sizeof(info.name) - 1] = '\0'; // 确保字符串终止

            res.push_back(info);

            // 调试输出
            cout << "医生ID: " << info.id << ", 姓名: " << info.name << endl;
        }

        cout << "科室 " << department << " 共查询到 " << res.size() << " 位医生" << endl;
        result = true;
    }
    catch (SQLException& e) {
        std::cerr << "数据库异常 code:" << e.getErrorCode()
            << " msg:" << e.what() << std::endl;
        result = false;
    }

    DbManager::getInstance().close_connection(conn, pstmt, rs);
    return result;
}

bool UserModel::get_guard_info(const string& department, GET_GUARD_RESP& resp, string start_day)
{
    // 1. 全部初始化为默认值
    memset(&resp, 0, sizeof(resp));
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 7; ++j) {
            resp.guards[i][j].isfree = true;
            // 科室名（从传入参数复制）
            strncpy(resp.guards[i][j].depart, department.c_str(),
                sizeof(resp.guards[i][j].depart) - 1);
            resp.guards[i][j].depart[sizeof(resp.guards[i][j].depart) - 1] = '\0';
            // 姓名为空
            resp.guards[i][j].name[0] = '\0';
        }
    }

    // 2. 日期解析：将 start_day 转为 struct tm，并计算 end_day（+6天）
    struct tm start_tm = {};
    if (strptime(start_day.c_str(), "%Y-%m-%d", &start_tm) == nullptr) {
        std::cerr << "无效的日期格式: " << start_day << std::endl;
        return false;
    }
    // 转为 time_t 方便加减
    time_t start_time = mktime(&start_tm);
    time_t end_time = start_time + 6 * 24 * 60 * 60; // 加6天
    struct tm end_tm = *localtime(&end_time);
    char end_day[20];
    strftime(end_day, sizeof(end_day), "%Y-%m-%d", &end_tm);

    Connection* conn = nullptr;
    PreparedStatement* pstmt = nullptr;
    ResultSet* rs = nullptr;
    bool success = false;

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");

        // 3. 查询：联表获取医生姓名
        string sql = "SELECT g.date_time, g.ob_time, g.is_free, d.name "
            "FROM guard_record g "
            "LEFT JOIN doctors d ON g.doctor_id1 = d.doctor_id "
            "WHERE g.department = ? AND g.date_time BETWEEN ? AND ? "
            "ORDER BY g.date_time, g.ob_time";
        pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, department);
        pstmt->setString(2, start_day);
        pstmt->setString(3, end_day);
        rs = pstmt->executeQuery();

        // 4. 填充结果
        while (rs->next()) {
            string dateStr = rs->getString("date_time");
            int obTime = rs->getInt("ob_time");
            bool isFree = rs->getBoolean("is_free");
            string doctorName = rs->getString("name");  // 若为 NULL 则返回空串

            // 解析日期，计算与 start_day 的天数差
            struct tm date_tm = {};
            if (strptime(dateStr.c_str(), "%Y-%m-%d", &date_tm) == nullptr) continue;
            time_t date_time = mktime(&date_tm);
            int daysDiff = (int)((date_time - start_time) / (24 * 60 * 60));
            if (daysDiff < 0 || daysDiff > 6) continue;

            int dayIndex = daysDiff;   // 0=周一, 6=周日
            if (obTime < 0 || obTime > 2) continue;

            GUARD_REPIX_T& guard = resp.guards[obTime][dayIndex];
            guard.time = obTime;
            guard.isfree = isFree;
            strncpy(guard.date, dateStr.c_str(), sizeof(guard.date) - 1);
            guard.date[sizeof(guard.date) - 1] = '\0';
            // 复制医生姓名（最多 14 字符 + '\0'）
            strncpy(guard.name, doctorName.c_str(), sizeof(guard.name) - 1);
            guard.name[sizeof(guard.name) - 1] = '\0';
            // depart 已在初始化时设置，不需要修改
        }

        success = true;
    }
    catch (SQLException& e) {
        std::cerr << "数据库异常 code:" << e.getErrorCode()
            << " msg:" << e.what() << std::endl;
        success = false;
    }

    // 5. 清理资源
    DbManager::getInstance().close_connection(conn, pstmt, rs);
    return success;
}
//---------------------------------------------------------------患者
bool UserModel::patient_reg(const string& name, const string& card, const string& phone, const string& pwd) {
    Connection* conn = nullptr;
    PreparedStatement* pstmt = nullptr;
    bool result = false;

    try {
        conn = DbManager::getInstance().get_connection();  // 内部已执行 SET NAMES utf8mb4
        conn->setSchema("hospital_db");

        // 如果 get_connection 没有设置字符集，在这里添加：
        // Statement* stmt = conn->createStatement();
        // stmt->execute("SET NAMES utf8mb4");
        // delete stmt;

        cout << "连接数据库成功" << endl;

        string sql = "INSERT INTO patients(patient_name, card_id, phone, account, pwd, state) "
            "VALUES (?, ?, ?, ?, ?, 1)";
        pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, name);   // name 必须为 UTF-8 编码
        pstmt->setString(2, card);
        pstmt->setString(3, phone);
        pstmt->setString(4, phone);  // account = phone
        pstmt->setString(5, pwd);

        int affected = pstmt->executeUpdate();
        if (affected == 1) {
            cout << "患者注册成功，姓名: " << name << ", 手机号: " << phone << endl;
            result = true;
        }
        else {
            cerr << "插入失败，影响行数: " << affected << endl;
            result = false;
        }
    }
    catch (SQLException& e) {
        std::cerr << "数据库异常 code:" << e.getErrorCode()
            << " msg:" << e.what() << std::endl;
        result = false;
    }

    DbManager::getInstance().close_connection(conn, pstmt, nullptr);
    return result;
}

bool UserModel::patient_login(const string& account, const string& pwd, int& id, string& name, int style)
{
    Connection* conn = nullptr;
    PreparedStatement* pstmt = nullptr;
    ResultSet* rs = nullptr;
    bool result = false;

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");

        // 确保连接字符集（可选，若已在 DbManager 中设置可省略）
        Statement* stmt = conn->createStatement();
        stmt->execute("SET NAMES utf8mb4");
        delete stmt;

        // 根据 style 构造不同的查询条件
        string sql;
        if (style == 1) {
            // 手机号登录
            sql = "SELECT patient_id, patient_name FROM patients WHERE phone = ? AND pwd = ? AND state = 1";
        }
        else if (style == 2) {
            // 卡号登录
            sql = "SELECT patient_id, patient_name FROM patients WHERE card_id = ? AND pwd = ? AND state = 1";
        }
        else {
            cerr << "无效的登录方式 style=" << style << endl;
            return false;
        }

        pstmt = conn->prepareStatement(sql);
        pstmt->setString(1, account);
        pstmt->setString(2, pwd);
        rs = pstmt->executeQuery();

        if (rs->next()) {
            id = rs->getInt("patient_id");
            name = rs->getString("patient_name");  // 可能为空，但字符串可正常赋值
            result = true;
            cout << "患者登录成功，ID: " << id << ", 姓名: " << name << endl;
        }
        else {
            cout << "账号或密码错误，或账户已停用" << endl;
            result = false;
        }
    }
    catch (SQLException& e) {
        std::cerr << "数据库异常 code:" << e.getErrorCode()
            << " msg:" << e.what() << std::endl;
        result = false;
    }

    DbManager::getInstance().close_connection(conn, pstmt, rs);
    return result;
}



bool UserModel::patient_get_doctor(vector<patient_doctor_infoo>& doctors) {
    Connection* conn = nullptr;
    PreparedStatement* pstmt = nullptr;
    ResultSet* rs = nullptr;
    bool result = false;

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");

        // 设置字符集（临时 Statement，立即释放）
        Statement* tempStmt = conn->createStatement();
        tempStmt->execute("SET NAMES utf8mb4");
        delete tempStmt;

        // 查询今天的排班，并关联医生信息
        string sql =
            "SELECT d.doctor_id, d.name, g.ob_time, g.department "
            "FROM guard_record g "
            "JOIN doctors d ON d.doctor_id = g.doctor_id1 "
            "WHERE g.date_time = CURDATE() "
            "UNION "
            "SELECT d.doctor_id, d.name, g.ob_time, g.department "
            "FROM guard_record g "
            "JOIN doctors d ON d.doctor_id = g.doctor_id2 "
            "WHERE g.date_time = CURDATE() "
            "ORDER BY ob_time, department, doctor_id";

        pstmt = conn->prepareStatement(sql);
        rs = pstmt->executeQuery();

        doctors.resize(0);

        while (rs->next()) {
            patient_doctor_infoo info;
            info.id = rs->getInt("doctor_id");
            info.time = rs->getInt("ob_time");

            string name_str = rs->getString("name");
            strncpy(info.name, name_str.c_str(), sizeof(info.name) - 1);
            info.name[sizeof(info.name) - 1] = '\0';

            string dept_str = rs->getString("department");
            strncpy(info.department, dept_str.c_str(), sizeof(info.department) - 1);
            info.department[sizeof(info.department) - 1] = '\0';

            doctors.push_back(info);

            cout << "医生ID: " << info.id
                << ", 姓名: " << info.name
                << ", 时间段: " << info.time
                << ", 科室: " << info.department << endl;
        }

        cout << "今日值班医生共 " << doctors.size() << " 条记录" << endl;
        result = true;
    }
    catch (SQLException& e) {
        cerr << "数据库异常 code:" << e.getErrorCode()
            << " msg:" << e.what() << endl;
        result = false;
    }
    cout << "任务完成" << endl;;
    // 统一释放资源，与 patient_login 一致
    DbManager::getInstance().close_connection(conn, pstmt, rs);
    return result;
}

bool UserModel::parseDate(const string& dateStr, tm& tm_out)
{
    // 格式必须为 "YYYY-MM-DD"
    int year, month, day;
    if (sscanf(dateStr.c_str(), "%d-%d-%d", &year, &month, &day) != 3)
        return false;
    tm_out.tm_year = year - 1900;
    tm_out.tm_mon = month - 1;
    tm_out.tm_mday = day;
    tm_out.tm_hour = 0;
    tm_out.tm_min = 0;
    tm_out.tm_sec = 0;
    // 注意：tm_isdst 设为 -1 让 mktime 自动判断夏令时
    tm_out.tm_isdst = -1;
    return true;
}
