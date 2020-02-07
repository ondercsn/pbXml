# pbxml
An XML based Asterisk API Service written in C++  

You don't need to struggle with spesific Asterisk commands, API commands. Your application can be integrated to Asterisk just using XML directives.

It's prety easy to initiate, hangup or transfer the call, set conference, get current channels, SIP informations, system status,
voice record path and more... 

Following XML template is being sent to predefined port of server and received the response. 

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

THIS COMMIT NEED TO BE UPDATE4D AND EDITED. INTALL NOTES ARE NOT UPLOADED YET
