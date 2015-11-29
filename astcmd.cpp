#include "astcmd.h"
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#include <fcntl.h>

using namespace std;
using namespace boost;

int FDID;

string apiKeyControl(void)
{
	int fd;
	struct ifreq ifr;
	char *iface;
	unsigned char *mac;
	
	iface = "eth0";
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	int one = 1; 
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);
	ioctl(fd, SIOCGIFHWADDR, &ifr);

	close(fd);
	
	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
	
	char macChar[50];
	string key;
	stringstream ss;
	sprintf(macChar,"%.2x%.2x%.2x%.2x%.2x%.2x" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	
	for (int i=0; i<=strlen(macChar);i++)
	{
		ss << hex << (int)macChar[i];
	}
	
	key = ss.str();
	
	return key;
}

int asteriskCommands::readConfig()
{
	char line[256];
	int linenum=0;
	FILE *file;
	file = fopen("/etc/pbxml.conf","r");
	
	while(fgets(line, 256, file) != NULL)
	{
		char param[256], value[256];
		linenum++;
		
		if(line[0] == '#' || line[0] == '\n') continue;

		if(sscanf(line, "%s = %s", param, value) != 2) 
		{
			fprintf(stderr, "Invalid character in config file at line : %d\n", linenum);
			continue;
		}
		
		if (strcmp(param,"api_key") == 0) 
			strcpy(asteriskCommands::API_KEY, value);
		if (strcmp(param,"service_port") == 0) 
			asteriskCommands::SERVICE_PORT = atoi(value);
		if (strcmp(param,"server") == 0) 
			strcpy(asteriskCommands::SERVER, value);
		if (strcmp(param,"ami_port") == 0)
			asteriskCommands::AST_PORT = atoi(value);
		if (strcmp(param,"ami_username") == 0)
			strcpy(asteriskCommands::AST_USERNAME, value);
		if (strcmp(param,"ami_password") == 0)
			strcpy(asteriskCommands::AST_PASSWORD, value);
		if (strcmp(param,"default_context") == 0)
		{
			//printf("AAA : %s \n",value);
			strcpy(asteriskCommands::AST_CONTEXT, value);
		}
		if (strcmp(param,"default_trunk") == 0) {
			//printf("BBB : %s %d  \n",value, atoi(value));
			strcpy(asteriskCommands::AST_TRUNK, value);
		}
		if (strcmp(param,"control_call_extension") == 0)
		{
			//printf("CCC : %s \n",value);
			strcpy(asteriskCommands::CONTROL_EXT, value);
		}

		if (strcmp(param,"receiver_server_port") == 0) 
			asteriskCommands::RECEIVER_PORT = atoi(value);
		if (strcmp(param,"receiver_server_ip") == 0) 
			strcpy(asteriskCommands::RECEIVER_IP, value);

		if (strcmp(param,"mysql_server") == 0) 
			strcpy(asteriskCommands::MYSQL_SERVER, value);
		if (strcmp(param,"mysql_user") == 0)
			strcpy(asteriskCommands::MYSQL_USER, value);
		if (strcmp(param,"mysql_pass") == 0)
			strcpy(asteriskCommands::MYSQL_PASSWORD, value);
		if (strcmp(param,"cdr_database") == 0)
			strcpy(asteriskCommands::CDR_DATABASE, value);
			
		string api_key = apiKeyControl();

		/*if (api_key != API_KEY)
		{
			cout << "\n.:: LISENCE KEY FAILED ::." <<endl;
			exit(1);
		}*/
	}
	fclose(file);
	return 1;
}

int asteriskCommands::connectToAsterisk()
{
	asteriskCommands astcommands;
	
	astcommands.readConfig();
	
	int sock;
	struct hostent *host;
	struct sockaddr_in server_addr;
	host = gethostbyname(astcommands.SERVER);
	
	while ((sock = socket(AF_INET, SOCK_STREAM, 0)) != -1)
	{
		cout << "CONNECTED : " << astcommands.AST_PORT << endl;
		
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(astcommands.AST_PORT);
		server_addr.sin_addr = *((struct in_addr *)host->h_addr);
		bzero(&(server_addr.sin_zero),8);
		
		if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) 
		{
			perror("CONNECTION ERROR");
			cout << "Could not conenct to Asterisk"<<endl;
			return -1;
		}
		else
		{
			close(sock);
			return sock;
		}
	}
}

string asteriskCommands::sendCommand(string command)
{
	asteriskCommands astcommands;

	int sock, loginBytesRecieved = 0;
	char loginResult[2048];
	char *sendCommandPtr;
	string result;
	stringstream readStream;
	
	struct hostent *host;
	struct sockaddr_in server_addr;

	int scannertoeven[2];

	if (command.size()>0)
	{
		astcommands.readConfig();
 
		host = gethostbyname(astcommands.SERVER);
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		{
			perror("Socket");
			cout << "Cannot open port"<<endl;
			return NULL;
		}
		else
		{
			int one = 1; 
			struct timeval timeout;      
			timeout.tv_sec = 10;
			timeout.tv_usec = 0;
			
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
			memset(&server_addr, '0', sizeof(server_addr));
			bzero((char *) &server_addr, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(astcommands.AST_PORT);
			server_addr.sin_addr = *((struct in_addr *)host->h_addr);
			
			if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
			{
				perror("CONNECTION ERROR");
				cout << "Could not conenct to Asterisk"<<endl;
				return NULL;
			}
			else
			{
				stringstream sendCommandStream;
				sendCommandStream << "Action: Login\r\nUserName: " << astcommands.AST_USERNAME << "\r\n"<<
									"Secret: "<< astcommands.AST_PASSWORD <<"\r\n" << "Event:Off\r\n\r\n" << 
									command << "\r\n\r\n"<< "Action: Logoff" <<"\r\n\r\n";
				string sendCommand = sendCommandStream.str();
				char *sendCommandPtr;
				sendCommandPtr = (char*)sendCommand.c_str();
				
				send(sock,sendCommandPtr,strlen(sendCommandPtr), 0);
				//usleep(500);
				string resultTemp;
				read(sock, loginResult, 1024);
				int ret=0;
				while ((ret = read(sock, loginResult, sizeof(loginResult)-1)) > 0) 
				{
					loginResult[ret] = 0x00;
					readStream << loginResult;
				}
				resultTemp = readStream.str(); //loginResult;//readStream.str();
				if (resultTemp.size()>0)
				{
					result = resultTemp;
				}
			}
			close(sock);
		}
	}
	return result;
}

vector<string> asteriskCommands::sipShowStatus(string channelName)
{
	asteriskCommands astcommands;

	vector<string> results;
	string uniqueResult;
	string commandResult;
	
	if (channelName.size() > 0)
	{
		string command = "Action: CoreShowChannels";
		commandResult = astcommands.sendCommand(command);
		//sleep(1);
		
		stringstream expressionStream;

		expressionStream << "Channel: SIP/" << channelName << "-(.*?)\r\n";

		string EXP = expressionStream.str();
		bool matches;
		boost::regex e(EXP);
		matches = boost::regex_search(commandResult.begin(), commandResult.end(), e);
		
		if(matches==true)
		{
			int subs[] = {1}; 
			boost::sregex_token_iterator i(commandResult.begin(), commandResult.end(), e, subs);
			boost::sregex_token_iterator j;
			string regexTextMonitor;
			vector<string> result;
			
			if (commandResult.size() > 0)
			{
				while(i != j) 
				{
					regexTextMonitor = *i++;
					boost::trim(regexTextMonitor);
					if (regexTextMonitor.size() > 0) {
						results.push_back(regexTextMonitor);
					}
				}
			}
		}
	}
	return results;
}

string asteriskCommands::readRegex(string sourceText, string EXP)
{
	bool matches_string;
	string result = "";
	
	boost::regex e(EXP);
	matches_string = boost::regex_search(sourceText.begin(), sourceText.end(), e);
	
	if(matches_string)
	{
		int subs[] = {1,2};
		string regexText;
		
		boost::sregex_token_iterator i(sourceText.begin(), sourceText.end(), e, subs);
		boost::sregex_token_iterator j;

		if (sourceText.size() > 0)
		{
			while(i != j) 
			{
				regexText = *i++;
				boost::trim(regexText);
				if (regexText.size() > 0) {
					result = regexText;
				}
			}
		}
	}
	return result;
}

vector<string> asteriskCommands::getChannelInfo(string channelName)
{
	asteriskCommands astcommands;
	vector<string> result;
	string command;
	string commandResult;
	
	if (channelName.size() > 0)
	{
		vector<string> sipStatusResult;
		string channelSipStatus;
		sipStatusResult = astcommands.sipShowStatus(channelName);
		
		if (!sipStatusResult.empty() and sipStatusResult.size() >= 0)
		{
			channelSipStatus = sipStatusResult[0];
		
			stringstream commandStream;
			commandStream << "Action: command\r\nCommand:core show channel SIP/"<<
							channelName << "-" << channelSipStatus << endl;;
			command = commandStream.str();
			commandResult = astcommands.sendCommand(command);
			
			
			string EXP_BRIDGE, EXP_MONITOR, EXP_UNIQUEID, EXP_CONNECTED_LINE;
			string EXP_STATE, EXP_DIALSTATUS, EXP_DIAL_NUMBER, EXP_DURATION;
			
			EXP_MONITOR = "MIXMONITOR_FILENAME=(.*?)\n";
			EXP_BRIDGE = "Direct Bridge: (.*?)\n";
			EXP_UNIQUEID = "UniqueID: (.*?)\n";
			EXP_CONNECTED_LINE = "Connected Line ID: (.*?)\n";
			EXP_STATE = "State: (.*?)\n";
			EXP_DURATION = "Elapsed Time: (.*?)\n";
			EXP_DIALSTATUS = "DIALSTATUS:(.*?)\n";
			EXP_DIAL_NUMBER = "DIAL_NUMBER:(.*?)\n";

			string reg_monitor = astcommands.readRegex(commandResult, EXP_MONITOR);
			string reg_bridge = astcommands.readRegex(commandResult, EXP_BRIDGE);
			string reg_uniqueid = astcommands.readRegex(commandResult, EXP_UNIQUEID);
			string reg_connected_line = astcommands.readRegex(commandResult, EXP_CONNECTED_LINE);
			string reg_state = astcommands.readRegex(commandResult, EXP_STATE);
			string reg_duration = astcommands.readRegex(commandResult, EXP_DURATION);
			string reg_dialstatus = astcommands.readRegex(commandResult, EXP_DIALSTATUS);
			string reg_dial_number = astcommands.readRegex(commandResult, EXP_DIAL_NUMBER);
			
			result.push_back(channelName);
			result.push_back(channelSipStatus);
			result.push_back(reg_uniqueid);
			result.push_back(reg_connected_line);
			result.push_back(reg_state);
			result.push_back(reg_connected_line);
			result.push_back(reg_duration);
			result.push_back(reg_monitor);
			result.push_back(reg_dialstatus);
			
			stringstream ss;
			ss << FDID;
			string FDID_STR = ss.str();
			result.push_back(FDID_STR);
		}
		else
		{
		}			
	}
	return result;
}


vector<string> asteriskCommands::originate(string channelName, string extension)
{
	asteriskCommands astcommands;
	astcommands.readConfig();
	
	vector<string> originateResult;
	vector<string> sipInfos;
	vector<string> result;
	stringstream commandStream;
	
	
	if (channelName.size() > 0 && extension.size() > 0)
	{
		//sleep(2);
		
		//sipInfos = astcommands.getChannelInfo(channelName);
		
		//if (sipInfos.empty())
		//{
			commandStream << "Action: Originate\r\nChannel: SIP/"  << channelName <<
							"\r\nContext: "<< astcommands.AST_CONTEXT <<"\r\nExten: "<< extension <<
							"\r\nPriority: 1\r\nCallerID: "<< channelName;

			string command = commandStream.str();
			astcommands.sendCommand(command);
			sleep(1);
			originateResult = astcommands.getChannelInfo(channelName);

			if (!originateResult.empty())
				result = originateResult;
		//}
	}
	return result;
}

vector<string> asteriskCommands::controlCall(string extension)
{
	asteriskCommands astcommands;
	astcommands.readConfig();
	
	vector<string> originateResult;
	vector<string> sipInfos;
	vector<string> result;
	stringstream commandStream;
	string results;
	
	if (sipInfos.empty())
	{
		extension.erase(std::remove(extension.begin(), extension.end(), '\n'), extension.end());
		
		commandStream << "Action: Originate\r\nChannel: Local/"  << astcommands.CONTROL_EXT << 
						 "@" << astcommands.AST_CONTEXT << "\r\n" << 
						 "Context: " << astcommands.AST_CONTEXT << "\r\n" << 
						 "Exten: "<< extension << "\r\n"<<
						 "Priority: 1\r\n" << 
						 "Variable: phone="<< extension <<"\r\n" <<
						 "Variable: control_trunk="<< astcommands.AST_TRUNK <<"\r\n" <<
						 "\r\nTimeout:30000";

		string command = commandStream.str();
		results = astcommands.sendCommand(command);
		
		 
		if (!originateResult.empty())
			result = originateResult;
	}
	return result;
	
}

int asteriskCommands::controlCallCheck(void)
{
	asteriskCommands astcommands;
	astcommands.readConfig();
	
	int result = 0;
	string commandResult;

	string channelsCommand = "Action: Command\r\nCommand: local show channels";
	commandResult = astcommands.sendCommand(channelsCommand);
	
	stringstream expressionStream;
	expressionStream << "Local/"<< astcommands.CONTROL_EXT << "(.*?)";
	string EXP = expressionStream.str();
	bool matches;
	boost::regex e(EXP);
	matches = boost::regex_search(commandResult.begin(), commandResult.end(), e);
	if (matches == true)
	{
		//int subs[] = {0}; 
		boost::sregex_token_iterator i(commandResult.begin(), commandResult.end(), e);
		boost::sregex_token_iterator j;
		int callCount = 0;
		if (commandResult.size() > 0) 
		{
			while(i != j)
			{
				*i++;
				callCount++;
			}
			result = callCount;
		}
	}
	return result;
}

vector<string> asteriskCommands::hangupCall(string channelToHangup)
{
	asteriskCommands astcommands;
	
	vector<string> sipInfos;
	vector<string> channelInfos;
	string currentChannel;
	
	if (channelToHangup.size() > 0)
	{
		sipInfos = astcommands.getChannelInfo(channelToHangup);
		
		if (!sipInfos.empty())
		{
			currentChannel = sipInfos[1];
			char cstr[currentChannel.size()+1];
			for (int i =0; i<=currentChannel.size();i++) 
			{
				cstr[i] = currentChannel[i];
			}
			
			if (!currentChannel.empty())
			{
				stringstream commandHangupStream;
				stringstream channelInfoStream;
				
				channelInfoStream << channelToHangup << "-" << cstr;
				channelInfos = astcommands.getChannelInfo(channelInfoStream.str());
				
				//if (!channelInfos.empty() && channelInfos[1] != "<none>") or (channelInfos[1])
				//((!channelInfos.empty() && channelInfos[1] != "<none>") or (channelInfos[1] != "<none>") )
				if (sipInfos[1] != "<none>" and sipInfos[1] != "")
				{
					commandHangupStream << "Action: Hangup\r\nChannel: SIP/"<< sipInfos[0] << "-" << sipInfos[1];
					astcommands.sendCommand(commandHangupStream.str());
				}
			}
		}
	}
	return sipInfos;
}

char *asteriskCommands::redirectIncomingCall(char *channel, char *target) {}
char *asteriskCommands::showParkedCalls() {}
char *asteriskCommands::parkCall(char *channelName) {}
char *asteriskCommands::showQueueCalls() {}
vector<string> asteriskCommands::checkInboundCalls() {}

/*
int main()
{
	asteriskCommands astcommands;
	vector<string> test;
	string test2;
	int deneme;
	string channel = "1001"; 
	string exten = "054245454";
	
	//string exten = "90565656556";
	//string exten = "10000";
	//string exten = "0545454545";
	
	//string comm = "Action: CoreShowChannels";
	//int test4 = astcommands.connectToAsterisk();
	//printf("%d",test4);

	//test = astcommands.originate(channel, exten);
	 //= astcommands.sipShowStatus(channel);
	//astcommands.controlCall(exten);
	//test2 = astcommands.sendCommand(comm);
	
	//cout << test2 <<endl;
	
	//string deneme;
	//astcommands.readConfig();
	//test = astcommands.getChannelInfo(channel);
	
	//astcommands.controlCall(exten);
	//deneme = astcommands.controlCallCheck();
	//test = astcommands.originate(channel, exten);
	//test = astcommands.hangupCall(channel);
	//test = astcommands.sipShowStatus(channel);
	//printf("gonderdi\n");
	
	//cout << test << endl;
	
	//test = astcommands.originate(channel, exten);
	
	//cout << test << endl;
	//astcommands.hangupCall(channel);
	
	//char *channels = "Action: command\r\nCommand: core show channels\r\n\r\n";
	//test2 = astcommands.sendCommand(channels);
	//cout << "KOD : \r\n" << test2 << endl;
	//test2 = astcommands.sendCommand(channels);
	/*vector<string> sipInfos;
	test = astcommands.getChannelInfo(channel);
	
	if (!test.empty() and test.size() > 1)
	{
		for (int i=0;i<=test.size()-1; i++)
		{
			cout << test[i] << endl;
		}
	}
	astcommands.connectToAsterisk();
	
	return 0;
	
}
*/