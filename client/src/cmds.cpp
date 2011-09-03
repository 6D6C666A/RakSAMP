#include "main.h"

int RunCommand(char *szCMD, int iFromAutorun)
{
	// return 0: should run server sided command.
	// return 1: found local command.
	// return 2: local command not found.
	// return 3: exit process.

	if(szCMD[0] == 0x00)
		return 2;

	if(settings.iConsole)
		memset(&szCMD[(strlen(szCMD) - 2)], 0, 2);

	if(szCMD[0] != '!')
	{
		// SERVER CHAT OR COMMAND
		if(szCMD[0] == '/')
			sendServerCommand(szCMD);
		else
			sendChat(szCMD);

		return 0;
	}

	szCMD++;

	// EXIT
	if(!strncmp(szCMD, "exit", 4) || !strncmp(szCMD, "EXIT", 4) ||
		!strncmp(szCMD, "quit", 4) || !strncmp(szCMD, "QUIT", 4))
	{
		sampDisconnect(0);
		ExitProcess(0);

		return 3;
	}

	// RECONNECT
	if(!strncmp(szCMD, "reconnect", 9) || !strncmp(szCMD, "RECONNECT", 9))
	{
		sampDisconnect(0);
		resetPools(1, 2000);

		return 1;
	}

	// RELOAD SETTINGS
	if(!strncmp(szCMD, "reload", 6) || !strncmp(szCMD, "RELOAD", 6))
	{
		ReloadSettings();

		return 1;
	}

	// PLAYER LIST
	if(!strncmp(szCMD, "players", 7) || !strncmp(szCMD, "PLAYERS", 7))
	{
		int iPlayerCount = 0;
		Log(" ");
		Log("=== PLAYER LIST ===");
		Log("ID Name Ping Score");
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			if(!playerInfo[i].iIsConnected)
				continue;

			Log("(%d) %s %d %d", i, playerInfo[i].szPlayerName, playerInfo[i].dwPing, playerInfo[i].iScore);
			iPlayerCount++;
		}
		Log(" ");
		Log("=== [COUNT: %d] ===", iPlayerCount);
		Log(" ");

		return 1;
	}

	// GOTO
	if(!strncmp(szCMD, "goto", 4) || !strncmp(szCMD, "GOTO", 4))
	{
		int iPlayerID = atoi(&szCMD[5]);
		if(iPlayerID == -1)
		{
			normalMode_goto = -1;
			return 1;
		}

		if(iPlayerID < 0 || iPlayerID > MAX_PLAYERS)
			return 1;

		if(playerInfo[iPlayerID].iIsConnected)
		{
			normalMode_goto = (PLAYERID)iPlayerID;

			ONFOOT_SYNC_DATA ofSync;
			memset(&ofSync, 0, sizeof(ONFOOT_SYNC_DATA));
			ofSync.byteHealth = 100;
			ofSync.vecPos[0] = playerInfo[iPlayerID].onfootData.vecPos[0];
			ofSync.vecPos[1] = playerInfo[iPlayerID].onfootData.vecPos[1];
			ofSync.vecPos[2] = playerInfo[iPlayerID].onfootData.vecPos[2];

			SendOnFootFullSyncData(&ofSync, 0, -1);
		}
		else
			Log("[GOTO] Player %d is not connected.", iPlayerID);

		return 1;
	}

	// IMITATE
	if(!strncmp(szCMD, "imitate", 7) || !strncmp(szCMD, "IMITATE", 7))
	{
		char *szPlayerName = &szCMD[8];
		if(!strcmp(szPlayerName,"-1"))
		{
			imitateID = -1;
			Log("[IMITATE] Imitate was disabled.");
			return 1;
		}

		PLAYERID playerID = getPlayerIDFromPlayerName(szPlayerName);

		if(playerID < 0 || playerID > MAX_PLAYERS)
			return 1;

		if(playerInfo[playerID].iIsConnected)
		{
			imitateID = (PLAYERID)playerID;
			Log("[IMITATE] Imitate ID set to %d (%s)", imitateID, szPlayerName);
		}
		else
			Log("[IMITATE] Player %s is not connected.", szPlayerName);

		return 1;
	}

	// VEHICLE LIST
	if(!strncmp(szCMD, "vlist", 5) || !strncmp(szCMD, "VLIST", 5))
	{
		for(VEHICLEID i = 0; i < MAX_VEHICLES; i++)
		{
			if(!vehiclePool[i].iDoesExist)
				continue;

			const struct vehicle_entry *vehicle = gta_vehicle_get_by_id( vehiclePool[i].iModelID );
			if(vehicle)
				Log("[VLIST] %d (%s)", i, vehicle->name);
		}

		return 1;
	}

	// SEND VEHICLE DEATH NOTIFICATION
	if(!strncmp(szCMD, "vdeath", 6) || !strncmp(szCMD, "VDEATH", 6))
	{
		int iSelectedVeh = atoi(&szCMD[7]);
		NotifyVehicleDeath((VEHICLEID)iSelectedVeh);
		Log("[VDEATH] Sent to vehicle ID %d", iSelectedVeh);

		return 1;
	}

	// VEHICLE PLAYING COMMANDS
	// SELECT VEHICLE ID TO PLAY THE ROUTE OR DRIVING MODE VEHICLE
	if(!strncmp(szCMD, "selveh", 6) || !strncmp(szCMD, "SELVEH", 6))
	{
		int iSelectedVeh = atoi(&szCMD[7]);

		if(settings.runMode == RUNMODE_PLAYROUTES)
			routeVehicleID = (VEHICLEID)iSelectedVeh;
		else if(settings.runMode == RUNMODE_FOLLOWPLAYERSVEHICLE)
			settings.iFollowingWithVehicleID = (VEHICLEID)iSelectedVeh;

		Log("[SELVEH] Changed to vehicle ID to %d.", iSelectedVeh);

		return 1;
	}
	// ROUTE LIST
	if(!strncmp(szCMD, "rtlist", 6) || !strncmp(szCMD, "RTLIST", 6))
	{
		// now get me some data from the database
		char table_name_[64];
		char *table_name;
		int num_tables = rec_sqlite_getNumTables();

		if ( num_tables <= 0 )
		{
			Log( (num_tables < 0) ? "Error Reading Database" : "No Saved Routes");
			return 1;
		}

		for ( int i = 0; i < num_tables; i++ )
		{
			table_name = rec_sqlite_getTableName(i);
			if ( table_name == NULL )
				continue;

			_snprintf_s( table_name_, sizeof(table_name_)-1, "ID %d: '%s'", i, table_name );
			Log(table_name_);
		}

		return 1;
	}

	// LOAD ROUTE
	if(!strncmp(szCMD, "loadrt", 6) || !strncmp(szCMD, "LOADRT", 6))
	{
		int iSelectedRoute = atoi(&szCMD[7]);

		char *table_name;
		int num_tables = rec_sqlite_getNumTables();

		if ( num_tables <= 0 )
		{
			Log( (num_tables < 0) ? "Error Reading Database" : "No Saved Routes");
			return 1;
		}

		for ( int i = 0; i < num_tables; i++ )
		{
			table_name = rec_sqlite_getTableName(i);
			if ( table_name == NULL )
				continue;

			if(i == iSelectedRoute)
				rec_sqlite_loadTable( table_name );
		}			

		return 1;
	}

	// PLAY ROUTE
	if(!strncmp(szCMD, "playrt", 6) || !strncmp(szCMD, "PLAYRT", 6))
	{
		rec_state = RECORDING_PLAY;

		return 1;
	}

	// STOP ROUTE
	if(!strncmp(szCMD, "stoprt", 6) || !strncmp(szCMD, "STOPRT", 6))
	{
		rec_state = RECORDING_OFF;

		return 1;
	}

	// SELECT AN ITEM FROM THE GTA MENU
	if(!strncmp(szCMD, "menusel", 7) || !strncmp(szCMD, "MENUSEL", 7))
	{
		BYTE bRow = (BYTE)atoi(&szCMD[8]);

		if(bRow != 0xFF)
		{
			RakNet::BitStream bsSend;
			bsSend.Write(bRow);
			pRakClient->RPC(&RPC_MenuSelect, &bsSend, HIGH_PRIORITY, RELIABLE, 0, FALSE, UNASSIGNED_NETWORK_ID, NULL);			
		}

		return 1;
	}

	Log("Command %s was not found.", szCMD);

	return 2;
}
