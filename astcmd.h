#ifndef ASTCMD_H
#define ASTCMD_H

#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <arpa/inet.h>

using namespace std;

class asteriskCommands 
{
	public:
		char AST_USERNAME[50], AST_PASSWORD[50], SERVER[20], AST_CONTEXT[100], API_KEY[150], CONTROL_EXT[20], AST_TRUNK[50];
		char MYSQL_SERVER[20], MYSQL_USER[50], MYSQL_PASSWORD[50], CDR_DATABASE[50];
		char RECEIVER_IP[20];
		int SERVICE_PORT, AST_PORT, RECEIVER_PORT;
		
		int readConfig();
		int connectToAsterisk();
		string sendCommand(string command);
		vector<string> sipShowStatus(string channelName);
		vector<string> getChannelInfo(string channelName);
		string readRegex(string sourceText, string EXP);
		vector<string> originate(string channelName, string extension);
		vector<string> checkInboundCalls();
		vector<string> hangupCall(string channelName);

		int controlCallCheck(void);
		vector<string> controlCall(string extension);
		
		char *showQueueCalls();
		char *redirectIncomingCall(char *channelName, char *target);
		char *showParkedCalls();
		char *parkCall(char *extension);

		vector<string> dialToControl();
};

#endif
