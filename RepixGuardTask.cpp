#include "RepixGuardTask.h"
#include "ManagerModel.h"
RepixGuardTask::RepixGuardTask(void* data, int size, int fd)
	:BusinessTask(data,size,fd)
{
}

RepixGuardTask::RepixGuardTask(void* data, int size, int fd,int num)
	:BusinessTask(data, size, fd)
{
	this->num = num;
}

void RepixGuardTask::execute()
{
    int now = 0;
    int single_pack = sizeof(GUARD_REPIX_T);
    vector<GUARD_REPIX_T> repixs;    // 默认空，无需指定大小

    // 假设 m_data 是 char*，size 是总数据长度
    while (now + single_pack <= this->size) {   // 确保剩余数据足够一个包
        GUARD_REPIX_T info;                     // 局部变量，栈上分配
        memcpy(&info, m_data + now, single_pack); // 拷贝一个完整结构体
        repixs.push_back(info);                 // 存入 vector
        now += single_pack;
    }

    // 打印调试信息
    cout << "解析到" << repixs.size() << "条排班记录：" << endl;
    for (size_t i = 0; i < repixs.size(); ++i) {
        const auto& item = repixs[i];
        cout << "记录" << i + 1 << ":"
            << "id=" << item.id
            << "date=" << item.date   // char[] 可直接输出
            << "time=" << item.time
            << "isfree=" << item.isfree
            << "depart=" << item.depart << endl;
    }

    // 如果后续需要用到这些数据，可以将 repixs 赋值给成员变量或发送信号
    // 例如：m_repixs = std::move(repixs);
    ManagerModel model;
    model.to_repix_guard(repixs,repixs.size());
}
