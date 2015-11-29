#include "astcmd.h"
#include "astcmd.h"
#include "xmlparse.h"
#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string>
#include <map>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include <pthread.h>

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


int clientSock = -1;
string clientAddress  = "";
int clientPort = 0;
struct sockaddr_in clientServer;	


char * killProcess()
{
	FILE *inpipe;
    char inbuf[1000];
    int lineno = 0;
	
    char *command = "netstat -tlnap | awk '/:5090 */ {split($NF,a,\"/\"); print a[1]}' 2>/dev/null";
    inpipe = popen(command, "r");
    
    if (inpipe)
    {
		while (fgets(inbuf, sizeof(inbuf), inpipe)) 
		{
			lineno++;
		}
		pclose(inpipe);
	}
	if (lineno < 1)
		inbuf[0] = '\0';
		
	return inbuf;
}


string controlCall(string maxChannel, string callType, string callSource)
{
	asteriskCommands asterisk;
	asterisk.readConfig();
	
	MYSQL *conn, mysql;
	MYSQL_RES *result;
	MYSQL_ROW row;
	mysql_init(&mysql);

	string results = "";
	vector<string> originateResult;
	vector<string> numbers;
	
	if (callType == "file")
	{
		FILE *file;
		char line[256];
		int linenum=0;

		file = fopen(callSource.c_str(),"r");
		
		while(fgets(line, 4096, file) != NULL) 
		{
			//char lineValue[50];
			//sscanf(line, "%63[^;];", lineValue);
			numbers.push_back(line);
		}
		
		if (numbers.size() > 0)
		{
			conn = mysql_real_connect(&mysql, asterisk.MYSQL_SERVER, asterisk.MYSQL_USER,asterisk.MYSQL_PASSWORD,asterisk.CDR_DATABASE,0,NULL,0);
			if(conn == NULL) {
				//cout << "Database connection error" << endl;
				return 0;
			}
			else
			{
				for (int i=0; i<=numbers.size()-1;)
				{
					ostringstream os;
					string insertQuery;
					
					int currentWeight = asterisk.controlCallCheck();
					if (currentWeight < atoi(maxChannel.c_str()))
					{
						asterisk.controlCall(numbers[i]);
						
						os << "insert into control_calls (id , Source, StartDateTime) " <<
							"VALUES('" << i << "', '" << numbers[i] << "', NOW()" << ");";
						insertQuery = os.str();
						mysql_query(conn, insertQuery.c_str());
						i++;
					}
				}
			}
			mysql_close(conn);
		}
	}
	else if (callType == "mysqldb")
	{
	}
	
	return results; 
}


int main( int argc, char *argv[] )
{

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

	XMLParse xmlparse;
	asteriskCommands asterisk;
	asterisk.readConfig();
	
	FILE *fp= NULL;
	fp = fopen ("/var/log/pbxml.log", "a+");
	
	char killCommand[20];
	cout << "\nAsterisk Service \n" << endl;
	
	if (argc < 2)
	{
        printf("ERROR : Invalid parameter. \nUsage : %s start|stop|restart \n", argv[0]);
        exit(1);
	}
	
	/* Call to check if phone number exists */
	else if (strcmp(argv[1], "control") == 0)
	{
		bool error = false;
		char paramChannel[25], valueChannel[10];
		char paramSource[25], valueSource[100];
		char maxChannel[10], callType[10], callSource[100];
		
		if (!argv[2]) 
		{
			int callCount = asterisk.controlCallCheck();
			printf("Active controll call count : %d\n",callCount);
			printf("Recommended max. call count : %d\n", 40-callCount);
		}
		else 
		{
			if (argv[2] and sscanf(argv[2], "--%254[^=]=%s", &paramChannel, &valueChannel)==2) 
			{
				if (strcmp(paramChannel,"max-channels") == 0) 
					strcpy(maxChannel, valueChannel);
				else
					exit(1);
			}
			
			if (argv[3] and sscanf(argv[3], "--%254[^=]=%s", &paramSource, &valueSource)==2) 
			{
				if (strcmp(paramSource,"from-file") == 0) 
				{
					strcpy(callType, "file");
					strcpy(callSource, valueSource);
				}
				else if (strcmp(paramSource,"from-mysqldb") == 0) 
				{
					strcpy(callType, "mysqldb");
					strcpy(callSource, valueSource);
				}
				else
					exit(1);
			}
			if (argv[4] and strcmp(argv[4], "--start") == 0) 
			{
				printf("baslat");
			}
			if (argv[4] and strcmp(argv[4], "--stop") == 0) 
			{
				printf("durdur");
			}
			
			if (strlen(maxChannel) > 0 && strlen(callType) > 0 && strlen(callSource) > 0)
			{
				string maxChannelstr = maxChannel;
				string callTypestr = callType;
				string callSourcestr = callSource;
				controlCall(maxChannelstr, callTypestr, callSourcestr);
			}
		}
	}
	
	else if (strcmp(argv[1], "stop") == 0)
	{
		char *statusProcess = killProcess();
		if (strlen(statusProcess) > 1)
		{
			sprintf(killCommand,"kill %s",statusProcess);
			//sprintf(killCommand,"killall pbxml");
			system(killCommand);
			//system("killall pbxml");
			cout << "Service stoping " << setw(w.ws_col/2) << "[\033[32mOK\033[0m]" << endl;
			//cout <<  << endl;
		}
		else
		{
			cout << "Service stoping" << setw(w.ws_col/2) << "[\033[31mERROR\033[0m]" << endl;
			cout << "No running process." << endl;
		}
		fprintf(fp, "Service Stopped\n");
		exit(1);
	}
	
    else if (strcmp(argv[1], "restart")==0 or strcmp(argv[1], "start")==0 )
    {
		bool softRestarted = false;
		
			//clientSock = -1;
			string clientAddress  = "";
			clientPort = 0;
		
			char *statusProcess = killProcess();
		
			if (strlen(statusProcess) > 1)
			{
				sprintf(killCommand,"kill %s",statusProcess);
				//sprintf(killCommand,"killall pbxml");
				system(killCommand);
				//system("killall pbxml");
				if (softRestarted == false)
					cout << "Terminating previous sessions" << setw(w.ws_col/2) << "[\033[32mOK\033[0m]" << endl;
			}
			
			pid_t process_id = 0;
			int main_process_id = 0;
			pid_t sid = 0;

			process_id = fork();
			//main_process_id = getpid();
			
			if (process_id < 0)
			{
				fprintf(fp, "System fork error\n");
				printf("System fork error!\n");
				exit(1);
			}
			if (process_id > 0)
			{
				if (softRestarted == false)
					cout << "Service starting\t\t" << setw(w.ws_col/2) << "[\033[32mOK\033[0m]" << endl;
				exit(0);
			}
			umask(0);
			sid = setsid();
			if(sid < 0)
			{
				exit(1);
			}			
			chdir("/");
			fprintf(fp, "Service started\n");
			
			struct timeval timeout;
			struct timeval timeout2;
			timeout.tv_sec = 3;
			timeout2.tv_sec = 5;
			timeout.tv_usec = 0;
			
			int serverSock 	= socket(AF_INET, SOCK_STREAM, 0);
			int one = 1;
			
			//setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
			//setsockopt(serverSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
			//setsockopt(serverSock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout2, sizeof(timeout2));
			
			int clientPort 		= asterisk.RECEIVER_PORT;
			string clientServer = asterisk.RECEIVER_IP;
		
			sockaddr_in serverAddr;
			
			bzero((char *) &serverAddr, sizeof(serverAddr));
			
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(asterisk.SERVICE_PORT);
			serverAddr.sin_addr.s_addr = INADDR_ANY;

			bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr));
			while (1) 
			{
				fprintf(fp,"ttt\n");
				clientSock = -1;
				//usleep(100);
				char receivedStr[1024];

				listen(serverSock, 10);
				
				sockaddr_in clientAddr;
				socklen_t sin_size = sizeof(struct sockaddr_in);
				clientSock = accept(serverSock,(struct sockaddr*)&clientAddr, &sin_size);
				
				bzero(receivedStr,1024);
				recv(clientSock, receivedStr, 1024, 0);

				string received = receivedStr;
				string resultToSend;
				
				if (received.size() > 0 )
				{
					resultToSend = xmlparse.parse(received);
					fprintf(fp,"-----");
					fprintf(fp,receivedStr);
					fprintf(fp,"\n => \n");
					fprintf(fp,resultToSend.c_str());
					fprintf(fp,"-----");
					if (resultToSend.size() > 0)
					{
						write(clientSock, resultToSend.c_str(), resultToSend.size());
					}
					else
					{
						perror("could not send data\n");
						fprintf(fp,"could not send data\n");
					}
				}
				else
				{
					fprintf(fp, "No data to retrieve\n");
				}
				close(clientSock);
			}
			
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
	}
	else
	{
        printf("ERROR : Invalid paramter. \nUsage : %s start|stop|restart \n", argv[0]);
        exit(1);
	}

	fclose(fp);
	return 0;
}
