#include "main.h"

struct stSettings settings;
TiXmlDocument xmlSettings;

int LoadSettings()
{
	// load xml
	if(!xmlSettings.LoadFile("RakSAMPClient.xml"))
	{
		MessageBox(NULL, "Failed to load the config file", "Error", MB_ICONERROR);
		ExitProcess(0);
	}

	TiXmlElement* rakSAMPElement = xmlSettings.FirstChildElement("RakSAMPClient");
	if(rakSAMPElement)
	{
		// get console
		rakSAMPElement->QueryIntAttribute("console", (int *)&settings.iConsole);

		// get runmode
		rakSAMPElement->QueryIntAttribute("runmode", (int *)&settings.runMode);

		// get autorun
		rakSAMPElement->QueryIntAttribute("autorun", (int *)&settings.iAutorun);

		// get find
		rakSAMPElement->QueryIntAttribute("find", (int *)&settings.iFind);

		// get selected class id
		rakSAMPElement->QueryIntAttribute("select_classid", (int *)&settings.iClassID);

		// get print_timestamps
		rakSAMPElement->QueryIntAttribute("print_timestamps", (int *)&settings.iPrintTimestamps);

		// get chat color
		rakSAMPElement->QueryColorAttribute("chatcolor_rgb",
			(unsigned char *)&settings.bChatColorRed, (unsigned char *)&settings.bChatColorGreen, (unsigned char *)&settings.bChatColorBlue);

		// get client message color
		rakSAMPElement->QueryColorAttribute("clientmsg_rgb",
			(unsigned char *)&settings.bCMsgRed, (unsigned char *)&settings.bCMsgGreen, (unsigned char *)&settings.bCMsgBlue);

		// get followplayer
		strcpy(settings.szFollowingPlayerName, (char *)rakSAMPElement->Attribute("followplayer"));
		rakSAMPElement->QueryIntAttribute("followplayerwithvehicleid", &settings.iFollowingWithVehicleID);
		rakSAMPElement->QueryFloatAttribute("followDistance", &settings.fFollowDistance);
		rakSAMPElement->QueryFloatAttribute("followZOffset", &settings.fFollowZOffset);

		// imitate chat
		strcpy(settings.szImitateChatPlayerName, (char *)rakSAMPElement->Attribute("imitatechat"));
		imitateID = getPlayerIDFromPlayerName(settings.szImitateChatPlayerName);

		// get the first server
		TiXmlElement* serverElement = rakSAMPElement->FirstChildElement("server");
		if(serverElement)
		{
			char *pszAddr = (char *)serverElement->GetText();
			if(pszAddr)
			{
				int iPort;
				char *pszAddrBak = pszAddr;

				while(*pszAddrBak)
				{
					if(*pszAddrBak == ':')
					{
						*pszAddrBak = 0;
						pszAddrBak++;
						iPort = atoi(pszAddrBak);
					}
					pszAddrBak++;
				}

				strcpy(settings.server.szAddr, pszAddr);
				settings.server.iPort = iPort;
				strcpy(settings.server.szNickname, (char *)serverElement->Attribute("nickname"));
				strcpy(settings.server.szPassword, (char *)serverElement->Attribute("password"));
			}
		}

		// get normal mode pos
		TiXmlElement* normalPosElement = rakSAMPElement->FirstChildElement("normal_pos");
		if(normalPosElement)
		{
			normalPosElement->QueryVectorAttribute("position", (float *)&settings.fNormalModePos);
			normalPosElement->QueryFloatAttribute("rotation", &settings.fNormalModeRot);
		}

		// get playing mode pos
		TiXmlElement* playPosElement = rakSAMPElement->FirstChildElement("play_pos");
		if(playPosElement)
		{
			playPosElement->QueryVectorAttribute("position", (float *)&settings.fPlayPos);
			playPosElement->QueryFloatAttribute("rotation", &settings.fPlayRot);
		}

		// get auto run commands
		TiXmlElement* autorunElement = rakSAMPElement->FirstChildElement("autorun");
		if(autorunElement)
		{
			for(int i = 0; i < MAX_AUTORUN_CMDS; i++)
			{
				if(autorunElement)
				{
					settings.autoRunCMDs[i].iExists = 1;
					strcpy(settings.autoRunCMDs[i].szCMD, autorunElement->GetText());
					autorunElement = autorunElement->NextSiblingElement("autorun");
				}
				else
					break;
			}
		}

		TiXmlElement* findElement = rakSAMPElement->FirstChildElement("find");
		if(findElement)
		{
			for(int i = 0; i < MAX_FIND_ITEMS; i++)
			{
				if(findElement)
				{
					settings.findItems[i].iExists = 1;
					strcpy(settings.findItems[i].szFind, findElement->Attribute("text"));
					strcpy(settings.findItems[i].szSay, findElement->Attribute("say"));
					findElement->QueryColorAttribute("bk_color",
						(unsigned char *)&settings.findItems[i].bBkRed,
						(unsigned char *)&settings.findItems[i].bBkGreen,
						(unsigned char *)&settings.findItems[i].bBkBlue);
					findElement->QueryColorAttribute("text_color",
						(unsigned char *)&settings.findItems[i].bTextRed,
						(unsigned char *)&settings.findItems[i].bTextGreen,
						(unsigned char *)&settings.findItems[i].bTextBlue);

					findElement = findElement->NextSiblingElement("find");
				}
				else
					break;
			}
		}
	}

	xmlSettings.Clear();

	return 1;
}

int UnLoadSettings()
{
	memset(&settings, 0, sizeof(settings));

	return 1;
}

int ReloadSettings()
{
	if(UnLoadSettings() && LoadSettings())
	{
		Log("Settings reloaded");
		return 1;
	}

	Log("Failed to reload settings");

	return 0;
}