#include "main.h"

#define REC_DB_NAME		"mod_sa_routes.db"
#define REC_ARRAYSIZE	4096
#define REC_DEFAULT_WAITTIME	25
float rec_angle[REC_ARRAYSIZE][9];
float rec_spin[REC_ARRAYSIZE][3];
float rec_speed[REC_ARRAYSIZE][3];
float rec_pos[REC_ARRAYSIZE][3];
int rec_maxNum = 0;
int rec_index = 0;
float rec_playNext;
eRecordingState rec_state;
bool rec_continueAfterFin = true;
VEHICLEID routeVehicleID;

// ret: -1 = fail, 0 = not exist, 1 = exist
int sqliteDB_checkTableExists ( sqlite3 *db, char *tableName )
{
	sqlite3_stmt *prep_stmt;
	char sql_cmd[256];

	if ( db == NULL || tableName == NULL )
	{
		Log ( "sqliteDB_checkTableExists: received %s", db == NULL ? "db = NULL" : "tableName = NULL" );
		return -1;
	}

	_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';", tableName );
	if ( sqlite3_prepare_v2( db, sql_cmd, sizeof(sql_cmd), &prep_stmt, NULL ) != SQLITE_OK )
	{
		Log( "SQLite - Error (preparing statement to find '%s' table): %s", tableName, sqlite3_errmsg(db) );
		return -1;
	}
	sqlite3_step( prep_stmt );

	// if we receive some text the table exists
	if ( sqlite3_column_text( prep_stmt, 0 ) != NULL )
	{
		sqlite3_finalize( prep_stmt );
		return 1;
	}
	sqlite3_finalize( prep_stmt );
	return 0;
}

int sqliteDB_getNumTables ( sqlite3 *db, bool filter_sqlite_tables )
{
	sqlite3_stmt *prep_stmt;
	int prep_step_ret;
	char sql_cmd[64];
	int num_tables;

	if ( db == NULL )
	{
		Log ( "sqliteDB_getNumTables: received db = NULL" );
		return -1;
	}

	num_tables = 0;

	_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "SELECT name FROM sqlite_master WHERE type='table';" );
	if ( sqlite3_prepare_v2( db, sql_cmd, sizeof(sql_cmd), &prep_stmt, NULL ) != SQLITE_OK )
	{
		Log( "SQLite - Error (preparing statement in getNumTables): %s", sqlite3_errmsg(db) );
		return -1;
	}
	prep_step_ret = sqlite3_step( prep_stmt );

	while ( prep_step_ret == SQLITE_ROW )
	{
		// filter sqlite_* tables
		if ( filter_sqlite_tables == false
			|| strncmp((char*)sqlite3_column_text(prep_stmt,0), "sqlite_", 7) != 0 )
			num_tables++;

		prep_step_ret = sqlite3_step( prep_stmt );
	}

	sqlite3_finalize( prep_stmt );
	return num_tables;
}

bool sqliteDB_dropTable ( sqlite3 *db, char *tableName )
{
	int exist_check;
	char sql_cmd[64];
	char *errmsgs;

	if ( db == NULL || tableName == NULL )
	{
		Log ( "sqliteDB_dropTable: received %s", db == NULL ? "db = NULL" : "tableName = NULL" );
		return false;
	}

	exist_check = sqliteDB_checkTableExists( db, tableName );
	if ( exist_check != 1 )
	{
		// table already doesn't exist anymore
		if ( exist_check == 0 )
			return true;

		// some error
		return false;
	}

	_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "DROP TABLE '%s';", tableName );
	sqlite3_exec( db, sql_cmd, NULL, NULL, &errmsgs );
	if ( errmsgs != NULL )
	{
		Log( "SQLite - Error (exec drop table '%s'): %s", tableName, sqlite3_errmsg(db) );
		return false;
	}
	return true;
}

int rec_sqlite_getNumTables ()
{
	sqlite3 *rec_db;
	int numTables;

	if ( sqlite3_open( REC_DB_NAME, &rec_db ) != SQLITE_OK )
	{
		Log( "SQLite - Error while connecting: %s", sqlite3_errmsg(rec_db) );
		sqlite3_close( rec_db );
		return -1;
	}

	numTables = sqliteDB_getNumTables( rec_db, true );
	sqlite3_close( rec_db );
	return numTables;
}

char rec_tempTableCpy[64];
char *rec_sqlite_getTableName( int RouteNum )
{
	sqlite3 *rec_db;
	sqlite3_stmt *prep_stmt;
	int prep_step_ret;
	char sql_cmd[64];
	char *cur_val;
	char *ret_val;
	int num_table;

	// start at -1 - array-pos, not actual num
	num_table = -1;
	ret_val = NULL;
	if ( sqlite3_open( REC_DB_NAME, &rec_db ) != SQLITE_OK )
	{
		Log( "SQLite - Error while connecting: %s", sqlite3_errmsg(rec_db) );
		sqlite3_close( rec_db );
		return NULL;
	}

	_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "SELECT name FROM sqlite_master WHERE type='table';" );
	if ( sqlite3_prepare_v2( rec_db, sql_cmd, sizeof(sql_cmd), &prep_stmt, NULL ) != SQLITE_OK )
	{
		Log( "SQLite - Error (preparing statement in getTableName): %s", sqlite3_errmsg(rec_db) );
		sqlite3_close( rec_db );
		return NULL;
	}
	prep_step_ret = sqlite3_step( prep_stmt );

	while ( prep_step_ret == SQLITE_ROW && ret_val == NULL )
	{
		cur_val = (char*)sqlite3_column_text( prep_stmt, 0 );
		// filter sqlite_* tables
		if ( strncmp(cur_val, "sqlite_", 7) != 0 )
			num_table++;

		if ( num_table == RouteNum )
		{
			ret_val = cur_val;
			_snprintf_s( rec_tempTableCpy, sizeof(rec_tempTableCpy)-1, "%s", cur_val );
			break;
		}

		prep_step_ret = sqlite3_step( prep_stmt );
	}

	sqlite3_finalize( prep_stmt );
	sqlite3_close( rec_db );
	if ( cur_val == NULL )
		return NULL;
	return &rec_tempTableCpy[0];
}

bool rec_sqlite_loadTable ( char *tableName )
{
	sqlite3 *rec_db;
	sqlite3_stmt *prep_stmt;
	int prep_step_ret;
	char sql_cmd[1024];

	if ( sqlite3_open( REC_DB_NAME, &rec_db ) != SQLITE_OK )
	{
		Log( "SQLite - Error while connecting: %s", sqlite3_errmsg(rec_db) );
		sqlite3_close( rec_db );
		return false;
	}

	// return false, if error happens, or table doesn't exist
	if ( sqliteDB_checkTableExists( rec_db, tableName ) != 1 )
	{
		sqlite3_close( rec_db );
		Log( "table doesn't exist" );
		return false;
	}

	// stop playing/recording when loading a new route
	rec_state = RECORDING_OFF;

	_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "SELECT * FROM '%s';", tableName );
	if ( sqlite3_prepare_v2( rec_db, sql_cmd, sizeof(sql_cmd), &prep_stmt, NULL ) != SQLITE_OK )
	{
		Log( "SQLite - Error (prepare statement to load from table '%s'): %s", tableName, sqlite3_errmsg(rec_db) );
		sqlite3_close( rec_db );
		return false;
	}

	// jump to first row and set the maxNum
	prep_step_ret = sqlite3_step( prep_stmt );
	rec_maxNum = sqlite3_column_int(prep_stmt,1);

	if ( rec_maxNum > (REC_ARRAYSIZE-1) || rec_maxNum <= 0 )
	{
		Log( "Recording - load table '%s': rec_maxNum(%i) is <= 0 or greater than maximum array size!", 
			tableName, rec_maxNum );
		sqlite3_finalize( prep_stmt );
		sqlite3_close( rec_db );

		Log( "failed to load" );
		// we changed a variable, so set maxNum to 0, so it can't be 
		// causing problems when trying to play record
		rec_maxNum = 0;
		return false;
	}

	for ( int i = 0; i < rec_maxNum; i++ )
	{
		// load our data from the table
		// do not forget to adjust these offsets when changing table design
		for ( int j = 0; j < 9; j++ )
			rec_angle[i][j] = (float)sqlite3_column_double( prep_stmt, j+2 );
		for ( int j = 0; j < 3; j++ )
		{
			rec_spin[i][j] = (float)sqlite3_column_double( prep_stmt, j+11 );
			rec_speed[i][j] = (float)sqlite3_column_double( prep_stmt, j+14 );
			rec_pos[i][j] = (float)sqlite3_column_double( prep_stmt, j+17 );
		}

		prep_step_ret = sqlite3_step( prep_stmt );

		// step() returned some error/unexpected value?
		if ( prep_step_ret != SQLITE_ROW && prep_step_ret != SQLITE_DONE )
		{
			Log( "SQLite - Error (prepare statement to load from table '%s' - cycle %i): %s", tableName, i, sqlite3_errmsg(rec_db) );
			sqlite3_finalize( prep_stmt );
			sqlite3_close( rec_db );

			Log( "failed to load" );
			// data has been changed.. destroy record for playback
			rec_maxNum = 0;
			return false;
		}

		// we somehow reached the end (end of rows/end of loop)
		if ( i == (rec_maxNum-1) || prep_step_ret == SQLITE_DONE )
		{
			// check if its only end of one = error while loading
			if ( i != (rec_maxNum-1) || prep_step_ret != SQLITE_DONE )
			{
				Log( "Problem while loading Recording '%s': %s - MaxNum %i - cycleNum %i",
					tableName,
					prep_step_ret == SQLITE_DONE ? "End of rows" : "Not at end of rows",
					rec_maxNum, i );
				sqlite3_finalize( prep_stmt );
				sqlite3_close( rec_db );

				Log( "failed to load" );
				// we probably got incorrect data in the recording? - set maxNum to 0
				rec_maxNum = 0;
				return false;
			}

			// we reached the end of rows at expected time (when the loop reaches maxNum-1)
			Log( "successfully loaded route" );
		}		
	}
	sqlite3_finalize( prep_stmt );
	sqlite3_close( rec_db );
	return true;
}

bool rec_sqlite_dropTable ( char *tableName )
{
	sqlite3 *rec_db;
	bool return_val = false;

	if ( sqlite3_open( REC_DB_NAME, &rec_db ) != SQLITE_OK )
	{
		Log( "SQLite - Error while connecting: %s", sqlite3_errmsg(rec_db) );
		sqlite3_close( rec_db );
		return false;
	}

	return_val = sqliteDB_dropTable(rec_db, tableName);
	if ( return_val == true )
		Log( "Dropped table '%s'", tableName );

	sqlite3_close( rec_db );
	return return_val;
}

bool rec_sqlite_writeTable ()
{
	sqlite3 *rec_db;
	char sql_cmd[1024];
	char *errmsgs = NULL;
	int ret_exists;

	if ( rec_state == RECORDING_RECORD )
	{
		Log( "Can't save while recording." );
		return false;
	}

	if ( rec_maxNum <= 0 )
	{
		Log( "Nothing to be saved." );
		return false;
	}

	if ( sqlite3_open( REC_DB_NAME, &rec_db ) != SQLITE_OK )
	{
		Log( "SQLite - Error while connecting: %s", sqlite3_errmsg(rec_db) );
		sqlite3_close( rec_db );
		return false;
	}

	for ( int i = 0; i < 64; i++ ) // max default name
	{
		_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "route%i", i );
		ret_exists = sqliteDB_checkTableExists( rec_db, sql_cmd );
		// continue, if table already exists
		if ( ret_exists == 1 )
			continue;
		// quit function on fail
		if ( ret_exists == -1 )
		{
			sqlite3_close( rec_db );
			return false;
		}

		// create table with default name 'route..'
		_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "CREATE TABLE 'route%i'(", i );
		_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "%s 'index' INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," \
			"'maxNum' INTEGER NULL DEFAULT NULL," \
			"'angle1' REAL NOT NULL,'angle2' REAL NOT NULL,'angle3' REAL NOT NULL,'angle4' REAL NOT NULL," \
			"'angle5' REAL NOT NULL,'angle6' REAL NOT NULL,"\
			"'spin1' REAL NOT NULL,'spin2' REAL NOT NULL,'spin3' REAL NOT NULL," \
			"'speed1' REAL NOT NULL,'speed2' REAL NOT NULL,'speed3' REAL NOT NULL," \
			"'pos1' REAL NOT NULL,'pos2' REAL NOT NULL,'pos3' REAL NOT NULL);"
			, sql_cmd );
		sqlite3_exec( rec_db, sql_cmd, NULL, NULL, &errmsgs );
		if ( errmsgs != NULL )
		{
			Log( "SQLite - Error (executing CREATE TABLE statement): %s", errmsgs );
			sqlite3_close( rec_db );
			return false;
		}

		// add our data into the new table
		for ( int j = 0; j < rec_maxNum && j < (REC_ARRAYSIZE-1); j++ )
		{
			if ( j != 0 )
				_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "INSERT INTO 'route%i' VALUES( null, null,", i );
			else
				_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "INSERT INTO 'route%i' VALUES( null, %i,", i, rec_maxNum );
			_snprintf_s( sql_cmd, sizeof(sql_cmd)-1, "%s %0.2f, %0.2f, %0.2f, %0.2f,"
				"%0.2f, %0.2f," \
				"%0.2f, %0.2f, %0.2f," \
				"%0.2f, %0.2f, %0.2f," \
				"%0.2f, %0.2f, %0.2f" \
				");", 
				sql_cmd,
				rec_angle[j][0], rec_angle[j][1], rec_angle[j][2],
				rec_angle[j][3], rec_angle[j][4], rec_angle[j][5],
				rec_angle[j][6], rec_angle[j][7], rec_angle[j][8],
				rec_spin[j][0], rec_spin[j][1], rec_spin[j][2],
				rec_speed[j][0], rec_speed[j][1], rec_speed[j][2],
				rec_pos[j][0], rec_pos[j][1], rec_pos[j][2] );

			//Log( sql_cmd );
			sqlite3_exec( rec_db, sql_cmd, NULL, NULL, &errmsgs );
			if ( errmsgs != NULL )
			{
				Log( "SQLite - Error (executing INSERT INTO statement): %s", errmsgs );
				sqlite3_close( rec_db );
				return false;
			}
		}

		Log( "saved to 'route%i'", i );
		break;
	}

	sqlite3_close( rec_db );
	return true;
}

bool rec_sqlite_optimizeDatabase ()
{
	sqlite3 *rec_db;
	char *errmsgs;

	if ( sqlite3_open( REC_DB_NAME, &rec_db ) != SQLITE_OK )
	{
		Log( "SQLite - Error while connecting: %s", sqlite3_errmsg(rec_db) );
		sqlite3_close( rec_db );
		return false;
	}

	sqlite3_exec( rec_db, "VACUUM", NULL, NULL, &errmsgs );
	if ( errmsgs != NULL )
	{
		Log( "SQLite - Error (optimize): %s", sqlite3_errmsg(rec_db) );
		sqlite3_close( rec_db );
		return false;
	}

	Log( "Database has been optimized" );

	sqlite3_close( rec_db );
	return true;
}


void vehicle_playback_handler()
{
	if ( rec_state >= RECORDING_PLAY )
	{
		float set_speed[3];
		float set_spin[3];

		vect3_copy( rec_spin[rec_index], set_spin );
		vect3_copy( rec_speed[rec_index], set_speed );	

		// inverse speed/spin (for rev mode) and set rec_index
		if ( rec_state >= RECORDING_PLAY_REV )
		{
			vect3_mult( set_spin, -1.0f, set_spin );
			vect3_mult( set_speed, -1.0f, set_speed );
			rec_index--;

			// reached end of recording
			if ( rec_index <= 0 )
			{
				if ( !rec_continueAfterFin )
					rec_state = RECORDING_OFF;
				else
				{
					/*if ( set.recording_maxDistToEntryPoint > 0.0f )
					{
						// deactivate, if new starting position is too far from this point
						if ( vect3_dist(rec_pos[rec_index], rec_pos[(rec_maxNum-1)]) > set.recording_maxDistToEntryPoint )
							rec_state = RECORDING_OFF;
					}*/
					rec_index = (rec_maxNum-1);
				}
			}
		}
		else
		{
			rec_index++;

			// reached end of recording
			if ( (rec_index >= (REC_ARRAYSIZE-1) || rec_index >= rec_maxNum) )
			{
				if ( !rec_continueAfterFin )
					rec_state = RECORDING_OFF;
				else
				{
					/*if ( set.recording_maxDistToEntryPoint > 0.0f )
					{
						// deactivate, if new starting position is too far from this point
						if ( vect3_dist(rec_pos[rec_index], rec_pos[0]) > set.recording_maxDistToEntryPoint )
							rec_state = RECORDING_OFF;
					}*/
					rec_index = 0;
				}
			}
		}	

		INCAR_SYNC_DATA icSync;
		memset(&icSync, 0, sizeof(INCAR_SYNC_DATA));
		icSync.VehicleID = routeVehicleID;
		icSync.fCarHealth = 1000.00f,
		icSync.bytePlayerHealth = 100;

		GetOrthMatrix(
			rec_angle[rec_index][0], rec_angle[rec_index][1], rec_angle[rec_index][2],
			rec_angle[rec_index][3], rec_angle[rec_index][4], rec_angle[rec_index][5],
			rec_angle[rec_index][6], rec_angle[rec_index][7], rec_angle[rec_index][8],
			&icSync.fQuaternion[0], &icSync.fQuaternion[1], &icSync.fQuaternion[2], &icSync.fQuaternion[3]);		

		icSync.vecPos[0] = rec_pos[rec_index][0];
		icSync.vecPos[1] = rec_pos[rec_index][1];
		icSync.vecPos[2] = rec_pos[rec_index][2];

		icSync.vecMoveSpeed[0] = rec_speed[rec_index][0];
		icSync.vecMoveSpeed[1] = rec_speed[rec_index][1];
		icSync.vecMoveSpeed[2] = rec_speed[rec_index][2];

		SendInCarFullSyncData(&icSync, 0, -1);
	}
	else
	{
		rec_state = RECORDING_OFF;
	}
}