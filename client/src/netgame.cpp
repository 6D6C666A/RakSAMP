#include "main.h"

#define NETGAME_VERSION 8830
#define AUTH_BS "5E1C1837D2C54B24EAAED18C3F96D9EA51A2A481003"

DWORD dwTimeReconnect = 10000;

int iPassengerNotificationSent = 0, iDriverNotificationSent = 0;
extern int iFollowingPassenger, iFollowingDriver;

void Packet_ConnectionSucceeded(Packet *p)
{
	RakNet::BitStream bsSuccAuth((unsigned char *)p->data, p->length, false);
	PLAYERID myPlayerID;
	unsigned int uiChallenge;

	bsSuccAuth.IgnoreBits(8); // ID_CONNECTION_REQUEST_ACCEPTED
	bsSuccAuth.IgnoreBits(32); // binaryAddress
	bsSuccAuth.IgnoreBits(16); // port

	bsSuccAuth.Read(myPlayerID);
	g_myPlayerID = myPlayerID;
	playerInfo[myPlayerID].iIsConnected = 1;
	strcpy(playerInfo[myPlayerID].szPlayerName, g_szNickName);

	bsSuccAuth.Read(uiChallenge);

	Log("Connected. Joining the game...");
	
	char *pszAuthBullshit = AUTH_BS;
	int iVersion = NETGAME_VERSION;
	BYTE byteMod = 1;
	BYTE byteNameLen = (BYTE)strlen(g_szNickName);
	BYTE byteAuthBSLen = (BYTE)strlen(pszAuthBullshit);
	unsigned int uiClientChallengeResponse = uiChallenge ^ iVersion;
	
	RakNet::BitStream bsSend;
	bsSend.Write(iVersion);
	bsSend.Write(byteMod);
	bsSend.Write(byteNameLen);
	bsSend.Write(g_szNickName, byteNameLen);
	bsSend.Write(uiClientChallengeResponse);
	bsSend.Write(byteAuthBSLen);
	bsSend.Write(pszAuthBullshit, byteAuthBSLen);
	pRakClient->RPC(&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	iAreWeConnected = 1;
}

void Packet_PlayerSync(Packet *p)
{
	RakNet::BitStream bsPlayerSync((unsigned char *)p->data, p->length, false);
	PLAYERID playerId;

	//Log("Packet_PlayerSync: %d \n%s\n", p->length, DumpMem((unsigned char *)p->data, p->length));

	bool bHasLR, bHasUD;
	bool bHasSurfInfo, bAnimation;

	bsPlayerSync.IgnoreBits(8);
	bsPlayerSync.Read(playerId);

	// Followed passenger exit
	if(settings.runMode == RUNMODE_FOLLOWPLAYER && playerId == getPlayerIDFromPlayerName(settings.szFollowingPlayerName))
	{
		if(iPassengerNotificationSent)
		{
			SendExitVehicleNotification(playerInfo[playerId].incarData.VehicleID);
			iPassengerNotificationSent = 0;
		}

		iFollowingPassenger = 0;

		if(iDriverNotificationSent)
		{
			SendExitVehicleNotification(playerInfo[playerId].incarData.VehicleID);
			iDriverNotificationSent = 0;
		}

		iFollowingDriver = 0;
	}
	playerInfo[playerId].incarData.VehicleID = -1;


	// clear last data
	memset(&playerInfo[playerId].onfootData, 0, sizeof(ONFOOT_SYNC_DATA));

	// LEFT/RIGHT KEYS
	bsPlayerSync.Read(bHasLR);
	if(bHasLR) bsPlayerSync.Read(playerInfo[playerId].onfootData.lrAnalog);

	// UP/DOWN KEYS
	bsPlayerSync.Read(bHasUD);
	if(bHasUD) bsPlayerSync.Read(playerInfo[playerId].onfootData.udAnalog);

	// GENERAL KEYS
	bsPlayerSync.Read(playerInfo[playerId].onfootData.wKeys);

	// VECTOR POS
	bsPlayerSync.Read(playerInfo[playerId].onfootData.vecPos[0]);
	bsPlayerSync.Read(playerInfo[playerId].onfootData.vecPos[1]);
	bsPlayerSync.Read(playerInfo[playerId].onfootData.vecPos[2]);

	// ROTATION
	bsPlayerSync.ReadNormQuat(
		playerInfo[playerId].onfootData.fQuaternion[0],
		playerInfo[playerId].onfootData.fQuaternion[1],
		playerInfo[playerId].onfootData.fQuaternion[2],
		playerInfo[playerId].onfootData.fQuaternion[3]);
	

	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	BYTE byteHealthArmour;
	BYTE byteHealth, byteArmour;
	BYTE byteArmTemp=0,byteHlTemp=0;

	bsPlayerSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) byteArmour = 100;
	else if(byteArmTemp == 0) byteArmour = 0;
	else byteArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) byteHealth = 100;
	else if(byteHlTemp == 0) byteHealth = 0;
	else byteHealth = byteHlTemp * 7;

	playerInfo[playerId].onfootData.byteHealth = byteHealth;
	playerInfo[playerId].onfootData.byteArmour = byteArmour;

	// CURRENT WEAPON
	bsPlayerSync.Read(playerInfo[playerId].onfootData.byteCurrentWeapon);

	// Special Action
	bsPlayerSync.Read(playerInfo[playerId].onfootData.byteSpecialAction);

	// READ MOVESPEED VECTORS
	bsPlayerSync.ReadVector(
		playerInfo[playerId].onfootData.vecMoveSpeed[0],
		playerInfo[playerId].onfootData.vecMoveSpeed[1],
		playerInfo[playerId].onfootData.vecMoveSpeed[2]);

	bsPlayerSync.Read(bHasSurfInfo);
	if(bHasSurfInfo)
	{
		bsPlayerSync.Read(playerInfo[playerId].onfootData.wSurfInfo);
		bsPlayerSync.Read(playerInfo[playerId].onfootData.vecSurfOffsets[0]);
		bsPlayerSync.Read(playerInfo[playerId].onfootData.vecSurfOffsets[1]);
		bsPlayerSync.Read(playerInfo[playerId].onfootData.vecSurfOffsets[2]);
	}
	else
		playerInfo[playerId].onfootData.wSurfInfo = -1;

	bsPlayerSync.Read(bAnimation);
	if(bAnimation)
		bsPlayerSync.Read(playerInfo[playerId].onfootData.iCurrentAnimationID);
}

//----------------------------------------------------

void Packet_UnoccupiedSync(Packet *p)
{
	RakNet::BitStream bsUnocSync((unsigned char *)p->data, p->length, false);
	PLAYERID playerId;

	//Log("\n%s\n", DumpMem((unsigned char *)p->data + bsUnocSync.GetReadOffset() / 8, p->length));

	bsUnocSync.IgnoreBits(8);
	bsUnocSync.Read(playerId);

	memset(&playerInfo[playerId].unocData, 0, sizeof(UNOCCUPIED_SYNC_DATA));

	bsUnocSync.Read((char *)&playerInfo[playerId].unocData, sizeof(UNOCCUPIED_SYNC_DATA));
}

void Packet_AimSync(Packet *p)
{  
	RakNet::BitStream bsAimSync((unsigned char *)p->data, p->length, false);
	PLAYERID playerId;

	//Log("Packet_AimSync:\n%s\n", DumpMem((unsigned char *)p->data, p->length));

	bsAimSync.IgnoreBits(8);
	bsAimSync.Read(playerId);

	memset(&playerInfo[playerId].aimData, 0, sizeof(AIM_SYNC_DATA));

	bsAimSync.Read((PCHAR)&playerInfo[playerId].aimData, sizeof(AIM_SYNC_DATA));
}

void Packet_VehicleSync(Packet *p)
{
	RakNet::BitStream bsSync((unsigned char *)p->data, p->length, false);
	PLAYERID playerId;

	VEHICLEID VehicleID;
	bool bLandingGear;
	bool bHydra,bTrain,bTrailer;
	bool bSiren;

	//Log("Packet_VehicleSync: %d \n%s\n", p->length, DumpMem((unsigned char *)p->data, p->length));

	bsSync.IgnoreBits(8);
	bsSync.Read(playerId);
	bsSync.Read(VehicleID);

	// Follower passenger enter
	playerInfo[playerId].incarData.VehicleID = VehicleID;
	if(settings.runMode == RUNMODE_FOLLOWPLAYER && playerId == getPlayerIDFromPlayerName(settings.szFollowingPlayerName))
	{
		if(!iPassengerNotificationSent)
		{
			SendEnterVehicleNotification(VehicleID, 1);
			iPassengerNotificationSent = 1;
		}
		
		SendPassengerFullSyncData(VehicleID);
		iFollowingPassenger = 1;
		return;
	}


	// clear last data
	memset(&playerInfo[playerId].incarData, 0, sizeof(INCAR_SYNC_DATA));

	// LEFT/RIGHT KEYS
	bsSync.Read(playerInfo[playerId].incarData.lrAnalog);

	// UP/DOWN KEYS
	bsSync.Read(playerInfo[playerId].incarData.udAnalog);

	// GENERAL KEYS
	bsSync.Read(playerInfo[playerId].incarData.wKeys);

	// ROLL / DIRECTION
	// ROTATION
	bsSync.ReadNormQuat(
		playerInfo[playerId].incarData.fQuaternion[0],
		playerInfo[playerId].incarData.fQuaternion[1],
		playerInfo[playerId].incarData.fQuaternion[2],
		playerInfo[playerId].incarData.fQuaternion[3]);

	// POSITION
	bsSync.Read(playerInfo[playerId].incarData.vecPos[0]);
	bsSync.Read(playerInfo[playerId].incarData.vecPos[1]);
	bsSync.Read(playerInfo[playerId].incarData.vecPos[2]);

	// SPEED
	bsSync.ReadVector(
		playerInfo[playerId].incarData.vecMoveSpeed[0],
		playerInfo[playerId].incarData.vecMoveSpeed[1],
		playerInfo[playerId].incarData.vecMoveSpeed[2]);

	// VEHICLE HEALTH
	WORD wTempVehicleHealth;
	bsSync.Read(wTempVehicleHealth);
	playerInfo[playerId].incarData.fCarHealth = (float)wTempVehicleHealth;

	// HEALTH/ARMOUR (COMPRESSED INTO 1 BYTE)
	BYTE byteHealthArmour;
	BYTE bytePlayerHealth, bytePlayerArmour;
	BYTE byteArmTemp=0,byteHlTemp=0;

	bsSync.Read(byteHealthArmour);
	byteArmTemp = (byteHealthArmour & 0x0F);
	byteHlTemp = (byteHealthArmour >> 4);

	if(byteArmTemp == 0xF) bytePlayerArmour = 100;
	else if(byteArmTemp == 0) bytePlayerArmour = 0;
	else bytePlayerArmour = byteArmTemp * 7;

	if(byteHlTemp == 0xF) bytePlayerHealth = 100;
	else if(byteHlTemp == 0) bytePlayerHealth = 0;
	else bytePlayerHealth = byteHlTemp * 7;

	playerInfo[playerId].incarData.bytePlayerHealth = bytePlayerHealth;
	playerInfo[playerId].incarData.bytePlayerArmour = bytePlayerArmour;

	// CURRENT WEAPON
	bsSync.Read(playerInfo[playerId].incarData.byteCurrentWeapon);

	// SIREN
	bsSync.ReadCompressed(bSiren);
	if(bSiren)
		playerInfo[playerId].incarData.byteSirenOn = 1;

	// LANDING GEAR
	bsSync.ReadCompressed(bLandingGear);
	if(bLandingGear)
		playerInfo[playerId].incarData.byteLandingGearState = 1;

	// HYDRA THRUST ANGLE AND TRAILER ID
	bsSync.ReadCompressed(bHydra);
	bsSync.ReadCompressed(bTrailer);

	DWORD dwTrailerID_or_ThrustAngle;
	bsSync.Read(dwTrailerID_or_ThrustAngle);
	playerInfo[playerId].incarData.TrailerID_or_ThrustAngle = (WORD)dwTrailerID_or_ThrustAngle;

	// TRAIN SPECIAL
	WORD wSpeed;
	bsSync.ReadCompressed(bTrain);
	if(bTrain)
	{
		bsSync.Read(wSpeed);
		playerInfo[playerId].incarData.fTrainSpeed = (float)wSpeed;
	}
}

void Packet_PassengerSync(Packet *p)
{
	RakNet::BitStream bsPassengerSync((unsigned char *)p->data, p->length, false);
	PLAYERID	playerId;
	PASSENGER_SYNC_DATA psSync;

	bsPassengerSync.IgnoreBits(8);
	bsPassengerSync.Read(playerId);
	bsPassengerSync.Read((PCHAR)&psSync,sizeof(PASSENGER_SYNC_DATA));

	// Followed wants to drive the vehicle
	playerInfo[playerId].passengerData.VehicleID = psSync.VehicleID;
	if(settings.runMode == RUNMODE_FOLLOWPLAYER && playerId == getPlayerIDFromPlayerName(settings.szFollowingPlayerName))
	{
		if(!iDriverNotificationSent)
		{
			SendEnterVehicleNotification(psSync.VehicleID, 0);
			iDriverNotificationSent = 1;
		}

		INCAR_SYNC_DATA icSync;
		memset(&icSync, 0, sizeof(INCAR_SYNC_DATA));
		icSync.VehicleID = psSync.VehicleID;
		icSync.fCarHealth = 1000.00f;
		icSync.bytePlayerHealth = 100;
		SendInCarFullSyncData(&icSync, 1, -1);

		iFollowingDriver = 1;
		return;
	}
}

void Packet_TrailerSync(Packet *p)
{
	RakNet::BitStream bsSpectatorSync((unsigned char *)p->data, p->length, false);

	PLAYERID playerId;
	//TRAILER_SYNC_DATA trSync;

	bsSpectatorSync.IgnoreBits(8);
	bsSpectatorSync.Read(playerId);
	//bsSpectatorSync.Read((PCHAR)&trSync, sizeof(TRAILER_SYNC_DATA));
}

void Packet_MarkersSync(Packet *p)
{
	RakNet::BitStream bsMarkersSync((unsigned char *)p->data, p->length, false);

	int i, iNumberOfPlayers;
	PLAYERID playerID;
	short sPosX, sPosY, sPosZ;
	bool bIsPlayerActive;

	bsMarkersSync.IgnoreBits(8);
	bsMarkersSync.Read(iNumberOfPlayers);

	if(iNumberOfPlayers < 0 || iNumberOfPlayers > MAX_PLAYERS) return;

	for(i = 0; i < iNumberOfPlayers; i++)
	{
		bsMarkersSync.Read(playerID);

		if(playerID < 0 || playerID > MAX_PLAYERS) return;

		bsMarkersSync.ReadCompressed(bIsPlayerActive);
		if(bIsPlayerActive == 0)
		{
			playerInfo[playerID].iGotMarkersPos = 0;
			continue;
		}

		bsMarkersSync.Read(sPosX);
		bsMarkersSync.Read(sPosY);
		bsMarkersSync.Read(sPosZ);

		playerInfo[playerID].iGotMarkersPos = 1;
		playerInfo[playerID].onfootData.vecPos[0] = (float)sPosX;
		playerInfo[playerID].onfootData.vecPos[1] = (float)sPosY;
		playerInfo[playerID].onfootData.vecPos[2] = (float)sPosZ;

		/*Log("Packet_MarkersSync: %d %d %0.2f, %0.2f, %0.2f", playerID, bIsPlayerActive,
			(float)sPosX, (float)sPosY, (float)sPosZ);*/
	}
}





void resetPools(int iRestart, DWORD dwTimeReconnect)
{
	memset(playerInfo, 0, sizeof(stPlayerInfo));
	memset(vehiclePool, 0, sizeof(stVehiclePool));

	if(iRestart)
	{
		iAreWeConnected = 0;
		iConnectionRequested = 0;
		iSpawned = 0;

		Sleep(dwTimeReconnect);
	}

}

void UpdatePlayerScoresAndPings(int iWait, int iMS)
{
	static DWORD dwLastUpdateTick = 0;

	if(iWait)
	{
		if ((GetTickCount() - dwLastUpdateTick) > (DWORD)iMS)
		{
			dwLastUpdateTick = GetTickCount();
			RakNet::BitStream bsParams;
			pRakClient->RPC(&RPC_UpdateScoresPingsIPs, &bsParams, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
		}
	}
	else
	{
		RakNet::BitStream bsParams;
		pRakClient->RPC(&RPC_UpdateScoresPingsIPs, &bsParams, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);
	}
}

void UpdateNetwork()
{
	unsigned char packetIdentifier;
	Packet *pkt;

	while(pkt = pRakClient->Receive())
	{
		if ( ( unsigned char ) pkt->data[ 0 ] == ID_TIMESTAMP )
		{
			if ( pkt->length > sizeof( unsigned char ) + sizeof( unsigned int ) )
				packetIdentifier = ( unsigned char ) pkt->data[ sizeof( unsigned char ) + sizeof( unsigned int ) ];
			else
				return;
		}
		else
			packetIdentifier = ( unsigned char ) pkt->data[ 0 ];

		switch(packetIdentifier)
		{
			case ID_DISCONNECTION_NOTIFICATION:
				Log("[RAKSAMP] Connection was closed by the server. Reconnecting in %d seconds.", iReconnectTime / 1000);
				resetPools(1, iReconnectTime);
				break;
			case ID_CONNECTION_BANNED:
				Log("[RAKSAMP] You are banned. Reconnecting in %d seconds.", iReconnectTime / 1000);
				resetPools(1, iReconnectTime);
				break;			
			case ID_CONNECTION_ATTEMPT_FAILED:
				Log("[RAKSAMP] Connection attempt failed. Reconnecting in %d seconds.", iReconnectTime / 1000);
				resetPools(1, iReconnectTime);
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				Log("[RAKSAMP] The server is full. Reconnecting in %d seconds.", iReconnectTime / 1000);
				resetPools(1, iReconnectTime);
				break;
			case ID_INVALID_PASSWORD:
				Log("[RAKSAMP] Invalid password. Reconnecting in %d seconds.", iReconnectTime / 1000);
				resetPools(1, iReconnectTime);
				break;
			case ID_CONNECTION_LOST:
				Log("[RAKSAMP] The connection was lost. Reconnecting in %d seconds.", iReconnectTime / 1000);
				resetPools(1, iReconnectTime);
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				Packet_ConnectionSucceeded(pkt);
				break;

			case ID_PLAYER_SYNC:
				Packet_PlayerSync(pkt);
				break;
			case ID_VEHICLE_SYNC:
				Packet_VehicleSync(pkt);
				break;
			case ID_PASSENGER_SYNC:
				Packet_PassengerSync(pkt);
				break;
			case ID_AIM_SYNC:
				Packet_AimSync(pkt);
				break;
			case ID_TRAILER_SYNC:
				Packet_TrailerSync(pkt);
				break;
			case ID_UNOCCUPIED_SYNC:
				Packet_UnoccupiedSync(pkt);
				break;
			case ID_MARKERS_SYNC:
				Packet_MarkersSync(pkt);
				break;
		}

		pRakClient->DeallocatePacket(pkt);
	}

	UpdatePlayerScoresAndPings(1, 3000);
}
