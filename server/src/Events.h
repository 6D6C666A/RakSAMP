void ScriptEvent_OnScriptStart(lua_State *L);
void ScriptEvent_OnScriptExit(lua_State *L);

void ScriptEvent_OnNewQuery(lua_State *L, char *pszIP, int iSourcePort, bool bBanned);
void ScriptEvent_OnNewConnection(lua_State *L, int iPlayerID, char *pszIP, int iSourcePort);
void ScriptEvent_OnPlayerJoin(lua_State *L, int iPlayerID, char *pszPlayerName, char *pszIP, int iSourcePort);
void ScriptEvent_OnPlayerDisconnect(lua_State *L, int iPlayerID, char *pszPlayerName, char *pszReason);
void ScriptEvent_OnPlayerSpawn(lua_State *L, int iPlayerID);
void ScriptEvent_OnPlayerDeath(lua_State *L, int iPlayerID, int iKillerID, int iWeaponID);
void ScriptEvent_OnPlayerWantsEnterVehicle(lua_State *L, int iPlayerID, int iVehicleID, int iAsPassenger);
void ScriptEvent_OnPlayerLeaveVehicle(lua_State *L, int iPlayerID, int iVehicleID);
void ScriptEvent_OnPlayerMessage(lua_State *L, int iPlayerID, char *pszMessage);
void ScriptEvent_OnPlayerCommand(lua_State *L, int iPlayerID, char *pszCommand);
