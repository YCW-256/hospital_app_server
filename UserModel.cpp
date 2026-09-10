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

bool UserModel::doctor_login(const string& account, const string& pwd,
    int & doctor_id,int & doctor_role)
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
            doctor_role = res->getInt("role");  // 添加这一行，字段名按实际表结构调整
			cout << "查询到医生id = " << doctor_id << ", 角色 = " << doctor_role << endl;
		}
		
	}
	catch (SQLException& e) {
		std::cerr << "数据库异常 code:" << e.getErrorCode()
			<< " msg:" << e.what() << std::endl;
	}
	DbManager::getInstance().close_connection(conn, pstmt, res);
	return result;

	
}

bool UserModel::doctor_get_meet(int id, int style, char patient_name[][15], char time[][15], int state[], int& count, int meet_id[], int patient_id[])
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
            sql = "SELECT p.patient_name AS patient_name, m.meet_date, m.meet_time, m.meet_state, m.meet_id, m.patient_id "
                "FROM meet_record m "
                "JOIN patients p ON m.patient_id = p.patient_id "
                "WHERE m.doctor_id = ? AND m.meet_date = CURDATE() "
                "ORDER BY m.meet_time DESC";
        }
        else if (style == 1) {
            sql = "SELECT p.patient_name AS patient_name, m.meet_date, m.meet_time, m.meet_state, m.meet_id, m.patient_id "
                "FROM meet_record m "
                "JOIN patients p ON m.patient_id = p.patient_id "
                "WHERE m.doctor_id = ? AND m.meet_date >= DATE_SUB(CURDATE(), INTERVAL 6 DAY) "
                "ORDER BY m.meet_date DESC, m.meet_time DESC";
        }
        else if (style == 2) {
            sql = "SELECT p.patient_name AS patient_name, m.meet_date, m.meet_time, m.meet_state, m.meet_id, m.patient_id "
                "FROM meet_record m "
                "JOIN patients p ON m.patient_id = p.patient_id "
                "WHERE m.doctor_id = ? AND m.meet_date >= DATE_SUB(CURDATE(), INTERVAL 1 MONTH) "
                "ORDER BY m.meet_date DESC, m.meet_time DESC";
        }
        else {
            // 如果 style 不在 0,1,2 范围内，可按需处理（比如返回全量，或直接返回 false）
            // 这里暂定为返回全量并倒序，防止调用处报错
            sql = "SELECT p.patient_name AS patient_name, m.meet_date, m.meet_time, m.meet_state, m.meet_id, m.patient_id "
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

            // 会诊ID
            meet_id[index] = res->getInt("meet_id");

            // 新增：病人ID
            patient_id[index] = res->getInt("patient_id");

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

bool UserModel::patient_set_app(const int& doctor_id, const int& patient_id, const string& date_time, const int ob_time)
{
    Connection* conn = nullptr;
    PreparedStatement* pstmt = nullptr;
    bool result = false;

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");

        // 设置字符集（临时 Statement，立即释放）
        Statement* tempStmt = conn->createStatement();
        tempStmt->execute("SET NAMES utf8mb4");
        delete tempStmt;

        // 插入预约记录，meet_state 默认为 0（待处理状态）
        string sql = "INSERT INTO meet_record (doctor_id, patient_id, meet_date, meet_time, meet_state) "
            "VALUES (?, ?, ?, ?, 0)";

        pstmt = conn->prepareStatement(sql);
        pstmt->setInt(1, doctor_id);
        pstmt->setInt(2, patient_id);
        pstmt->setString(3, date_time);   // 日期格式需为 YYYY-MM-DD
        pstmt->setInt(4, ob_time);        // 时间段整数

        int affectedRows = pstmt->executeUpdate();
        if (affectedRows > 0) {
            result = true;
            cout << "预约插入成功，影响行数: " << affectedRows << endl;
        }
        else {
            cerr << "预约插入失败，未影响任何行" << endl;
        }
    }
    catch (SQLException& e) {
        cerr << "数据库异常 code:" << e.getErrorCode()
            << " msg:" << e.what() << endl;
        result = false;
    }

    // 统一释放资源（无结果集，传 nullptr）
    DbManager::getInstance().close_connection(conn, pstmt, nullptr);
    return result;
}

bool UserModel::set_record(const int& doctor_id, const int& patient_id, const string& diagnosis, const string& treat_plan)
{
    Connection* conn = nullptr;
    PreparedStatement* pstmt = nullptr;
    ResultSet* res = nullptr;
    bool result = false;

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");

        // 开启事务
        conn->setAutoCommit(false);

        // ---------- 任务1：更新 meet_record 状态 ----------
        int meet_id = -1;
        // 查找该医生和患者最近一条未完成的会诊记录
        string findSql =
            "SELECT meet_id FROM meet_record "
            "WHERE doctor_id = ? AND patient_id = ? AND meet_state = 0 "
            "ORDER BY meet_date DESC, meet_time DESC LIMIT 1";
        pstmt = conn->prepareStatement(findSql);
        pstmt->setInt(1, doctor_id);
        pstmt->setInt(2, patient_id);
        res = pstmt->executeQuery();
        if (res->next()) {
            meet_id = res->getInt("meet_id");
        }
        // 释放查询资源
        DbManager::getInstance().close_connection(nullptr, pstmt, res);
        pstmt = nullptr;
        res = nullptr;

        if (meet_id != -1) {
            string updateSql = "UPDATE meet_record SET meet_state = 1 WHERE meet_id = ?";
            pstmt = conn->prepareStatement(updateSql);
            pstmt->setInt(1, meet_id);
            pstmt->executeUpdate();
            // 释放更新资源
            DbManager::getInstance().close_connection(nullptr, pstmt, nullptr);
            pstmt = nullptr;
        }

        // ---------- 任务2：插入 medical_record ----------
        string insertSql =
            "INSERT INTO medical_record "
            "(patient_id, doctor_id, main_symptom, diagnosis, treat_plan, record_time, state, create_time, update_time) "
            "VALUES (?, ?, NULL, ?, ?, NOW(), 0, NOW(), NOW())";
        pstmt = conn->prepareStatement(insertSql);
        pstmt->setInt(1, patient_id);
        pstmt->setInt(2, doctor_id);
        pstmt->setString(3, diagnosis);
        pstmt->setString(4, treat_plan);
        pstmt->executeUpdate();

        // 提交事务
        conn->commit();
        result = true;

    }
    catch (SQLException& e) {
        cerr << "数据库异常 code:" << e.getErrorCode() << " msg:" << e.what() << endl;
        if (conn) {
            try {
                conn->rollback();   // 回滚事务
            }
            catch (SQLException& ex) {
                cerr << "回滚失败: " << ex.what() << endl;
            }
        }
        result = false;
    }

    // 最终释放连接和未释放的语句
    DbManager::getInstance().close_connection(conn, pstmt, res);
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

bool UserModel::get_medical_records(const MEDICAL_RECORD_REQ& req, GET_MEDICAL_RECORD_RESP& resp)
{
    Connection* conn = nullptr;
    PreparedStatement* count_stmt = nullptr;
    PreparedStatement* list_stmt = nullptr;
    ResultSet* count_res = nullptr;
    ResultSet* list_res = nullptr;
    bool result = false;

    memset(&resp, 0, sizeof(resp));

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");

        string where = " WHERE m.doctor_id = ?";
        if (req.state == 0 || req.state == 1)
            where += " AND m.state = ?";
        if (req.patient_name[0] != '\0')
            where += " AND p.patient_name LIKE ?";
        if (req.date_begin[0] != '\0')
            where += " AND DATE(m.record_time) >= ?";
        if (req.date_end[0] != '\0')
            where += " AND DATE(m.record_time) <= ?";

        string from = " FROM medical_record m JOIN patients p ON m.patient_id = p.patient_id";

        count_stmt = conn->prepareStatement("SELECT COUNT(*)" + from + where);
        int parameter = 1;
        count_stmt->setInt(parameter++, req.doctor_id);
        if (req.state == 0 || req.state == 1)
            count_stmt->setInt(parameter++, req.state);
        if (req.patient_name[0] != '\0')
            count_stmt->setString(parameter++, string("%") + req.patient_name + "%");
        if (req.date_begin[0] != '\0')
            count_stmt->setString(parameter++, req.date_begin);
        if (req.date_end[0] != '\0')
            count_stmt->setString(parameter++, req.date_end);
        count_res = count_stmt->executeQuery();
        if (count_res->next())
            resp.total = count_res->getInt(1);
        DbManager::getInstance().close_connection(nullptr, count_stmt, count_res);
        count_stmt = nullptr;
        count_res = nullptr;

        string sql = "SELECT m.record_id, m.patient_id, m.state, p.patient_name, "
            "DATE_FORMAT(m.record_time, '%Y-%m-%d %H:%i:%s') AS record_time, "
            "m.main_symptom" + from + where +
            " ORDER BY m.record_time DESC, m.record_id DESC LIMIT ?";
        list_stmt = conn->prepareStatement(sql);
        parameter = 1;
        list_stmt->setInt(parameter++, req.doctor_id);
        if (req.state == 0 || req.state == 1)
            list_stmt->setInt(parameter++, req.state);
        if (req.patient_name[0] != '\0')
            list_stmt->setString(parameter++, string("%") + req.patient_name + "%");
        if (req.date_begin[0] != '\0')
            list_stmt->setString(parameter++, req.date_begin);
        if (req.date_end[0] != '\0')
            list_stmt->setString(parameter++, req.date_end);
        list_stmt->setInt(parameter, MEDICAL_RECORD_MAX_ITEMS);
        list_res = list_stmt->executeQuery();

        while (list_res->next() && resp.count < MEDICAL_RECORD_MAX_ITEMS) {
            MEDICAL_RECORD_LIST_ITEM& item = resp.items[resp.count];
            item.record_id = list_res->getInt("record_id");
            item.patient_id = list_res->getInt("patient_id");
            item.state = list_res->getInt("state");
            string patient_name = list_res->getString("patient_name");
            string record_time = list_res->getString("record_time");
            string main_symptom = list_res->getString("main_symptom");
            strncpy(item.patient_name, patient_name.c_str(), sizeof(item.patient_name) - 1);
            item.patient_name[sizeof(item.patient_name) - 1] = '\0';
            strncpy(item.record_time, record_time.c_str(), sizeof(item.record_time) - 1);
            item.record_time[sizeof(item.record_time) - 1] = '\0';
            strncpy(item.main_symptom, main_symptom.c_str(), sizeof(item.main_symptom) - 1);
            item.main_symptom[sizeof(item.main_symptom) - 1] = '\0';
            ++resp.count;
        }

        result = true;
    }
    catch (SQLException& e) {
        cerr << "数据库异常 code:" << e.getErrorCode() << " msg:" << e.what() << endl;
    }

    DbManager::getInstance().close_connection(nullptr, list_stmt, list_res);
    DbManager::getInstance().close_connection(conn, count_stmt, count_res);
    return result;
}

bool UserModel::get_medical_record_detail(const MEDICAL_RECORD_DETAIL_REQ& req, MEDICAL_RECORD_DETAIL_RESP& resp)
{
    Connection* conn = nullptr;
    PreparedStatement* pstmt = nullptr;
    ResultSet* res = nullptr;
    bool result = false;

    memset(&resp, 0, sizeof(resp));

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");
        string sql =
            "SELECT m.record_id, m.patient_id, m.doctor_id, m.state, p.patient_name, "
            "DATE_FORMAT(m.record_time, '%Y-%m-%d %H:%i:%s') AS record_time, "
            "d.name AS doctor_name, m.main_symptom, m.diagnosis, m.treat_plan "
            "FROM medical_record m "
            "JOIN patients p ON m.patient_id = p.patient_id "
            "JOIN doctors d ON m.doctor_id = d.doctor_id "
            "WHERE m.record_id = ? AND m.doctor_id = ?";
        pstmt = conn->prepareStatement(sql);
        pstmt->setInt(1, req.record_id);
        pstmt->setInt(2, req.doctor_id);
        res = pstmt->executeQuery();

        if (res->next()) {
            resp.record_id = res->getInt("record_id");
            resp.patient_id = res->getInt("patient_id");
            resp.doctor_id = res->getInt("doctor_id");
            resp.state = res->getInt("state");
            string patient_name = res->getString("patient_name");
            string record_time = res->getString("record_time");
            string doctor_name = res->getString("doctor_name");
            string main_symptom = res->getString("main_symptom");
            string diagnosis = res->getString("diagnosis");
            string treat_plan = res->getString("treat_plan");
            strncpy(resp.patient_name, patient_name.c_str(), sizeof(resp.patient_name) - 1);
            resp.patient_name[sizeof(resp.patient_name) - 1] = '\0';
            strncpy(resp.record_time, record_time.c_str(), sizeof(resp.record_time) - 1);
            resp.record_time[sizeof(resp.record_time) - 1] = '\0';
            strncpy(resp.doctor_name, doctor_name.c_str(), sizeof(resp.doctor_name) - 1);
            resp.doctor_name[sizeof(resp.doctor_name) - 1] = '\0';
            strncpy(resp.main_symptom, main_symptom.c_str(), sizeof(resp.main_symptom) - 1);
            resp.main_symptom[sizeof(resp.main_symptom) - 1] = '\0';
            strncpy(resp.diagnosis, diagnosis.c_str(), sizeof(resp.diagnosis) - 1);
            resp.diagnosis[sizeof(resp.diagnosis) - 1] = '\0';
            strncpy(resp.treat_plan, treat_plan.c_str(), sizeof(resp.treat_plan) - 1);
            resp.treat_plan[sizeof(resp.treat_plan) - 1] = '\0';
            result = true;
        }
    }
    catch (SQLException& e) {
        cerr << "数据库异常 code:" << e.getErrorCode() << " msg:" << e.what() << endl;
    }

    DbManager::getInstance().close_connection(conn, pstmt, res);
    return result;
}
