#include "version.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


#include "init.h"
#include "net.h"
#include "handletime.h"
#include "char.h"
#include "map_warppoint.h"
#include "npcgen.h"
#include "log.h"
#include "configfile.h"
#include "battle.h"
#include "petmail.h"
#include "autil.h"
#include "family.h"
#include "defend.h"
#include "trade.h"

#ifdef _NPCSERVER_NEW
#include "npcserver.h"
#endif

#ifdef _RECAL_SEND_COUNT		// WON ����GS��Ѷ 
#include "mclient.h"
#endif

#ifdef _DEATH_CONTEND
#include "deathcontend.h"
#endif
#ifdef _CHATROOMPROTOCOL			// (���ɿ�) Syu ADD ������Ƶ��
#include "chatroom.h"
#endif

static void ShutdownProc( void);
void mainloop( void );
struct  tm tmNow, tmOld;
void family_proc();
// Terry add 2001/10/11
#ifdef _SERVICE
extern int g_nServiceSocket;
#endif

#ifdef _CHANNEL_MODIFY
extern int InitOccChannel(void);
#endif

#ifdef _ANGEL_SUMMON
#define ANGELTIMELIMIT	3*24*60*60 // �������ʱ��(��)
int AngelReady =0;
int AngelTimelimit = ANGELTIMELIMIT;
time_t AngelNextTime;
void AngelReadyProc();
#endif

#include "genver.h"

void warplog_proc();

int main( int argc , char** argv, char** env )
{
    /*  ��Իؤ��������ëɬ���ƻ�����    */
    setNewTime();

    if ( argc > 1 && 0==strcmp(argv[1],"-v"))
    {
        printf("%s",genver);
        exit(0);
    }
    else fprintf(stderr,"%s",genver);

    EXITWITHEXITCODEIFFALSE( util_Init() , 1);

    LoadAnnounce();	// Arminius 7.12 loginannounce

	/* ����ɬ�� */
	memcpy( &tmOld, localtime( (time_t *)&NowTime.tv_sec), sizeof( tmNow ) );

    EXITWITHEXITCODEIFFALSE( init(argc , argv ,env ) , 1);

    LoadPetTalk();	// Arminius 8.14 pet talk

#ifdef _GAMBLE_BANK
	Load_GambleBankItems();
#endif

#ifdef _CFREE_petskill
	Load_PetSkillCodes();
#endif
#ifdef _ITEM_PILEFORTRADE
	TRADE_InitTradeList();
#endif

#ifdef _DEATH_CONTEND
	if( PKLIST_InitPkTeamList( MAXTEAMNUM ) == -1 ) return 1;
#endif

#if USE_MTIO
    /* Ѩ�������������巴���ƥ���� */    
    if( MTIO_setup() < 0 ){
        print( "cannot setup MT environment\n" );
        return 1;
    }

    /* ���ƥ join �ƻ���   */
    MTIO_join();
#else
    mainloop();
#endif
    
    return 0;
}

void mainloop( void )
{
    NPC_generateLoop( 1 );
    signal(SIGUSR1,sigusr1);
    signal(SIGUSR2,sigusr2);
#ifdef _MAP_WARPPOINT
	MAPPOINT_InitMapWarpPoint();
	if( !MAPPOINT_loadMapWarpPoint() ){
		return;
	}
#endif

#ifdef _ASSESS_SYSEFFICACY
	Assess_InitSysEfficacy();
#endif

#ifdef _CHECK_BATTLETIME
	check_battle_com_init();
#endif

#ifdef _CHATROOMPROTOCOL			// (���ɿ�) Syu ADD ������Ƶ��
	InitChatRoom();
#endif

#ifdef _CHANNEL_MODIFY
	if(!InitOccChannel()) return;			// ��ʼ��ְҵƵ��
#endif

#ifdef _GM_BROADCAST					// WON ADD �ͷ�����ϵͳ
	Init_GM_BROADCAST( 0, 0, 0, "" );
#endif

#ifdef _DEATH_FAMILY_STRUCT		// WON ADD ����ս���ʤ������
	Init_FM_PK_STRUCT();			
#endif

#ifdef _ANGEL_SUMMON
	AngelReady = 0;
	AngelNextTime = time(NULL) + 1*60;
#endif

//#ifdef _ALLDOMAN
//	InitHeroList();
//#endif
    while(1){
#ifdef _ASSESS_SYSEFFICACY
		Assess_SysEfficacy( 0 );
#endif
        setNewTime();
        memcpy(&tmNow, localtime( (time_t *)&NowTime.tv_sec),
               sizeof( tmNow ) );
		if( tmOld.tm_hour != getLogHour( ) && tmNow.tm_hour == getLogHour( ) ){
			backupAllLogFile( &tmOld );
		}
        setNewTime();

#ifdef _ASSESS_SYSEFFICACY_SUB //��ʾLOOPʱ��
Assess_SysEfficacy_sub( 0, 1);
        netloop_faster();
Assess_SysEfficacy_sub( 1, 1);

//Assess_SysEfficacy_sub( 0, 2);
        NPC_generateLoop( 0 );
//Assess_SysEfficacy_sub( 1, 2);

Assess_SysEfficacy_sub( 0, 3);
        BATTLE_Loop();
Assess_SysEfficacy_sub( 1, 3);

Assess_SysEfficacy_sub( 0, 4);
        CHAR_Loop();
Assess_SysEfficacy_sub( 1, 4);

//Assess_SysEfficacy_sub( 0, 5);
        PETMAIL_proc();
//Assess_SysEfficacy_sub( 1, 5);

//Assess_SysEfficacy_sub( 0, 6);
        family_proc();
//Assess_SysEfficacy_sub( 1, 6);

//Assess_SysEfficacy_sub( 0, 7);
        chardatasavecheck();
//Assess_SysEfficacy_sub( 1, 7);
#ifdef _GM_BROADCAST					// WON ADD �ͷ�����ϵͳ
//Assess_SysEfficacy_sub( 0, 8);
		GM_BROADCAST();
//Assess_SysEfficacy_sub( 1, 8);
#endif

#else	//����ʾLOOPʱ��
        netloop_faster();
        NPC_generateLoop( 0 );
        BATTLE_Loop();
        CHAR_Loop();
        PETMAIL_proc();
        family_proc();
        chardatasavecheck();
#ifdef _GM_BROADCAST					// WON ADD �ͷ�����ϵͳ
		GM_BROADCAST();
#endif
#endif

#ifdef _ANGEL_SUMMON
		AngelReadyProc();
#endif

		if( tmOld.tm_sec != tmNow.tm_sec ) {
			CHAR_checkEffectLoop();
		}
        if( SERVSTATE_getShutdown()> 0 ) {
            ShutdownProc();
        }

		tmOld = tmNow;
#ifdef _ASSESS_SYSEFFICACY
		Assess_SysEfficacy( 1);
#endif
    }
#ifdef _SERVICE
    close(g_nServiceSocket);
#endif    
}

static void sendmsg_toall( char *msg )
{
    int     i;
    int     playernum = CHAR_getPlayerMaxNum();

    for( i = 0 ; i < playernum ; i++) {
        if( CHAR_getCharUse(i) != FALSE ) {
			CHAR_talkToCli( i, -1, msg, CHAR_COLORYELLOW);
		}
	}
}
static void ShutdownProc( void)
{
#define		SYSINFO_SHUTDOWN_MSG		"�ٹ� %d �����ᣬ����ʼ����serverϵͳά�������λ��ǰ�ǳ������⵵���𻵣�"
#define		SYSINFO_SHUTDOWN_MSG_COMP	"server�ѹرա�"
	int diff,hun;

	diff = NowTime.tv_sec - SERVSTATE_getShutdown();
	hun = SERVSTATE_getLimittime() - (diff/60);
	if( hun != SERVSTATE_getDsptime() ){
		char	buff[256];
		if( hun != 0 ) {
			snprintf( buff, sizeof(buff), SYSINFO_SHUTDOWN_MSG, hun);
		}
		else {
			strcpy( buff, SYSINFO_SHUTDOWN_MSG_COMP);
		}
		sendmsg_toall( buff);
		SERVSTATE_setDsptime(hun);
		if( hun == 1 ) {
		    SERVSTATE_SetAcceptMore( 0 );
		}
	}
	/* closesallsockets */
	if( hun == 0) {
    	closeAllConnectionandSaveData();
		SERVSTATE_setShutdown(0);
		SERVSTATE_setDsptime(0);
		SERVSTATE_setLimittime(0);
#ifdef _KILL_12_STOP_GMSV      // WON ADD ��sigusr2��ر�GMSV
		//andy_reEdit 2003/04/28��׼��...
//		system("./stop.sh"); 
#endif
	}
	
}

void family_proc()
{
	static	unsigned long gettime = 0;
	static  unsigned long checktime = 0;
	static  unsigned long proctime = 0;

#ifdef _CK_ONLINE_PLAYER_COUNT    // WON ADD ������������	
	static	unsigned long player_count_time = 0;
    int PLAYER_COUNT_TIME = 60*5;	  // 30�봫һ�������� AC
#endif

#ifdef _RECAL_SEND_COUNT		// WON ����GS��Ѷ 
	static	unsigned long recal_count_time = 0;
    int RECAL_COUNT_TIME = 60;	  
	if( (unsigned long)NowTime.tv_sec > recal_count_time  ){
		recal_get_count();
#ifdef _GSERVER_RUNTIME //����GSERVERִ�ж���ʱ���MSERVER
	    gserver_runtime();
#endif
		recal_count_time = (unsigned long)NowTime.tv_sec + RECAL_COUNT_TIME;
	}
#endif

	if( time(NULL) < proctime ) return;
	proctime = time(NULL)+5;

	if( (unsigned long)NowTime.tv_sec > gettime ){
		getNewFMList();
		gettime = (unsigned long)NowTime.tv_sec + 60*10;
	}

	if( (unsigned long)NowTime.tv_sec > checktime ){
		checkFamilyIndex();
#ifdef _ADD_FAMILY_TAX			   // WON ADD ����ׯ԰˰��	
		GS_ASK_TAX();			   // GS �� AC Ҫ��ׯ԰˰��
#endif
		checktime = (unsigned long)NowTime.tv_sec + 60*30;
	}

#ifdef _CK_ONLINE_PLAYER_COUNT    // WON ADD ������������
	if( (unsigned long)NowTime.tv_sec > player_count_time  ){
		GS_SEND_PLAYER_COUNT();
		player_count_time = (unsigned long)NowTime.tv_sec + PLAYER_COUNT_TIME;
	}
#endif
}

void warplog_proc()
{
	static  unsigned long checktime = 0;
	
	if( (unsigned long)NowTime.tv_sec > checktime ){
		warplog_to_file();
		checktime = (unsigned long)NowTime.tv_sec + 300;
	}
}

#ifdef _ANGEL_SUMMON

extern int player_online;

void AngelReadyProc()
{
	//static time_t lastCreateTime = time(NULL);
	time_t nowTime;
	//static unsigned long AngelNextTime = 30*60;
	struct tm *temptime;
	char msg[1024];

	nowTime = time(NULL);

	if( nowTime < AngelNextTime )
		return;

	if( player_online <= 10 )
	{
		//print(" ANGEL:������������=%d ", player_online);
		return;
	}

	AngelReady = 1;
	//AngelNextTime = min( (int)(10000/player_online), 100)*60 + (unsigned long)nowTime;
	AngelNextTime = min( (int)(5000/player_online), 100)*60 + (unsigned long)nowTime;

	temptime = localtime( &AngelNextTime );
	sprintf( msg, " ANGEL:����һλȱ��  �´β���ʱ��=(%d/%d %d:%d) Ŀǰ����=%d ",
		temptime->tm_mon+1, temptime->tm_mday, temptime->tm_hour, temptime->tm_min, player_online );
	print( msg);
	//LogAngel( msg);
	
}

#endif
