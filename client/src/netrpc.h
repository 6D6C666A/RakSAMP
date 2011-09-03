#define REJECT_REASON_BAD_VERSION 1
#define REJECT_REASON_BAD_NICKNAME 2
#define REJECT_REASON_BAD_MOD 3
#define REJECT_REASON_BAD_PLAYERID 4

extern int iNetModeNormalOnfootSendRate, iNetModeNormalIncarSendRate, iNetModeFiringSendRate, iNetModeSendMultiplier;
extern char g_szHostName[256];

#pragma pack(1)
typedef struct _NEW_VEHICLE {
	VEHICLEID VehicleId;
	int		  iVehicleType;
	float	  vecPos[3];
	float	  fRotation;
	char	  aColor1;
	char	  aColor2;
	float	  fHealth;
	BYTE	  byteInterior;
	BYTE	  byteDoorsLocked;
	DWORD	  dwDoorDamageStatus;
	DWORD	  dwPanelDamageStatus;
	BYTE	  byteLightDamageStatus;
} NEW_VEHICLE;

#define MAX_MENU_ITEMS 12
#define MAX_MENU_LINE 32
#define MAX_COLUMNS 2
struct MENU_INT
{
	BOOL bMenu;
	BOOL bRow[MAX_MENU_ITEMS];
	BOOL bPadding[8 - ((MAX_MENU_ITEMS + 1) % 8)]; 
};
struct stGTAMenu
{
	char szTitle[MAX_MENU_LINE];
	char szSeparator[MAX_MENU_LINE]; // ???
	BYTE byteColCount;
	char szColumnContent[MAX_COLUMNS][MAX_MENU_LINE];
};

#define IDB_BUTTON1				10
#define IDB_BUTTON2				11
#define IDE_INPUTEDIT			12
#define IDL_LISTBOX				13

#define DIALOG_STYLE_MSGBOX		0
#define DIALOG_STYLE_INPUT		1
#define DIALOG_STYLE_LIST		2

struct stSAMPDialog
{
	int iIsActive;
	BYTE bDialogStyle;
	WORD wDialogID;
	BYTE bTitleLength;
	char szTitle[257];
	BYTE bButton1Len;
	char szButton1[257];
	BYTE bButton2Len;
	char szButton2[257];
	char szInfo[257];
};
extern struct stSAMPDialog sampDialog;

extern int iGettingNewName;

void RegisterRPCs(RakClientInterface *pRakClient);
void UnRegisterRPCs(RakClientInterface * pRakClient);
