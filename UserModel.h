#pragma once
#include "DbManager.h"
#include <vector>
class UserModel
{
public:

	//医生
	bool db_login(char* name, char* password);
	
	bool doctor_login(const string& account, const string& pwd,int& doctor_id,int& doctor_role);

	bool doctor_get_meet(int id, int style, char patient_name[][15], char time[][15], int state[],int &count);
	
	bool get_depart_info(const string& department,std::vector<DOCCTOR_INFO>& res);

	bool get_guard_info(const string & department, GET_GUARD_RESP& resp, string start_day);
	//患者
	bool patient_reg(const string &name,const string &card,const string &phone,const string &pwd);

	bool patient_login(const string& account, const string& pwd,int &id,string &name,int style);

	bool patient_get_doctor(vector<patient_doctor_infoo> &doctors);

	static bool parseDate(const string& dateStr, struct tm& tm_out);
	
	

};

