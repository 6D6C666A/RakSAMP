#include "main.h"

using namespace RakNet;
int iScriptsRunning = 0;
struct stScript script;

int OutputConsole(lua_State *L)
{
	Log("%s", lua_tostring(L, 1));

	return 1;
}

int IsPlayerConnected(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);
	
	int iIsConnected = isPlayerConnected(playerID);
	lua_pushnumber(L, iIsConnected);

	return iIsConnected;
}

int GetPlayerName(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);

	if(isPlayerConnected(playerID))
	{
		lua_pushstring(L, playerPool[playerID].szPlayerName);
		return 1;
	}

	lua_pushstring(L, "nil");

	return 0;
}

int GetPlayerPos(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);

	if(isPlayerConnected(playerID))
	{
		float x, y, z;
		if(playerInfo[playerID].incarData.VehicleID)
		{
			x = playerInfo[playerID].incarData.vecPos[0];
			y = playerInfo[playerID].incarData.vecPos[1];
			z = playerInfo[playerID].incarData.vecPos[2];
		}
		else
		{
			x = playerInfo[playerID].onfootData.vecPos[0];
			y = playerInfo[playerID].onfootData.vecPos[1];
			z = playerInfo[playerID].onfootData.vecPos[2];
		}

		lua_pushnumber(L, (lua_Number)x);
		lua_pushnumber(L, (lua_Number)y);
		lua_pushnumber(L, (lua_Number)z);

		return 3;
	}

	return 0;
}

int GetPlayerScore(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);

	if(isPlayerConnected(playerID))
	{
		int iScore = playerPool[playerID].iPlayerScore;

		lua_pushnumber(L, (int)iScore);

		return 1;
	}

	return 0;
}

int SetPlayerScore(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);
	int iScore = lua_tointeger(L, 2);

	if(isPlayerConnected(playerID))
	{
		playerPool[playerID].iPlayerScore = iScore;
		return 1;
	}

	return 0;
}

int GetPlayerIP(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);

	if(isPlayerConnected(playerID))
	{
		const char *ip = pRakServer->GetPlayerIDFromIndex(playerID).ToString(false);
		if(ip)
			lua_pushstring(L, ip);

		return 1;
	}

	return 0;
}

int GetPlayerPing(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);
	if(isPlayerConnected(playerID))
	{
		DWORD dwPing = playerPool[playerID].dwPlayerPing;

		lua_pushnumber(L, (DWORD)dwPing);

		return 1;
	}

	return 0;
}

int AddStaticVehicle(lua_State *L)
{
	int iVehModelID = lua_tointeger(L, 1);
	lua_Number fX = lua_tonumber(L, 2);
	lua_Number fY = lua_tonumber(L, 3);
	lua_Number fZ = lua_tonumber(L, 4);
	lua_Number fRot = lua_tonumber(L, 5);
	BYTE bColor1 = lua_tointeger(L, 6);
	BYTE bColor2 = lua_tointeger(L, 7);
	int bRespawnOnDeath = lua_tointeger(L, 8);
	int iTimeUntilRespawn = lua_tointeger(L, 9);

	addStaticVehicle(iVehModelID, (float)fX, (float)fY, (float)fZ, (float)fRot, bColor1, bColor2, bRespawnOnDeath, iTimeUntilRespawn);

	return 1;
}

int CreateVehicle(lua_State *L)
{
	int iVehModelID = lua_tointeger(L, 1);
	lua_Number fX = lua_tonumber(L, 2);
	lua_Number fY = lua_tonumber(L, 3);
	lua_Number fZ = lua_tonumber(L, 4);
	lua_Number fRot = lua_tonumber(L, 5);
	BYTE bColor1 = lua_tointeger(L, 6);
	BYTE bColor2 = lua_tointeger(L, 7);

	addVehicle(iVehModelID, (float)fX, (float)fY, (float)fZ, (float)fRot, bColor1, bColor2);

	return 1;
}

int RemoveVehicle(lua_State *L)
{
	VEHICLEID vehicleID = lua_tointeger(L, 1);
	if(!doesVehicleExist(vehicleID)) return 0;

	BitStream bs;
	bs.Write(vehicleID);
	pRakServer->RPC(&RPC_WorldVehicleRemove, &bs, HIGH_PRIORITY, RELIABLE,
		0, UNASSIGNED_PLAYER_ID, TRUE, FALSE, UNASSIGNED_NETWORK_ID, NULL);

	removeVehicle(vehicleID);

	return 1;
}

int SendPlayerMessage(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);
	lua_Number dwColor = lua_tonumber(L, 2);
	char *szMessage = (char *)lua_tostring(L, 3);

	BYTE bMessageLen = strlen(szMessage);
	SendClientMessage(playerID, (DWORD)dwColor, szMessage);

	return 1;
}

int SendPlayerChatMessage(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);
	char *szMessage = (char *)lua_tostring(L, 2);

	BYTE bMessageLen = strlen(szMessage);
	SendChatMessage(playerID, szMessage, bMessageLen, 0);

	return 1;
}

int SendPlayerChatMessageToAll(lua_State *L)
{
	PLAYERID playerID = lua_tointeger(L, 1);
	char *szMessage = (char *)lua_tostring(L, 2);

	BYTE bMessageLen = strlen(szMessage);
	SendChatMessage(playerID, szMessage, bMessageLen, 1);

	return 1;
}

int SetPlayerPos(lua_State *L)
{
	BitStream bs;
	PLAYERID playerID = lua_tointeger(L, 1);
	lua_Number fX = lua_tonumber(L, 2);
	lua_Number fY = lua_tonumber(L, 3);
	lua_Number fZ = lua_tonumber(L, 4);

	/*bs.Write((float)fX);
	bs.Write((float)fY);
	bs.Write((float)fZ);
	rpc.Call("RPC_Script_SetPlayerPos", &bs, HIGH_PRIORITY, RELIABLE, 0, server->GetGUIDFromIndex(playerID), false);*/

	return 1;
}

int SetPlayerRotation(lua_State *L)
{
	BitStream bs;
	PLAYERID playerID = lua_tointeger(L, 1);
	lua_Number fRot = lua_tonumber(L, 2);

	/*bs.Write((float)fRot);
	rpc.Call("RPC_Script_SetPlayerRotation", &bs, HIGH_PRIORITY, RELIABLE, 0, server->GetGUIDFromIndex(playerID), false);*/

	return 1;
}

int SetPlayerMoney(lua_State *L)
{
	BitStream bs;
	PLAYERID playerID = lua_tointeger(L, 1);
	lua_Number iMoney = lua_tonumber(L, 2);

	/*bs.Write((int)iMoney);
	rpc.Call("RPC_Script_SetPlayerMoney", &bs, HIGH_PRIORITY, RELIABLE, 0, server->GetGUIDFromIndex(playerID), false);*/

	return 1;
}

int SetPlayerHealth(lua_State *L)
{
	BitStream bs;
	PLAYERID playerID = lua_tointeger(L, 1);
	lua_Number iHealth = lua_tonumber(L, 2);

	/*iHealth += 100;
	bs.Write((int)iHealth);
	rpc.Call("RPC_Script_SetPlayerHealth", &bs, HIGH_PRIORITY, RELIABLE, 0, server->GetGUIDFromIndex(playerID), false);*/

	return 1;
}

int SetPlayerArmour(lua_State *L)
{
	BitStream bs;
	PLAYERID playerID = lua_tointeger(L, 1);
	lua_Number iArmour = lua_tonumber(L, 2);

	/*bs.Write((int)iArmour);
	rpc.Call("RPC_Script_SetPlayerArmour", &bs, HIGH_PRIORITY, RELIABLE, 0, server->GetGUIDFromIndex(playerID), false);*/

	return 1;
}

int GiveWeapon(lua_State *L)
{
	BitStream bs;
	PLAYERID playerID = lua_tointeger(L, 1);
	lua_Number iWeapon = lua_tonumber(L, 2);
	lua_Number iAmmo = lua_tonumber(L, 3);

	/*bs.Write((int)iWeapon);
	bs.Write((int)iAmmo);
	rpc.Call("RPC_Script_GiveWeapon", &bs, HIGH_PRIORITY, RELIABLE, 0, server->GetGUIDFromIndex(playerID), false;*/

	return 1;
}

int SetWeaponAmmo(lua_State *L)
{
	BitStream bs;
	PLAYERID playerID = lua_tointeger(L, 1);
	lua_Number iWeapon = lua_tonumber(L, 2);
	lua_Number iAmmo = lua_tonumber(L, 3);

	/*bs.Write((int)iWeapon);
	bs.Write((int)iAmmo);
	rpc.Call("RPC_Script_SetWeaponAmmo", &bs, HIGH_PRIORITY, RELIABLE, 0, server->GetGUIDFromIndex(playerID), false);*/

	return 1;
}

int ClearPlayerWeapons(lua_State *L)
{
	BitStream bs;
	PLAYERID playerID = lua_tointeger(L, 1);

	//rpc.Call("RPC_Script_ClearPlayerWeapons", &bs, HIGH_PRIORITY, RELIABLE, 0, server->GetGUIDFromIndex(playerID), false);
	return 1;
}

void RegisterScriptingFunctions(lua_State *L)
{
	generateAndLoadInternalScript(L);

	lua_register(L, "outputConsole", OutputConsole);

	lua_register(L, "isPlayerConnected", IsPlayerConnected);
	lua_register(L, "getPlayerName", GetPlayerName);
	lua_register(L, "getPlayerPos", GetPlayerPos);
	lua_register(L, "getPlayerScore", GetPlayerScore);
	lua_register(L, "setPlayerScore", SetPlayerScore);
	lua_register(L, "getPlayerIP", GetPlayerIP);
	lua_register(L, "getPlayerPing", GetPlayerPing);
	lua_register(L, "sendPlayerMessage", SendPlayerMessage);
	lua_register(L, "sendPlayerChatMessage", SendPlayerChatMessage);
	lua_register(L, "sendPlayerChatMessageToAll", SendPlayerChatMessageToAll);
	lua_register(L, "setPlayerPos", SetPlayerPos);
	lua_register(L, "setPlayerRotation", SetPlayerRotation);
	lua_register(L, "setPlayerMoney", SetPlayerMoney);
	lua_register(L, "setPlayerHealth", SetPlayerHealth);
	lua_register(L, "setPlayerArmour", SetPlayerArmour);

	lua_register(L, "addStaticVehicle", AddStaticVehicle);
	lua_register(L, "createVehicle", CreateVehicle);
	lua_register(L, "removeVehicle", RemoveVehicle);

	lua_register(L, "giveWeapon", GiveWeapon);
	lua_register(L, "setWeaponAmmo", SetWeaponAmmo);
	lua_register(L, "clearPlayerWeapons", ClearPlayerWeapons);
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

extern char szWorkingDirectory[MAX_PATH];
extern char szScripts[512];

bool LoadScript(char *szScriptName, bool bFirstLoad)
{
	char szTemp[256];
	sprintf_s(szTemp, sizeof(szTemp), "%s\\scripts\\%s.lua", szWorkingDirectory, szScriptName);

	for(int i = 0; i < MAX_SCRIPTS; i++)
	{
		if(strcmp(script.szScriptName[i], szScriptName) == 0)
		{
			Log("  %s.lua... FAIL (already loaded)", szScriptName);
			Log("  ");
			return false;
		}
	}

	FILE *fExists = fopen(szTemp, "rb");
	if(!fExists)
	{
		Log("  %s.lua... FAIL (does not exist)", szScriptName);
		Log("  ");
		return false;
	}

	strcpy_s(script.szScriptName[iScriptsRunning], 32, szScriptName);
	script.scriptVM[iScriptsRunning] = lua_open();
	if(script.scriptVM[iScriptsRunning] == NULL)
	{
		Log("  %s.lua... FAIL (LUA virtual machine)", szScriptName);
		Log("  ");
		return false;
	}
	luaL_openlibs(script.scriptVM[iScriptsRunning]);

	RegisterScriptingFunctions(script.scriptVM[iScriptsRunning]);

	int iCurrentScriptStatus = luaL_loadfile(script.scriptVM[iScriptsRunning], szTemp);
	if(bFirstLoad) Log("  %s.lua... %s", szScriptName, iCurrentScriptStatus ? "FAIL" : "OK");
	if(iCurrentScriptStatus == 0)
	{
		iCurrentScriptStatus = lua_pcall(script.scriptVM[iScriptsRunning], 0, LUA_MULTRET, 0);
		if(iCurrentScriptStatus == 0)
		{
			ScriptEvent_OnScriptStart(script.scriptVM[iScriptsRunning]);

			if(!bFirstLoad)
			{
				// respawn all static vehicles if any
				BitStream bsAnnounceStaticVehicle;
				for(VEHICLEID i = 1; i < MAX_VEHICLES; i++)
				{
					if(doesVehicleExist(i))
					{
						NEW_VEHICLE veh;
						bsAnnounceStaticVehicle.Reset();
						veh.VehicleId = (VEHICLEID)i;
						veh.iVehicleType = vehiclePool[i].iModelID;
						veh.vecPos[0] = vehiclePool[i].fVehiclePos[0];
						veh.vecPos[1] = vehiclePool[i].fVehiclePos[1];
						veh.vecPos[2] = vehiclePool[i].fVehiclePos[2];
						veh.fRotation = vehiclePool[i].fRotation;
						veh.aColor1 = vehiclePool[i].bColor1;
						veh.aColor2 = vehiclePool[i].bColor2;
						veh.fHealth = vehiclePool[i].fHealth;
						veh.byteInterior = 0;
						veh.byteDoorsLocked = 0;
						veh.dwDoorDamageStatus = 0;
						veh.dwPanelDamageStatus = 0;
						veh.byteLightDamageStatus = 0;
						bsAnnounceStaticVehicle.Write((const char *)&bsAnnounceStaticVehicle, sizeof(NEW_VEHICLE));
						pRakServer->RPC(&RPC_WorldVehicleAdd, &bsAnnounceStaticVehicle, HIGH_PRIORITY, RELIABLE,
							0, UNASSIGNED_PLAYER_ID, TRUE, FALSE, UNASSIGNED_NETWORK_ID, NULL);

						Sleep(5); // well, shit.
					}
				}
			}
		}
		if(bFirstLoad)
			Log(" ");
		else
			Log("  Script %s was succesfully loaded.", script.szScriptName[iScriptsRunning]);

		iScriptsRunning++;
	}
	else
	{
		Log("  %s", lua_tostring(script.scriptVM[iScriptsRunning], -1));
		Log("  ");
		return false;
	}

	return true;
}

bool UnloadScript(char *szScriptName)
{
	bool bFoundAndUnloaded = false;
	for(int i = 0; i < MAX_SCRIPTS; i++)
	{
		if(strcmp(script.szScriptName[i], szScriptName) == 0)
		{
			if(script.scriptVM[i] != NULL)
			{
				ScriptEvent_OnScriptExit(script.scriptVM[i]);
				lua_close(script.scriptVM[i]);
				script.scriptVM[i] = NULL;
			}
			memset(script.szScriptName[i], 0, 32);
			iScriptsRunning--;

			Log("  Script %s was succesfully unloaded.", szScriptName);
			bFoundAndUnloaded = true;
		}
	}

	if(bFoundAndUnloaded == false)
	{
		Log("  Could not find %s for unload.", szScriptName);
		return false;
	}

	return true;
}

bool LoadScripts()
{
	Log("  Loading scripts...");
	Log(" ");

	TiXmlElement* serverElement = xmlSettings.FirstChildElement("server");
	if(serverElement)
	{
		char *pLine = (char *)serverElement->Attribute("scripts");
		char *pch = strtok(pLine, " ");
		while(pch != NULL)
		{
			LoadScript(pch, true);
			pch = strtok (NULL, " ");
		}
	}

	return true;
}

void HandleScriptingConsoleInputCommands(char *szInputBuffer)
{
	if(!strncmp(szInputBuffer, "loadscr", 7) || !strncmp(szInputBuffer, "LOADSCR", 7))
	{
		if(szInputBuffer[8] == 0x0D || szInputBuffer[8] == 0x0A || szInputBuffer[8] == 0x00) return;
		memset(szInputBuffer + strlen(szInputBuffer) - 2, 0, 2);
		LoadScript(&szInputBuffer[8], false);
	}
	if(!strncmp(szInputBuffer, "unloadscr", 9) || !strncmp(szInputBuffer, "UNLOADSCR", 9))
	{
		if(szInputBuffer[10] == 0x0D || szInputBuffer[10] == 0x0A || szInputBuffer[10] == 0x00) return;
		memset(szInputBuffer + strlen(szInputBuffer) - 2, 0, 2);
		UnloadScript(&szInputBuffer[10]);
	}
	if(!strncmp(szInputBuffer, "reloadscr", 9) || !strncmp(szInputBuffer, "RELOADSCR", 9)) // !!! CHECK TODO
	{
		if(szInputBuffer[10] == 0x0D || szInputBuffer[10] == 0x0A || szInputBuffer[10] == 0x00) return;
		memset(szInputBuffer + strlen(szInputBuffer) - 2, 0, 2);
		if(UnloadScript(&szInputBuffer[10]))
			LoadScript(&szInputBuffer[10], false);
	}
	if(!strncmp(szInputBuffer, "listscr", 7) || !strncmp(szInputBuffer, "LISTSCR", 7))
	{
		Log(" Scripts running: %d", iScriptsRunning);
		for(int i = 0; i < iScriptsRunning; i++)
		{
			if(script.szScriptName[i][0] == 0x00 && script.scriptVM[i] == NULL) continue;
			Log("[%d] %s", i, script.szScriptName[i]);
		}
	}

	return;
}

void CleanUpScripting()
{
	for(int i = 0; i < iScriptsRunning; i++)
	{
		if(script.scriptVM[iScriptsRunning] != NULL)
			lua_close(script.scriptVM[iScriptsRunning]);
	}

	return;
}
