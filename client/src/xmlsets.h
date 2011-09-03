#define MAX_AUTORUN_CMDS 32
#define MAX_FIND_ITEMS	 128

struct stServer
{
	char szAddr[256];
	int iPort;
	char szNickname[20];
	char szPassword[32];
};

struct stAutoRunCMD
{
	int iExists;
	char szCMD[512];
};

struct stFindItem
{
	int iExists;
	char szFind[512];
	char szSay[512];
	unsigned char bBkRed;
	unsigned char bBkGreen;
	unsigned char bBkBlue;
	unsigned char bTextRed;
	unsigned char bTextGreen;
	unsigned char bTextBlue;
};

struct stSettings
{
	struct stServer server;
	int iConsole;
	eRunModes runMode;
	int iAutorun;
	int iFind;
	int iClassID;
	int iPrintTimestamps;

	unsigned char bChatColorRed;
	unsigned char bChatColorGreen;
	unsigned char bChatColorBlue;

	unsigned char bCMsgRed;
	unsigned char bCMsgGreen;
	unsigned char bCMsgBlue;

	char szFollowingPlayerName[20];
	int iFollowingWithVehicleID;
	float fFollowDistance;
	float fFollowZOffset;
	char szImitateChatPlayerName[20];

	float fNormalModePos[3];
	float fNormalModeRot;
	float fPlayPos[3];
	float fPlayRot;

	struct stAutoRunCMD autoRunCMDs[MAX_AUTORUN_CMDS];
	struct stFindItem findItems[MAX_FIND_ITEMS];
};

extern struct stSettings settings;

int LoadSettings();
int UnLoadSettings();
int ReloadSettings();
