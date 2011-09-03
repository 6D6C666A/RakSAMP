struct stPlayerPool
{
	int iIsConnected;
	PlayerID rakPlayerID;
	PLAYERID playerID;
	char szPlayerName[32];
	char szIP[17];
	unsigned short usClientPort;
	int iPlayerScore;
	DWORD dwPlayerPing;

	// spawn info
	BYTE bTeam;
	int iSkin;
	float fSpawnPos[3];
	float fRotation;
	DWORD dwColor;
	BYTE fightingStyle;
};

extern PLAYERID playerCount;
extern struct stPlayerPool playerPool[MAX_PLAYERS];

void addPlayerToPool(PlayerID rakPlayerID, PLAYERID playerID, char *szNick);
void removePlayerFromPool(PLAYERID playerID, int iTimeout);
int isPlayerConnected(PLAYERID playerID);
