#include "ManagerModel.h"

#include <algorithm>  // for std::min
#include <iostream>

bool ManagerModel::to_repix_guard(const vector<GUARD_REPIX_T>& repixs, int size)
{
    if (repixs.empty() || size <= 0) {
        return true;
    }

    int count = std::min(size, static_cast<int>(repixs.size()));

    Connection* conn = nullptr;
    PreparedStatement* pstmtCheck = nullptr;
    PreparedStatement* pstmtUpdate = nullptr;
    PreparedStatement* pstmtInsert = nullptr;
    ResultSet* rs = nullptr;
    bool success = false;

    try {
        conn = DbManager::getInstance().get_connection();
        conn->setSchema("hospital_db");
        conn->setAutoCommit(false);

        // 1. 查询是否存在（根据日期 + 时段 + 科室）
        string checkSQL = "SELECT guard_id FROM guard_record WHERE date_time = ? AND ob_time = ? AND department = ?";
        // 2. 更新（只更新医生ID1 和 is_free，department 不变）
        string updateSQL = "UPDATE guard_record SET doctor_id1 = ?, is_free = ? WHERE guard_id = ?";
        // 3. 插入（包含 department）
        string insertSQL = "INSERT INTO guard_record (date_time, ob_time, department, doctor_id1, is_free) VALUES (?, ?, ?, ?, ?)";

        pstmtCheck = conn->prepareStatement(checkSQL);
        pstmtUpdate = conn->prepareStatement(updateSQL);
        pstmtInsert = conn->prepareStatement(insertSQL);

        for (int i = 0; i < count; ++i) {
            const GUARD_REPIX_T& item = repixs[i];

            string dateStr = item.date;
            int obTime = item.time;
            string department = item.depart;   // char[] -> string
            int doctorId = item.id;
            bool isFree = item.isfree;

            // 1) 查询是否存在（加入 department）
            pstmtCheck->setString(1, dateStr);
            pstmtCheck->setInt(2, obTime);
            pstmtCheck->setString(3, department);
            rs = pstmtCheck->executeQuery();

            if (rs->next()) {
                // 存在 → 更新（只改医生和空闲状态）
                int guardId = rs->getInt("guard_id");
                pstmtUpdate->setInt(1, doctorId);
                pstmtUpdate->setBoolean(2, isFree);
                pstmtUpdate->setInt(3, guardId);
                pstmtUpdate->executeUpdate();
            }
            else {
                // 不存在 → 插入（含 department）
                pstmtInsert->setString(1, dateStr);
                pstmtInsert->setInt(2, obTime);
                pstmtInsert->setString(3, department);
                pstmtInsert->setInt(4, doctorId);
                pstmtInsert->setBoolean(5, isFree);
                pstmtInsert->executeUpdate();
            }

            // 清理结果集
            rs->close();
            delete rs;
            rs = nullptr;

            // 清除参数
            pstmtCheck->clearParameters();
            pstmtUpdate->clearParameters();
            pstmtInsert->clearParameters();
        }

        conn->commit();
        success = true;
        std::cout << "成功保存 " << count << " 条排班记录（含科室）" << std::endl;
    }
    catch (SQLException& e) {
        if (conn) {
            try { conn->rollback(); }
            catch (...) {}
        }
        std::cerr << "数据库异常 code:" << e.getErrorCode()
            << " msg:" << e.what() << std::endl;
        success = false;
    }

    // 清理资源
    if (pstmtCheck) delete pstmtCheck;
    if (pstmtUpdate) delete pstmtUpdate;
    if (pstmtInsert) delete pstmtInsert;
    if (conn) {
        DbManager::getInstance().close_connection(conn, nullptr, nullptr);
    }

    return success;
}
