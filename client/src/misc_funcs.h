void onFootUpdateAtNormalPos();
void onFootUpdateFollow(PLAYERID followID);
void inCarUpdateFollow(PLAYERID followID, VEHICLEID withVehicleID);

int sampConnect(char *szHostname, int iPort, char *szNickname, char *szPassword);
void sampDisconnect(int iTimeout);
void sampRequestClass(int iClass);
void sampSpawn();
void sendServerCommand(char *szCommand);
void sendChat(char *szMessage);
void SendScmEvent(int iEventType, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
void sendDialogResponse(WORD wDialogID, BYTE bButtonID, WORD wListBoxItem, char *szInputResp);

int isPlayerConnected(PLAYERID iPlayerID);
int getPlayerID(char *szPlayerName);
char *getPlayerName(PLAYERID iPlayerID);
int getPlayerPos(PLAYERID iPlayerID, float *fPos);

PLAYERID getPlayerIDFromPlayerName(char *szName);
unsigned short getPlayerCount();

const struct vehicle_entry *gta_vehicle_get_by_id ( int id );
