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

	int updata;					//����ʱ��

	PkTeamMans MyTeamMans[MAXTEAMMANNUM];
	BattleHistorys BHistory[MAXBATTLENUM];
}PkTeamLists;

//��-------------------------------------------------------
int PKLIST_getStringPkTeamData( int ti, char *data, int sizes);
int PKLIST_SetOneBHistory( int ti, int hi, int use, int teamnum, int flg );
int PKLIST_GetOneBHistory( int ti );
void PKLIST_ResetOneTeamMan( int ti );
void PKLIST_ResetOneBHistory( int ti );
void PKLIST_ResetOnePkTeamList( int ti );
int PKLIST_UpdateOnePkTeamData( int ti, int forti, int winerflg);
//-----------------------------------------------------------
int PKLIST_InitPkTeamList( int maxteam );
void PKLIST_UpdatePkTeamList( int fd, int menum, char *mecdkey, int tonum, char *tocdkey, int winer, int flg);
void PKLIST_GetPkTeamListData( int fd, int startTi, int count);
void PKLIST_GetPkTeamListDataOne( int fd, int ti);
//-----------------------------------------------------------
int PKLIST_LoadPkTeamListfromFile( char *dirpath, char *listfilename );
int PKLIST_BackupPkTeamListfromFile( char *dirpath, char *listfilename );
void PKLIST_SortTheCharts( int ti);//���а���
void PKLIST_TheCharts( int fd, int type, int flg);

int PKLIST_LoadUserfromFile( char *dirpath, char *listfilename );
int PKLIST_CutStartUserfromFile( char *dirpath, char *listfilename );

void PKLIST_CheckRepeatBattleHistory( void);
int PKLIST_alteUserfromFile( char *dirpath, char *listfilename );
void PKLIST_Sort_PKListSort( void);

void PKLIST_CheckAllUser( void);

#endif
