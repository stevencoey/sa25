#include "version.h"

#ifdef _DEATH_CONTEND

//�Ŷ���������
typedef struct 
{
	int use;
	char cdkey[64];
	char name[64];
}PkTeamMans;
//ս����¼
typedef struct 
{
	int use;
	int teamnum;
	int flg;	//0 1
}BattleHistorys;

#define MAXTEAMMANNUM 5		//�����������
#define MAXBATTLENUM 100	//�������ս����¼
#define MAXTEAMNUM 1000

#define DEFMAXBATTLENUM 50 //��߾�������
#define DEFWINSCORE 90
#define DEFLOSERATE 0.4
//������������info
typedef struct _tagPkTeamLists
{
	int use;					//flg
	int teamnum;				//�������
	char teamname[64];			//��������
	char pathdir[64];			//��������Ŀ¼
	char leadercdkey[64];		//�ӳ�CDKEY
	int win;					//ʤ
	int lost;					//��
	int battleplay;				//�ܳ���
	int score;
	int inside;					//¼ȡ���
	int read;
	PkTeamMans MyTeamMans[MAXTEAMMANNUM];
	BattleHistorys BHistory[MAXBATTLENUM];
}PkTeamLists;

#define MAXJOINTEAM 40
#define MAXWATCHMAP 16
typedef struct 
{
	int use;
	int teamnum;
	char cdkey[256];
	char name[256];
	int toindex;
	int fd;
}JoinTeamList;

typedef struct _tagPKProcedureRow
{
	int use;
	int time;
	int type;
	JoinTeamList Team[2];
}PKProcedures;

enum{
	PKTYPE_NONE=0,		//��
	PKTYPE_WAIT,		//�ȴ������������
	PKTYPE_STANDBY,		//׼����ս�ȴ�ʱ��
	PKTYPE_PK,			//��ս��
};

void del_rn( char *s );
void PKLIST_ResetOneTeamMan( int ti ); //���ö�Ա����
void PKLIST_ResetOneBHistory( int ti ); //���ö�ս����
void PKLIST_ResetOnePkTeamList( int ti ); //���ò�����������
int PKLIST_InitPkTeamList( int teamnum ); //����

int PKLIST_GetPkTeamListArray( int teamnum, char *cdkey);
int PKLIST_GetPkTeamListArrayFromNum( int teamnum);
//ȷ���ظ�Լս
BOOL PKLIST_CHECKPkTeamSame( int teamnum, int charaindex, char *cdkey, int toteamnum );
//ȷ�϶�ս����
int PKLIST_CHECKPkTeamNew( int teamnum, int charaindex, char *cdkey );

//LOAD DATA
BOOL PKLIST_LoadPkTeamListDataSub( int ti, char *data);
BOOL PKLIST_LoadPkTeamListDataMyTeamMans( int ti, char *data);
BOOL PKLIST_LoadPkTeamListDataBHistory( int ti, char *data);
//BOOL PKLIST_LoadPkTeamListData( char *data); //����ac������ pklist
BOOL PKLIST_LoadPkTeamListData(void); // �ĳɶ���
void PKLIST_SavePkTeamListData(void);
void PKLIST_LoadInitPkTeamListData(void);	// ��ȡ��ԭʼ�Ĳ�������
void PKLIST_UpData(char *mycdkey,char *tocdkey,int menum,int tonum,int winer,int flg);
int PKLIST_GetOneBHistory( int ti );
int PKLIST_SetOneBHistory( int ti, int hi, int use, int teamnum, int flg );
int PKLIST_UpdateOnePkTeamData( int ti, int forti, int winerflg);

void PKLIST_InsertTeamNum( int charaindex );
BOOL PKLIST_GetTeamLeaderCdkey( int teamnum, char *buf);
void PKLIST_ShowPkListTeamData( void);

int NPC_PKLIST_Finish_Exit( int menum, int tonum, int winside, int battlemap);


BOOL PKLIST_HandleChartsMess( int fd, char *data, int type, int flg);
BOOL PKLIST_GetChartsListData( int ti, char *data, int sizes );
BOOL PKLIST_GetMyPKListTeamData( int teamnum, char *data, int sizes );


/*
BOOL PKLIST_CheckPKSameTeam( int charaindex );
BOOL PKLIST_JoinPKProcedures( int charaindex );
void PKLIST_DelPKProcedures( int ti, int side );
void PKLIST_CheckPKProcedures( void );

void PKLIST_CheckPKProcedures_PKTYPEWAIT( int ti);
void PKLIST_CheckPKProcedures_PKTYPESTANDBY( int ti);
*/

//����
void PKLIST_DelPKProcedures( int ti, int side, int type);
BOOL PKLIST_CheckPklistInServerMap( int ti, int side);
BOOL PKLIST_CheckPKSameTeam( int charaindex );
BOOL PKLIST_CheckPKReapetTeam( int menum, int tonum);
BOOL PKLIST_JoinPKProcedures( int charaindex );
void PKLIST_CheckTeamBeEnable( void);
void PKLIST_warp( int ti, int side, int fl, int x, int y );
void NPC_PKLIST_PlayerLogout_Exit( int charaindex );
int PKLIST_GetPKProcedureArray( int menum );

BOOL PKLIST_CheckLOCKTeam( int menum);
void PKLIST_LOCKTeam( int menum);
void PKLIST_UNLOCKTeam( int menum);
void PKLIST_Sort_PKListSort( void);

//��ʽ��
typedef struct _tagArrangeBattle
{
	int use;
	int fl;
	int code;
	int teamnum;
	int type; //0 NULL // 1 in battle
	int time;
	char teamname[256];
	int toindex;

	struct _tagArrangeBattle *next[2];
	struct _tagArrangeBattle *top;
}ArrangeBattleC;

#define MAXBAFLOOR 20
#define MAXNOWBATTLE 128

#define MAXBAHEAD 16
#define MAXBATTLEPAGE MAXJOINTEAM

void ABATTLE_InitABattle( int maxnums );
int ABATTLE_CreateNet( ArrangeBattleC *now, int ti, int fl, int maxfl);
void ABATTLE_ShowNet( ArrangeBattleC *now, int fl);
void ABATTLE_ShowBattlefromFl( int ti, int fl);
ArrangeBattleC *ArrangeBattleC_getNew( void);


BOOL ABATTLE_InsertBattle( ArrangeBattleC *aB); //��������
void ABATTLE_EnterBattle( ArrangeBattleC *aB); //��Χ
void ABATTLE_EliminateBattlefromFl( ArrangeBattleC *aB);//�޳�

BOOL ABATTLE_CheckInABattle( int ti);//ȷ������ս��״̬ ����ʱ��
int ABATTLE_FindBattlefromFl( int ti, int fl); //��Ѱ�ɼ������̶������

void ABATTLE_CheckBattlefromFl(int charindex, int ti,int battleindex); //ȷ�ϲ���Ƿ���� �� ��������
int ABATTLE_CheckBattlefromFl_sub(int charindex, int ti, int fl,int battleindex); //ȷ�ϲ���Ƿ����


ArrangeBattleC *ABATTLE_getInBattle( int teamnum); //ȡ������head form teamnum

void ABATTLE_MakeInABattleString( void); //���������ִ�

BOOL PKLIST_GetABattlelistDataString( int ti, int *tindex, int *stime,
									 char *buf1, char *buf2, char *buf3, int flg);//ȡ�������ִ�

ArrangeBattleC *ArrangeBattleC_getInBattleArray( int ti);




void ABATTLE_RecordBattle( int ti, char *buf1, char *tstr1,char *buf2, char *tstr2);
void remove_r( char *s );
void ABATTLE_GetRecordBattle( void);

#endif
