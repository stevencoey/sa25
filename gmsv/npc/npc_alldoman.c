#include "version.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "object.h"
#include "char_base.h"
#include "char.h"
#include "util.h"
#include "npcutil.h"
#include "item.h"
#include "readmap.h"
#include "enemy.h"
#include "chatmagic.h"
#include "log.h"
#include "npc_itemchange.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "npc_eventaction.h"
#include "npc_charm.h"

#ifdef _ALLDOMAN
#include "npc_alldoman.h"

#ifdef _FIX_ALLDOMAN
void Send_S_herolist( int ti );
#define MAX_HERO_LIST 50
typedef struct _tagHerolistBase
{
	char strings[3][64];
	int intdata[3];
	int use;
}HerolistBase;
HerolistBase Herolist[MAX_HERO_LIST];
#else
#define MAX_HERO_COLUMN 7
#define MAX_HERO_LIST 100
char Herolist[MAX_HERO_LIST][MAX_HERO_COLUMN][72];
#endif



int sort[MAX_HERO_LIST][2];
int countnum = -1 ;
int showpage = 1 ;
int totalpage = 1;

static void NPC_Alldoman_selectWindow( int meindex, int talker, int seqno,char *data);

void NPC_AlldomanWindowTalked(int meindex, int talkerindex, int seqno, int select,char *data)
{
	char buf2[256];
	char buf[256];
    int i;
    int playernum = CHAR_getPlayerMaxNum();
	//andy_log
	print( "seqno:%d\n", seqno);
	switch(seqno)
	{
	case NPC_ALLDOMAN_MAIN_WND:
		if ( select == WINDOW_BUTTONTYPE_NEXT ) {
			showpage ++ ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex , 4 ,"");
		}
		if ( select == WINDOW_BUTTONTYPE_OK ) {
			//CHAR_talkToCli(talkerindex,meindex,"������ˣ�",CHAR_COLORRED);
			showpage = 1 ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex, 5 ,"");
		}
		break;
	case NPC_ALLDOMAN_SELECT_WND:
		if(atoi(data)==2) {
			//CHAR_talkToCli(talkerindex,meindex,"����",CHAR_COLORRED);
			if ( CHAR_getInt( talkerindex , CHAR_FLOOR ) != 8200 ) {
				CHAR_talkToCli(talkerindex,-1,"����ֻ�ܸ�������Ӣ�ۿ�д��",CHAR_COLORRED);
				break;
			}
			if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) > 132 ||
				CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) <= 0 ) {
				CHAR_setWorkInt( talkerindex  , CHAR_WORKHEROFLOOR , 0 ) ;
				CHAR_talkToCli(talkerindex,-1,"����ֻ�ܸ�������Ӣ�ۿ�д��",CHAR_COLORRED);
				break;
			}
			sprintf( buf2 , "���ó��˴��������%3d��֤������ͼ���Լ�����������ʯ����" ,	 CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) );
			CHAR_talkToCli(talkerindex,-1,buf2,CHAR_COLORRED);
			if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) == 132 ) {
				sprintf( buf2 , "%s" , "��˹��½Ӣ��" ) ; 
				//���ﶥ���ȫ��ϵ�㲥
				sprintf( buf , "%s������Ӣ��ս�����ɷ�֮���齫�����Ϣ֪ͨȫ���磬�µ�Ӣ�۵�����!!" , CHAR_getChar( talkerindex, CHAR_NAME ) );
				for( i = 0 ; i < playernum ; i++) {
					if( CHAR_getCharUse(i) != FALSE ) {
						CHAR_talkToCli( i, -1, buf, CHAR_COLORBLUE2);
					}
				}
			}
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 130 )
				sprintf( buf2 , "%s" , "��˹��½սʿ" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 125 )
				sprintf( buf2 , "%s" , "��˹��½��ʿ" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 120 )
				sprintf( buf2 , "%s" , "������˹Ӣ��" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 115 )
				sprintf( buf2 , "%s" , "˹�ʹ�սʿ" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 110 )
				sprintf( buf2 , "%s" , "��ķ����սʿ" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 100 )
				sprintf( buf2 , "%s" , "������˿սʿ" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 80 )
				sprintf( buf2 , "%s" , "��̹սʿ" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 60 )
				sprintf( buf2 , "%s" , "���ض�սʿ" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 40 )
				sprintf( buf2 , "%s" , "��ħ��ʿ" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 20 )
				sprintf( buf2 , "%s" , "ʥ����ʿ" ) ; 
			else if ( CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) >= 1 )
				sprintf( buf2 , "%s" , "������ʿ" ) ; 
			else
				sprintf( buf2 , " " );
			saacproto_UpdataStele_send ( 				
				acfd , 
				CHAR_getChar( talkerindex , CHAR_CDKEY ),
				CHAR_getChar( talkerindex , CHAR_NAME ) , 
				buf2 , 
				CHAR_getInt( talkerindex , CHAR_LV ) , 
				CHAR_getInt( talkerindex , CHAR_TRANSMIGRATION) , 
				10 	, 
				CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ) 
				) ; 
			//���WorkFloor
			CHAR_setWorkInt( talkerindex  , CHAR_WORKHEROFLOOR , 0 ) ;
		}
		if(atoi(data)==4) {
			//CHAR_talkToCli(talkerindex,meindex,"����",CHAR_COLORRED);
			showpage = 1 ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex, 6 ,"");
		}
		if(atoi(data)==6) {
			//CHAR_talkToCli(talkerindex,meindex,"����",CHAR_COLORRED);
		}
		break;
	case NPC_ALLDOMAN_LIST_WND:
		if( select == WINDOW_BUTTONTYPE_OK ){
			print("\nSyu log FloorWorkint => %d" , CHAR_getWorkInt( talkerindex  , CHAR_WORKHEROFLOOR ));
		}
		if( select == WINDOW_BUTTONTYPE_PREV && showpage > 1 ) {
			showpage -- ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex, 6 ,"" );
		}
		if( select == WINDOW_BUTTONTYPE_NEXT && showpage < totalpage ) {
			showpage ++ ; 
			NPC_Alldoman_selectWindow(meindex , talkerindex, 6 ,"");
		}
		break;
	}
}

static void NPC_Alldoman_selectWindow( int meindex, int talker, int num, char *data)
{
	 int buttontype = 0,windowtype = 0,windowno = 0 , i ;
     int fd = getfdFromCharaIndex( talker);
	 char token[1024];
	 char token2[100 * 100];


	 //andy_log
	 print( "NPC_Alldoman_selectWindow( num:%d)\n", num);
     switch(num)
	 {
	 case 4:
		 if ( showpage == 1 ) {
			 buttontype = WINDOW_BUTTONTYPE_NEXT;
			 sprintf( token, "������������ʥ��ʱ����³���ִ����������У�����֮������һ���⣬��˹����ĳ��ϴ�ɹ�ʶ������������ʿ��ǰ��������Ⱥ����Ļ�������Ⱥ��ʿ�ں��������ڸ���Ⱥ������ս�����ϣ�ʵ��������⣬��ʿ���������أ����ŵ���ʿ��ѡ���ڻ��ض�����ͨ·���������");
		 }
		 else if ( showpage == 2 ) {
			 buttontype = WINDOW_BUTTONTYPE_NEXT;
			 sprintf( token, "��ĵĵֿ������峤���ټ�������������������ͨ����³�ĺ���ͨ��������ڴ�������ӿ��ǰ������ڷ������˵�Ϊ������������ʥ��ʱ�����죬Ϊ�������������ʥ��ʱ����������ʿ������ʯ�������˵ض�������Ϊ��Ӣ��ս��������������Ч�����ҵ��о����ﵽǿ��ǿ");
		 }
		 else if ( showpage == 3) {
			 buttontype = WINDOW_BUTTONTYPE_OK;
			 sprintf( token, "��ΪĿ�ģ�����˹��½������������Ϣ�������Ӣ��������´���Ӣ��ս�������������ģ��ش�گ��ȫ��˹Ӣ�ۣ�\n\n                      Ӣ��ս������ίԱ��");
		 }
		 windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		 windowno = NPC_ALLDOMAN_MAIN_WND;
		 lssproto_WN_send(fd, windowtype, buttontype, windowno,
			 CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX), token );
		 break;
	 case 5:
		 buttontype = WINDOW_BUTTONTYPE_NONE;
		 windowtype = WINDOW_MESSAGETYPE_SELECT;
		 windowno = NPC_ALLDOMAN_SELECT_WND;
		 sprintf(token,
			"0\n\n����������������      ��дʯ��"
			 "\n\n������      ��������������������"
			 "\n\n����������      ��������ȡ��"
			 );
		 lssproto_WN_send( fd, windowtype, buttontype, windowno,
			 CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),token);
		 break;
	 case 6:
		 if ( totalpage != 1 && showpage == 1 ) 
			 buttontype = WINDOW_BUTTONTYPE_OK | WINDOW_BUTTONTYPE_NEXT;
		 else if ( totalpage == 1 )
			 buttontype = WINDOW_BUTTONTYPE_OK ; 
		 else if ( totalpage != 1 && showpage == totalpage )
			 buttontype = WINDOW_BUTTONTYPE_OK | WINDOW_BUTTONTYPE_PREV;
		 else if ( totalpage != 1 && showpage != 1 && showpage != totalpage ) 
			 buttontype = WINDOW_BUTTONTYPE_OK | WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_NEXT;
		 windowtype = WINDOW_MESSAGETYPE_WIDEMESSAGEANDLINEINPUT;
		 windowno = NPC_ALLDOMAN_LIST_WND;
		 sprintf ( token2 , "            ����          ͷ��    ¥��    �ȼ�    ת��\n");
		 for ( i=0+(showpage-1 )*15; i<showpage*15; i++) {
#ifdef _FIX_ALLDOMAN
			if( i >= MAX_HERO_LIST || i<0 ) break;
			if( Herolist[i].use == 0 ) continue;
			sprintf( token , "%16s %13s %7d %7d %7d\n" , 
				Herolist[i].strings[1], Herolist[i].strings[2],
				Herolist[i].intdata[3], Herolist[i].intdata[0],
				Herolist[i].intdata[1] );
			strcat( token2, token);
#else
			 if( atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][6] ) == 0 || 
				 atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][3] ) == 0 )
				 continue ; 
			sprintf( token , "%16s %13s %7d %7d %7d\n" , 
				Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][1] , 
				Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][2] , 
				atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][6] ), 
				atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][3] ), 
				atoi ( Herolist[sort[MAX_HERO_LIST - 1 - i ][1]][4] ) 
				); 
			strcat ( token2 , token ) ; 
			if ( i + 1 == countnum )
				break;
#endif
		 }
		 lssproto_WN_send( fd, windowtype, buttontype, windowno,
			 CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),token2);
		 break;
    }
}

static int loadherolist = 0;
BOOL NPC_AlldomanInit( int meindex )
{
//andy_log
//print( "\n\nNPC_AlldomanInit( %d)\n\n", meindex);
	if( loadherolist == 0){
		loadherolist = 1;
		saacproto_UpdataStele_send ( acfd , "FirstLoad", "LoadHerolist" , "����" , 0 , 0 , 0 , 999 ) ; 
	}
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPEALLDOMAN);
    CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );
    return TRUE;
}

void NPC_AlldomanTalked( int meindex , int talker , char *msg ,int color )
{
     if( NPC_Util_CharDistance( talker, meindex ) > 1) return;
     if( NPC_Util_isFaceToFace( talker, meindex , 1 ) == FALSE ) return;
	 showpage = 1 ; 
	 NPC_Alldoman_selectWindow(meindex, talker , 4 ,"");
}
#ifdef _FIX_ALLDOMAN
void NPC_rePlaceHeroList( int fti, int tti)
{
	int i;
	if( tti<0 || tti>=MAX_HERO_LIST ) return;
	if( fti<0 || fti>=MAX_HERO_LIST ) return;
	for( i=0; i<3; i++ ){
		sprintf( Herolist[tti].strings[i], "%s", Herolist[fti].strings[i]);
	}
	for( i=0; i<4; i++ ){
		Herolist[tti].intdata[i] = Herolist[fti].intdata[i];
	}
	Herolist[tti].use = Herolist[fti].use;
	Herolist[fti].use = 0;
}
#endif
void NPC_AlldomanWriteStele ( char *token )
{
/*#ifdef _FIX_ALLDOMAN
	char buf1[512], buf2[256];
	int k=0, ti=-1, i;
	BOOL rePlace = FALSE;

	if( token==NULL || strlen( token)<=0 ) return;

//andy_log
print( "token:[%s]\n", token);

	if( getStringFromIndexWithDelim( token, ",", 1, buf1, sizeof( buf1) ) == FALSE ) return;
//andy_log
print( "buf1:[%s]\n", buf1);
	if( !strcmp( buf1, "A|") ){
		for( i=0; i<MAX_HERO_LIST; i++ )
			Herolist[i].use = 0;
	}else{
		rePlace = TRUE;
	}
	k = 2;
	while( getStringFromIndexWithDelim( token, ",", k, buf1, sizeof( buf1)) != FALSE ){
		k++;
		if( getStringFromIndexWithDelim( buf1, "|", 1, buf2, sizeof( buf2) ) == FALSE ) continue;
		ti = atoi( buf2);
		if( ti<0 || ti>=MAX_HERO_LIST ) continue;
		if( rePlace == TRUE ){
			for( i=MAX_HERO_LIST-1; i>ti; i++ ){
				NPC_rePlaceHeroList( i-1, i);
			}
			Herolist[ti].use = 0;
		}
		if( getStringFromIndexWithDelim( buf1, "|", 2, buf2, sizeof( buf2) ) == FALSE ) continue;
		sprintf( Herolist[ti].strings[0], "%s", buf2 );
		if( getStringFromIndexWithDelim( buf1, "|", 3, buf2, sizeof( buf2) ) == FALSE ) continue;
		sprintf( Herolist[ti].strings[1], "%s", buf2 );
		if( getStringFromIndexWithDelim( buf1, "|", 4, buf2, sizeof( buf2) ) == FALSE ) continue;
		sprintf( Herolist[ti].strings[3], "%s", buf2 );

		if( getStringFromIndexWithDelim( buf1, "|", 5, buf2, sizeof( buf2) ) == FALSE ) continue;
		Herolist[ti].intdata[0] = atoi( buf2);
		if( getStringFromIndexWithDelim( buf1, "|", 6, buf2, sizeof( buf2) ) == FALSE ) continue;
		Herolist[ti].intdata[1] = atoi( buf2);
		if( getStringFromIndexWithDelim( buf1, "|", 7, buf2, sizeof( buf2) ) == FALSE ) continue;
		Herolist[ti].intdata[3] = atoi( buf2);
		Herolist[ti].use = 1;
//andy_log
print( "ti:[%d]\n", ti);
	}
	//andy_log
	for( i=0; i<MAX_HERO_LIST; i++){
		if( Herolist[i].use == 0 ) continue;
		print( "hero[%d]:[%s,%s,%s,%d,%d,%d,%d]\n", i,
			Herolist[i].strings[0], Herolist[i].strings[1],
			Herolist[i].strings[2],
			Herolist[i].intdata[0], Herolist[i].intdata[1],
			Herolist[i].intdata[2], Herolist[i].intdata[3] );
	}
#else*/
	int linenum = 0 , i , flag , temp , temp1 , j  ; 
	int lens=0;
	char *addr;
	if( token == NULL || (lens = strlen( token)) <= 0 ) return;
	while( linenum < 100 ){
		for ( i = 0 ; i < MAX_HERO_COLUMN ; i ++ ) {
			if( getStringFromIndexWithDelim( token, "|", i + 1 , Herolist[ linenum ][ i ] , sizeof( Herolist[ linenum ][ i ] ) ) == FALSE ){
#ifdef _ALLDOMAN_DEBUG
				countnum = 0 ; 
	            for ( i = 0 ; i < MAX_HERO_LIST ; i ++ ) {
		            sort[i][0] = atoi ( Herolist[i][6] ) ;
		            sort[i][1] = i ; 
		            if ( sort[i][0] != 0 && sort[i][0] != -1 ) 
			            countnum ++ ; 
				}

	            for ( i = 0 ; i < MAX_HERO_LIST - 1 ; i ++ ) {
		            flag = 0 ; 
		            for ( j = 0 ; j < MAX_HERO_LIST - 1 ; j ++ ) {
			            if ( sort[j][0] > sort[j+1][0] ){
				            flag = 1 ; 
				            temp = sort[j][0];
				            temp1 = sort[j][1];
				            sort[j][0] = sort[j + 1][0];
				            sort[j][1] = sort[j + 1][1];
				            sort[j+1][0] = temp;
				            sort[j+1][1] = temp1;
						}
					}
		            if ( flag != 1 ) 
			            break;
				}
                totalpage = countnum / 15 ;
	            if ( ( countnum % 15 ) != 0 )
		            totalpage ++;
#endif
				return;
			}
		}
		linenum ++;
//#endif

		if ( (addr = strstr ( token , "\n" )) == NULL ) break;
		strcpy ( token , addr + 1 ) ;
		if( token == NULL || (lens = strlen( token)) <= 0 ) {
#ifdef _ALLDOMAN_DEBUG	    
			countnum = 0 ; 
	        for ( i = 0 ; i < MAX_HERO_LIST ; i ++ ) {
		        sort[i][0] = atoi ( Herolist[i][6] ) ;
		        sort[i][1] = i ; 
		        if ( sort[i][0] != 0 && sort[i][0] != -1 ) 
			        countnum ++ ; 
			}
	        for ( i = 0 ; i < MAX_HERO_LIST - 1 ; i ++ ) {
		        flag = 0 ; 
		        for ( j = 0 ; j < MAX_HERO_LIST - 1 ; j ++ ) {
			        if ( sort[j][0] > sort[j+1][0] ){
				        flag = 1 ; 
				        temp = sort[j][0];
				        temp1 = sort[j][1];
				        sort[j][0] = sort[j + 1][0];
				        sort[j][1] = sort[j + 1][1];
				        sort[j+1][0] = temp;
				        sort[j+1][1] = temp1;
					}
				}
		        if ( flag != 1 ) 
			        break;
			}
	        totalpage = countnum / 15 ;
	        if ( ( countnum % 15 ) != 0 )
		        totalpage ++;
#endif
	        return;
		}		
	}

	countnum = 0 ; 
	for ( i = 0 ; i < MAX_HERO_LIST ; i ++ ) {
		sort[i][0] = atoi ( Herolist[i][6] ) ;
		sort[i][1] = i ; 
		if ( sort[i][0] != 0 && sort[i][0] != -1 ) 
			countnum ++ ; 
	}

	for ( i = 0 ; i < MAX_HERO_LIST - 1 ; i ++ ) {
		flag = 0 ; 
		for ( j = 0 ; j < MAX_HERO_LIST - 1 ; j ++ ) {
			if ( sort[j][0] > sort[j+1][0] ){
				flag = 1 ; 
				temp = sort[j][0];
				temp1 = sort[j][1];
				sort[j][0] = sort[j + 1][0];
				sort[j][1] = sort[j + 1][1];
				sort[j+1][0] = temp;
				sort[j+1][1] = temp1;
			}
		}
		if ( flag != 1 ) 
			break;
	}


	totalpage = countnum / 15 ;
	if ( ( countnum % 15 ) != 0 )
		totalpage ++;
//#endif
}

void NPC_Alldoman_S_WriteStele( char *ocdkey , char *oname , char *ncdkey , 
		   char *nname , char *title , int level , int trns , int floor ) 
{
#ifdef _FIX_ALLDOMAN
	return;
#else
	int i , j , temp , temp1 , flag ; 
	for ( i = 0 ; i < MAX_HERO_COLUMN ; i ++ ) {
		if ( ( strcmp( Herolist[i][0] , ocdkey ) == 0 ) && ( strcmp( Herolist[i][1] , oname ) == 0 ) ) {
			sprintf( Herolist[i][0] , "%s" , ncdkey ) ; 
			sprintf( Herolist[i][1] , "%s" , nname ) ; 			
			sprintf( Herolist[i][2] , "%s" , title ) ; 			
			sprintf( Herolist[i][3] , "%d" , level ) ; 			
			sprintf( Herolist[i][4] , "%d" , trns ) ; 			
			sprintf( Herolist[i][6] , "%d" , floor ) ; 		
			break;
		}
	}
	countnum = 0 ; 
	for ( i = 0 ; i < MAX_HERO_LIST ; i ++ ) {
		sort[i][0] = atoi ( Herolist[i][6] ) ;
		sort[i][1] = i ; 
		if ( sort[i][0] != 0 && sort[i][0] != -1 ) 
			countnum ++ ; 
	}
	for ( i = 0 ; i < MAX_HERO_LIST - 1 ; i ++ ) {
		flag = 0 ; 
		for ( j = 0 ; j < MAX_HERO_LIST - 1 ; j ++ ) {
			if ( sort[j][0] > sort[j+1][0] ){
				flag = 1 ; 
				temp = sort[j][0];
				temp1 = sort[j][1];
				sort[j][0] = sort[j + 1][0];
				sort[j][1] = sort[j + 1][1];
				sort[j+1][0] = temp;
				sort[j+1][1] = temp1;
			}
		}
		if ( flag != 1 ) 
			break;
	}
	totalpage = countnum / 15 ;
	if ( ( countnum % 15 ) != 0 )
		totalpage ++ ; 

#endif
}

#endif

