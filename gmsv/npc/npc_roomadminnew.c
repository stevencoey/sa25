#include "version.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "object.h"
#include "char.h"
#include "char_base.h"
#include "char_data.h"
#include "net.h"
#include "npcutil.h"
#include "handletime.h"
#include "readmap.h"
#include "npc_roomadminnew.h"
#include "npc_door.h"


/* 
 * ��  ǲ֧���� By Kawata
 *
 * ��    ���  �ʼ���̤��  ����  ��������  ����ƥ̤��ئ����
 */


enum{
	/* ����������  �� */
	CHAR_WORKINT_RANGE     = CHAR_NPCWORKINT1,
	/* �ݼ������� */
	CHAR_WORKINT_NEXTTIME  = CHAR_NPCWORKINT2,
	/*   ���ã��婣���󡣽 */
	CHAR_WORKINT_PRINTCLOSETIME = CHAR_NPCWORKINT3,
	/*   �и�������  �� */
	CHAR_WORKINT_SEARCHROOMTIME = CHAR_NPCWORKINT4,
	
};
enum {
	/* ���м�  � */
	CHAR_WORKCHAR_MYNAME   = CHAR_NPCWORKCHAR1,
	/* �ݼ�  �� */
	CHAR_WORKCHAR_NEXTROOM = CHAR_NPCWORKCHAR2,
	
};

/* �¼�ƽ����  ��      �÷�����*/
enum {
	/* ��  �м�  � */
	CHAR_WORKCHAR_ADMNAME = CHAR_NPCWORKCHAR1,
};


/* �����������̻�ëɡ  ���·�ū����������ls2/src/lss ���ռ��¸�϶�� */
#define     NPC_ROOMADMINNEW_DATADIR        "roomadmin/"
/* �����������̻Ｐ������ */
#define     NPC_ROOMADMINNEW_EXTENSION      ".room"
/* �¼�ƽ���뷸���������̻Ｐ������    ���̷���   */
#define     NPC_ROOMADMINNEW_RANKING_EXTENSION      ".score"
#define     NPC_ROOMADMINNEW_RANKING_EXTENSION_TMP  ".score.tmp"
/*   ��  ڿ���¼�ƽ��������̻� */
#define     NPC_ROOMADMINNEW_SCOREFILE      ".score"
/* 󻻧�Ի����󡾮�ն��������� */
#define     NPC_ROOMADMINNEW_PRINTCLOSETIME_DEFAULT     5

#define     NPC_ROOMADMINNEW_MSG_CLOSE_BEFORE       \
"%s %s %s�Ļ����ֹ��%d��ǰ��"
#define     NPC_ROOMADMINNEW_MSG_CLOSE              \
"�������ѽ�ֹ��"
#define     NPC_ROOMADMINNEW_MSG_NONE               \
"�ܿ�ϧ����û���˻�÷����ӵ��Ȩ��"
#define     NPC_ROOMADMINNEW_MSG_CONGRATULATION     \
"��ϲ����%s��÷��������Ȩ��%s�Ľ�Ǯȫ���������ˡ�%s�������Աѯ�ʷ��䰵�š�"
#define     NPC_ROOMADMINNEW_MSG_PRINTPASSWD_NONE    \
"�����δ���з������û�а��š�"
#define     NPC_ROOMADMINNEW_MSG_DSPPASSWD1          \
"��ķ����� %s %s %s��"
#define     NPC_ROOMADMINNEW_MSG_DSPPASSWD2          \
"����İ����� ��%s ����ֻҪ�ڶ�����˵��ʱ����β���ϡ�...�������˾Ϳ�������ࡣ�"
#define     NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION_NONE        \
"���յĻ������"
#define     NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION1     \
"�������ķ����� %s %s %s��"
#define     NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION2     \
"��ֹ���� %d:%d ������ֹ��Ϊֹ����%2dСʱ%02d�֡�"
#define     NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION3     \
"��͵ñ����� %s ��"

#define     NPC_ROOMADMINNEW_MSG_RANKING_INFO       \
"���Ǹ߶�ñ������а�"
#define     NPC_ROOMADMINNEW_MSG_RANKING            \
"Rank %2d:%4d/%02d/%02d %8d GOLD %s%s "

/* ��NPC�������������� */
#define     NPC_ROOMADMINNEW_MSGCOLOR           CHAR_COLORWHITE
#define     NPC_RANKING_MSGCOLOR                CHAR_COLORWHITE
#define		NPC_ROOMADMINNEW_2WEEK				60*60*24*14

static void NPC_RoomAdminNew_DspNextAuction( int meindex, int talkerindex);
static void NPC_RoomAdminNew_DspRoomPasswd( int meindex, int adminindex, int talkerindex);
static BOOL NPC_RoomAdminNew_SetNextAuction( int meindex);
static BOOL NPC_RoomAdminNew_WriteDataFile( int meindex, int pindex, char *roomname,
											char *passwd, BOOL flg);
static void NPC_RoomAdminNew_CreatePasswd( char *passwd, int passlen);
static void NPC_RoomAdminNew_SendMsgThisFloor( int index, char *buff);
static int NPC_RoomAdminNew_SearchPlayerHasMostMoney( int meindex);
static void NPC_RoomAdminNew_CloseProc( int meindex);
static int NPC_RoomAdminNew_WriteScoreFile( int meindex, int pindex);
static void NPC_RoomAdminNew_ReadScoreFile( int meindex, int talkerindex, 
											char *admname);

/************************************************
 * ������  
 ************************************************/
BOOL NPC_RoomAdminNewInit( int meindex )
{
	int     i;
	char    buf[256];
	char    argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	
	/* ���м���଼���  ����ʧ����  ��������ƥ��NPC��������ľئ�� */
	for( i = 3; ; i ++ ) {
		if( getStringFromIndexWithDelim(argstr, "|" , i , buf,sizeof(buf) )) {
			if( NPC_DoorSearchByName( buf) < 0 ) {
				print( "admin not created.������������֮ǰ�ȵȴ���\n");
				return FALSE;
			}
		}
		else{ 
			break;
		}
	}
	
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
	
	/* �������   */
	
	/* type������ */
	CHAR_setInt( meindex, CHAR_WHICHTYPE, CHAR_TYPEROOMADMIN);
	/*   ��  ����   */
	getStringFromIndexWithDelim(argstr, "|" , 1 , buf,sizeof(buf) );
	CHAR_setWorkInt( meindex , CHAR_WORKINT_RANGE , atoi( buf ));
	/* ���м�  ���   */
	if( !getStringFromIndexWithDelim(argstr, "|" , 2 , buf,sizeof(buf) )) {
		print( "arg err.admin not created.\n");
		return( FALSE);
	}
	CHAR_setWorkChar( meindex , CHAR_WORKCHAR_MYNAME , buf);
	
	/*   ���ã��婣���󡣽����� */
	CHAR_setWorkInt( meindex, CHAR_WORKINT_PRINTCLOSETIME, 0);
	
	/* �ݼ�  ����������������������ë������ */
	NPC_RoomAdminNew_SetNextAuction( meindex);
	
	return TRUE;
}
/************************************************
 * �������  
 * ��̫����  պ������ئ��    ƥئ�ʻ������У�
 * ƥ��姸������    ��������
 ************************************************/
void NPC_RoomAdminNewLoop( int meindex )
{
	int     hun;
	/* �����ַ������޻�����Ȼ���ئ����������  ��ئ���� */
	if( CHAR_getWorkInt( meindex, CHAR_WORKINT_NEXTTIME) == 0 ) {
		/* �Ը�������  ������ɸ�����ئ���� */
		struct  tm      tm1,tm2;
		time_t          t;
		memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));
		t = CHAR_getWorkInt( meindex, CHAR_WORKINT_SEARCHROOMTIME);
		memcpy( &tm2, localtime( &t), 
								sizeof( tm2));
		if( tm1.tm_year != tm2.tm_year ||
			tm1.tm_mday != tm2.tm_mday ||
			tm1.tm_mon != tm2.tm_mon )
		{
			NPC_RoomAdminNew_SetNextAuction( meindex);
		}
		return;
	}
	
	hun = ceil((double)(CHAR_getWorkInt( meindex, CHAR_WORKINT_NEXTTIME) - 
		   NowTime.tv_sec) / 60.0);
	/* �����ּ��ݼ��б�hun��Ѩ�̹ϵ����ݷ��߱�  �� */
	if( hun < 0 ) hun = 0;
	
	if( hun < NPC_ROOMADMINNEW_PRINTCLOSETIME_DEFAULT ) {
		/* 󻻧�Ի���ʱ�ئ�����գ�󻻧�Ի�����ƥ�ʣ����ɴ��� */
		if( hun != CHAR_getWorkInt( meindex, CHAR_WORKINT_PRINTCLOSETIME)) {
			char    buff[256];
			if( hun != 0 ) {
				char    buf2[1024];
				char	msgbuff[16];
				char    argstr2[NPC_UTIL_GETARGSTR_BUFSIZE];
				int		doorindex;

				/*   ����  ��   */
				doorindex = NPC_DoorSearchByName( CHAR_getWorkChar( meindex, CHAR_WORKCHAR_NEXTROOM));
				NPC_Util_GetArgStr( doorindex, argstr2, sizeof( argstr2));
				getStringFromIndexWithDelim( argstr2, "|" , 9 , 
											 buf2,sizeof(buf2) );
				/*   ����  */
				if( strlen(CHAR_getChar( doorindex, CHAR_NAME) ) != 0 ) {
					strcpy( msgbuff, "����");
				}
				else {
					msgbuff[0] = '\0';
				}
				/* ��󻻧�Ի�����ƥ�ʣ� */
				snprintf( buff, sizeof(buff), 
							NPC_ROOMADMINNEW_MSG_CLOSE_BEFORE, 
							buf2,
							CHAR_getChar( doorindex, CHAR_NAME),
							msgbuff,
							hun);
				
				CHAR_setWorkInt( meindex, CHAR_WORKINT_PRINTCLOSETIME, hun);
			}
			else {
				/* ��󻻧�Ի�����ף� */
				strcpy( buff, NPC_ROOMADMINNEW_MSG_CLOSE);
				CHAR_setWorkInt( meindex, CHAR_WORKINT_PRINTCLOSETIME, 0);
			}
			/* �𼰰׷�ʧ���������幻��˪�� */
			NPC_RoomAdminNew_SendMsgThisFloor( meindex, buff);
		}
	}
	/* 󻻧������ݼ���   */
	if( hun == 0 ) {
		NPC_RoomAdminNew_CloseProc( meindex);
	}
}

/************************************************
 *   �ƾ�����ľ���ݼ���  
 ************************************************/
void NPC_RoomAdminNewTalked( int meindex , int talkerindex , char *msg ,
							 int color )
{
	
	/* ��������������ʧѨ����1��������  ئ��  ɱ���£� */
	if(NPC_Util_CharDistance( talkerindex, meindex ) > 1)return;
	
	/*   �ƾ�����ľ���л�  ��ëŷԻ���������ɵ�������ë  ������ */
	/*NPC_RoomAdminNew_DspRoomPasswd( meindex, talkerindex);*/
	
	/* �ݼԷ������ޣ�  ��ë�ɵ� */
	NPC_RoomAdminNew_DspNextAuction( meindex, talkerindex);
}
/*-----------------------------------------------
 * 󻻧���ľ���ݼ���  
 *----------------------------------------------*/
static void NPC_RoomAdminNew_CloseProc( int meindex)
{
	int     pindex;
	char    buff[256];
	char    pname[32];
	int     failed = TRUE;
	
	/* ��  ����ë����  �Ȼ���������������ë���� */
	pindex = NPC_RoomAdminNew_SearchPlayerHasMostMoney( meindex);
	/* ��񲷮ʧ�� */
	if( pindex != -1) {
		/* ���ƥ  ���»����ƥ�� */
		while( 1) {
			char    passwd[9];
			int		doorindex;
			doorindex = NPC_DoorSearchByName( 
							CHAR_getWorkChar( meindex, CHAR_WORKCHAR_NEXTROOM));
			/* �ɵ�������ëϷ������ */
			NPC_RoomAdminNew_CreatePasswd( passwd, sizeof( passwd));
			/* �����̻����м�    ƽ����ƽ�ҷ�  ��  ��  ���ɵ�ë̤��  �� */
			if( NPC_RoomAdminNew_WriteDataFile( meindex, pindex,
							CHAR_getWorkChar( meindex, CHAR_WORKCHAR_NEXTROOM),
												passwd, 1)
				== FALSE) 
			{
				break;
			}
			/* ��ʧ���ɵ�ë������ */
			if( NPC_DoorSetPasswd( doorindex, passwd) == -1 ) {
				break;
			}
			/*   ������ë������ */
			CHAR_setWorkInt( doorindex, CHAR_WORKDOOREXPIRETIME, 
							CHAR_getWorkInt( meindex, CHAR_WORKINT_NEXTTIME)
							+ NPC_ROOMADMINNEW_2WEEK);
			/*   ڿ���¼�ƽ�����������͡������̻�̤��  �� */
			NPC_RoomAdminNew_WriteScoreFile( meindex, pindex);

			CHAR_setInt( pindex, CHAR_GOLD, 0);

			CHAR_send_P_StatusString( pindex , CHAR_P_STRING_GOLD);
			
			/* �ƻ�ƥ�絤�嶪�������� */
			strcpy( pname, CHAR_getChar( pindex, CHAR_NAME));
			snprintf( buff, sizeof(buff), 
						NPC_ROOMADMINNEW_MSG_CONGRATULATION, pname, pname, pname);
			failed = FALSE;
			break;
		}
	}
	
	/* ��񲷮�������׷���  ��  ���� */
	if( failed) {
		/* �����̻ﾮ�ճ�  �м���  ë���� */
		NPC_RoomAdminNew_WriteDataFile( meindex, -1,
					CHAR_getWorkChar( meindex, CHAR_WORKCHAR_NEXTROOM),
										NULL, 0);
		/* ��񲷮���������϶��������� */
		snprintf( buff, sizeof(buff), 
				NPC_ROOMADMINNEW_MSG_NONE);
	}
	/* �𼰰׷�ʧ���������幻��˪�� */
	NPC_RoomAdminNew_SendMsgThisFloor( meindex, buff);
	
	/* �ݼ�  ����������������������ë������ */
	NPC_RoomAdminNew_SetNextAuction( meindex);
	
}
/*-----------------------------------------------
 * ��  ����ë����  �Ȼ���������������ë����
 *----------------------------------------------*/
static int NPC_RoomAdminNew_SearchPlayerHasMostMoney( int meindex)
{
	int     i,j, range;
	int     fl,x,y, gold;
	int     doorindex;
	int     topindex;
	
	fl = CHAR_getInt( meindex, CHAR_FLOOR);
	x  = CHAR_getInt( meindex, CHAR_X);
	y  = CHAR_getInt( meindex, CHAR_Y);
	range = CHAR_getWorkInt( meindex , CHAR_WORKINT_RANGE);
	topindex = -1;
	
	/*   �м�  󡾮��  �  ڿ�ź�ëƩ���� */
	doorindex = NPC_DoorSearchByName( CHAR_getWorkChar( meindex, 
														CHAR_WORKCHAR_NEXTROOM));
	gold = atoi( CHAR_getWorkChar( doorindex, CHAR_WORKDOORGOLDLIMIT));
	/*   ��  ������ƽ�ҷ¼���ëƩ���� */
	for( i = x-range ; i <= x+range ; i++ ){
		for( j = y-range ; j <= y+range ; j ++ ){
			OBJECT  object;
			for( object = MAP_getTopObj( fl, i,j); object; 
				 object = NEXT_OBJECT(object) ) 
			{
				int objindex = GET_OBJINDEX( object);
				if( OBJECT_getType(objindex) == OBJTYPE_CHARA ){
					int     index;
					index = OBJECT_getIndex(objindex);
					if( CHAR_getInt( index,
								 CHAR_WHICHTYPE ) == CHAR_TYPEPLAYER )
					{
						if( CHAR_getInt( index, CHAR_GOLD) >= gold ) {
							if( topindex == -1 ) {
								topindex =  index;
							}
							else {
								/* ��������������  ʤ */
								int nowgold,topgold;
								nowgold = CHAR_getInt( index, CHAR_GOLD );
								topgold = CHAR_getInt( topindex, CHAR_GOLD);
								if( nowgold > topgold ) {
									topindex = index;
								}
								else if( nowgold == topgold ) {
									/* ����ئ����ì�ﻥ����� */
									int nowlv, toplv;
									nowlv = CHAR_getInt( index, CHAR_LV );
									toplv = CHAR_getInt( topindex, CHAR_LV);
									if( nowlv < toplv ) {
										topindex = index;
									}
									/* ��ľƥ��ݩ���ئ����ئ�շ¼�ĸةԪ��*/
									/* index���Ƽ��л��н��������Ƿ����� */
									else if( nowlv == toplv ) {
										if( RAND( 0,1)) {
											topindex = index;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return( topindex);
}
/*-----------------------------------------------
 * ϶�ý�ľ��index���׷�ʧ�����������������幻��
 * ����������ë˪�����£�
 *----------------------------------------------*/
static void NPC_RoomAdminNew_SendMsgThisFloor( int index, char *buff)
{
	int     i, fl;
	int     playernum = CHAR_getPlayerMaxNum();
	
	fl = CHAR_getInt( index, CHAR_FLOOR);
	for( i = 0; i < playernum; i ++ ) {
		if( CHAR_getInt( i, CHAR_FLOOR) == fl ) {
			/*   ���׻��������������������� */
			if( CHAR_getInt( i, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER ) {
				CHAR_talkToCli( i, index, buff, NPC_ROOMADMINNEW_MSGCOLOR);
			}
		}
	}
}
/*-----------------------------------------------
 * �ɵ�������ëϷ�����£�passlen��-1  ٯ������ľ�£�
 * makecdn.c������Ի��
 *----------------------------------------------*/
static void NPC_RoomAdminNew_CreatePasswd( char *passwd, int passlen)
{
#define NPC_ROOMADMINNEW_RAND()    ( (rand() /256) % 32768 )       /* 256ƥ��¼������Ѽ�  ߤ���׻� */
	int     i;
	char *candidates[]= {
		"A","B","D",   "E","F","G","H",   "J","K","L","M",   "N","P","Q","R",
		"T","W","X","Y",   "a","b","d","e",   "f","g","m","n",   "q","r","t",
		"0","1","2",   "3","4","5","6",   "7","8","9"
	};
	strcpy( passwd , "");
	for(i = 0;i < passlen -1 ; i++){
		strcat( passwd , candidates[ NPC_ROOMADMINNEW_RAND() % (sizeof( candidates)/sizeof(candidates[0])) ] );
	}
#undef NPC_ROOMADMINNEW_RAND
}
/*-----------------------------------------------
 * ��  ���ü������̻�巸����ë̤��  ��
 *
 * �����̻�  ��"���������.room"
 * ë������
 *
 * �����̻Ｐ���ɡ�Ѩ��������
 * cdkey|charaname|passwd|expiretime
 *
 *
 * ¦�Ѽ�flg �����������̤��  �ģݨ߷������հ����̻����
 *   Ի����TRUE����  
 *         FALSE:���վ�����  
 *----------------------------------------------*/
static BOOL NPC_RoomAdminNew_WriteDataFile( int meindex, int pindex, 
											char *roomname,
											char *passwd, int flg)
{
	char    *cdkey;
	char    charaname[32*2];
	char    expiretime[64];
	char    filename[128];
	char    outbuf[1024];
	FILE    *fp_src;
	
	/* �����̻�  ëϷ�� */
	strcpy( filename, NPC_ROOMADMINNEW_DATADIR);
	/*strcat( filename, CHAR_getWorkChar( meindex, CHAR_WORKCHAR_NEXTROOM));*/
	strcat( filename, roomname);
	strcat( filename, NPC_ROOMADMINNEW_EXTENSION);

	if( flg) {
		/*   ڿ�����м�    ƽ�� */
        //getcdkeyFromCharaIndex( pindex, cdkey, sizeof(cdkey ));
		cdkey = CHAR_getChar( pindex, CHAR_CDKEY);
		/*   ڿ�����м�ƽ�ҷ�   */
		makeEscapeString(  CHAR_getChar( pindex, CHAR_NAME),
							charaname, sizeof( charaname));
		/* �����ľ���� */
		snprintf( expiretime, sizeof( expiretime), "%d", 
					CHAR_getWorkInt( meindex, CHAR_WORKINT_NEXTTIME)
					+ NPC_ROOMADMINNEW_2WEEK   /* �����   */
					);

		/* ��������ƥ������ */
		snprintf( outbuf, sizeof( outbuf), "%s|%s|%s|%s|",
					cdkey, charaname, passwd, expiretime);
		fp_src = fopen( filename, "w");
		if( !fp_src) {
			print( "[%s:%d] File Open Error\n", __FILE__, __LINE__);
			return FALSE;
		}
		fputs( outbuf, fp_src);
		fclose( fp_src);
	}
	else {
		unlink( filename);
	}
	
	return TRUE;
	
}
/*-----------------------------------------------
 * �����������̻�ë  ��ƥ�������  �л�ؤľ��
 * ������ëɡ  �ƻ�߯�ʣ�
 *----------------------------------------------*/
BOOL NPC_RoomAdminNew_ReadFile( char *roomname, NPC_ROOMINFO *data)
{
	FILE    *fp;
	char    line[1024];
	char    buf2[128];
	char    filename[128];
	
	/* �����̻�  ëϷ�� */
	strcpy( filename, NPC_ROOMADMINNEW_DATADIR);
	strcat( filename, roomname);
	strcat( filename, NPC_ROOMADMINNEW_EXTENSION);
	
	fp = fopen( filename, "r");
	if( !fp) return FALSE;
	fgets( line, sizeof( line), fp);
	/* ������������ */
	getStringFromIndexWithDelim( line, "|" , 1 , buf2,sizeof(buf2) );
	strcpy( data->cdkey, buf2);
	getStringFromIndexWithDelim( line, "|" , 2 , buf2,sizeof(buf2) );
	strcpy( data->charaname, makeStringFromEscaped( buf2));
	getStringFromIndexWithDelim( line, "|" , 3 , buf2,sizeof(buf2) );
	strcpy( data->passwd, buf2);
	getStringFromIndexWithDelim( line, "|" , 4 , buf2,sizeof(buf2) );
	if( strcmp( buf2, "max") == 0 ) {
		data->expire = 0xffffffff;
	}
	else {
		data->expire = atoi( buf2);
	}
	fclose( fp);
	return TRUE;
}
/*-----------------------------------------------
 * �ݼ�������������������  ��  ë�����ͱ屾��������
 *----------------------------------------------*/
static BOOL NPC_RoomAdminNew_SetNextAuction( int meindex)
{
	int     i, j;
	int     hitindex = -1;
	int     charnum = CHAR_getCharNum();
	char    argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	struct  tm  tmwk;
	
	/*     ئ��ë��  ����*/
	memcpy( &tmwk, localtime( ( time_t *)&NowTime.tv_sec), sizeof( tmwk));
	
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));

	/* ��ʧë�������£�NPC_DoorSearchByName()ë���������������
	 *    �ͻ�����ئԻ����ئ��ƥ���������ϻ�
	 */
	for( i = CHAR_getPlayerMaxNum()+ CHAR_getPetMaxNum(); 
		 i < charnum;
		 i ++ )
	{
		if( CHAR_getInt( i, CHAR_WHICHTYPE) != CHAR_TYPEDOOR) {
			continue;
		}
		if( CHAR_getWorkInt( i, CHAR_WORKDOORPASSFLG) == 0 ) {
			continue;
		}
		/*   󡻥��଼��ּ��������������� */
		for( j = 3; ; j ++ ) {
			int rc;
			char    *doorname;
			char    buf[256];
			int     dataexist;
			NPC_ROOMINFO data;
			
			rc = getStringFromIndexWithDelim( argstr, "|" , j,
												buf,sizeof(buf) );
			if( rc != TRUE ) {
				break;
			}
			doorname = CHAR_getWorkChar( i, CHAR_WORKDOORNAME);
			/* ��ʧ  ���������� */
			if( strcmp( buf, doorname ) != 0 ) {
				continue;
			}
			/*     ���������� */
			if( atoi( CHAR_getWorkChar( i, CHAR_WORKDOORWEEK)) != tmwk.tm_wday) {
				break;
			}
			/* ��ɱ����� */
			memset( &data, 0, sizeof( data));
			/* �����������̻ﾮ�շ�����ë  �� */
			if( NPC_RoomAdminNew_ReadFile( doorname, &data) ) {
				struct tm   tmdoor;
				/* ���ƥ�����ľ��������ë֧�Ȼ�֧�£�
				 * ���׻��ľ�������հ����̻�ë��������
				 * ��  ����ؤ��Ի�𼰻�����ɧ���Ƿ�ئ����  ����
				 */
				if( data.expire < NowTime.tv_sec) {
					NPC_RoomAdminNew_WriteDataFile( meindex, -1,doorname, NULL, 0);
					/* ��ʧ����ľ�б�*/
					NPC_DoorSetPasswd( i, "8hda8iauia90494jasd9asodfasdf89");
					CHAR_setWorkInt( i, CHAR_WORKDOOREXPIRETIME, 0xffffffff);
					
					break;
				}
				/* ��  ƥ�ľ��  ��ئ��     */
				memcpy( &tmdoor, localtime( ( time_t *)&data.expire), sizeof( tmdoor));
				if( tmdoor.tm_mon != tmwk.tm_mon ||
					tmdoor.tm_mday != tmwk.tm_mday)
				{
					break;
				}
				dataexist = TRUE;
			}
			else {
				dataexist = FALSE;
			}
			/* �ػ������޷�Ի  ƥؤ���ǣ� */
			if( tmwk.tm_hour > atoi( CHAR_getWorkChar( i, CHAR_WORKDOORHOUR))){
				break;
			}
			if( tmwk.tm_hour == atoi( CHAR_getWorkChar( i, CHAR_WORKDOORHOUR))) {
				if( tmwk.tm_min >= atoi( CHAR_getWorkChar( i, CHAR_WORKDOORMINUTE))) {
					break;
				}
			}
			/* �����ƥ��������Իؤ���ڳ���ʧ���ݷ�������·�ɡ��
			 * ؤ�����е��ǣ۳�ľ����ְ����ʧ�������������£�
			 */
			if( hitindex == -1 ) {
				hitindex = i;
			}
			else {
				/* �������޻����ʣ��ػ�ŷԻ��ľ���������е�������ƴ��� */
				if( atoi( CHAR_getWorkChar( hitindex, CHAR_WORKDOORHOUR)) 
					> atoi( CHAR_getWorkChar( i, CHAR_WORKDOORHOUR)))
				{
					hitindex = i;
				}
				else if( atoi( CHAR_getWorkChar( hitindex, CHAR_WORKDOORHOUR)) 
						== atoi( CHAR_getWorkChar( i, CHAR_WORKDOORHOUR)))
				{
					if( atoi( CHAR_getWorkChar( hitindex, CHAR_WORKDOORMINUTE)) 
						> atoi( CHAR_getWorkChar( i, CHAR_WORKDOORMINUTE)))
					{
						hitindex = i;
					}
					else if( atoi( CHAR_getWorkChar( hitindex, CHAR_WORKDOORMINUTE)) 
						== atoi( CHAR_getWorkChar( i, CHAR_WORKDOORMINUTE)))
					{
						NPC_ROOMINFO    datawk;
						int     hitindexdataexist;
						hitindexdataexist = NPC_RoomAdminNew_ReadFile( 
									CHAR_getWorkChar( hitindex, CHAR_WORKDOORNAME),
									&datawk);
						if( hitindexdataexist < dataexist ) {
							hitindex = i;
						}
						/* �����ƥ  �·���ئ���Ƽ�index�����������£�*/
						
					}
				}
			}
		}
	}
	if( hitindex > 0 ) {
		/* ��������ë������ */
		tmwk.tm_sec = 0;
		tmwk.tm_min = atoi( CHAR_getWorkChar( hitindex, CHAR_WORKDOORMINUTE));
		tmwk.tm_hour = atoi( CHAR_getWorkChar( hitindex, CHAR_WORKDOORHOUR));
		CHAR_setWorkInt( meindex, CHAR_WORKINT_NEXTTIME, mktime( &tmwk));
		/*   ��  ë������ */
		CHAR_setWorkChar( meindex, CHAR_WORKCHAR_NEXTROOM,
						 CHAR_getWorkChar( hitindex, CHAR_WORKDOORNAME));
		
	}
	else {
		CHAR_setWorkInt( meindex, CHAR_WORKINT_NEXTTIME, 0);
		/*   ��  ë������ */
		CHAR_setWorkChar( meindex, CHAR_WORKCHAR_NEXTROOM,"" );
	
	}
	/* ��������ë������ */
	CHAR_setWorkInt( meindex, CHAR_WORKINT_SEARCHROOMTIME, NowTime.tv_sec);
	return( TRUE);
}
/*-----------------------------------------------
 *   �ƾ�����ľ���л�  ��ëŷԻ���������ɵ�������ë  ������
 *----------------------------------------------*/
static void NPC_RoomAdminNew_DspRoomPasswd( int index, int adminindex, int talkerindex)
{
	int     i, rc, flg = FALSE;
	char    argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	char    argstr2[NPC_UTIL_GETARGSTR_BUFSIZE];
	char    *cdkey;
	char    buf[128];
	char	buff[16];
	char    buf2[1024];

	NPC_ROOMINFO    info;

    //getcdkeyFromCharaIndex( talkerindex, cdkey, sizeof(cdkey));
	cdkey = CHAR_getChar( talkerindex, CHAR_CDKEY);
	
	NPC_Util_GetArgStr( adminindex, argstr, sizeof( argstr));
	for( i = 3; ; i ++ ) {
		if( getStringFromIndexWithDelim(argstr, "|" , i , buf,sizeof(buf) )) {
			rc = NPC_RoomAdminNew_ReadFile( buf, &info);
			if( rc == TRUE ) {
				/* CDKEY ��ƽ�ҷ¼�  󡻥���������� */
				if( strcmp( info.cdkey, cdkey) == 0 &&
					strcmp( info.charaname, 
							CHAR_getChar( talkerindex, CHAR_NAME)) == 0 )
				{
					int doorindex;
					char    msgbuf[256];
					/*   ����  ��   */
					doorindex = NPC_DoorSearchByName( buf);
					NPC_Util_GetArgStr( doorindex, argstr2, sizeof( argstr2));
					getStringFromIndexWithDelim( argstr2, "|" , 9 , 
												 buf2,sizeof(buf2) );
					/*   ����  */
					if( strlen(CHAR_getChar( doorindex, CHAR_NAME) ) != 0 ) {
						strcpy( buff, "����");
					}
					else {
						buff[0] = '\0';
					}
					
					/* ���������Ȿ���� */
					snprintf( msgbuf, sizeof( msgbuf), 
								NPC_ROOMADMINNEW_MSG_DSPPASSWD1,
								buf2,
								CHAR_getChar( doorindex, CHAR_NAME),
								buff
							);
					/* ����������˪�� */
					CHAR_talkToCli( talkerindex, index,msgbuf, 
									NPC_ROOMADMINNEW_MSGCOLOR);

					/* ���������Ȿ���� */
					snprintf( msgbuf, sizeof( msgbuf), 
								NPC_ROOMADMINNEW_MSG_DSPPASSWD2,
								CHAR_getWorkChar(doorindex, CHAR_WORKDOORPASSWD)
							);
					/* ����������˪�� */
					CHAR_talkToCli( talkerindex, index,msgbuf, 
									NPC_ROOMADMINNEW_MSGCOLOR);
					flg = TRUE;
				}
			}
		}
		else {
			break;
		}
	}
	if( !flg) {
		CHAR_talkToCli( talkerindex, index,
						NPC_ROOMADMINNEW_MSG_PRINTPASSWD_NONE,
						NPC_ROOMADMINNEW_MSGCOLOR);
	}
}
/*-----------------------------------------------
 * �ݼԷ������ޣ�  ��ë�ɵ�
 *----------------------------------------------*/
static void NPC_RoomAdminNew_DspNextAuction( int meindex, int talkerindex)
{
	char    msgbuf[2048];
	time_t  closetime;
	
	closetime = CHAR_getWorkInt( meindex, CHAR_WORKINT_NEXTTIME);
	/* �ݼ��������޻�����Ȼ���ئ��ľ�ɣ�����Ȼ������ϣۣ�����������*/
	if( closetime == 0 ) {
		snprintf( msgbuf, sizeof( msgbuf),
					NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION_NONE
				);
		CHAR_talkToCli( talkerindex, meindex,msgbuf, NPC_ROOMADMINNEW_MSGCOLOR);
	}
	else {
		int     doorindex;
		char    argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
		char    explain[1024];      /*    */
		char	buff[16];
		struct  tm tmwk;
		int     difftime;
		/*   ����  ë��   */
		doorindex = NPC_DoorSearchByName( 
					CHAR_getWorkChar( meindex, CHAR_WORKCHAR_NEXTROOM));
		NPC_Util_GetArgStr( doorindex, argstr, sizeof( argstr));
		getStringFromIndexWithDelim(argstr, "|" , 9 , explain,sizeof(explain));
		/*   ����  */
		if( strlen(CHAR_getChar( doorindex, CHAR_NAME) ) != 0 ) {
			strcpy( buff, "����");
		}
		else {
			buff[0] = '\0';
		}
		snprintf( msgbuf, sizeof( msgbuf),
					NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION1,
					explain,
					CHAR_getChar( doorindex, CHAR_NAME),
					buff
				);
		CHAR_talkToCli( talkerindex, meindex,msgbuf, NPC_ROOMADMINNEW_MSGCOLOR);
		/* 󻻧�Ի���� */
		memcpy( &tmwk, localtime( &closetime), sizeof( tmwk));
		difftime = closetime - NowTime.tv_sec;
		snprintf( msgbuf, sizeof( msgbuf),
					NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION2,
					tmwk.tm_hour,
					tmwk.tm_min,
					difftime / 3600,
					(difftime % 3600) / 60
				);
		CHAR_talkToCli( talkerindex, meindex,msgbuf, NPC_ROOMADMINNEW_MSGCOLOR);
		
		/* Ʊ���ź� */
		snprintf( msgbuf, sizeof( msgbuf),
					NPC_ROOMADMINNEW_MSG_DSPNEXTAUCTION3,
					CHAR_getWorkChar( doorindex, CHAR_WORKDOORGOLDLIMIT)
				);
		CHAR_talkToCli( talkerindex, meindex,msgbuf, NPC_ROOMADMINNEW_MSGCOLOR);
	}
	
}
/*-----------------------------------------------
 *   ڿ���¼�ƽ�����������͡������̻�̤��  ��
 *
 * �����̻�  ��"�����ߵ�����.score"
 *   ���̷��������̻�  ��"�����ߵ�����.score.tmp"
 *
 * �����̻���ɡ�Ѩ��������
 * gold|time|cdkey|charaname|owntitle
 * ƥ��ߵ���ƥ��
 *
 *   Ի���� ��  ���¼�ƽ�������˾�߯�ʣ۷¼�ƽ�����  ��ئ��ľ�ɨߣ�
 *          ��  ��-1
 *----------------------------------------------*/
static int NPC_RoomAdminNew_WriteScoreFile( int meindex, int pindex)
{
#define NPC_RANKING_DEFAULT     10
	int     ret = 0;
	char    *cdkey;
	char    charaname[32*2],owntitle[32*2];
	char    filename_dest[128],filename[128];
	char    outbuf[1024], line[1024];
	FILE    *fp_src, *fp_dest;

	/* �����̻�  ëϷ�� */
	strcpy( filename, NPC_ROOMADMINNEW_DATADIR);
	strcat( filename, CHAR_getWorkChar( meindex, CHAR_WORKCHAR_MYNAME));
	strcpy( filename_dest, filename);
	strcat( filename, NPC_ROOMADMINNEW_RANKING_EXTENSION);
	strcat( filename_dest, NPC_ROOMADMINNEW_RANKING_EXTENSION_TMP);
	
	/*   ڿ�����м�    ƽ�� */
	//getcdkeyFromCharaIndex( pindex , cdkey , sizeof(cdkey ));
	cdkey = CHAR_getChar( pindex, CHAR_CDKEY);
	
	/*   ڿ�����м�ƽ�ҷ�   */
	makeEscapeString(  CHAR_getChar( pindex, CHAR_NAME),
						charaname, sizeof( charaname));
	
	/*   ڿ�����м���į */
	makeEscapeString(  CHAR_getChar( pindex, CHAR_OWNTITLE),
						owntitle, sizeof( owntitle));

	/* ��������ƥ������ */
	snprintf( outbuf, sizeof( outbuf), "%d|%d|%s|%s|%s|\n",
				CHAR_getInt( pindex, CHAR_GOLD),
				CHAR_getWorkInt( meindex,CHAR_WORKINT_NEXTTIME),
				cdkey, charaname, owntitle );


	fp_dest = fopen( filename_dest, "w");
	if( !fp_dest) return -1;
	
	fp_src = fopen( filename, "r");
	if( !fp_src ) {
		/*   �ټ�����ئ��ƥ��������̤�л��δ�Ի */
		fclose( fp_dest);
		fp_src = fopen( filename, "w");
		if( !fp_src) return -1;
		fputs( outbuf, fp_src);
		fclose( fp_src);
		return 1;
	}
	else {
		int cnt = 1;
		char buf[128];
		while( fgets( line, sizeof( line), fp_src)) {
			getStringFromIndexWithDelim( line, "|" , 1 , buf,sizeof(buf) );
			if( ret == 0 ) {
				/*   ڿ�ź�  ʤ */
				if( atoi( buf) < CHAR_getInt( pindex, CHAR_GOLD) ) {
					/* �ź���  �Ȼ����¼�ƥ��   */
					fputs( outbuf, fp_dest);
					fputs( line, fp_dest);
					ret = cnt;
					cnt ++;
				}
				else {
					/* ��ľ��½����������̤�� */
					fputs( line, fp_dest);
				}
			}
			else {
				/* ��ľ��½����������̤�� */
				fputs( line, fp_dest);
			}
			cnt ++;
			if( cnt > NPC_RANKING_DEFAULT ) break;
		}
		/* �¼�ƽ���뻥����۹�Ƿ�Ȼ���ئ���� */
		if( ret == 0 && cnt <= NPC_RANKING_DEFAULT ) {
			fputs( outbuf, fp_dest);
			ret = cnt;
		}
	}
	fclose( fp_src);
	fclose( fp_dest);
	/*   ��  ľ���� */
	/* ���������̻���������¿�ǩئ  ܷ���Ȼ�����  ����Ʊ���У� */
	fp_src = fopen( filename_dest, "r");
	if( !fp_src) return -1;
	fp_dest = fopen( filename, "w");
	if( !fp_dest) {
		fclose( fp_src);
		return -1;
	}
	while( fgets( line, sizeof( line), fp_src)) {
		fputs( line, fp_dest);
	}
	fclose( fp_dest);
	fclose( fp_src);
	return( ret);
#undef  NPC_RANKING_DEFAULT
}
/*-----------------------------------------------
 *   ڿ���¼�ƽ��������̻�  ��  �ġ�  ��
 *
 * ¦�� meindex     ƽ�ҷ��̼������͵��۷¼�ƽ����NPC���̼������͵���
 *      talkerindex   �ƾ�����ľ���м�index�۳�ľ��talkë˪�����£�
 *      admname     ��  ǲ�н��ϼ�  �
 *      
 *----------------------------------------------*/
static void NPC_RoomAdminNew_ReadScoreFile( int meindex, int talkerindex, 
											char *admname)
{
	int     cnt;
	char    filename[128];
	char    line[1024];
	FILE    *fp;
	
	/* �����̻�  ëϷ�� */
	strcpy( filename, NPC_ROOMADMINNEW_DATADIR);
	strcat( filename, admname);
	strcat( filename, NPC_ROOMADMINNEW_RANKING_EXTENSION);
	
	fp = fopen( filename, "r");
	if( !fp) return;
	cnt = 1;
	while( fgets( line, sizeof( line), fp)) {
		int     gold,bidtime;
		char    charaname[32],owntitle[32];
		char    tmp[64];
		char    outbuf[1024], buf[256];
		struct  tm tmwk;
		
		getStringFromIndexWithDelim( line, "|" , 1 , buf,sizeof(buf) );
		gold = atoi(buf);
		getStringFromIndexWithDelim( line, "|" , 2 , buf,sizeof(buf) );
		bidtime = atoi(buf);
		getStringFromIndexWithDelim( line, "|" , 4 , buf,sizeof(buf) );
		strcpy( charaname, makeStringFromEscaped( buf));
		getStringFromIndexWithDelim( line, "|" , 5 , buf,sizeof(buf) );
		strcpy( owntitle, makeStringFromEscaped( buf));
		if( strlen( owntitle) != 0 ) {
			strcpy( tmp, "[");
			strcat( tmp, owntitle);
			strcat( tmp, "] ");
			strcpy( owntitle, tmp);
		}
		else {
			strcpy( owntitle, " ");
		}
		memcpy( &tmwk, localtime( (time_t *)&bidtime), sizeof( tmwk));
		
		snprintf( outbuf, sizeof( outbuf),
				NPC_ROOMADMINNEW_MSG_RANKING,
				cnt, 
				tmwk.tm_year + 1900,
				tmwk.tm_mon+1,
				tmwk.tm_mday,
				gold,charaname,owntitle
				);
		CHAR_talkToCli( talkerindex, meindex,outbuf, 
						NPC_RANKING_MSGCOLOR);
		cnt ++;
	}
	fclose( fp);
	
}
/************************************************
 * ������  
 ************************************************/
BOOL NPC_RankingInit( int meindex )
{
	char    buf[256];
	char    argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
	
	/* �������   */
	
	/* type������ */
	CHAR_setInt( meindex, CHAR_WHICHTYPE, CHAR_TYPERANKING);
	/* ��  �м�  ���   */
	if( !getStringFromIndexWithDelim(argstr, "|" , 1 , buf,sizeof(buf) )) {
		print( "arg err.rankingNPC not created.\n");
		return( FALSE);
	}
	CHAR_setWorkChar( meindex , CHAR_WORKCHAR_ADMNAME , buf);
	
	return TRUE;
}
/************************************************
 *   �ƾ�����ľ���ݼ���  
 ************************************************/
void NPC_RankingTalked( int meindex , int talkerindex , char *msg ,
							 int color )
{
	/* ��������������ʧѨ����1��������  ئ��  ɱ���£� */
	if(NPC_Util_CharDistance( talkerindex, meindex ) > 1)return;
	
	CHAR_talkToCli( talkerindex, meindex,
					NPC_ROOMADMINNEW_MSG_RANKING_INFO, 
					NPC_RANKING_MSGCOLOR);
	
	NPC_RoomAdminNew_ReadScoreFile( meindex, talkerindex, 
						CHAR_getWorkChar( meindex, CHAR_WORKCHAR_ADMNAME));
	
}
/************************************************
 * ������  
 ************************************************/
BOOL NPC_PrintpassmanInit( int meindex )
{
	char    buf[256];
	char    argstr[NPC_UTIL_GETARGSTR_BUFSIZE];
	
	NPC_Util_GetArgStr( meindex, argstr, sizeof( argstr));
	
	/* �������   */
	
	/* type������ */
	CHAR_setInt( meindex, CHAR_WHICHTYPE, CHAR_TYPEPRINTPASSMAN);
	/* ��  �м�  ���   */
	if( !getStringFromIndexWithDelim(argstr, "|" , 1 , buf,sizeof(buf) )) {
		print( "arg err.passmanNPC not created.\n");
		return( FALSE);
	}
	CHAR_setWorkChar( meindex , CHAR_WORKCHAR_ADMNAME , buf);
	
	return TRUE;
}
/************************************************
 *   �ƾ�����ľ���ݼ���  
 ************************************************/
void NPC_PrintpassmanTalked( int meindex , int talkerindex , char *msg ,
							 int color )
{
	int i;
	int charnum;
	/* ��������������ʧѨ����2��������  ئ��  ɱ���£� */
	if(NPC_Util_CharDistance( talkerindex, meindex ) > 2)return;
	
	/* �����ü���  �м�  �ë��������*/
	charnum  = CHAR_getCharNum();
	for(i= CHAR_getPlayerMaxNum() +CHAR_getPetMaxNum();
		i<charnum;
		i++){

		if( CHAR_getCharUse(i) &&
			CHAR_getInt( i ,CHAR_WHICHTYPE) == CHAR_TYPEROOMADMIN ){
			if( strcmp( CHAR_getWorkChar(i,CHAR_WORKCHAR_MYNAME),
						CHAR_getWorkChar( meindex, CHAR_WORKCHAR_ADMNAME))
				==0 )
			{
				break;
			}
		}
	}
	if( i < charnum ) {
		NPC_RoomAdminNew_DspRoomPasswd( meindex, i, talkerindex);
	}
	else {
		CHAR_talkToCli( talkerindex, meindex, "������", NPC_RANKING_MSGCOLOR);
	}
}
/* end of roomadminnew */
