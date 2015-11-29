#include "xmlparse.h"
#include "astcmd.h"

using namespace std;
using namespace boost;

asteriskCommands asterisk;

int XMLParse::opDB(map<string,string> XMLarray)
{
/*
	asterisk.readConfig();

	MYSQL *conn, mysql;
	MYSQL_RES *result;
	MYSQL_ROW row;
	mysql_init(&mysql);
	
	conn = mysql_real_connect(&mysql, asterisk.MYSQL_SERVER, asterisk.MYSQL_USER,asterisk.MYSQL_PASSWORD,asterisk.CDR_DATABASE,0,NULL,0);
	if(conn == NULL) {
		printf("Database error");
		return 0;
	}
	else
	{
		ostringstream os;
		string insertQuery;
		
		os << "insert into pbxml " <<
			"(id , Source, Dest, ChannelName, CallType, StartDateSaat, EndDateSaat, MonitorUrl, Status) " <<
			"VALUES("<<
			"'" << XMLarray["Id"] << "', " <<
			"'" << XMLarray["Source"] << "', " <<
			"'" << XMLarray["Dest"] << "', " <<
			"'" << XMLarray["ChannelName"] << "', " <<
			"'" << XMLarray["CallType"] << "', " <<
			"'" << XMLarray["StartDate"] << "', " <<
			"'" << XMLarray["EndDate"] << "', " <<
			"'" << XMLarray["MonitorUrl"] << "', " <<
			"'" << XMLarray["Status"] << "'" <<
			");";
		insertQuery = os.str();
		mysql_query(conn, insertQuery.c_str());
		
		mysql_close(conn);
	}

	return 1;*/
}

string XMLParse::parse(string XMLData)
{
	map<string,string> resultmap;
	vector<string> resultTemp;
	string resultEnd;
	
	FILE *file;
	char expression[500];
	char line[256];
	
	string EXP_CALL;
	string fileContent;
	
	fileContent = XMLData;
	
	sprintf(expression, "<PBX>(.*?)</PBX>");
	
	EXP_CALL = expression;
	boost::regex e(EXP_CALL);
	bool matchesMain = boost::regex_search(fileContent.begin(), fileContent.end(), e);

	if (matchesMain) {
		boost::sregex_token_iterator i(fileContent.begin(), fileContent.end(), e, 0);
		boost::sregex_token_iterator j;

		sprintf(expression, "<Call>(.*?)</Call>");
		EXP_CALL = expression;
		boost::regex e(EXP_CALL);
		
		bool matchesCall = boost::regex_search(fileContent.begin(), fileContent.end(), e);
		string values;
		
		if (matchesCall) {
			boost::sregex_token_iterator i(fileContent.begin(), fileContent.end(), e, 1);
			boost::sregex_token_iterator j;
			values = *i++;
			
			sprintf(expression, "<(.*?)>(.*?)</(.*?)>");
			EXP_CALL = expression;
			boost::regex et(EXP_CALL);
			bool matchesTags = boost::regex_search(values.begin(), values.end(), et);
			
			if (matchesTags) 
			{
				int tagKeys[] = {1,2};
				boost::sregex_token_iterator k(values.begin(), values.end(), et, tagKeys);
				boost::sregex_token_iterator t;
				string tags;
				
				while(k != t) {
					resultTemp.push_back(*k++);
				}

				if (resultTemp.size()>0)
				{
					for(int res=0; res<=resultTemp.size()-1; res++ ) 
					{
						boost::trim(resultTemp[res]);
						boost::trim(resultTemp[res+1]);
						if (resultTemp[res+1].empty())
							resultTemp[res+1] = "";
						resultmap.insert(pair<string, string>(resultTemp[res], resultTemp[res+1]));
						res++;
					}
					resultEnd = XMLParse::operateXml(resultmap);
				}
			}
		}
	}
	return resultEnd;
}

string XMLParse::operateXml(map<string,string> XMLarray)
{
	vector<string> result;
	string endResult;
	
	time_t rawtime;
	struct tm * timeinfo;
	char curr_date [80];
	char curr_time [80];
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strftime (curr_date,80,"%Y-%m-%d",timeinfo);
	strftime (curr_time,80,"%H:%M:%S",timeinfo);
	
	string CallType = XMLarray["CallType"];
	
	if (CallType != "")
	{
		if (CallType == "10") 
		{
			result = asterisk.hangupCall(XMLarray["Source"]);
			if (!result.empty())
			{
				XMLarray["ChannelName"] = result[2];
				XMLarray["Status"] = result[4];
				XMLarray["EndDate"] = curr_date;
				XMLarray["EndTime"] = curr_time;
				
				if (result.size() > 7)
					XMLarray["MonitorUrl"] = result[7];
				
				//XMLParse::opDB(XMLarray);
				endResult = XMLParse::createXML(XMLarray);
			}
			else
			{
				XMLarray["Status"] = "Hangup failed. No call to hangup or Source/Dest not exists";
				endResult = XMLParse::createXML(XMLarray);
			}
			
		}
		
		else if (CallType == "11") 
		{
			result = asterisk.originate(XMLarray["Source"], XMLarray["Dest"]);
			if (!result.empty())
			{
				XMLarray["ChannelName"] = result[2];
				XMLarray["Status"] = result[4];
				XMLarray["StartDate"] = curr_date;
				XMLarray["StartTime"] = curr_time;
				
				if (result.size() > 7)
					XMLarray["MonitorUrl"] = result[7];

				//XMLParse::opDB(XMLarray);
				endResult = XMLParse::createXML(XMLarray);
			}
			else
			{
				XMLarray["Status"] = "Call failed. Check source and/or dest";
				endResult = XMLParse::createXML(XMLarray);
			}
		}
		else if (CallType == "19") 
		{
			result = asterisk.getChannelInfo(XMLarray["Source"]);
			if (!result.empty())
			{
				XMLarray["Source"] = result[0];
				XMLarray["ChannelName"] = result[2];
				XMLarray["Id"] = result[2];
				if(!result[5].empty())
					XMLarray["Dest"] = result[5];
				else
					XMLarray["Dest"] = result[3];
					
				XMLarray["Status"] = result[4];
				XMLarray["StartDate"] = curr_date;
				XMLarray["StartTime"] = "Duration: "+result[6];
				
				if (result.size() > 7)
					XMLarray["MonitorUrl"] = result[7];

				//XMLParse::opDB(XMLarray);
				endResult = XMLParse::createXML(XMLarray);
			}
			else
			{
				XMLarray["Status"] = "Cannot retrieved call info. Source unreachable or mismatch";
				endResult = XMLParse::createXML(XMLarray);
			}
		}
	}
	/*shutdown(sock,SHUT_WR);
	close(sock);*/
	/*close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	*/
	return endResult;
}

string XMLParse::createXML(map<string,string> XMLarray)
{
	string result;
	string CallType = XMLarray["CallType"];

	if (CallType.size() > 0)
	{
		ostringstream os;
		os << 
			"<?xml version='1.0' standalone='yes'?><PBX><Call>" << 
			"<Id>"<< XMLarray["Id"] <<"</Id>"<<
			"<Source>"<< XMLarray["Source"] <<"</Source>"<<
			"<Dest>"<< XMLarray["Dest"] <<"</Dest>"<<
			"<ChannelName>"<< XMLarray["ChannelName"] <<"</ChannelName>"<<
			"<CallType>"<< XMLarray["CallType"] <<"</CallType>"<<
			"<StartDate>"<< XMLarray["StartDate"] <<"</StartDate>"<<
			"<StartTime>"<< XMLarray["StartTime"] <<"</StartTime>"<<
			"<MonitorUrl>"<< XMLarray["MonitorUrl"] <<"</MonitorUrl>"<<
			"<EndDate>"<< XMLarray["EndDate"] <<"</EndDate>"<<
			"<EndTime>"<< XMLarray["EndTime"] <<"</EndTime>"<<
			"<Status>"<< XMLarray["Status"] <<"</Status>"<<
			"</Call>" <<
			"</PBX>"
			<<endl;
		result = os.str();
	}
	return result;
}

/*
int main()
{
	XMLParse xmlparse;
	vector<string> test;
 
	string tempData = "<?xml version='1.0' standalone='yes'?> \
	<PBX> \
		<Call> \
			<Id></Id> \
			<Source>1001</Source> \
			<Dest>905427702100</Dest> \
			<ChannelName></ChannelName> \
			<CallType>11</CallType> \
			<StartDate></StartDate> \
			<StartTime></StartTime> \
			<MonitorUrl></MonitorUrl> \
			<EndDate></EndDate> \
			<EndTime></EndTime> \
			<Status>1</Status> \
		</Call> \
	</PBX> \
	";
	string res;
	res = xmlparse.parse(tempData);
	cout << "result : " << res <<endl;
	
	
	return 0;
}*/