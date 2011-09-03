#pragma once

#pragma comment (lib, "sqlite3/sqlite3.lib")

enum eRecordingState
{
	RECORDING_OFF = 0,
	RECORDING_RECORD,

	// All play states
	RECORDING_PLAY,
	RECORDING_PLAY_CUSTOMSPEED,

	// All reverse play states [rev states need to be after other play states]
	RECORDING_PLAY_REV,
	RECORDING_PLAY_REV_CUSTOMSPEED,
};
extern eRecordingState rec_state;
extern VEHICLEID routeVehicleID;

// generic sqlite functions
int sqliteDB_checkTableExists ( sqlite3 *db, char *tableName );
int sqliteDB_getNumTables ( sqlite3 *db, bool filter_sqlite_tables );
bool sqliteDB_dropTable ( sqlite3 *db, char *tableName );
// end of generic sqlite functions

// recording functions
int rec_sqlite_getNumTables ();
char *rec_sqlite_getTableName( int RouteNum );
bool rec_sqlite_loadTable ( char *tableName );
bool rec_sqlite_dropTable ( char *tableName );
bool rec_sqlite_writeTable ();
bool rec_sqlite_optimizeDatabase ();

void vehicle_playback_handler();
