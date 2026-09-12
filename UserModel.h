#pragma once
#include "DbManager.h"
#include <vector>
class UserModel
{
public:

	//医生
	bool db_login(char* name, char* password);
	
	bool doctor_login(const string& account, const string& pwd,int& doctor_id,int& doctor_role);

	bool doctor_get_meet(int id, int style, char patient_name[][15], char time[][15], int state[],int &count, int meet_id[],int patient_id[]);
	
	bool get_depart_info(const string& department,std::vector<DOCCTOR_INFO>& res);

	bool get_guard_info(const string & department, GET_GUARD_RESP& resp, string start_day);
	//患者
	bool patient_reg(const string &name,const string &card,const string &phone,const string &pwd);

	bool patient_login(const string& account, const string& pwd,int &id,string &name,int style);

	bool patient_get_doctor(vector<patient_doctor_infoo> &doctors);

	bool patient_set_app(const int &doctor_id, const int& patient_id, const string& date_time, const int ob_time);

	bool set_record(const int &doctor_id, const int &patient_id, const string &diagnosis, const string &treat_plan);
	bool get_medical_records(const MEDICAL_RECORD_REQ& req, GET_MEDICAL_RECORD_RESP& resp);
	bool get_medical_record_detail(const MEDICAL_RECORD_DETAIL_REQ& req, MEDICAL_RECORD_DETAIL_RESP& resp);

	bool set_img_record(const string &path,int id);

	static bool parseDate(const string& dateStr, struct tm& tm_out);
	
	

};

