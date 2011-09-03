// MAIN STUFF /////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////

#define RAKSAMP_VERSION "v0.1-beta"

#define BP _asm int 3

#define MAX_PLAYERS 1000
#define MAX_VEHICLES 2000
#define MAX_SCRIPTS 32
typedef unsigned short PLAYERID;
typedef unsigned short VEHICLEID;

typedef signed char			int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed int			int32_t;
typedef unsigned int		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;
#define snprintf	_snprintf
#define vsnprintf	_vsnprintf
#define isfinite	_finite

#define EVENT_TYPE_PAINTJOB			1
#define EVENT_TYPE_CARCOMPONENT		2
#define EVENT_TYPE_CARCOLOR			3
#define EVENT_ENTEREXIT_MODSHOP		4

#define KEY_UP									-128
#define KEY_DOWN								128
#define KEY_LEFT								-128
#define KEY_RIGHT								128
#define KEY_ACTION								1
#define KEY_CROUCH								2
#define KEY_FIRE								4
#define KEY_SPRINT								8
#define KEY_SECONDARY_ATTACK					16
#define KEY_JUMP								32
#define KEY_LOOK_RIGHT							64
#define KEY_HANDBRAKE							128
#define KEY_LOOK_LEFT							256
#define KEY_SUBMISSION							512
#define KEY_LOOK_BEHIND							512
#define KEY_WALK								1024
#define KEY_ANALOG_UP							2048
#define KEY_ANALOG_DOWN							4096
#define KEY_ANALOG_LEFT							8192
#define KEY_ANALOG_RIGHT						16384

#pragma pack(1)
typedef struct _ONFOOT_SYNC_DATA
{
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	float vecPos[3];
	float fQuaternion[4];
	BYTE byteHealth;
	BYTE byteArmour;
	BYTE byteCurrentWeapon;
	BYTE byteSpecialAction;	
	float vecMoveSpeed[3];
	float vecSurfOffsets[3];
	WORD wSurfInfo;
	int	iCurrentAnimationID;
} ONFOOT_SYNC_DATA;

#pragma pack(1)
typedef struct _INCAR_SYNC_DATA
{
	VEHICLEID VehicleID;
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	float fQuaternion[4];
	float vecPos[3];
	float vecMoveSpeed[3];
	float fCarHealth;
	BYTE bytePlayerHealth;
	BYTE bytePlayerArmour;
	BYTE byteCurrentWeapon;
	BYTE byteSirenOn;
	BYTE byteLandingGearState;
	WORD TrailerID_or_ThrustAngle;
	FLOAT fTrainSpeed;
} INCAR_SYNC_DATA;

#pragma pack(1)
typedef struct _PASSENGER_SYNC_DATA
{
	VEHICLEID VehicleID;
	BYTE byteSeatFlags : 7;
	BYTE byteDriveBy : 1;
	BYTE byteCurrentWeapon;
	BYTE bytePlayerHealth;
	BYTE bytePlayerArmour;
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	float vecPos[3];
} PASSENGER_SYNC_DATA;

enum eWeaponState
{
	WS_NO_BULLETS = 0,
	WS_LAST_BULLET = 1,
	WS_MORE_BULLETS = 2,
	WS_RELOADING = 3,
};

#pragma pack(1)
typedef struct _AIM_SYNC_DATA
{
	BYTE byteCamMode;
	float vecAimf1[3];
	float vecAimPos[3];
	float fAimZ;
	BYTE byteCamExtZoom : 6;	// 0-63 normalized
	BYTE byteWeaponState : 2;	// see eWeaponState
	BYTE bUnk;
} AIM_SYNC_DATA;

#pragma pack(1)
typedef struct _UNOCCUPIED_SYNC_DATA // 67
{
	VEHICLEID VehicleID;
	short cvecRoll[3];
	short cvecDirection[3];
	BYTE unk[13];
	float vecPos[3];
	float vecMoveSpeed[3];
	float vecTurnSpeed[3];
	float fHealth;
} UNOCCUPIED_SYNC_DATA;

#pragma pack(1)
typedef struct _SPECTATOR_SYNC_DATA
{
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	float vecPos[3];
} SPECTATOR_SYNC_DATA;


struct stPlayerInfo
{
	int iIsConnected;
	int iIsStreamedIn;
	int iGotMarkersPos;
	char szPlayerName[20];
	int iScore;
	DWORD dwPing;
	int iAreWeInAVehicle;

	// STORED INFO
	ONFOOT_SYNC_DATA onfootData;
	INCAR_SYNC_DATA incarData;
	PASSENGER_SYNC_DATA passengerData;
	AIM_SYNC_DATA aimData;
	UNOCCUPIED_SYNC_DATA unocData;
};

// GTA STUFF //////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////

#define VEHICLE_LIST_ID_START		400
#define VEHICLE_LIST_SIZE			212

#define VEHICLE_CLASS_CAR			0
#define VEHICLE_CLASS_CAR_FAST		1
#define VEHICLE_CLASS_HEAVY			2
#define VEHICLE_CLASS_HELI			3
#define VEHICLE_CLASS_AIRPLANE		4
#define VEHICLE_CLASS_BIKE			5
#define VEHICLE_CLASS_BOAT			6
#define VEHICLE_CLASS_MINI			7
#define VEHICLE_CLASS_TRAILER		8
#define VEHICLE_CLASS_COUNT			9	/* # of classes */

struct vehicle_entry
{
	int			id;			// model id
	int			class_id;	// class id
	const char	*name;		// vehicle name
	int			passengers; // total passenger seats, 0-10
};

extern struct vehicle_entry vehicle_list[VEHICLE_LIST_SIZE];
