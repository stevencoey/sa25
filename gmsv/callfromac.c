#include <stdio.h>
#include "version.h"
#include "common.h"
#include "saacproto_cli.h"
#include "lssproto_serv.h"
#include "net.h"
#include "char.h"
#include "object.h"
#include "readmap.h"
#include "msignal.h"
#include "map_deal.h"
#include "npc_duelranking.h"
#include "npc_warpman.h"
#include "handletime.h"
#include "npc_scheduleman.h"
#include "configfile.h"
#include "chatroom.h"
#ifdef _PAUCTION_MAN
#include "npc_pauctionman.h"
#endif
#include "npc_eventaction.h"
// CoolFish: Family 2001/5/24
#include "family.h"
#include "defend.h"
#include "log.h"
#include "buf.h"
#ifdef _ALLDOMAN // (���ɿ�) Syu ADD ���а�NPC
#include "npc_alldoman.h"
#endif

#ifdef _RACEMAN
#include "npc_raceman.h"
#endif

#ifdef _DEATH_CONTEND
#include "deathcontend.h"
#endif
time_t initTime =0;

extern BOOL bNewServer;

extern int i_shutdown_time;//ttom

void saacproto_ACGmsvDownRequest_recv( int fd, int min)
{
	char	buff[256];
    int     i;
    int     playernum = CHAR_getPlayerMaxNum();

	snprintf( buff, sizeof( buff),"ϵͳ����!");
    for( i = 0 ; i < playernum ; i++) {
        if( CHAR_getCharUse(i) != FALSE ) {
			CHAR_talkToCli( i, -1, buff, CHAR_COLORYELLOW);
		}
	}
	SERVSTATE_setLimittime(min);
	SERVSTATE_setShutdown( NowTime.tv_sec );
	SERVSTATE_setDsptime( 0 );
	
}
//����Ƿ��к�ac����,�Լ��������
void saacproto_ACServerLogin_recv (int fd, char* result,char* data)
{
    if( strcmp( result , SUCCESSFUL ) != 0 ){
        /*  ����̼���      */
        print( "saacproto_ACServerLogin fail! so quitting...[%s][%s]\n",
              result,data );
        shutdownProgram( );
        exit(1);
    }
    print("ac login ok\n");
	time(&initTime);
    if( bNewServer )
    	print(" New Server! ");
    else
    	print(" Old Server! ");
#ifdef _SERVER_NUMS
		print( " version : <%s>" , SERVER_VERSION);
#endif
	print(" NORMAL_SERVER! ");

    // Robin 0530
    //saacproto_ACShowFMList_send( acfd );
    FAMILY_Init();
#ifdef _UNIVERSE_CHATROOM
	CHATROOM_getChatRoomList();
#endif

#ifdef _ANGEL_SUMMON
	saacproto_ACMissionTable_send( acfd, -1, 1, "", "");
#endif
}

#ifdef _ACNT_LOGIN
void saacproto_ACQueryAccount_recv (int fd, char *result, char *data, int retfd)
{
	int clifd = getfdFromFdid (retfd);
	if (CONNECT_checkfd (clifd) == FALSE)
		return;
	lssproto_ClientLogin_send (clifd, result);
}
#endif

void saacproto_ACCharList_recv(int fd, char* result, char* data,int retfd)
{
    int clifd = getfdFromFdid(retfd);


    if( CONNECT_checkfd(clifd) == FALSE ){
		return;
	}
	lssproto_CharList_send( clifd, result , data );
    CONNECT_setState( clifd, NOTLOGIN );
}

void saacproto_ACCharLoad_recv(int fd , char* result,char* data,int retfd,int saveindex)
{
    int clifd = getfdFromFdid(retfd);
    if( CONNECT_checkfd( clifd ) == FALSE )return;

    if(( strcmp( result , SUCCESSFUL ) == 0 )&&(data[0])) {
#ifdef _BAD_PLAYER             // WON ADD �ͻ����ȥ��
        CHAR_login( clifd, data, saveindex, 0 );		// �������
	}else if( ( strcmp( result , BADPLAYER ) == 0 ) && (data[0]) ){
		CHAR_login( clifd, data, saveindex, 1 );		// �����
#else		
        CHAR_login( clifd,data,saveindex );
#endif

    }else{
		char cdkey[64];
#ifndef _FIX_MESSAGE             // WON ADD �޸�WORKER�������        
		data = "";		
#endif
		CONNECT_getCdkey( clifd, cdkey , sizeof( cdkey ));
		print(" (%s)ACCharLoad����:%s ", cdkey, data);
        lssproto_CharLogin_send( clifd, result ,data );
        CONNECT_setState( clifd, NOTLOGIN );
    }
}

#ifdef _DEATH_CONTEND
void saacproto_LoadPkTeamListData_recv(int fd , char* result, char* data, int ti ) 
{
	//andy_log
	print("LoadPkTeamListData_recv( %s, %d) data len:%d\n", result, ti, strlen(data) );
	if( !strcmp( result, "success") ){
		if( PKLIST_LoadPkTeamListData() == FALSE ) return;
		if( ti != - 1 )
			saacproto_LoadPkTeamListData_send( fd , ti,  1000);
	}
}
//Ҫ����Ѷ
void saacproto_PKLISTCHARTS_recv( int fd, char *data, int type, int flg)
{
	//andy_log
	print("PKLISTCHARTS_recv() \n");
	if( data == NULL ) return;
	PKLIST_HandleChartsMess( fd, data, type, flg);
}

#endif

#ifdef _PAUCTION_MAN
void saacproto_ACItemAuction_recv( int fd, char *ITEMNAME, char *data, int itemID, int ret, int flg)
{
	//��Ʒ����
	switch( flg)	{
	case 1:
		{
			int itemindex=-1;
			itemindex = NPC_PAItemShop_AddItem( itemID, ret);
			if( !ITEM_CHECKINDEX( itemindex) )
				return;
			NPC_PAItemShop_reItem( itemindex, data);
		}
		break;
	case 2:
		{
			NPC_PAItemShop_DelItem( ret, -1);
		}
		break;
	}
}
#endif

/*
 *����Ƥ����������߯�ϣ�
 * state�巽�Ȼ���  ë������
 */
 
void saacproto_ACCharSave_recv(int fd ,char* result, char* data,int retfd)
{
    int clifd = getfdFromFdid(retfd) ,  fdid;
    char cdkey[CDKEYLEN], passwd[PASSWDLEN], charname[CHARNAMELEN];
    /*
     *   �˾޷¡�ƥ��ئ�м�����ƥ��Ԫ��ľ���Ƿ�ؤ��
     * ��ľ��������ئԻ����ë������ݷ֣�
     */
    if( CONNECT_checkfd( clifd )== FALSE ) return;

    CONNECT_getCdkey( clifd, cdkey, sizeof( cdkey ));
    CONNECT_getPasswd( clifd, passwd, sizeof( passwd));

    CONNECT_getCharname( clifd, charname, sizeof( charname ));

    fdid = CONNECT_getFdid( clifd );
    switch( CONNECT_getState( clifd ) ){
    case WHILECREATE:
        /* ƽ�ҷ���Իƥ������Ƥ   */
        /* ��ľë�ͷ���ʧ�������е�( �������� )*/
        if( strcmp(result,FAILED) ==0 ) data ="";
        lssproto_CreateNewChar_send( clifd , result , data);
        CONNECT_setState( clifd, NOTLOGIN );
        break;

    case WHILELOGOUTSAVE:
        /*   ɧ�����ʧ�������ݼ�����Ƥ��߯�� */
        if( strcmp( result , SUCCESSFUL ) == 0 )
            /* ��   */
            lssproto_CharLogout_send( clifd, result , "success" );
        else
            /* �޷¡�֧����������   */
            lssproto_CharLogout_send( clifd, result , "Cannot save" );
        CONNECT_setState( clifd, NOTLOGIN );
        CONNECT_setCharaindex( clifd, -1 );
        break;

    case WHILELOSTCHARSAVE:
        /*  ��������ƥؤľ����ƽ�ҷ·���������  */
        saacproto_ACCharDelete_send( acfd, cdkey,passwd,charname,"",fdid );
        CONNECT_setState( clifd, WHILELOSTCHARDELETE );
        CONNECT_setCharaindex( clifd, -1 );
        break;

    case WHILECLOSEALLSOCKETSSAVE:
        CONNECT_setState( clifd, NOTLOGIN );
        // Arminius debug
        // CONNECT_endOne_debug(clifd);
        CONNECT_setUse( clifd , FALSE);
        
        SERVSTATE_decrementCloseallsocketnum();
        if( SERVSTATE_getCloseallsocketnum() == 0 ) {
		    SERVSTATE_SetAcceptMore( -1 );
            print( "All character saved\n" );
            print("\n the shutdown time =%d",i_shutdown_time);//ttom
            if(i_shutdown_time==1){
		    system( "touch normalstop" );
			sigshutdown( 0 );
	    }		
	}
    default:
        break;
    }
}


void saacproto_ACCharDelete_recv(int fd,char* result,char* data,int retfd)
{
    int clifd = getfdFromFdid(retfd);
    if( CONNECT_checkfd( clifd ) == FALSE )return;

    switch( CONNECT_getState(clifd) ){
    case WHILELOSTCHARDELETE:
        /*  ����ƥƽ�ҷ�����  ��߯��    */
        /*  ������ئ��  */
        /*  ����ë  ��  */
        CONNECT_setState(clifd, NOTLOGIN );
        CONNECT_setCharaindex( clifd, -1 );
        break;
    case WHILECHARDELETE:
        /*    ɧ��ƽ�ҷ�����    */
        /*  ��������˪��  */
        if( strcmp(result,FAILED) == 0)data="";
        lssproto_CharDelete_send( clifd, result , data );
        CONNECT_setState( clifd,  NOTLOGIN );
        CONNECT_setCharaindex( clifd, -1 );
        break;
    default:
        break;
    }
}

/*
 *  �����ͼ�߯�ϣ۳�ľ��ݩ�ֱ��ƾ�����ئ�У��ػ���إ����ľ��  �÷�ئ��
 */
void saacproto_ACLock_recv(int fd , char* result, char* data ,int retfd)
{
    int clifd = getfdFromFdid(retfd);
    char cdkey[CDKEYLEN];
    int cindex = getCharindexFromFdid(retfd);

    if( CONNECT_checkfd(clifd) == FALSE) return;

    // Arminius 7.25 GM unlock test
    if (strncmp(data, "USRLOCKED", 9)==0) {
      char buf[4096];
      sprintf(buf, "%s", &data[10]);
      CHAR_talkToCli(cindex, -1, buf, CHAR_COLORYELLOW);
      return;
    } else if (strncmp(data, "USRUNLOCKED", 11)==0) {
      CHAR_talkToCli(cindex, -1, "User not locked.", CHAR_COLORYELLOW);
      return;
    } else if (strncmp(data, "GMUNLOCKSUCCESS", 15)==0) {
      CHAR_talkToCli(cindex, -1, "User unlock success", CHAR_COLORYELLOW);
      return;
    } else if (strncmp(data, "GMUNLOCKFAIL", 12)==0) {
      CHAR_talkToCli(cindex, -1, "User unlock failed", CHAR_COLORYELLOW);
      return;
    } else if (strncmp(data, "GMUNLOCKALL", 11)==0) {
      CHAR_talkToCli(cindex, -1, "Server unlocked", CHAR_COLORYELLOW);
      return;
    }

    CONNECT_getCdkey( clifd, cdkey , sizeof( cdkey ));
    
    switch( CONNECT_getState( clifd ) ){
    case WHILECANNOTLOGIN:
        /* ����̼���  �����ݼ������ͼ�߯��   */
        if( strcmp( result , SUCCESSFUL ) == 0 ){
            /* ��  ƥ�����ƻ���ئ�����ر����� */
            CONNECT_setState( clifd, NOTLOGIN );

        }else{
            /*
             * ����������ëέ��
             */
            char    mesg[128];
            snprintf( mesg ,sizeof( mesg ), "%s hasn't been locked" ,cdkey);
            if( strcmp( data , mesg ) == 0 ){
                /*
                 * �����ͽ�ľ����ئ�м�ƥ�ֵ�������ئ�У�
                 * ��  ���׼���  Ԫ���б����£�
                 */
                CONNECT_setState( clifd, NOTLOGIN );
            }else{
                /*
                 * ��պ������ݩ���Ʒ���������
                 * state ��  ��ئ�ƣ�
                 */
                saacproto_ACLock_send(fd, cdkey ,
                                      UNLOCK, CONNECT_getFdid( clifd) );
            }
        }

        break;
    default:
        break;
    }
}

void saacproto_ACUCheck_recv( int fd , char *cd )
{
    int conind;
    int flag = 0;
    conind = getfdFromCdkeyWithLogin( cd );
    flag = ( conind >= 0 ) ? 1 : 0;
    saacproto_ACUCheck_send( fd , cd , flag );
}


/* ---------------------------------------------
 * ������ì������ޥ������Ի��    �Ѱ�  
 * --------------------------------------------*/
void saacproto_DBUpdateEntryInt_recv( int fd,char* result,
									char* table,char* key,int msgid,int msgid2 )
{
	if( strcmp( result, NET_STRING_FAILED) == 0 ) {
		print( "failed: DBUpdateEntryInt\n");
		return;
	}
}
/* ---------------------------------------------
 * ������ì������ޥ������Ի��      ٯ    
 * --------------------------------------------*/
void saacproto_DBUpdateEntryString_recv( int fd,char* result,
									char* table,char* key,int msgid,int msgid2 )
{
	if( strcmp( result, NET_STRING_FAILED) == 0 ) {
			print( "failed: DBUpdateEntryString\n");
		return;
	}
}

/* ---------------------------------------------
 * ������ì����������Ի��    �¼���  
 * --------------------------------------------*/
void saacproto_DBGetEntryRank_recv( int fd,char* result,int rank,int count,
									char* table,char* key,int msgid,int msgid2 )
{
	if( strcmp( result, NET_STRING_FAILED) == 0 ) {
		print( "failed: DBGetEntryRank\n");
		return;
	}
	if( strcmp( table, DB_DUELPOINT) == 0 ) {
	    if( CHAR_getInt( OBJECT_getIndex( msgid2), CHAR_WHICHTYPE)	== CHAR_TYPEDUELRANKING){
			NPC_Duelranking_CallMyRanking( count, msgid, msgid2);
		}else {
			NPC_GetDuelRank( rank, msgid, msgid2);
		}
	}
}

void saacproto_DBDeleteEntryInt_recv( int fd,char* result,
									char* table,char* key,int msgid,int msgid2 )
{
	if( strcmp( result, NET_STRING_FAILED) == 0 ) {
		print( "failed: DBDeleteEntryInt\n");
		return;
	}

}
/* ---------------------------------------------
 * ������ì�������ְ�����Ի��      ٯ    
 * --------------------------------------------*/
void saacproto_DBDeleteEntryString_recv( int fd,char* result,
									char* table,char* key,int msgid,int msgid2 )
{
	if( strcmp( result, NET_STRING_FAILED) == 0 ) {
		print( "failed: DBDeleteEntryString\n");
		return;
	}

}


/* ---------------------------------------------
 * ������ì����������Ի��    �Ѱ�  
 * --------------------------------------------*/
void saacproto_DBGetEntryInt_recv( int fd,char* result,int value,
									char* table,char* key,int msgid,int msgid2 )
{
	if( strcmp( result, NET_STRING_FAILED) == 0 ) {
		print( "failed: DBGetEntryInt\n");
		return;
	}
}
/* ---------------------------------------------
 * ������ì����������Ի��      ٯ    
 * --------------------------------------------*/
void saacproto_DBGetEntryString_recv( int fd,char* result,char *value,
									char* table,char* key,int msgid,int msgid2 )
{
	char	cdkey[32];
	char	charaname[32];

	if( strcmp( table, DB_ADDRESSBOOK) == 0 ){
		int mode = 1;
		if( strcmp( result, NET_STRING_FAILED) == 0 ) {
			mode = 0;
		}
		makeStringFromEscaped( key);
		if( !getStringFromIndexWithDelim( key, "_", 1, cdkey, sizeof( cdkey)) ){
			return;
		}
		if( !getStringFromIndexWithDelim( key, "_", 2, charaname, sizeof( charaname)) ){
			return;
		}
		ADDRESSBOOK_DispatchMessage( cdkey, charaname, value, mode);
	}else {
		if( strcmp( result, NET_STRING_FAILED) == 0 ) {
			print( "failed: DBGetEntryString\n");
			return;
		}
	}
}

void saacproto_DBGetEntryByRank_recv( int fd,char* result,char* list,
										char* table,int msgid,int msgid2 )
{
	if( strcmp( result, NET_STRING_FAILED) == 0 ) {
		print( "failed: DBGetEntryByRank\n");
		return;
	}
}
/* ---------------------------------------------
 * ������ì����������Ի��    ��������  
 * --------------------------------------------*/
void saacproto_DBGetEntryByCount_recv( int fd,char* result,char* list,
										char* table,int count_start,int msgid,int msgid2 )
{
	if( strcmp( result, NET_STRING_FAILED) == 0 ) {
		print( "failed: DBGetEntryByCount\n");
		return;
	}
	/* duelpoint database ����   */
	if( strcmp( table, DB_DUELPOINT) == 0 ) {
		NPC_Duelranking_PrintRanking( list, count_start, msgid, msgid2);
	}
}
#ifdef _ALLDOMAN // (���ɿ�) Syu ADD ���а�NPC
void saacproto_UpdataStele_recv( int fd , char *token)
{
	NPC_AlldomanWriteStele( token ); 
}
void saacproto_S_UpdataStele_recv( int i , char *ocdkey , char *oname , char *ncdkey , 
			  char *nname , char *title	, int level , int trns , int floor ) 
{
	print("\nSyu log �յ�Single=> %s , %s , %s , %s ",ocdkey , oname , ncdkey , nname);
	NPC_Alldoman_S_WriteStele( ocdkey , oname , ncdkey , nname , title	, level , trns , floor ) ;
}

#endif
/* ---------------------------------------------
 * Ƥ�����ƽ�ҵ������������ⰾ����Ի
 * --------------------------------------------*/
void saacproto_Broadcast_recv( int fd,char* id,char* charname,char *message )
{
#ifndef _DEATH_CONTEND
	if( strstr( message, "online") == 0 ||
		strstr( message, "offline") == 0 || 
		strstr( message, "param") == 0  ||
		strstr( message, "chardelete") == 0 ){
		char buff[512];
		char escapebuf[1024];

		snprintf( buff, sizeof( buff),"%s_%s", id, charname );
		makeEscapeString( buff, escapebuf, sizeof(escapebuf));

		saacproto_DBGetEntryString_send( acfd, DB_ADDRESSBOOK, escapebuf, 0,0);
	}
#endif
}

void saacproto_Message_recv( int fd,char* id_from,char* charname_from,
							char* id_to,char* charname_to,
							char* message,int option,int mesgid )
{
	BOOL	ret;
	ret = ADDRESSBOOK_sendMessage_FromOther( id_from, charname_from, 
										id_to, charname_to,
										message, option
										);
	if( ret == TRUE) {
		saacproto_MessageAck_send( acfd, id_to, charname_to, 
							  		"successful",mesgid);
	}
}

/* ---------------------------------------------
 *	CoolFish: Family 2001/5/9
 * --------------------------------------------*/
void saacproto_ACAddFM_recv(int fd, char *result, int fmindex, int charfdid,
	int index)
{
	int ret;
	int clifd = getfdFromFdid(charfdid);
//	print("ACAddFM_0\n");
	if (CONNECT_checkfd(clifd) == FALSE)	return;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
//	print("ACAddFM_1 clifd:%d ret:%d fmindex:%d index:%d\n",
//		clifd, ret, fmindex, index);
	ACAddFM(clifd, ret, fmindex, index);
}
void saacproto_ACJoinFM_recv(int fd, char *result, int recv, int charfdid)
{
	int ret;
	int clifd = getfdFromFdid(charfdid);
	if (CONNECT_checkfd(clifd) == FALSE)	return;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
	ACJoinFM(clifd, ret, recv);
}
void saacproto_ACLeaveFM_recv(int fd, char *result, int resultflag, int charfdid)
{
	int ret;
	int clifd = getfdFromFdid(charfdid);
	if (CONNECT_checkfd(clifd) == FALSE)	return;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
	ACLeaveFM(clifd, ret, resultflag);
	
	print(" ACLeaveFM_%d ", ret );
}
void saacproto_ACChangeFM_recv(int fd, char *result, int charfdid)
{

}
void saacproto_ACDelFM_recv(int fd, char *result, int charfdid)
{
	int ret;
	int clifd = getfdFromFdid(charfdid);
	if (CONNECT_checkfd(clifd) == FALSE)	return;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
	
	ACDelFM(clifd, ret);
	
	print(" ACDelFM_%d ", ret);
}
void saacproto_ACShowFMList_recv(int fd, char *result, int fmnum, char *data)
{
	int ret;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
	ACShowFMList(ret, fmnum, data);
}

#ifdef _DEATH_FAMILY_GM_COMMAND	// WON ADD ����սGMָ��
void saacproto_ACRELOADFMOK_recv( int fd, int charindex )
{
	CHAR_talkToCli( charindex, -1, "�ض���������OK!",  CHAR_COLORYELLOW);

}

void saacproto_ACSHOWMEMBERLIST2_recv( int fd, int charaindex, int fm1, char *fm1_name, int fm2, char *fm2_name, int time, int id )
{
	int i, hour = 0, min = 0;
	int fmpks_pos = id * 24;
	char f1[20] = {0}, f2[20] = {0};

	hour = time / 100;
	min = time % 100;

	if( fm1_name == NULL || fm2_name == NULL ) return;
	
	sprintf( f1, "%s", fm1_name );
	sprintf( f2, "%s", fm2_name );

	for( i=0; i<MAX_SCHEDULE; i++){		

		if( fmpks[fmpks_pos+i].dueltime == hour*100 ){
			char msg[256] = {0};
					
			fmpks[fmpks_pos+i].host_index = fm1;
			fmpks[fmpks_pos+i].guest_index = fm2;
				
			strcpy( fmpks[fmpks_pos+i].host_name, f1 );
			strcpy( fmpks[fmpks_pos+i].guest_name, f2 );			

			fmpks[fmpks_pos+i].prepare_time = min;
			fmpks[fmpks_pos+i].max_player = 50;			// �Ҷ�����ս
			fmpks[fmpks_pos+i].win = 0;			
			fmpks[fmpks_pos+i].flag = FMPKS_FLAG_SCHEDULED;
			fmpks[fmpks_pos+i].setting_timeout = 60 * 2;  // ��Сʱ

			sprintf( msg, "%s �� %s ������ս�趨���", f1, f2 );
			CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORYELLOW );

#define SCHEDULEFILEDIR		"./Schedule/"

			// save file
			{
				  char filename[256],tmp[4096],n1[256],n2[256];
				  FILE *f;
				  int j;
				  int floor = 141, x=0, y=0;

				  switch( id ){
				  case 10:	x = 46;
							y = 17;
							break;
				  case 11:	x = 46;
							y = 34;
							break;
				  case 12:	x = 29;
							y = 46;
							break;
				  case 13:	x = 3;
							y = 31;
							break;
				  case 14:	x = 3;
							y = 15;
							break;
				  }

				  snprintf(filename,sizeof(filename), "%s%d_%d_%d", SCHEDULEFILEDIR, floor, x, y );

				  if( (f=fopen(filename, "w") ) ){
					  for( j = 0; j < MAX_SCHEDULE; j++ ){
						makeEscapeString(fmpks[fmpks_pos+j].host_name,n1,sizeof(n1));
						makeEscapeString(fmpks[fmpks_pos+j].guest_name,n2,sizeof(n2));

						sprintf(tmp, "%d|%d|%s|%d|%s|%d|%d|%d|%d\n",
    						fmpks[fmpks_pos+j].dueltime,
    						fmpks[fmpks_pos+j].host_index,
    						n1,
    						fmpks[fmpks_pos+j].guest_index,
    						n2,
    						fmpks[fmpks_pos+j].prepare_time,
    						fmpks[fmpks_pos+j].max_player,
    						fmpks[fmpks_pos+j].flag,
    						fmpks[fmpks_pos+j].win);
  
						fwrite(tmp, strlen(tmp), 1, f);
					  }
					  fclose(f);				
				  }else{
					  print("\n open %s Err", filename );
				  }

			}

			return;
		}
	}

}
#endif

#ifdef _DEATH_FAMILY_STRUCT		// WON ADD ����ս���ʤ������
void saacproto_ACSendFmPkStruct_recv( int fd, char *data )
{
	int i;
	char all_data[fm_pk_max][128] = {0};
	FILE *f;

	for( i=0; i<fm_pk_max; i++ ){
		char buf1[128] = {0};
		if( getStringFromIndexWithDelim( data, "|", i, buf1, sizeof( buf1)) )
			sprintf( all_data[i], "%s", buf1 );
	}

	if( (f=fopen( "d_fm_pk_log.txt", "w+") ) ){
		for( i=0; i<fm_pk_max; i++ ){
			if( all_data[i][0] != 0 )
				fprintf( f, "%s\n", all_data[i] );
		}
		fclose( f );
	}else{
		print("\n open d_fm_pk_log.txt Err !!");
	}
}
#endif


#ifdef _DEATH_FAMILY_LOGIN_CHECK   // WON ADD ����ս������
void saacproto_ACSendMember_recv(int fd, char *result, char *data, int charindex )
{	
	char token[128] = {0};
	int fmarray = -1;

	if( strcmp( result, "failed" ) == 0 ){
		CHAR_warpToSpecificPoint( charindex, 887, 57, 14 );
		CHAR_talkToCli( charindex, -1, "��δ��������ϵ����ս", CHAR_COLORYELLOW);
		return;
	}


	SetFMPetVarInit(charindex); // ����ػ��� Flag
	if(getStringFromIndexWithDelim(data,"|",1,token,sizeof(token))){
		fmarray = atoi(token);
		if( fmarray < 0 ){
			CHAR_warpToSpecificPoint( charindex, 887, 57, 14 );
			CHAR_talkToCli( charindex, -1, "��δ��������ϵ����ս", CHAR_COLORYELLOW);
			return;
		}
		CHAR_setInt( charindex, CHAR_FMINDEX, fmarray + 1 );
		CHAR_setWorkInt( charindex, CHAR_WORKFMINDEXI, fmarray );
		JoinMemberIndex( charindex, fmarray );
	}

	if(getStringFromIndexWithDelim(data,"|",2,token,sizeof(token))){
		CHAR_setInt( charindex, CHAR_FMSPRITE, atoi(token)  );
	}
	if(getStringFromIndexWithDelim(data,"|",3,token,sizeof(token))){
		CHAR_setInt( charindex, CHAR_FMLEADERFLAG, atoi(token) );
	}
	if(getStringFromIndexWithDelim(data,"|",4,token,sizeof(token))){
		CHAR_setWorkInt( charindex, CHAR_WORKFMSETUPFLAG, atoi(token) );
	}
	if(getStringFromIndexWithDelim(data,"|",5,token,sizeof(token))){
		CHAR_setChar( charindex, CHAR_FMNAME, token );
	}

   	CHAR_setWorkInt(charindex, CHAR_WORKFMCHARINDEX, charindex );

	CHAR_sendStatusString(charindex, "f");
	CHAR_complianceParameter( charindex );
	CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charindex , CHAR_WORKOBJINDEX ));
}
#endif 


void saacproto_ACShowMemberList_recv(int fd, char *result, int index, int fmmemnum,
	int fmacceptflag, int fmjoinnum, char *data)
{
	int ret;
	// fmmemnum -1:�޴�����ֵ, -2:�޸��� else:��ʾ�˼�������
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
	ACShowMemberList(ret, index, fmmemnum, fmacceptflag, fmjoinnum, data);
}
void saacproto_ACFMDetail_recv(int fd, char *result, char *data, int charfdid)
{
	int ret;
	int clifd = getfdFromFdid(charfdid);
	if (CONNECT_checkfd(clifd) == FALSE)	return;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
	ACFMDetail(ret, data, clifd );
}
void saacproto_ACMemberJoinFM_recv(int fd, char *result, char *data, int charfdid)
{

}
void saacproto_ACMemberLeaveFM_recv(int fd, char *result, char *data, int charfdid)
{

}
#ifdef _PERSONAL_FAME   // Arminius: �����������
void saacproto_ACFMCharLogin_recv(int fd, char *result, int index, int floor,
	int fmdp, int joinflag, int fmsetupflag, int flag, int charindex,
	int charfame, int charfdid
#ifdef _NEW_MANOR_LAW
	,int momentum
#endif
	)
#else
void saacproto_ACFMCharLogin_recv(int fd, char *result, int index, int floor,
	int fmdp, int joinflag, int fmsetupflag, int flag, int charindex,
	int charfdid)
#endif
{
	int ret;
	int clifd = getfdFromFdid(charfdid);
	if (CONNECT_checkfd(clifd) == FALSE)	return;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
//	print("Login fd:%d result:%s index:%d floor:%d fmdp:%d joinflag:%d setup:%d charfdid:%d\n",
//		fd, result, index, floor, fmdp, joinflag, fmsetupflag, charfdid);
#ifdef _PERSONAL_FAME   // Arminius: �����������
//        print("Login charfame:%d\n", charfame);
	ACFMCharLogin(clifd, ret, index, floor, fmdp, joinflag, fmsetupflag,
		flag, charindex, charfame
	#ifdef _NEW_MANOR_LAW
		,momentum
	#endif
		);
#else
	ACFMCharLogin(clifd, ret, index, floor, fmdp, joinflag, fmsetupflag,
		flag, charindex);
#endif
}
void saacproto_ACFMCharLogout_recv(int fd, char *result, int charfdid)
{
}
void saacproto_ACFMReadMemo_recv(int fd, char *result, int index, int num,
        int dataindex, char *data)
{
	int ret;
	// index(���� WORK ������)��num(���ϱ��������35��)
	// dataindex(���µ�������)��data(����)��
	// ע��num -1:�޴�����ֵ, -2:�޸��� else:��ʾ�˼�������
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
	ACShowFMMemo(ret, index, num, dataindex, data);
}
void saacproto_ACFMWriteMemo_recv(int fd, char *result, int index)
{
}
void saacproto_ACFMPointList_recv(int fd, char *result, char *data)
{
	int ret;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
	ACShowPointList(ret, data);
}


#ifdef _ADD_FAMILY_TAX			   // WON ADD ����ׯ԰˰��

// GS Ҫ��ׯ԰˰��
void saacproto_GS_ASK_TAX_recv(int fd, int fm_tax, int fm_point)
{
	extern struct  FM_POINTLIST fmpointlist;  // ����ݵ�
	if( FAMILY_MAXHOME <= fm_point || fm_point < 0 )
		return;
	fmpointlist.fm_tax[fm_point] = fm_tax;
}

// ׯ԰�峤Ҫ�޸�˰��
void saacproto_ACFMSetTAX_recv(int fd, char *result, int fm_tax, int fm_point, int index)
{	
	extern struct  FM_POINTLIST fmpointlist;  // ����ݵ�
	char msg[128];
    //int playernum = CHAR_getPlayerMaxNum();
	//int i;

	if (strcmp(result, SUCCESSFUL) == 0){	
		if( FAMILY_MAXHOME <= fm_point || fm_point < 0 ){
			print("err fm_point=%d\n", fm_point);
			return;
		}

		// ֪ͨclient
		getStringFromIndexWithDelim(fmpointlist.pointlistarray[fm_point], "|", 6, msg, sizeof(msg));
		sprintf(msg,"%s ׯ԰˰�ʵ���Ϊ %d��",msg, fm_tax);
		CHAR_talkToCli(index, -1, msg, CHAR_COLORYELLOW);
/*
		for( i = 0 ; i < playernum ; i++) {
			if( CHAR_getCharUse(i) != FALSE ) {
				CHAR_talkToCli( i, -1, msg, CHAR_COLORWHITE);
			}
		}
*/
		// �趨ׯ԰˰��
		fmpointlist.fm_tax[fm_point] = fm_tax;

	}else{									  // �趨ʧ��
		CHAR_talkToCli(index, -1, "�趨ׯ԰˰��ʧ�ܣ�����������", CHAR_COLORYELLOW);
	}

}
#endif



void saacproto_ACSetFMPoint_recv(int fd, char *result, int r, int charfdid)
{
	int ret;
	int clifd = getfdFromFdid(charfdid);
	if( CONNECT_checkfd(clifd) == FALSE )return;
	if (strcmp(result, SUCCESSFUL) == 0){
		ret = 1;
	}else{
		ret = 0;
	}
	ACSetFMPoint(ret, r, clifd);

}
void saacproto_ACFixFMPoint_recv(int fd, char *result, int r)
{
}
void saacproto_ACFMAnnounce_recv(int fd, char *result, char *fmname,
	int fmindex, int index, int kindflag, char *data, int color)
{
	int ret;
	if (strcmp(result, SUCCESSFUL) == 0){
		ret = 1;
	}else{
		ret = 0;
	}
	ACFMAnnounce(ret, fmname, fmindex, index, kindflag, data, color);	
}	
#ifdef _NEW_MANOR_LAW
extern void SortManorSchedule();
#endif
void saacproto_ACShowTopFMList_recv(int fd, char *result, int kindflag, int num, char *data)
{
	int ret;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else	ret = 0;
	ACShowDpTop(ret, num, data, kindflag);
#ifdef _NEW_MANOR_LAW
	SortManorSchedule();
#endif
}
#ifdef _FAMILYBANKSTONELOG
void saacproto_ACgetFMBankgold_recv(int fd, int charfdid,int r)
{
	int clifd=getfdFromFdid(charfdid);
	int charaindex = CONNECT_getCharaindex( clifd);
	if( CONNECT_checkfd(clifd) == FALSE) return;
	CHAR_setInt(charaindex,CHAR_FMBANKGOLD,r);
}
#endif
#ifdef _NEW_MANOR_LAW
extern struct  FMS_DPTOP fmdptop;
#endif
void saacproto_ACFixFMData_recv(int fd, char *result, int kindflag, char *data1,
	char *data2, int charfdid)
{
	int ret;
	int intdata;
	int clifd = getfdFromFdid(charfdid);
	int charaindex = CONNECT_getCharaindex( clifd );
	if( CONNECT_checkfd(clifd) == FALSE )return;
	
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else    ret = 0;
	
	if( !CHAR_CHECKINDEX(charaindex) )	return;
	if( kindflag == FM_FIX_FMRULE ){
		CHAR_talkToCli( charaindex , -1, "������ּ�޸ģϣˣ�", CHAR_COLORWHITE);
	}else if( kindflag == FM_FIX_FMGOLD ) {
		intdata = atoi( data1 );
		if( ret == 0 ) {
			if( atoi(data1) > 0 )	{
				CHAR_setInt( charaindex, CHAR_GOLD, CHAR_getInt( charaindex, CHAR_GOLD ) +atoi(data1) );
			}
			CHAR_send_P_StatusString( charaindex , CHAR_P_STRING_GOLD);
			return;
		}else {
			if( atoi(data1) < 0 )	{
				CHAR_setInt( charaindex, CHAR_GOLD, CHAR_getInt( charaindex, CHAR_GOLD ) -atoi(data1) );
				CHAR_send_P_StatusString( charaindex , CHAR_P_STRING_GOLD);
			}
#ifdef _FAMILYBANKSTONELOG
			CHAR_setInt(charaindex,CHAR_FMBANKGOLD,CHAR_getInt(charaindex,CHAR_FMBANKGOLD)+intdata);
			LogFamilyBankStone(
				CHAR_getChar( charaindex, CHAR_NAME ),
				CHAR_getChar( charaindex, CHAR_CDKEY ),
				intdata,
				CHAR_getInt( charaindex, CHAR_GOLD ),
				(intdata>0)?"FMBank(���)(��������)":"FMBank(ȡ��)(��������)",
				CHAR_getInt( charaindex,CHAR_FLOOR),
				CHAR_getInt( charaindex,CHAR_X ),
				CHAR_getInt( charaindex,CHAR_Y ),
				CHAR_getInt( charaindex,CHAR_FMBANKGOLD)
				);
#else
			LogStone(
				atoi(data2),
				CHAR_getChar( charaindex, CHAR_NAME ),
				CHAR_getChar( charaindex, CHAR_CDKEY ),
				-intdata,
				CHAR_getInt( charaindex, CHAR_GOLD ),
				"FMBank(��������)",
				CHAR_getInt( charaindex,CHAR_FLOOR),
				CHAR_getInt( charaindex,CHAR_X ),
				CHAR_getInt( charaindex,CHAR_Y )
				);
#endif
		}
	}else if( kindflag == FM_FIX_FMLEADERCHANGE ){
		ACFMJob( clifd, ret, data1, data2 );
	}
	// CoolFish: 2001/10/03
	else if (kindflag == FM_FIX_FMADV || kindflag == FM_FIX_FMFEED
		|| kindflag == FM_FIX_FMSYNTHESIZE || kindflag == FM_FIX_FMDEALFOOD)
	{
		char	tmpbuf[1024], flag[128];
#ifdef _NEW_MANOR_LAW
		int i;
#endif
		if (kindflag == FM_FIX_FMADV)	strcpy(flag, "ADV");
		else if (kindflag == FM_FIX_FMFEED)	strcpy(flag, "FEED");
		else if (kindflag == FM_FIX_FMSYNTHESIZE)	strcpy(flag, "SYN");
		else if (kindflag == FM_FIX_FMDEALFOOD)	strcpy(flag, "FOOD");
		sprintf(tmpbuf, "%s::%d %s %s", flag,
			CHAR_getInt(charaindex, CHAR_FMINDEX),
			CHAR_getChar(charaindex, CHAR_FMNAME), data1);
		// print("ACFixFMData tmpbuf:%s\n", tmpbuf);
		LogFMPOP(tmpbuf);
#ifdef _NEW_MANOR_LAW
		for(i=0;i<FAMILY_MAXNUM;i++){
			// ��������
			if(fmdptop.fmtopid[i] == CHAR_getWorkInt(charaindex,CHAR_WORKFMINDEXI)) break;
		}
		if(i < FAMILY_MAXNUM){
			sprintf(tmpbuf, "MOMENTUM::%d %s %d",CHAR_getInt(charaindex,CHAR_FMINDEX),
				CHAR_getChar(charaindex,CHAR_FMNAME),fmdptop.fmMomentum[i]);
			LogFMPOP(tmpbuf);
		}
#endif
	}
}
void saacproto_ACFixFMPK_recv(int fd, char *result, int data, int winindex,
	int loseindex)
{
	int ret;
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else    ret = 0;
         
//        print("FixFMPK_recv result:%s data:%d winindex:%d loseindex:%d\n", result, data, winindex, loseindex);
        if (ret == 1)
        {
        	char	tmpbuf[256];
        	ACFixFMPK(winindex, loseindex, data);
        	sprintf(tmpbuf, "FMPK(����PK)::win(��ʤ����):%d lose(��ܼ���):%d point:%d", winindex, loseindex, data);
		//print("FixFMPK tmpbuf:%s\n", tmpbuf);
        	LogFMPOP(tmpbuf);
        }
}
void saacproto_ACGMFixFMData_recv(int fd, char *result, char *fmname, int charfdid)
{
	int ret;
	int clifd = getfdFromFdid(charfdid);
	char buf[256];
	if( CONNECT_checkfd(clifd) == FALSE )return;
	
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else    ret = 0;
         
        print("GMFixFMData_recv result:%s\n", result);
        
        if (ret == 1)
        {
           sprintf(buf, "�޸�%s���ϣϣˣ�", fmname);
           CHAR_talkToCli(CONNECT_getCharaindex(clifd) , -1, buf, CHAR_COLORWHITE);
        }
        else
        {
           sprintf(buf, "�޸�%s����ʧ�ܣ�", fmname);
           CHAR_talkToCli(CONNECT_getCharaindex(clifd) , -1, buf, CHAR_COLORWHITE);
        }
}
extern int familyTax[];
void saacproto_ACGetFMData_recv(int fd, char *result, int kindflag, int data,
	int charfdid)
{
	int ret;
	char buf[256];
	int clifd = getfdFromFdid(charfdid);
	if( CONNECT_checkfd(clifd) == FALSE )return;
	
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else    ret = 0;
	
	//print(" FMBank2_%d_%d_%d ", ret, kindflag, data );
	if( ret == 0 ) return;
	
	if( kindflag == 1 ) {
		int fmindex = CHAR_getWorkInt(CONNECT_getCharaindex(clifd), CHAR_WORKFMINDEXI);
		if( fmindex < 0 || fmindex >= FAMILY_MAXNUM ){
			print(" fmindex:%d if FALSE !!\n", fmindex);
			return;
		}
		familyTax[ fmindex] = data;
		sprintf( buf, "B|T|%d", data );
		lssproto_FM_send( clifd, buf );
	}
		
}
void saacproto_ACFMClearPK_recv(int fd, char *result, char *fmname, int fmindex,
        int index)
{
	int ret, i = 0;
	
	if (strcmp(result, SUCCESSFUL) == 0)	ret = 1;
	else    ret = 0;
	for (i = 0; i < MAX_SCHEDULEMAN * MAX_SCHEDULE; i++)
	{
	   {
	      if (fmpks[i].flag == -1)	continue;
	      if (fmpks[i].host_index == index
	      	 || fmpks[i].guest_index == index)
	      {
	         fmpks[i].host_index = -1;
	         strcpy(fmpks[i].host_name, "");
	         fmpks[i].guest_index = -1;
	         strcpy(fmpks[i].guest_name, "");
	         fmpks[i].prepare_time = -1;
	         fmpks[i].max_player = -1;
	         fmpks[i].win = -1;
	         fmpks[i].flag = -1;
	      }
	   }
	}
}

#ifdef _ACFMPK_LIST
void saacproto_ACSendFmPk_recv( int fd, int toindex, int flg)
{
	char buf[256];
	if( flg == 0){
		sprintf( buf, "ׯ԰��ս��¼ʧ�ܣ���");
		CHAR_talkToCli( toindex, -1, buf, CHAR_COLORRED);
	}else{
		sprintf( buf, "ׯ԰��ս��¼ȷ��OK����ú�׼����");
		CHAR_talkToCli( toindex, -1, buf, CHAR_COLORYELLOW);
	}
}
void saacproto_ACLoadFmPk_recv(int fd, char *data)
{
	char buf[10], fm_pk_data[128];
	char token[256], skip[256];
	int fm_pk_num , fmpks_pos=-1;
	
	easyGetTokenFromString( data , 1 , buf , sizeof(buf));
	fm_pk_num = atoi(buf);
	sprintf(fm_pk_data , "%s", data+2 );
	
	if( ((fmpks_pos=fm_pk_num*MAX_SCHEDULE)== -1) || (fm_pk_num > FMPOINTNUM) ){		
		print("\n err fm_pk_num(%d)", fm_pk_num );
		return;
	}
	// ʱ��
	if (getStringFromIndexWithDelim(data,"|",2,token,sizeof(token))) {
		fmpks[fmpks_pos+1].dueltime=atoi(token);
	 }      
	 // ���� familyindex    
	 if (getStringFromIndexWithDelim(data,"|",3,token,sizeof(token))) {                 
		 fmpks[fmpks_pos+1].host_index=atoi(token);                
	 }                
	 // ���� ������                
	 if (getStringFromIndexWithDelim(data,"|",4,token,sizeof(token))) {                  
		 strcpy(fmpks[fmpks_pos+1].host_name,makeStringFromEscaped(token));                
	 }                
	 // �Ͷ� familyindex                
	 if (getStringFromIndexWithDelim(data,"|",5,token,sizeof(token))) {                  
		 fmpks[fmpks_pos+1].guest_index=atoi(token);                
	 }                
	 // �Ͷ� ������                
	 if (getStringFromIndexWithDelim(data,"|",6,token,sizeof(token))) {                  
		 strcpy(fmpks[fmpks_pos+1].guest_name,makeStringFromEscaped(token));                
	 }                
	 // ׼��ʱ��                
	 if (getStringFromIndexWithDelim(data,"|",7,token,sizeof(token))) {                  
		 fmpks[fmpks_pos+1].prepare_time=atoi(token);                
	 }                
	 // �������                
	 if (getStringFromIndexWithDelim(data,"|",8,token,sizeof(token))) {
		 int maxnum = atoi( token);
		 if( maxnum < 50 ) maxnum = 50;
		 fmpks[fmpks_pos+1].max_player = maxnum;
	 }  
	 // ��� 		
	 if (getStringFromIndexWithDelim(data,"|",9,token,sizeof(token))) {	 
		 fmpks[fmpks_pos+1].flag=atoi(token);
	 }	 
	 if( fmpks[fmpks_pos+1].flag == FMPKS_FLAG_MANOR_OTHERPLANET ){
		 // ��ս����				
		 if (getStringFromIndexWithDelim(data,"|",10,token,sizeof(token))) {
			 strcpy(fmpks[fmpks_pos+2].host_name,makeStringFromEscaped(token));
			 strcpy(skip, makeStringFromEscaped(token));
			 if (strcmp(getGameserverID(), skip)==0) {
				 fmpks[fmpks_pos+1].flag=8;
			 }else if( fmpks[fmpks_pos+1].flag == 8) {
				 fmpks[fmpks_pos+1].flag=10;			
			 } 
		 }
	 }else{
		 strcpy( fmpks[fmpks_pos+2].host_name, "");
	 }
}
#endif


void saacproto_ACManorPKAck_recv(int fd, char *data)
{
  // ������� server �� ׯ԰ pk scheduleman
  int i;
  char token[256], skip[256];
  int char_max=CHAR_getCharNum();

  print("ACManorPKAck_recv: %s\n",data);

  for (i=0; i<char_max; i++) {
    if (CHAR_getCharUse(i)) {
      if (CHAR_getInt( i, CHAR_WHICHTYPE )==CHAR_TYPEMANORSCHEDULEMAN) {
        if (getStringFromIndexWithDelim(data,"|",1,token,sizeof(token))) {
          if (CHAR_getWorkInt(i, CHAR_NPCWORKINT2)==atoi(token)) {	// manor id
            int fmpks_pos = CHAR_getWorkInt(i, CHAR_NPCWORKINT1)*MAX_SCHEDULE;

            // ��ս����
            if (getStringFromIndexWithDelim(data,"|",10,token,sizeof(token))) {
              strcpy(skip, makeStringFromEscaped(token));
              print("ACManorPKAck: %d %s\n", fmpks_pos, skip);
              
              if (strcmp(getGameserverID(), skip)!=0) {
                strcpy(fmpks[fmpks_pos+2].host_name, skip);
                
                // ʱ��
                if (getStringFromIndexWithDelim(data,"|",2,token,sizeof(token))) {
                  fmpks[fmpks_pos+1].dueltime=atoi(token);
                }
                // ���� familyindex
                if (getStringFromIndexWithDelim(data,"|",3,token,sizeof(token))) {
                  fmpks[fmpks_pos+1].host_index=atoi(token);
                }
                // ���� ������
                if (getStringFromIndexWithDelim(data,"|",4,token,sizeof(token))) {
                  strcpy(fmpks[fmpks_pos+1].host_name,makeStringFromEscaped(token));
                }
                // �Ͷ� familyindex
                if (getStringFromIndexWithDelim(data,"|",5,token,sizeof(token))) {
                  fmpks[fmpks_pos+1].guest_index=atoi(token);
                }
                // �Ͷ� ������
                if (getStringFromIndexWithDelim(data,"|",6,token,sizeof(token))) {
                  strcpy(fmpks[fmpks_pos+1].guest_name,makeStringFromEscaped(token));
                }
                // ׼��ʱ��
                if (getStringFromIndexWithDelim(data,"|",7,token,sizeof(token))) {
                  fmpks[fmpks_pos+1].prepare_time=atoi(token);
                }
                // �������
                if (getStringFromIndexWithDelim(data,"|",8,token,sizeof(token))) {
                  fmpks[fmpks_pos+1].max_player=atoi(token);
                }
                // ���
                if (getStringFromIndexWithDelim(data,"|",9,token,sizeof(token))) {
                  fmpks[fmpks_pos+1].flag=atoi(token);
                }
                // ��ս����
                if (getStringFromIndexWithDelim(data,"|",10,token,sizeof(token))) {
                  strcpy(fmpks[fmpks_pos+2].host_name,makeStringFromEscaped(token));
                }
              }
            }
          }
        }
      }
    }
  }
}

#ifdef _NEW_PLAYERGOLD
void saacproto_ACNEWPlayerList_recv( int fd, char *CdKey, char *UserName, int charaindex, int RunType)
{
	char buf[256];
	if( !CHAR_CHECKINDEX( charaindex)){
		return;
	}
	switch( RunType){
	case NEW_ERR: //�޷�Ԥ�ڵĴ���
		CHAR_talkToCli( charaindex, -1, "�޷�Ԥ�ڵĴ���", CHAR_COLORYELLOW);
		break;
	case NEW_DELOK://���ʺ�����ɾ����ɣ�
		sprintf( buf,"���ʺ�����[%s]ȷ����ɣ�", CdKey);
		CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
		if( CHAR_getInt( charaindex, CHAR_GOLD) >= 990000 ){
			sprintf( buf,"�ʺ�����[%s]����ʯ�ҽ�����100��ȡ����ȡ�ʸ�", CdKey);
			CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
		}else{
			NPC_ActionAddGold( charaindex, 10000);
		}
		break;
	case NEW_DELERR: //���ʺ�����ɾ����������
		CHAR_talkToCli( charaindex, -1, "���ʺ������޷���ȡʯ�ң�", CHAR_COLORYELLOW);
		break;
	case NEW_ADDOK:
		sprintf( buf,"���ʺ�����[%s]������ɣ�", CdKey);
		CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
		break;
	case NEW_ADDERR:
		sprintf( buf,"���ʺ�����[%s]���ӷ�������", CdKey);
		CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
		break;
	case NEW_LISTFULL:
		CHAR_talkToCli( charaindex, -1, "���ʺ������б�������", CHAR_COLORYELLOW);
		break;
	}
}
#endif


#ifdef _NEW_PLAYERGOLD
void saacproto_LoadNEWPlayer_recv(int fd , int charaindex, char *data)
{
	CHAR_talkToCli( charaindex, -1, data, CHAR_COLORYELLOW);
}
#endif

#ifdef _WAEI_KICK
void saacproto_ACKick_recv(int fd , int act, char* data ,int retfd)
{
    int clifd = getfdFromFdid(retfd);
    //char cdkey[CDKEYLEN];
    int cindex = getCharindexFromFdid(retfd);
	
	print(" ACKick:%s ", data);

	switch( act)	{
	case 0:	//FAIL
		if( CONNECT_checkfd(clifd) == FALSE) return;
		CHAR_talkToCli( cindex, -1, data, CHAR_COLORYELLOW);	
		break;
	case 1:	//����������
		{
			char cdkey[256];
			char szName[256];
			int i, MAX_USER=0;
			BOOL find=FALSE;
			
			//print(" AC����:%s ", data);

			MAX_USER=getFdnum();
			for(i=0;i<MAX_USER;i++){
				int i_use;
				i_use=CONNECT_getUse(i);
				if(i_use){
					CONNECT_getCdkey( i, cdkey, sizeof( cdkey ));
					if( !strcmp( cdkey, data) ){
						int fd_charaindex = CONNECT_getCharaindex( i );
						CONNECT_getCharname( i, szName, sizeof( szName ) );
						if( retfd == -1 ){
							print(" ���ظ����� ");
							CHAR_talkToCli( fd_charaindex, -1, "���ظ��ʺŵ�������ߣ�",
								CHAR_COLORYELLOW);
						}
#ifdef _CHANGEGALAXY
						else if( retfd == -2 ){ // Robin add
							print(" ��׼������:%d->%s ", fd_charaindex, cdkey);
							CHAR_talkToCli( fd_charaindex, -1, "��׼����������ߣ�",
								CHAR_COLORYELLOW);
						}
#endif
#ifdef _TIMEOUTKICK
						else if( retfd == -3 ){ // Robin add
							print(" ��ףǣӵ�������:%d->%s ", fd_charaindex, cdkey);
							CHAR_talkToCli( fd_charaindex, -1, "���ģףǣӵ������㣡",
								CHAR_COLORYELLOW);
						}
#endif
						else{
							print(" �������� ");
							CHAR_talkToCli(fd_charaindex, -1, "�������¶����ߡ�",
								CHAR_COLORYELLOW);
						}
						CONNECT_setCloseRequest( i , 1 );
						find=TRUE;
					}
				}
			}
			
			if( find == TRUE && retfd != -1 && CHAR_CHECKINDEX( cindex) )	{
				char buf1[256];//, buf2[256];
				sprintf(buf1,"ϵͳ��[%s]����ŷ�����", /*CHAR_getChar( cindex, CHAR_NAME),*/ szName);
				for( i=0; i<MAX_USER; i++){
					if( CONNECT_getUse(i) )	{
						int fd_charaindex = CONNECT_getCharaindex( i);
						if( !CHAR_CHECKINDEX( fd_charaindex) ) continue;
						CHAR_talkToCli( fd_charaindex, -1, buf1, CHAR_COLORYELLOW);
						//						CHAR_talkToCli(fd_charaindex, -1, buf2, CHAR_COLORYELLOW);
					}
				}
			} 
		}
		break;
	case 2:
		print("\n CASE 2");
		break;
	}
}

#endif


#ifdef _CHAR_POOLITEM
void saacproto_ACCharSavePoolItem_recv(int fd ,char* result, char* data,int retfd)
{
	int charaindex = getCharindexFromFdid( retfd);
	//print("\n ACCharSavePoolItem_recv:%s ", data);
	if( !CHAR_CHECKINDEX(charaindex) ) return;
	if( strstr( result, FAILED ) != NULL ){
		CHAR_talkToCli( charaindex, -1, "���ֿ߲����ϴ浵ʧ�ܣ�", CHAR_COLORYELLOW);
		return;

	}
}

void saacproto_ACCharGetPoolItem_recv(int fd ,char* result, char* data,int retfd, int meindex) 
{
	Char*   ch = NULL;
	int i, clifd, charaindex;

	//print("\n ACCharGetPoolItem_recv:%s ", data);

	if( strcmp( result , SUCCESSFUL ) != 0 ) return;
	charaindex = getCharindexFromFdid(retfd);
	if( !CHAR_CHECKINDEX( charaindex) ) return;
	clifd = getfdFromCharaIndex( charaindex);
	if( CHAR_CheckDepotItem( charaindex) ) return;//�ֿ��Ѵ���

	if( (ch = CHAR_getCharPointer( charaindex)) == NULL ) return;
	ch->indexOfExistDepotItems = allocateMemory( sizeof( int ) * CHAR_MAXDEPOTITEMHAVE );
	if( ch->indexOfExistDepotItems == NULL ) return;
	for( i=0; i<CHAR_MAXDEPOTITEMHAVE; i++)
		ch->indexOfExistDepotItems[i] = -1;

	if( CHAR_makeDepotItemStringToChar( charaindex, data) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "���ֿ߲����϶�ȡʧ�ܣ�", CHAR_COLORYELLOW);
		CHAR_removeDepotItem( charaindex);
		return;
	}
	if( !CHAR_CHECKINDEX( meindex) ) return;
	if( clifd != -1 ) {
		char	message[1024];
		char	buf[1024];
		strcpy( message, 
			"3\n\n"
			"          ʹ�õ��ֿ߲�\n\n"
			"          ����ŵ��ߣ�\n"
			"          ��ȡ�ص��ߣ�\n"
		);
		lssproto_WN_send( clifd, WINDOW_MESSAGETYPE_SELECT, 
						WINDOW_BUTTONTYPE_CANCEL,
						311, //CHAR_WINDOWTYPE_DEPOTITEMSHOP_HANDLE,
						CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
						makeEscapeString( message, buf, sizeof(buf)) );
	}

}

#endif//������


#ifdef _CHAR_POOLPET
void saacproto_ACCharSavePoolPet_recv(int fd ,char* result, char* data,int retfd)
{
	int charaindex = getCharindexFromFdid( retfd);
	if( !CHAR_CHECKINDEX(charaindex) ) return;
	if( strstr( result, FAILED ) != NULL ){
		CHAR_talkToCli( charaindex, -1, "����ֿ����ϴ浵ʧ�ܣ�", CHAR_COLORYELLOW);
		return;

	}
}

void saacproto_ACCharGetPoolPet_recv(int fd ,char* result, char* data,int retfd, int meindex) 
{
	Char*   ch = NULL;
	int i, clifd, charaindex;

	if( strcmp( result , SUCCESSFUL ) != 0 ) return;
	charaindex = getCharindexFromFdid(retfd);
	if( !CHAR_CHECKINDEX( charaindex) ) return;
	clifd = getfdFromCharaIndex( charaindex);
	if( CHAR_CheckDepotPet( charaindex) ) return;//�ֿ��Ѵ���

	if( (ch = CHAR_getCharPointer( charaindex)) == NULL ) return;
	ch->indexOfExistDepotPets = allocateMemory( sizeof( int ) * CHAR_MAXDEPOTPETHAVE );
	if( ch->indexOfExistDepotPets == NULL ) return;
	for( i=0; i<CHAR_MAXDEPOTPETHAVE; i++)
		ch->indexOfExistDepotPets[i] = -1;

	if( CHAR_makeDepotPetStringToChar( charaindex, data) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "����ֿ����϶�ȡʧ�ܣ�", CHAR_COLORYELLOW);
		CHAR_removeDepotPet( charaindex);
		return;
	}
	if( !CHAR_CHECKINDEX( meindex) ) return;
	if( clifd != -1 ) {
		char	message[1024];
		char	buf[1024];
		strcpy( message, 
			"3\n\n"
			"			ʹ�ó���ֿ�\n\n"
			"			����ų��\n"
			"			��ȡ�س��\n"
		);
		lssproto_WN_send( clifd, WINDOW_MESSAGETYPE_SELECT, 
						WINDOW_BUTTONTYPE_CANCEL,
						CHAR_WINDOWTYPE_DEPOTPETSHOP_HANDLE,
						CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
						makeEscapeString( message, buf, sizeof(buf)) );
	}

}

#endif


#ifdef _UNIVERSE_CHATROOM//ACUniChatroom
void saacproto_ACUniChatroom_recv( int acfd, int charaindex, int clifdid, char *result,
									   char *CdKey, char *data)
{
	saac_ChatRoom_recvall ( acfd, result, data, charaindex, clifdid );
}
#endif


#ifdef _ANGEL_SUMMON
//void saacproto_ACMissionTable_recv( int fd, int num, int type, char *data, int charaindex)
void saacproto_ACMissionTable_recv( int fd, int num, int type, char *data, char* angelinfo)
{
	char msg[1024];
	
	if( type == 1 ) { // get data list
		//if( num == -1) // get all data
		{ 
			int di, ti;
			char onedata[1024], token[1024];

			if( num == -1 ) {
				print("\n Get_All_Mission_Table!!  ");
				memset( missiontable, 0, sizeof(missiontable));
			}
			else
				print("\n Get_One_Mission_Table!!:%s ", data);

			for( di =1; di <MAXMISSIONTABLE; di++) {
				if( !getStringFromIndexWithDelim( data, " ", di, onedata, sizeof(onedata)))
					break;
				if( onedata[0] == NULL || onedata[0] == ' ') break;
				print(" data%d:%s ", di, onedata);
				getStringFromIndexWithDelim( onedata, "|", 1, token, sizeof(token));
				ti = atoi( token);
				if( ti <0 || ti >= MAXMISSIONTABLE)	continue;
				getStringFromIndexWithDelim( onedata, "|", 2, token, sizeof(token));
				strcpy( missiontable[ti].angelinfo, token);
				getStringFromIndexWithDelim( onedata, "|", 3, token, sizeof(token));
				strcpy( missiontable[ti].heroinfo, token);
				getStringFromIndexWithDelim( onedata, "|", 4, token, sizeof(token));
				missiontable[ti].mission = atoi( token);
				getStringFromIndexWithDelim( onedata, "|", 5, token, sizeof(token));
				missiontable[ti].flag = atoi( token);
				getStringFromIndexWithDelim( onedata, "|", 6, token, sizeof(token));
				missiontable[ti].time = atoi( token);
				getStringFromIndexWithDelim( onedata, "|", 7, token, sizeof(token));
				missiontable[ti].limittime = atoi( token);
			}
		}
	}
	else if( type == 2 ) { // add data return
		int angelindex =-1, i, maxChar;
		char nameinfo[128], msgbuf[1024];

		if( num == -1) {
			sprintf( msg, " MissionTable����!! ");
			print( msg);
			LogAngel( msg);
			return;
		}

		sprintf( msgbuf, " �յ�AC��Ӧ(add data) num=%d nameinfo=%s ", num, angelinfo);
		print( msgbuf);
		LogAngel( msgbuf);

		maxChar = CHAR_getPlayerMaxNum();
		for( i =0; i <maxChar; i++) {
			if( !CHAR_CHECKINDEX( i) )	continue;
			getMissionNameInfo( i, nameinfo);
			if( strcmp( nameinfo, angelinfo) )	continue;
			
			angelindex = i;
			break;
		}

		if( angelindex == -1 ) {
			print(" ʹ�������� ");
			return;
		}
			
		lssproto_WN_send( getfdFromCharaIndex(angelindex), WINDOW_MESSAGETYPE_ANGELMESSAGE, 
			WINDOW_BUTTONTYPE_YESNO, 
			CHAR_WINDOWTYPE_ANGEL_ASK,
			-1,
			"Ŀǰħ����Ű����������Ҫ��İ�æ��ǰ��Ѱ��������������Щħ�壬���Ƿ�Ը���æ��");
		
		CHAR_talkToCli( angelindex, -1, "���������ٻ��㡣", CHAR_COLORYELLOW);
	}
	else if( type == 3) {
		if( num <0 || num >= MAXMISSIONTABLE)	return;

		sprintf( msg, " AC��� ����:%d %s %s ", num, missiontable[num].angelinfo, missiontable[num].heroinfo);
		print( msg);
		LogAngel( msg);

		strcpy( missiontable[num].angelinfo, "");
		strcpy( missiontable[num].heroinfo, "");
		missiontable[num].mission = 0;
		missiontable[num].flag = 0;
		missiontable[num].time = 0;
		missiontable[num].limittime = 0;

	}

}

#endif

#ifdef _TEACHER_SYSTEM
void saacproto_ACCheckCharacterOnLine_recv( int acfd, int charaindex, int iOnline,char *data,int flag)
{
	switch(flag){
		case R_F_TEACHER_SYSTEM:
			CHAR_Teacher_system_View(charaindex,iOnline,data);
			break;
		default:printf("saacproto_ACCheckCharacterOnLine_recv(): error flag type!!(%d)\n",flag);
	}
}
#endif

#ifdef _RACEMAN
void saacproto_ACRaceRecordandSort_recv(int fd,int charaindex,int racetype,char *data)  
{	
	int  i,j=0;
	char perdata[128];	
	char buuf[16384];

	//sprintf(buuf,"recv_type:%d  data:%s",racetype,data);
	//CHAR_talkToCli( charaindex , -1, buuf , CHAR_COLORYELLOW);

	print("race_recv:%s",data);

	if(racetype == 0 || racetype == 9 ) return;  //��Ϊ�ǼǶ��������յ�ac�Ļش������Բ��ô���

	//�������ԭ�ⲻ������CLIENT
	
	if(racetype == 1){	//gamemode 1 
		memset(ranktable1,0,sizeof(ranktable1));
		for(i=0;i<DEFMAXRANK*3;i++)  //ԭ���ϲ�������
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable1[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable1[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				ranktable1[j].catchcnt = atoi(perdata);
				ranktable1[j].rank = j+1;
				j++;
			}		
		}
	}else if(racetype == 2){ //gamemode 2 
		memset(ranktable2,0,sizeof(ranktable2));
		for(i=0;i<DEFMAXRANK*3;i++)  
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable2[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable2[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				ranktable2[j].catchcnt = atoi(perdata);
				ranktable2[j].rank = j+1;
				j++;
			}		
		}
	}else if(racetype == 3){ //gamemode 3 	
		memset(ranktable3,0,sizeof(ranktable3));
		for(i=0;i<DEFMAXRANK*3;i++)  
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable3[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable3[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				ranktable3[j].catchcnt = atoi(perdata);
				ranktable3[j].rank = j+1;
				j++;
			}		
		}
	}else if(racetype == 4){ //gamemode 4 		
		memset(ranktable4,0,sizeof(ranktable4));
		for(i=0;i<DEFMAXRANK*3;i++)  //ԭ���ϲ�������
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable4[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable4[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				ranktable4[j].catchcnt = atoi(perdata);
				ranktable4[j].rank = j+1;
				j++;
			}		
		}
	}else if(racetype == 5){	//gamemode 5 		
		memset(ranktable5,0,sizeof(ranktable5));
		for(i=0;i<DEFMAXRANK*3;i++) 
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable5[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(ranktable5[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				ranktable5[j].catchcnt = atoi(perdata);
				ranktable5[j].rank = j+1;
				j++;
			}		
		}
	}else if(racetype == 11){
		memset(history1,0,sizeof(history1));
		for(i=0;i<DEFMAXRANK*3;i++) 
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history1[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history1[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				history1[j].catchcnt = atoi(perdata);
				history1[j].rank = j+1;
				j++;
			}		
		}
	}else if(racetype == 12){
		memset(history2,0,sizeof(history2));
		for(i=0;i<DEFMAXRANK*3;i++) 
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history2[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history2[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				history2[j].catchcnt = atoi(perdata);
				history2[j].rank = j+1;
				j++;
			}		
		}
	}else if(racetype == 13){
		memset(history3,0,sizeof(history3));
		for(i=0;i<DEFMAXRANK*3;i++) 
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history3[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history3[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				history3[j].catchcnt = atoi(perdata);
				history3[j].rank = j+1;
				j++;
			}		
		}
	}else if(racetype == 14){
		memset(history4,0,sizeof(history4));
		for(i=0;i<DEFMAXRANK*3;i++) 
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history4[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history4[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				history4[j].catchcnt = atoi(perdata);
				history4[j].rank = j+1;
				j++;
			}		
		}
	}else if(racetype == 15){
		memset(history5,0,sizeof(history5));
		for(i=0;i<DEFMAXRANK*3;i++) 
		{
			if( i % 3 == 0){     
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history5[j].id,perdata);
			}else if( i % 3 == 1){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;		
				strcpy(history5[j].name,perdata);
			}else if( i % 3 == 2){
				if( !getStringFromIndexWithDelim( data, "|", i+1 , perdata, sizeof(perdata)))
					break;
				if( strlen(perdata) == 0 ) break;
				history5[j].catchcnt = atoi(perdata);
				history5[j].rank = j+1;
				j++;
			}		
		}
	}

}

#endif
