# pbxml
An XML based Asterisk API Service written in C++  

You needn't to know Asterisk commands, API commands, dialplans etc. Any development platform (.Net, Php, Python, Delphi and many more)
which supports tcp/ip socket connections, can be integrated to Asterisk just using XML command. 

You can originate, hangup a call, redirect, conference etc. with a simple xml format and receive action result. Get Channel, SIP informations, system status,
voice record path and more... 

As you below seen, you send only an XML (with any programming language) to Asterisk Server 5095 port and receive the answer. 

```
<?xml version='1.0' standalone='yes'?>
<PBX>
	<Call>
		<Id></Id>
		<Source>1000</Source>
		<Dest>12365</Dest>
		<ChannelName></ChannelName>
		<CallType>11</CallType>
		<StartDate></StartDate>
		<StartTime></StartTime>
		<RecordPath></RecordPath>
		<EndDate></EndDate>
		<EndTime></EndTime>
		<Status></Status>
	</Call>
	<Redirect>
		<ChannelName></ChannelName>
		<Source></Source>
		<Dest></Dest>
	</Redirect>
	<Conference>
		<ChannelName></ChannelName>
		<Host></Host>
		<Invited></Invited>
	</Conference>
	<CustomDatas>
		<Name></Name>
		<Value></Value>
	</CustomDatas>
</PBX>
```

I will upload the files as soon as possible and thanks in advance for your supports
