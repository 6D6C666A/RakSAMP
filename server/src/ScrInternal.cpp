#include "main.h"

void generateAndLoadInternalScript(lua_State *L)
{
	char *szScript = new char[10334];

	sprintf(szScript, "MAX_PLAYERS = %u\nMAX_VEHICLES = %u\n", MAX_PLAYERS, MAX_VEHICLES);

	char buf[256];
	for(int n = 0; n < VEHICLE_LIST_SIZE; n++)
	{
		sprintf(buf, "%s = %u\n", vehicle_list[n].name, vehicle_list[n].id);
		strcat(szScript, buf);
	}

	int error = luaL_loadbuffer(L, szScript, strlen(szScript), "line");
	if(!error)
		lua_pcall(L, 0, 0, 0);
	else
	{
		Log("Error loading memory script: %s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}

	free(szScript);
}
