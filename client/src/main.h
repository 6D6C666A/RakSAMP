#include <stdio.h>
#include <stdlib.h>
#include <conio.h> // need sum coños and vaginas
#include <windows.h>
#include "../../common/common.h"

// window stuff
#include <commctrl.h>
#include "resource.h"

// raknet stuff
#include "PacketEnumerations.h"
#include "RakNetworkFactory.h"
#include "RakClientInterface.h"
#include "NetworkTypes.h"
#include "BitStream.h"
#include "StringCompressor.h"

enum eRunModes
{
	RUNMODE_BARE,
	RUNMODE_NORMAL,
	RUNMODE_FOLLOWPLAYER,
	RUNMODE_FOLLOWPLAYERSVEHICLE,
	RUNMODE_PLAYROUTES,
};

#pragma warning(disable:4996)

#include "samp_netencr.h"
#include "SAMPRPC.h"

#include "netgame.h"
#include "netrpc.h"
#include "localplayer.h"
#include "misc_funcs.h"
#include "CVector.h"
#include "math_stuff.h"

#include "../../tinyxml/tinyxml.h"
#include "sqlite3/sqlite3.h"
#include "console.h"
#include "window.h"
#include "vehplayer.h"
#include "cmds.h"
#include "xmlsets.h"

struct stVehiclePool
{
	int iDoesExist;
	float fPos[3];
	int iModelID;
};

extern RakClientInterface *pRakClient;
extern int iAreWeConnected, iConnectionRequested, iSpawned, iGameInited;
extern int iReconnectTime;
extern PLAYERID g_myPlayerID;
extern char g_szNickName[32];

extern struct stPlayerInfo playerInfo[MAX_PLAYERS];
extern struct stVehiclePool vehiclePool[MAX_VEHICLES];



extern PLAYERID normalMode_goto;
extern PLAYERID imitateID;

void Log ( char *fmt, ... );
void gen_random(char *s, const int len);
