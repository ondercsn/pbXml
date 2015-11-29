#ifndef XMLPARSE_H
#define XMLPARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <time.h>
#include <mysql/mysql.h>

using namespace std;

class XMLParse 
{
	public:
		string parse(string XMLData);
		string operateXml(map<string,string> XMLarray);
		string createXML(map<string,string> XMLarray);
		int opDB(map<string,string> XMLarray);	
};

#endif
