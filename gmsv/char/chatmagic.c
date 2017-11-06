#include "version.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include "common.h"
#include "char.h"
#include "util.h"
#include "buf.h"
#include "npcutil.h"
#include "char_base.h"
#include "object.h"
#include "char_data.h"
#include "handletime.h"
#include "chatmagic.h"
#include "configfile.h"
#include "readmap.h"
#include "map_deal.h"
#include "lssproto_serv.h"
#include "log.h"
#include "battle.h"
#include "pet.h"
#include "enemy.h"
#include "encount.h"
#include "magic_base.h"
#include "magic.h"
#include "pet_skill.h"
#include "pet_event.h"
#include "item_gen.h"
#include "mclient.h"
#include "npc_eventaction.h"
#include "map_warppoint.h"
#include "npc_manorsman.h"
#include "net.h"
#include "saacproto_cli.h"
#include "npc_scheduleman.h"
#include "trade.h"
#include "npccreate.h"
#ifdef _DEATH_CONTEND
#include "deathcontend.h"
#endif

#ifdef _CHAR_PROFESSION			// WON ADD ����ְҵ
#include "profession_skill.h"
#endif
#include "char_talk.h"

// WON ADD �����峤����
#include "family.h"

#include "petmail.h"

#ifdef _WON_TEST            // WON TEST 
#include "unpipc.h"
#endif

// WON ADD ���¶�ȡITEMSET.TXT
#include "item.h"
#include "npc_raceman.h"


extern	tagRidePetTable	ridePetTable[122];
extern int *pWorkAttackPower;
extern time_t initTime;
int *pWorkAttackPower = NULL;

struct GMINFO gminfo[GMMAXNUM];

void CHAR_CHAT_DEBUG_warp( int charindex , char *message )
{
	char token[100];
	int fl,x,y;
	easyGetTokenFromString( message , 1 , token, sizeof(token));
	fl = atoi( token );
	easyGetTokenFromString( message , 2 , token , sizeof(token));
	x = atoi( token );
	easyGetTokenFromString( message , 3 , token , sizeof(token));
	y = atoi( token );

	CHAR_warpToSpecificPoint(charindex, fl, x, y);

}


void CHAR_CHAT_DEBUG_hp( int charindex , char *message )
{
	CHAR_setInt( charindex , CHAR_HP,atoi( message ) );
	CHAR_complianceParameter( charindex );
	CHAR_send_P_StatusString( charindex , CHAR_P_STRING_HP);
}
void CHAR_CHAT_DEBUG_setmp( int charindex , char *message )
{
	CHAR_setInt( charindex , CHAR_MAXMP, atoi( message ) );
	CHAR_setInt( charindex , CHAR_MP, atoi( message ) );	
	CHAR_complianceParameter( charindex );
	CHAR_send_P_StatusString( charindex , CHAR_P_STRING_MP);

}

void CHAR_CHAT_DEBUG_mp( int charindex , char *message )
{
	int Mymp;
	CHAR_setInt( charindex , CHAR_MP, atoi( message ) );
	Mymp = CHAR_getWorkInt( charindex , CHAR_WORKMAXMP);
	if( Mymp > CHAR_getWorkInt( charindex , CHAR_WORKMAXMP) )
		CHAR_setInt( charindex , CHAR_MP, CHAR_getWorkInt( charindex , CHAR_WORKMAXMP));

	CHAR_complianceParameter( charindex );
	//CHAR_sendStatusString( charindex , "P");
	CHAR_send_P_StatusString( charindex , CHAR_P_STRING_MP);

}

void CHAR_CHAT_DEBUG_str( int charindex , char *message )
{
	CHAR_setInt( charindex , CHAR_STR,atoi( message ) );
	CHAR_complianceParameter( charindex );
	//CHAR_sendStatusString( charindex , "P");
	CHAR_send_P_StatusString( charindex ,
							CHAR_P_STRING_STR|CHAR_P_STRING_MAXHP|
							CHAR_P_STRING_ATK
							);
}
void CHAR_CHAT_DEBUG_tgh( int charindex , char *message )
{
	CHAR_setInt( charindex , CHAR_TOUGH,atoi( message ) );
	CHAR_complianceParameter( charindex );
	//CHAR_sendStatusString( charindex , "P");
	CHAR_send_P_StatusString( charindex ,
							CHAR_P_STRING_TOUGH|CHAR_P_STRING_MAXHP|
							CHAR_P_STRING_DEF
							);
}
void CHAR_CHAT_DEBUG_dex( int charindex , char *message )
{
	CHAR_setInt( charindex , CHAR_DEX,atoi( message ) );
	CHAR_complianceParameter( charindex );
	//CHAR_sendStatusString( charindex , "P");
	CHAR_send_P_StatusString( charindex ,
							CHAR_P_STRING_DEX|CHAR_P_STRING_MAXHP|
							CHAR_P_STRING_ATK|CHAR_P_STRING_QUICK
							);
}
void CHAR_CHAT_DEBUG_vital( int charindex , char *message )
{
	CHAR_setInt( charindex , CHAR_VITAL,atoi( message ) );
	CHAR_complianceParameter( charindex );
	//CHAR_sendStatusString( charindex , "P");
	CHAR_send_P_StatusString( charindex ,
							CHAR_P_STRING_VITAL|CHAR_P_STRING_MAXHP|
							CHAR_P_STRING_DEF);
}
void CHAR_CHAT_DEBUG_luck( int charindex , char *message )
{
	CHAR_setInt( charindex , CHAR_LUCK,atoi( message ) );
	CHAR_complianceParameter( charindex );
	//CHAR_sendStatusString( charindex , "P");
	CHAR_send_P_StatusString( charindex , CHAR_P_STRING_LUCK);
}

void CHAR_CHAT_DEBUG_gold( int charindex , char *message )
{
	int MaxGold;
	//ANDY_add
	int money = atoi( message );
	MaxGold = CHAR_getMaxHaveGold( charindex);
	if( money > MaxGold )
		money = MaxGold;
	CHAR_setInt( charindex , CHAR_GOLD , money );
	CHAR_complianceParameter( charindex );
	CHAR_send_P_StatusString( charindex , CHAR_P_STRING_GOLD);
}

#ifdef _TEST_DROPITEMS
void CHAR_CHAT_DEBUG_dropmypet( int charindex, char *message )
{
	char buf[256];
	int petID, x, y, floor, objindex, petindex, count=0;
	int i, j, k, petarray, cLeve=1;

	if( getStringFromIndexWithDelim( message, " ", 1, buf, sizeof( buf)) == FALSE ) return;
	petID = atoi( buf);
	if( getStringFromIndexWithDelim( message, " ", 2, buf, sizeof( buf)) != FALSE ){
		cLeve = atoi( buf);
	}

	floor = CHAR_getInt( charindex, CHAR_FLOOR);
	x = CHAR_getInt( charindex, CHAR_X);
	y = CHAR_getInt( charindex, CHAR_Y);
	if( getStringFromIndexWithDelim( message, " ", 3, buf, sizeof( buf)) != FALSE ){
		floor = atoi( buf);
	}
	if( getStringFromIndexWithDelim( message, " ", 4, buf, sizeof( buf)) != FALSE ){
		x = atoi( buf);
	}
	if( getStringFromIndexWithDelim( message, " ", 5, buf, sizeof( buf)) != FALSE ){
		y = atoi( buf);
	}
	for( i=(x-3); i<(x+3); i++)	{
		for( j=(y-3); j<(y+3); j++)	{
			petarray = ENEMY_getEnemyArrayFromId( petID);
			petindex = ENEMY_TEST_createPetIndex( petarray );
			if( !CHAR_CHECKINDEX( petindex) ){
				CHAR_talkToCli( charindex, -1, "err Can't create pet", CHAR_COLORYELLOW);
				return;
			}
			CHAR_complianceParameter( petindex );
			objindex = PET_dropPetAbsolute( petindex, floor, i, j, FALSE );
			if( objindex == -1 ){
				CHAR_talkToCli( charindex, -1, "err Can't Drop", CHAR_COLORYELLOW);
				return;
			}
			for( k = 1; k < cLeve; k ++ ){	//����
				CHAR_PetLevelUp( petindex );
				CHAR_PetAddVariableAi( petindex, AI_FIX_PETLEVELUP );
				CHAR_setInt( petindex, CHAR_LV, CHAR_getInt( petindex, CHAR_LV) +1 );
			}
			CHAR_complianceParameter( petindex );
			CHAR_setInt( petindex , CHAR_HP , CHAR_getWorkInt( petindex , CHAR_WORKMAXHP ) );

			CHAR_setWorkInt( petindex, CHAR_WORKOBJINDEX, objindex );
			CHAR_setInt( petindex, CHAR_FLOOR, floor);
			CHAR_setInt( petindex, CHAR_X, i);
			CHAR_setInt( petindex, CHAR_Y, j);
			CHAR_sendCToArroundCharacter( objindex);
			CHAR_setInt( petindex, CHAR_PUTPETTIME, NowTime.tv_sec);
			CHAR_setChar( petindex, CHAR_OWNERCDKEY, "SYSTEM_WAYI" );
			CHAR_setChar( petindex, CHAR_OWNERCHARANAME, "SYSTEM_WAYI" );
			count++;
		}
	}
	sprintf( buf, "�ܹ����� %dֻ���", count);
	CHAR_talkToCli( charindex, -1, buf, CHAR_COLORYELLOW);


}

void CHAR_CHAT_DEBUG_dropmyitem( int charindex, char *message )
{
	char buf[256];
	int itemID, SitemID, maxflg=0,emptyitemindexinchara, i, j;
	int floor, x, y, objindex;

	if( getStringFromIndexWithDelim( message, " ", 1, buf, sizeof( buf)) == FALSE ) return;
	SitemID = atoi( buf);
	if( getStringFromIndexWithDelim( message, " ", 2, buf, sizeof( buf)) != FALSE ){
		maxflg = atoi( buf);
	}
	floor = CHAR_getInt( charindex, CHAR_FLOOR);
	x = CHAR_getInt( charindex, CHAR_X);
	y = CHAR_getInt( charindex, CHAR_Y);
	if( getStringFromIndexWithDelim( message, " ", 3, buf, sizeof( buf)) != FALSE ){
		floor = atoi( buf);
	}
	if( getStringFromIndexWithDelim( message, " ", 4, buf, sizeof( buf)) != FALSE ){
		x = atoi( buf);
	}
	if( getStringFromIndexWithDelim( message, " ", 5, buf, sizeof( buf)) != FALSE ){
		y = atoi( buf);
	}
	itemID = SitemID;
	

	for( i=(x-3); i<(x+3); i++)	{
		for( j=(y-3); j<(y+3); j++)	{
			int ret = 0 , count=0;
			int itemindex;
			while( count < 10 ){
				count++;
				itemindex = ITEM_makeItemAndRegist( itemID );
				if( maxflg == 1 ) itemID++;
				if( !ITEM_CHECKINDEX( itemindex)) continue;
				break;
			}
			emptyitemindexinchara = CHAR_findEmptyItemBox( charindex );

			CHAR_setItemIndex( charindex, emptyitemindexinchara, itemindex );
			ITEM_setWorkInt(itemindex, ITEM_WORKOBJINDEX,-1);
			ITEM_setWorkInt(itemindex, ITEM_WORKCHARAINDEX,charindex);
			CHAR_sendItemDataOne( charindex, emptyitemindexinchara);
			//drop
			if( (ret = CHAR_DropItemFXY( charindex, emptyitemindexinchara,
				floor, i, j, &objindex )) != 0 ){
				char buf[256];
				sprintf( buf, "����%d", ret);
				CHAR_talkToCli( charindex, -1, buf, CHAR_COLORYELLOW);
			}
			ITEM_setInt(itemindex,ITEM_PUTTIME,NowTime.tv_sec);
			CHAR_sendWatchEvent(objindex,CHAR_ACTSTAND,NULL,0,TRUE);
			CHAR_setItemIndex(charindex, emptyitemindexinchara ,-1);
			CHAR_sendItemDataOne(charindex, emptyitemindexinchara);
			CHAR_sendCToArroundCharacter(CHAR_getWorkInt( charindex, CHAR_WORKOBJINDEX));
		}
	}
	{
				char buf[256];
				sprintf( buf, "����ID%d", itemID-1);
				CHAR_talkToCli( charindex, -1, buf, CHAR_COLORYELLOW);
	}
}
#endif

void CHAR_CHAT_DEBUG_additem( int charindex , char *message )
{

    int emptyitemindexinchara, itemindex;
	int	i;
	int num = 0;
	int itemid;
	char buf[64];
	char	msgbuf[128];
	getStringFromIndexWithDelim( message, " ", 1, buf, sizeof( buf));
	itemid = atoi( buf);
	if( getStringFromIndexWithDelim( message, " ", 2, buf, sizeof( buf))){
		num = itemid;
		itemid = atoi( buf);
	}
	if( num <= 0 ) num = 1;

	for( i = 0; i < num; i ++ ) {
	    emptyitemindexinchara = CHAR_findEmptyItemBox( charindex );

		if( emptyitemindexinchara < 0 ){
			sprintf( msgbuf, "�ռ���λ���㡣" );

			CHAR_talkToCli( charindex, -1, msgbuf, CHAR_COLORYELLOW);

			return;
		}

		itemindex = ITEM_makeItemAndRegist( itemid );

	    if( itemindex != -1 ){
	        CHAR_setItemIndex( charindex, emptyitemindexinchara, itemindex );
	        ITEM_setWorkInt(itemindex, ITEM_WORKOBJINDEX,-1);
	        ITEM_setWorkInt(itemindex, ITEM_WORKCHARAINDEX,charindex);
	       	CHAR_sendItemDataOne( charindex, emptyitemindexinchara);
			LogItem(
				CHAR_getChar( charindex, CHAR_NAME ),
				CHAR_getChar( charindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
				itemindex,
#else
				atoi( message),
#endif
				"AddItem(��������GM)",
		       	CHAR_getInt( charindex,CHAR_FLOOR),
				CHAR_getInt( charindex,CHAR_X ),
	 	      	CHAR_getInt( charindex,CHAR_Y ),
                ITEM_getChar( itemindex, ITEM_UNIQUECODE),
					ITEM_getChar( itemindex, ITEM_NAME),
					ITEM_getInt( itemindex, ITEM_ID)

			);
			snprintf( msgbuf, sizeof( msgbuf), "����%s�ɹ�",
										ITEM_getChar( itemindex, ITEM_NAME));
			CHAR_talkToCli( charindex, -1,msgbuf, CHAR_COLORYELLOW);
		}else{
			sprintf( msgbuf, "��������ʧ�ܡ�" );
			CHAR_talkToCli( charindex, -1, msgbuf, CHAR_COLORYELLOW);
		}
	}
}


void CHAR_CHAT_DEBUG_metamo( int charindex , char *message )
{
		CHAR_setInt( charindex , CHAR_BASEIMAGENUMBER , atoi( message ) );
		CHAR_setInt( charindex , CHAR_BASEBASEIMAGENUMBER , atoi( message ) );

	CHAR_complianceParameter( charindex );
	CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charindex , CHAR_WORKOBJINDEX ));
	CHAR_send_P_StatusString( charindex , CHAR_P_STRING_BASEBASEIMAGENUMBER);
}

void CHAR_CHAT_DEBUG_info( int charindex , char *message )
{
	char line[256];
	int		index;

	if( strlen( message) != 0 ) {
		index = atoi( message);
	}
	else {
		index = charindex;
	}
	if( !CHAR_CHECKINDEX( index)) {
		CHAR_talkToCli( charindex, -1 , "��index�޷�ʹ�á�" , CHAR_COLORWHITE );
		return;
	}

	snprintf( line, sizeof( line),
			  "name:%s i:%d B:%d BB:%d LV:%d EXP:%d MakeSeq:%d",
			  CHAR_getChar( index, CHAR_NAME),
			  charindex,
			  CHAR_getInt( index, CHAR_BASEIMAGENUMBER ),
			  CHAR_getInt( index, CHAR_BASEBASEIMAGENUMBER ),
			  CHAR_getInt( index, CHAR_LV ),
			  CHAR_getInt( index, CHAR_EXP ),
			  CHAR_getCharMakeSequenceNumber( index));

	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );

	snprintf( line, sizeof( line),
			  "Gold:%d HP:%d MP/MHP:%d/%d" ,
			  CHAR_getInt( index, CHAR_GOLD ),
			  CHAR_getInt( index, CHAR_HP ),
			  CHAR_getInt( index, CHAR_MP ),
			  CHAR_getInt( index, CHAR_MAXMP ));
	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );


	if( CHAR_getInt( index, CHAR_WHICHTYPE ) == CHAR_TYPEPET ){
		snprintf( line, sizeof(line),
			  "��:%d ��:%d ��:%d ��:%d ��:%d ��:%d ��:%d ��:%d ����:%d �ұ�:%.2f ����:%s",
			  CHAR_getInt( index, CHAR_VITAL )/100,
			  CHAR_getInt( index, CHAR_STR )/100,
			  CHAR_getInt( index, CHAR_TOUGH )/100,
			  CHAR_getInt( index, CHAR_DEX )/100,
			  CHAR_getWorkInt( index, CHAR_WORKFIXVITAL ),
			  CHAR_getWorkInt( index, CHAR_WORKATTACKPOWER ),
			  CHAR_getWorkInt( index, CHAR_WORKDEFENCEPOWER ),//CHAR_WORKFIXDEX
			  CHAR_getWorkInt( index, CHAR_WORKQUICK ),
			  CHAR_getInt( index, CHAR_MODAI ),
			  CHAR_getInt( index, CHAR_VARIABLEAI ) * 0.01,			  
			  CHAR_getChar( index, CHAR_UNIQUECODE) );
	}else{
		snprintf( line, sizeof(line),
			  "��:%d ��:%d ��:%d ��:%d ��:%d ��:%d ��:%d ��:%d ��:%d",
			  CHAR_getInt( index, CHAR_VITAL )/100,
			  CHAR_getInt( index, CHAR_STR )/100,
			  CHAR_getInt( index, CHAR_TOUGH )/100,
			  CHAR_getInt( index, CHAR_DEX )/100,
			  CHAR_getWorkInt( index, CHAR_WORKFIXVITAL ),
			  CHAR_getWorkInt( index, CHAR_WORKATTACKPOWER ),
			  CHAR_getWorkInt( index, CHAR_WORKDEFENCEPOWER ),
			  CHAR_getWorkInt( index, CHAR_WORKFIXCHARM ),
			  CHAR_getWorkInt( index, CHAR_WORKFIXLUCK ));
	}
	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );


	snprintf( line, sizeof(line),
			  "��:%d ˮ:%d ��:%d ��:%d",
			  CHAR_getWorkInt( index, CHAR_WORKFIXEARTHAT ),
			  CHAR_getWorkInt( index, CHAR_WORKFIXWATERAT ),
			  CHAR_getWorkInt( index, CHAR_WORKFIXFIREAT ),
			  CHAR_getWorkInt( index, CHAR_WORKFIXWINDAT ));
	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );

	snprintf( line, sizeof( line),
			  "Sk Itm:%d ChV:%d Rad:%d" ,
			  CHAR_MAXITEMHAVE,
			  CHAR_getInt( index, CHAR_CHATVOLUME ),
			  CHAR_getInt( index, CHAR_RADARSTRENGTH ) );
	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );


	snprintf( line, sizeof( line),
			  "Status ��:%d ��:%d Ĭ:%d ʯ:%d ��:%d ��:%d" ,
			  CHAR_getInt( index , CHAR_POISON ),
			  CHAR_getInt( index , CHAR_PARALYSIS ),
			  CHAR_getInt( index , CHAR_SLEEP ),
			  CHAR_getInt( index , CHAR_STONE ),
			  CHAR_getInt( index , CHAR_DRUNK ),
			  CHAR_getInt( index , CHAR_CONFUSION ) );
	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );

#ifdef _PERSONAL_FAME	// Arminius: �����������
	snprintf( line, sizeof( line),
#ifdef _NEW_MANOR_LAW
				"TitInd:%d FL:%d X:%d Y:%d DIR:%d FAME:%d MOMENTUM:%d" ,
#else
			  "TitInd:%d FL:%d X:%d Y:%d DIR:%d FAME:%d" ,
#endif
			  CHAR_getInt( index , CHAR_INDEXOFEQTITLE ),
			  CHAR_getInt( index , CHAR_FLOOR ),
			  CHAR_getInt( index , CHAR_X ),
			  CHAR_getInt( index , CHAR_Y ),
			  CHAR_getInt( index , CHAR_DIR ),
			  CHAR_getInt( index , CHAR_FAME)
#ifdef _NEW_MANOR_LAW
				,CHAR_getInt( index , CHAR_MOMENTUM)
#endif
				);
#else
	snprintf( line, sizeof( line),
				  "TitInd:%d FL:%d X:%d Y:%d DIR:%d" ,
			  CHAR_getInt( index , CHAR_INDEXOFEQTITLE ),
			  CHAR_getInt( index , CHAR_FLOOR ),
			  CHAR_getInt( index , CHAR_X ),
			  CHAR_getInt( index , CHAR_Y ),
			  CHAR_getInt( index , CHAR_DIR ) );
#endif
	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );


	snprintf( line, sizeof( line),
				  "walk:%d dead:%d ItemMergecnt:%d" ,
			  CHAR_getInt( index , CHAR_WALKCOUNT ) ,
			  CHAR_getInt( index , CHAR_DEADCOUNT ) ,
			  CHAR_getInt( index, CHAR_MERGEITEMCOUNT)
			  );
	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );

	if( CHAR_getInt( index, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER) {
		snprintf( line, sizeof( line),
					  "petindex [%d][%d][%d][%d][%d] " ,
				CHAR_getCharPet( index, 0),
				CHAR_getCharPet( index, 1),
				CHAR_getCharPet( index, 2),
				CHAR_getCharPet( index, 3),
				CHAR_getCharPet( index, 4));
	}
	else {
		snprintf( line, sizeof( line),
					  "PetskillIndex [%d][%d][%d][%d][%d][%d][%d] " ,
				CHAR_getPetSkill( index, 0),
				CHAR_getPetSkill( index, 1),
				CHAR_getPetSkill( index, 2),
				CHAR_getPetSkill( index, 3),
				CHAR_getPetSkill( index, 4),
				CHAR_getPetSkill( index, 5),
				CHAR_getPetSkill( index, 6));
	}
	
	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );
	if( CHAR_getInt( index, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER) {
		snprintf( line, sizeof( line),
					  "poolpetindex [%d][%d][%d][%d][%d] " ,
				CHAR_getCharPoolPet( index, 0),
				CHAR_getCharPoolPet( index, 1),
				CHAR_getCharPoolPet( index, 2),
				CHAR_getCharPoolPet( index, 3),
				CHAR_getCharPoolPet( index, 4));
		CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );

		snprintf( line, sizeof( line),
					  "poolpetindex [%d][%d][%d][%d][%d] " ,
				CHAR_getCharPoolPet( index, 5),
				CHAR_getCharPoolPet( index, 6),
				CHAR_getCharPoolPet( index, 7),
				CHAR_getCharPoolPet( index, 8),
				CHAR_getCharPoolPet( index, 9));
		CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );
	}

	snprintf( line, sizeof( line),
#ifdef _DROPSTAKENEW
			  "levelup [%d], �ۻ� %d   �ĳ�����[%d]" ,
#else
			  "levelup [%d], �ۻ� %d " ,
#endif
			CHAR_getInt( index, CHAR_LEVELUPPOINT),
			CHAR_getWorkInt( index, 	CHAR_WORKULTIMATE)
#ifdef _DROPSTAKENEW
			,CHAR_getInt( index, CHAR_GAMBLENUM) 
#endif
		);
	CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );

}

void CHAR_CHAT_DEBUG_sysinfo( int charindex , char *message )
{
	char msg[1024],name[256];
	int i;
	time_t new_t;
	int dd,hh,mm,ss;

	int items=0 , chars=0, golds=0, nouses=0;
	int	players=0,pets=0,others=0;
	int	whichtype= -1;
	int objnum = OBJECT_getNum();


	int itemnum = ITEM_getITEM_itemnum();
	int itemuse =0;

	makeEscapeString( getGameserverID(), name, sizeof( name));
	sprintf( msg, "GSID:%s GSNAME:%s", getGameserverID(), name);
	CHAR_talkToCli( charindex, -1, msg, CHAR_COLORYELLOW);


	for( i=0 ; i<objnum ; i++){
		switch( OBJECT_getType( i )){
		case OBJTYPE_NOUSE:
			nouses++;
			break;
		case OBJTYPE_CHARA:
			chars++;
			whichtype =  CHAR_getInt( OBJECT_getIndex( i), CHAR_WHICHTYPE);
			if( whichtype == CHAR_TYPEPLAYER) players++;
			else if( whichtype == CHAR_TYPEPET) pets++;
			else others ++;
			break;
		case OBJTYPE_ITEM:
			items++;
			break;
		case OBJTYPE_GOLD:
			golds++;
			break;
		default:
			break;
		}
	}

	for( i=0 ; i<itemnum ; i++)
		if( ITEM_getITEM_use(i) )
			itemuse++;

	snprintf( msg , sizeof(msg),
			  "Object Char:%d Item:%d Gold:%d Nouse:%d Item use:%d",
			  chars,items,golds, nouses, itemuse );

	CHAR_talkToCli(charindex,-1,msg,CHAR_COLORYELLOW );

	snprintf( msg , sizeof(msg),
			  "player : %d  pet : %d  others  : %d",
			  players, pets, others );

	CHAR_talkToCli(charindex,-1,msg,CHAR_COLORYELLOW );
	
	time(&new_t);
	new_t -= initTime;
        dd=(int) new_t / 86400; new_t=new_t % 86400;
        hh=(int) new_t / 3600;  new_t=new_t % 3600;
        mm=(int) new_t / 60;    new_t=new_t % 60;
        ss=(int) new_t;

	snprintf( msg, sizeof( msg ) ,
          	"start: %d days %d hours %d min %d sec��",dd,hh,mm,ss);
        CHAR_talkToCli( charindex, -1, msg, CHAR_COLORYELLOW );

	{
		showMem( msg);
		CHAR_talkToCli( charindex, -1, msg, CHAR_COLORYELLOW);
	}
	snprintf( msg, sizeof( msg ) ,
			"NPC_createnum:%d��",NPC_createnum);
	CHAR_talkToCli( charindex, -1, msg, CHAR_COLORYELLOW );

}


void CHAR_CHAT_DEBUG_announce(int charindex , char *message )
{
    int     i;
    int     playernum = CHAR_getPlayerMaxNum();

    for( i = 0 ; i < playernum ; i++) {
        if( CHAR_getCharUse(i) != FALSE ) {
			CHAR_talkToCli( i, charindex, message, CHAR_COLORWHITE);
        }
	}
}

void CHAR_CHAT_DEBUG_level( int charaindex, char* message )
{
	char buf1[256];
	CHAR_setInt(charaindex,CHAR_LV, atoi( message ));
	CHAR_send_P_StatusString( charaindex , CHAR_P_STRING_LV);
	sprintf( buf1, "LEVEL ��Ϊ%d��", atoi( message ));
	CHAR_talkToCli( charaindex, -1, buf1, CHAR_COLORYELLOW);
}


void CHAR_CHAT_DEBUG_exp( int charaindex, char* message )
{
	char buf1[256];

	CHAR_setMaxExp( charaindex, atoi( message ));
	CHAR_send_P_StatusString( charaindex , CHAR_P_STRING_EXP);
	sprintf( buf1, "EXP ��Ϊ%d��", atoi( message ));
	CHAR_talkToCli( charaindex, -1, buf1, CHAR_COLORYELLOW);
}

void CHAR_CHAT_DEBUG_delitem( int charindex , char *message )
{
	int		itemid;

	if( strcmp( message, "all") == 0 ) {
		int		i;
		for( i = CHAR_STARTITEMARRAY; i < CHAR_MAXITEMHAVE; i ++ ) {
		    itemid = CHAR_getItemIndex(charindex, i);
			if( itemid != -1 ) {
				{
					LogItem(
						CHAR_getChar( charindex, CHAR_NAME ),
						CHAR_getChar( charindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
						itemid,
#else
			       		ITEM_getInt( itemid, ITEM_ID ),
#endif
						"DelItem(ɾ������GM)",
						CHAR_getInt( charindex,CHAR_FLOOR),
						CHAR_getInt( charindex,CHAR_X ),
 			    	  	CHAR_getInt( charindex,CHAR_Y ),
                        ITEM_getChar( itemid, ITEM_UNIQUECODE),
						ITEM_getChar( itemid, ITEM_NAME),
						ITEM_getInt( itemid, ITEM_ID)
					);
				}
				CHAR_DelItem( charindex, i);
			}
		}
	}else {
		int		equipplace;
		equipplace = atoi( message);
		if( equipplace < CHAR_STARTITEMARRAY || equipplace > CHAR_MAXITEMHAVE -1 ) {
			CHAR_talkToCli( charindex, -1, "���벻��ȷ��",  CHAR_COLORWHITE);
			return;
		}
	    itemid = CHAR_getItemIndex(charindex, equipplace);
		if( itemid == -1 ) return;
		{
			LogItem(
				CHAR_getChar( charindex, CHAR_NAME ),
				CHAR_getChar( charindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
				itemid,
#else
	       		ITEM_getInt( itemid, ITEM_ID ),
#endif
				"DelItem(ɾ������GM)",
				CHAR_getInt( charindex,CHAR_FLOOR),
				CHAR_getInt( charindex,CHAR_X ),
 	      		CHAR_getInt( charindex,CHAR_Y ),
                ITEM_getChar( itemid, ITEM_UNIQUECODE),
				ITEM_getChar( itemid, ITEM_NAME),
				ITEM_getInt( itemid, ITEM_ID)
			);
		}
		CHAR_DelItem( charindex, equipplace);
	}
}

#ifdef _CHAR_PROFESSION			// WON ADD ����ְҵ
void CHAR_CHAT_DEBUG_addsk( int charaindex, char* message )
{

	CHAR_HaveSkill *pSkil;
	int	skill, i, level=1;

	char msg1[20], msg2[20];

	getStringFromIndexWithDelim( message, " ", 1, msg1, sizeof( msg1));
	getStringFromIndexWithDelim( message, " ", 2, msg2, sizeof( msg2));

	skill = atoi(msg1);
	level = atoi(msg2);

	if( level > 100 ){
		CHAR_talkToCli( charaindex, -1, "�ȼ�����ȷ",  CHAR_COLORWHITE);
		return;
	}
	if( level < 1 ) level = 1;

	for( i = 0; i < CHAR_SKILLMAXHAVE; i ++ ){
		int skill_id = -1;

		pSkil = CHAR_getCharHaveSkill( charaindex, i );
		skill_id = SKILL_getInt( &pSkil->skill, SKILL_IDENTITY);
		if( skill == skill_id && skill_id != -1 ){
			CHAR_talkToCli(charaindex,-1, "���Ѿ�ѧϰ���˼���", CHAR_COLORYELLOW);
			return;
		}

		if( pSkil == NULL )		continue;
		if( pSkil->use != 0 )	continue;

		break;
	}

	if( i >= CHAR_SKILLMAXHAVE ){
		CHAR_talkToCli(charaindex,-1, "���������Ѵ�����", CHAR_COLORYELLOW);
		return;
	}else{
		level *= 100;
		SKILL_makeSkillData( &pSkil->skill, skill, level);
		pSkil->use = TRUE;
	}

	CHAR_sendStatusString( charaindex , "S");

	return;
}

void CHAR_CHAT_DEBUG_delsk( int charaindex, char* message )
{
	int	i;
	CHAR_HaveSkill *pSkil;
	
	if( strcmp( message, "all" ) == 0 ){
//		CHAR_talkToCli( charaindex, -1, "������м���",  CHAR_COLORWHITE);
		for( i = CHAR_SKILLMAXHAVE-1; i >= 0; i -- ){
			pSkil = CHAR_getCharHaveSkill( charaindex, i );
			if( pSkil == NULL )continue;
			if( pSkil->use == 0 )continue;			
			SKILL_makeSkillData( &pSkil->skill, 0, 0 );
			pSkil->use = 0;
		}
	}

	for( i = CHAR_SKILLMAXHAVE-1; i >= 0; i -- ){
		pSkil = CHAR_getCharHaveSkill( charaindex, i );
		if( pSkil == NULL )continue;
		if( pSkil->use == 0 )continue;
		print( "SKILUSE %d\n", pSkil->use );
		break;
	}

	if( i < 0 ){
//		CHAR_talkToCli( charaindex, -1, "���޷��������ˡ�",  CHAR_COLORWHITE);
		;;
	}else{
		SKILL_makeSkillData( &pSkil->skill, 0, 0 );
		pSkil->use = 0;
	}

	CHAR_sendStatusString( charaindex , "S");
}
#endif

void CHAR_CHAT_Kusabana( int charaindex, char* message ){

	int 	dir, ff, fx, fy;
    int		tile, obj;

	ff = CHAR_getInt( charaindex, CHAR_FLOOR );
	fx = CHAR_getInt( charaindex, CHAR_X );
	fy = CHAR_getInt( charaindex, CHAR_Y );
	dir = CHAR_getInt( charaindex, CHAR_DIR );

	fx += CHAR_getDX(dir);
	fy += CHAR_getDY(dir);

	if( MAP_getTileAndObjData( ff, fx, fy, &tile, &obj)	== FALSE )return;

#if 0
	{	char szBuffer[256];
		snprintf( szBuffer, sizeof( szBuffer ), "TILE = %d, OBJ = %d",
			tile, obj );
		CHAR_talkToCli( charaindex, -1, szBuffer,  CHAR_COLORWHITE);
	}
#else

	if( obj != 0 ){
		CHAR_talkToCli( charaindex, -1, "�������",  CHAR_COLORWHITE);
		return;
	}

	if( MAP_walkAbleFromPoint( ff, fx, fy, FALSE ) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "�޷����ˡ�",  CHAR_COLORWHITE);
		return;
	}

	MAP_setObjData( ff, fx, fy, 3416, 100 );


#if 1
	{
		char *stringdata;
		RECT seekr , retr;
		seekr.x = fx;
		seekr.y = fy;
		seekr.width = 1;
		seekr.height = 1;
		stringdata = MAP_getdataFromRECT(ff,&seekr,&retr);
		if( stringdata ){

			if( CHAR_getInt( charaindex , CHAR_WHICHTYPE )
			== CHAR_TYPEPLAYER ){
				int fd = getfdFromCharaIndex( charaindex );
				lssproto_M_send(fd,ff,
					retr.x,retr.y,
					retr.x+retr.width,retr.y+retr.height,
					stringdata );
			}
		} else {
		}
	}
#endif
#endif

    return ;
}


void CHAR_CHAT_Fukuwa( int charaindex, char* message ){
	int 	dir, ff, fx, fy, objindex, vs_index, vs_fd, stringlen;
    OBJECT obj;
	char szBuffer[2048];

	stringlen = strlen( message );

    if( message[0] == '[' && message[stringlen-1] == ']' ){
		CHAR_talkToCli( charaindex, -1, "����û��ת����",  CHAR_COLORWHITE);
		return ;
    }

	ff = CHAR_getInt( charaindex, CHAR_FLOOR );
	fx = CHAR_getInt( charaindex, CHAR_X );
	fy = CHAR_getInt( charaindex, CHAR_Y );
	dir = CHAR_getInt( charaindex, CHAR_DIR );

	fx += CHAR_getDX(dir);
	fy += CHAR_getDY(dir);

	stringlen = strlen(  message  );


	snprintf( szBuffer, sizeof( szBuffer ), "P|%s", message );

    for( obj=MAP_getTopObj( ff, fx, fy ); obj; obj=NEXT_OBJECT(obj) ){
        objindex = GET_OBJINDEX(obj);
        if( OBJECT_getType(objindex) != OBJTYPE_CHARA )continue;
		vs_index = OBJECT_getIndex(objindex);
       	vs_fd = getfdFromCharaIndex( vs_index );
		CHAR_Talk( vs_fd, vs_index,
			szBuffer, CHAR_COLORWHITE, 3 );
		break;
	}

}

void CHAR_CHAT_DEBUG_superman( int charaindex, char* message )
{
#define		CHAT_CHAT_DEBUG_SUPERMAN_PARAMDEF		100000

	CHAR_setInt(charaindex,CHAR_SKILLUPPOINT, 32);
	CHAR_Skillupsend(charaindex);
	CHAR_setInt( charaindex , CHAR_HP, CHAT_CHAT_DEBUG_SUPERMAN_PARAMDEF);
	CHAR_setInt( charaindex , CHAR_MP,CHAT_CHAT_DEBUG_SUPERMAN_PARAMDEF );
	CHAR_setInt( charaindex , CHAR_MAXMP,CHAT_CHAT_DEBUG_SUPERMAN_PARAMDEF );
	CHAR_setInt( charaindex , CHAR_STR,CHAT_CHAT_DEBUG_SUPERMAN_PARAMDEF );
	CHAR_setInt( charaindex , CHAR_TOUGH,CHAT_CHAT_DEBUG_SUPERMAN_PARAMDEF );
	CHAR_setInt( charaindex , CHAR_GOLD , CHAT_CHAT_DEBUG_SUPERMAN_PARAMDEF );
	CHAR_setInt( charaindex,CHAR_LV, 80);

	CHAR_complianceParameter( charaindex );
	CHAR_sendStatusString( charaindex , "P");
#undef	CHAT_CHAT_DEBUG_SUPERMAN_PARAMDEF

}

void CHAR_CHAT_printcount( int charaindex, char* message )
{
	char	msgbuf[512];

	snprintf( msgbuf,sizeof( msgbuf),
			  "��˵�� %d �λ�,���� %d ��·,���� %d ��",
			  CHAR_getInt( charaindex, CHAR_TALKCOUNT),
			  CHAR_getInt( charaindex, CHAR_WALKCOUNT),
			  CHAR_getInt( charaindex, CHAR_DEADCOUNT)
			  );
	CHAR_talkToCli( charaindex, -1, msgbuf,  CHAR_COLORWHITE);

}

void CHAR_CHAT_DEBUG_battlein( int charaindex, char* message )
{
	int fd = getfdFromCharaIndex( charaindex );
	if( fd != -1) {
		lssproto_EN_recv( fd,
			CHAR_getInt( charaindex, CHAR_X),
			CHAR_getInt( charaindex, CHAR_Y));
	}
}

void CHAR_CHAT_DEBUG_battleout( int charaindex, char* message )
{

	BATTLE_WatchStop( charaindex );
}

void CHAR_CHAT_DEBUG_petmake( int charaindex, char* message )
{
	int	ret;
	char msgbuf[64], buf1[256];
	int	enemynum;
	int	enemyid;
	int		i;
	int level=-1;

	if( getStringFromIndexWithDelim( message, " ", 1, buf1, sizeof( buf1)) == FALSE ) return;
	enemyid = atoi( buf1);
	if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof( buf1)) != FALSE ) {
		level = atoi( buf1);
	}

	enemynum = ENEMY_getEnemyNum();
	for( i = 0; i < enemynum; i ++ ) {
		if( ENEMY_getInt( i, ENEMY_ID) == enemyid) {
			break;
		}
	}
	if( i == enemynum ) return;

	ret = ENEMY_createPetFromEnemyIndex( charaindex, i);
	if( !CHAR_CHECKINDEX( ret))return;
	snprintf( msgbuf,sizeof( msgbuf), "���������ɹ�! index[%d]",ret);
	CHAR_talkToCli( charaindex, -1, msgbuf,  CHAR_COLORWHITE);

	for( i = 0; i < CHAR_MAXPETHAVE; i ++ ){
		if( CHAR_getCharPet( charaindex, i ) == ret )break;
	}
	if( i == CHAR_MAXPETHAVE )i = 0;

	if( CHAR_CHECKINDEX( ret ) == TRUE ){
		CHAR_setMaxExpFromLevel( ret, CHAR_getInt( ret, CHAR_LV ));
	}
	if( level != -1 ){
		if( level > 140 ) level = 140;
		if( level < 0 ) level = 0;
		level = level - CHAR_getInt( ret, CHAR_LV );
		if( level > 0 ){
			int k;
			for( k = 1; k < level; k ++ ){	//����
				CHAR_PetLevelUp( ret );
				CHAR_PetAddVariableAi( ret, AI_FIX_PETLEVELUP );
				CHAR_setInt( ret, CHAR_LV, CHAR_getInt( ret, CHAR_LV) +1 );
			}
		}
	}

	snprintf( msgbuf, sizeof( msgbuf ), "K%d", i );
	CHAR_sendStatusString( charaindex, msgbuf );
	snprintf( msgbuf, sizeof( msgbuf ), "W%d", i );
	CHAR_sendStatusString( charaindex, msgbuf );
}

#ifdef _NEW_PETMAKE
void CHAR_CHAT_DEBUG_newpetmake(int charaindex, char *message)
{
  int	ret;
  char  msgbuf[64], buf1[256];
  int	enemynum;
  int	enemyid;
  int	i;
  int HP, STR, TGH, DEX;
  //float vital, str, tough, dex;
  int level=-1;

  if( getStringFromIndexWithDelim( message, " ", 1, buf1, sizeof( buf1)) == FALSE )
    return;
  enemyid = atoi( buf1);
  
  if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof( buf1)) == FALSE )
    return;
  HP = atoi(buf1);

  if( getStringFromIndexWithDelim( message, " ", 3, buf1, sizeof( buf1)) == FALSE )
    return;
  STR = atoi(buf1);

  if( getStringFromIndexWithDelim( message, " ", 4, buf1, sizeof( buf1)) == FALSE )
    return;
  TGH = atoi(buf1);

  if( getStringFromIndexWithDelim( message, " ", 5, buf1, sizeof( buf1)) == FALSE )
    return;
  DEX = atoi(buf1);

  if( getStringFromIndexWithDelim( message, " ", 6, buf1, sizeof( buf1)) != FALSE ) {
    level = atoi( buf1);
  }

  enemynum = ENEMY_getEnemyNum();
  for( i = 0; i < enemynum; i ++ ) {
    if( ENEMY_getInt( i, ENEMY_ID) == enemyid) {
      break;
    }
  }
  if( i == enemynum ) return;
  
  /*
  str = (3900 * STR - 90 * HP - 90 * DEX - 300 * TGH) / 37.8;
  vital = (90 * HP - 85.5 * DEX - 90 * TGH - 0.81 * str) / 3.51;
  tough = 100 * HP - 100 * DEX - str - 4 * vital;
  dex = 100 * DEX;
  */

  ret = ENEMY_createPetFromEnemyIndex_new(charaindex, i, HP, STR, TGH, DEX);
  if( !CHAR_CHECKINDEX( ret))return;
  snprintf( msgbuf,sizeof( msgbuf), "���������ɹ�!index[%d]",ret);
  CHAR_talkToCli( charaindex, -1, msgbuf,  CHAR_COLORWHITE);

  for( i = 0; i < CHAR_MAXPETHAVE; i ++ ){
    if( CHAR_getCharPet( charaindex, i ) == ret )break;
  }
  if( i == CHAR_MAXPETHAVE )i = 0;

  if( CHAR_CHECKINDEX( ret ) == TRUE ){
    CHAR_setMaxExpFromLevel( ret, CHAR_getInt( ret, CHAR_LV ));
  }
  if( level != -1 ){
    if( level > 140 ) level = 140;
    if( level < 0 ) level = 0;
    if( level > 0 ){
      CHAR_setInt( ret, CHAR_LV, level );
    }
  }

  snprintf( msgbuf, sizeof( msgbuf ), "K%d", i );
  CHAR_sendStatusString( charaindex, msgbuf );
  snprintf( msgbuf, sizeof( msgbuf ), "W%d", i );
  CHAR_sendStatusString( charaindex, msgbuf );
}
#endif

void CHAR_CHAT_DEBUG_deletepet( int charaindex, char* message )
{
	int i, pindex;
	char category[12];

	for( i = 0; i < CHAR_MAXPETHAVE; i ++ ) {
		pindex = CHAR_getCharPet( charaindex, i);
		if( CHAR_CHECKINDEX( pindex)) {
			CHAR_endCharOneArray( pindex );
			CHAR_setCharPet( charaindex, i, -1);
			snprintf( category, sizeof( category), "K%d",i);
			CHAR_sendStatusString( charaindex, category );
			snprintf( category, sizeof( category), "W%d",i);
			CHAR_sendStatusString( charaindex, category );
		}
	}
	
	CHAR_talkToCli( charaindex, -1, "����������г��",  CHAR_COLORWHITE);

}

#define MAXMESS_LISTNUM 7
#define ALLRESET (MAXMESS_LISTNUM - 1)

void CHAR_CHAT_DEBUG_reset( int charaindex, char* message )
{
	int i;
	char mess[MAXMESS_LISTNUM][256]={ "enemy", "encount", "magic", "warppoint", "petskill", "pettalk", "all"};

	for( i=0; i<MAXMESS_LISTNUM; i++){
		if( !strcmp(  mess[i], message) )
			break;
	}
	if( i >= MAXMESS_LISTNUM ){
		CHAR_talkToCli( charaindex, -1, "ָ�����",  CHAR_COLORRED);//CHAR_COLORYELLOW
		return;
	}
	switch( i){
	case ALLRESET:
	case 0:
		if( !ENEMYTEMP_reinitEnemy() || !ENEMY_reinitEnemy() ){
			CHAR_talkToCli( charaindex, -1, "���¶�ȡ���˻�������ʧ�ܡ�",  CHAR_COLORRED);
			return;
		}
		if( i != ALLRESET ) break;
	case 1:
		if( !GROUP_reinitGroup() || !ENCOUNT_reinitEncount() ){
			CHAR_talkToCli( charaindex, -1, "���¶�ȡ�����Ŷ�����ʧ�ܡ�",  CHAR_COLORRED);
		}
		if( i != ALLRESET ) break;
	case 2:
		if( !MAGIC_reinitMagic() || !ATTMAGIC_reinitMagic() ) {
			CHAR_talkToCli( charaindex, -1, "���¶�ȡ��������ʧ�ܡ�",  CHAR_COLORRED);
		}
		if( i != ALLRESET ) break;
	case 3:
		MAPPOINT_resetMapWarpPoint( 1);
		if( !MAPPOINT_loadMapWarpPoint() ){
			CHAR_talkToCli( charaindex, -1, "���¶�ȡ��Ծ������ʧ�ܡ�",  CHAR_COLORRED);
		}
		if( i != ALLRESET ) break;
	case 4:
		if( !PETSKILL_reinitPetskill() ) {
			CHAR_talkToCli( charaindex, -1, "���¶�ȡ���＼������ʧ�ܡ�",  CHAR_COLORRED);
		}
		if( i != ALLRESET ) break;
	case 5:
		LoadPetTalk();
		if( i != ALLRESET ) break;
		break;
	}
	CHAR_talkToCli( charaindex, -1, "���¶�ȡ��Ϸ���ϡ�",  CHAR_COLORYELLOW);
}

void CHAR_CHAT_DEBUG_enemyrestart( int charaindex, char* message )
{
	if( ENEMYTEMP_reinitEnemy()) {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ���˻������ϡ�",  CHAR_COLORWHITE);
	}else {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ���˻�������ʧ�ܡ�",  CHAR_COLORWHITE);
	}

	if( ENEMY_reinitEnemy()) {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ�������ϡ�",  CHAR_COLORWHITE);
	}else {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ���˻�������ʧ�ܡ�",  CHAR_COLORWHITE);
	}

	if( GROUP_reinitGroup()) {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ�Ŷ����ϡ�",  CHAR_COLORWHITE);
	}else {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ�Ŷ�����ʧ�ܡ�",  CHAR_COLORWHITE);
	}
	if( ENCOUNT_reinitEncount()) {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ�������ϡ�",  CHAR_COLORWHITE);
	}
	else {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ��������ʧ�ܡ�",  CHAR_COLORWHITE);
	}

	if( MAGIC_reinitMagic()) {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ�������ϡ�",  CHAR_COLORWHITE);
	}
	else {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ��������ʧ�ܡ�",  CHAR_COLORWHITE);
	}

#ifdef __ATTACK_MAGIC
  if( ATTMAGIC_reinitMagic() )
    CHAR_talkToCli( charaindex , -1 , "�ٴζ�ȡ�������ϡ�" , CHAR_COLORWHITE );
  else
    CHAR_talkToCli( charaindex , -1 , "�ٴζ�ȡ��������ʧ�ܡ�" , CHAR_COLORWHITE );
#endif
	if( PETSKILL_reinitPetskill()) {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ���＼�����ϡ�",  CHAR_COLORWHITE);
	}
	else {
		CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ���＼������ʧ�ܡ�",  CHAR_COLORWHITE);
	}
	
	LoadPetTalk();	// Arminius 8.15 pet talk
	CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ���ｲ�����ϡ�", CHAR_COLORWHITE);

}

void CHAR_CHAT_DEBUG_battlewatch( int charaindex, char* message )
{
	int fd = getfdFromCharaIndex( charaindex );
	if( fd != -1) {
		lssproto_LB_recv( fd,
			CHAR_getInt( charaindex, CHAR_X),
			CHAR_getInt( charaindex, CHAR_Y));
	}
}

// shan 2001/12/18 Begin   ��춸ı���࣬�ʽ�ԭ�ȵ�functionע��
void CHAR_CHAT_DEBUG_eventclean( int charaindex, char* message )
{	
#ifdef _ADD_NEWEVENT              // WON �����������
	int event_num = 8;
#else
	int event_num = 6;
#endif

	int point;
	int shiftbit;
	int eventno=0;
	char token[1024];
	int array;
	int shift;
	char token_cdkey[256]="", token_name[256]="", cdkey[CDKEYLEN], token_flag[256]="";
	unsigned int max_user = 0;
	BOOL find = FALSE;
	BOOL ret1, ret2;
	int  charaindex_tmp = 0;
	int i;
     
    getStringFromIndexWithDelim( message, " ", 1, token_flag, sizeof( token));
	ret1 = getStringFromIndexWithDelim( message, " ", 2, token_cdkey, sizeof( token_cdkey));
    ret2 = getStringFromIndexWithDelim( message, " ", 3, token_name, sizeof( token_name));
	shiftbit = atoi(token_flag);
	eventno = atoi(token_flag);

	if( shiftbit!=-1 && ret1==FALSE && ret2==FALSE ){
		charaindex_tmp = charaindex;
		find=TRUE;
	}else{
		if( shiftbit==-1 || strlen(token_cdkey)==0 || strlen(token_name)==0 ){
			sprintf( token, "%s", "��������ȷ����ȷ��ʽ��[eventsetend ������� �ʺ� ��������]");
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORWHITE);
			return; 
		}
		max_user = getFdnum();
		for( i=0; i<max_user; i++){
			char szName[256];
			int i_use;			

			i_use = CONNECT_getUse( i);
			if( i_use){
				CONNECT_getCdkey( i, cdkey, sizeof( cdkey));
				CONNECT_getCharname( i, szName, sizeof( szName));
				if( strcmp( cdkey, token_cdkey)==0 && strcmp( szName, token_name)==0){
					charaindex_tmp = CONNECT_getCharaindex( i);					
					find=TRUE;
				}
			}
		}
	}
	
	if( strcmp(token_flag, "all")==0 && find ){
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT, 0);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT2, 0);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT3, 0);
#ifdef _NEWEVENT
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT4, 0);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT5, 0);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT6, 0);
#endif
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT, 0);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT2, 0);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT3, 0);
#ifdef _NEWEVENT
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT4, 0);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT5, 0);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT6, 0);
#endif
#ifdef _ADD_NEWEVENT              // WON �����������
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT7, 0);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT7, 0);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT8, 0);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT8, 0);
#endif

		sprintf( token, "����ȫ�����¼��졣" );
		CHAR_talkToCli( charaindex, -1,token, CHAR_COLORWHITE);
		return ;
	}

	if(find){
		array = shiftbit / 32;
		shift = shiftbit % 32;
		
		if( array>=event_num ){
			sprintf( token, "���󣡣������������������ѳ���  Χ(0~%d)��",32*event_num-1);
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORWHITE);
			return;
		}
		point = CHAR_getInt( charaindex_tmp, CHAR_ENDEVENT+array);
        point = point & ~(1<<shift);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT+array, point);
		point = CHAR_getInt( charaindex_tmp, CHAR_NOWEVENT+array);	
		point = point & ~(1<<shift);	
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT+array, point);
   
		sprintf( token, "Success!! Character Name:%s delete eventflag:[%d].", CHAR_getChar( charaindex_tmp, CHAR_NAME), eventno);
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORWHITE);
		// display
		if( charaindex_tmp==charaindex )
			sprintf( token, "%s", "");
		else
			sprintf( token, "%s %s", token_cdkey, token_name);
		//CHAR_CHAT_DEBUG_watchevent( charaindex, token );
	}else{
		sprintf( token,"%s\n","ʧ�ܣ����ʺ������������޷������");
		CHAR_talkToCli(charaindex, -1, token, CHAR_COLORWHITE);
	}	
}

// shan 2001/12/15 End

// shan 2001/12/18 Begin   ��춸ı���࣬�ʽ�ԭ�ȵ�functionע��

void CHAR_CHAT_DEBUG_eventsetnow( int charaindex, char* message )
{
#ifdef _ADD_NEWEVENT              // WON �����������
	int event_num = 8;
#else
	int event_num = 6;
#endif

	int point;
	int shiftbit;
	int eventno=0;
	char token[1024];
	int array;
	int shift;
	char token_cdkey[256]="", token_name[256]="", cdkey[CDKEYLEN];
	unsigned int max_user = 0;
	BOOL find = FALSE;
	BOOL ret1, ret2;
	int  charaindex_tmp = 0;
	int i;
     
    getStringFromIndexWithDelim( message, " ", 1, token, sizeof( token));
	ret1 = getStringFromIndexWithDelim( message, " ", 2, token_cdkey, sizeof( token_cdkey));
    ret2 = getStringFromIndexWithDelim( message, " ", 3, token_name, sizeof( token_name));
	shiftbit = atoi(token);
	eventno = atoi(token);

	if( shiftbit!=-1 && ret1==FALSE && ret2==FALSE ){
		charaindex_tmp = charaindex;
		find=TRUE;
	}
	else{
		if( shiftbit==-1 || strlen(token_cdkey)==0 || strlen(token_name)==0 ){
			sprintf( token, "%s", "��������ȷ����ȷ��ʽ��[eventsetend ������� �ʺ� ��������]");
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORWHITE);
			return; 
		}
		max_user = getFdnum();
		for( i=0; i<max_user; i++){
			char szName[256];
			int i_use;			

			i_use = CONNECT_getUse( i);
			if( i_use){
				CONNECT_getCdkey( i, cdkey, sizeof( cdkey));
				CONNECT_getCharname( i, szName, sizeof( szName));
				if( strcmp( cdkey, token_cdkey)==0 && strcmp( szName, token_name)==0){
					charaindex_tmp = CONNECT_getCharaindex( i);					
					find=TRUE;
				}
			}
		}
	}

	if( strcmp(token, "all")==0 && find ){
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT, -1);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT2, -1);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT3, -1);
#ifdef _NEWEVENT
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT4, -1);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT5, -1);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT6, -1);
#endif
#ifdef _ADD_NEWEVENT              // WON �����������
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT7, -1);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT8, -1);
#endif

		sprintf( token, "Success, set all nowflag.." );
		CHAR_talkToCli( charaindex, -1,token, CHAR_COLORWHITE);
		return ;
	}

	if(find){
		array = shiftbit / 32;
		shift = shiftbit % 32;
		
		if( array>=event_num ){
			sprintf( token, "���󣡣������������������ѳ���  Χ(0~%d)��",32*event_num-1);
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORWHITE);
			return;
		}		
		point = CHAR_getInt( charaindex_tmp, CHAR_NOWEVENT+array);
		point = point | (1<<shift);
		CHAR_setInt( charaindex_tmp, CHAR_NOWEVENT+array, point);
   
		sprintf( token, "Success!! Character Name:%s eventnow_number:[%d].", CHAR_getChar( charaindex_tmp, CHAR_NAME), eventno);
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORWHITE);
		// display
		if( charaindex_tmp==charaindex )
			sprintf( token, "%s", "");
		else
			sprintf( token, "%s %s", token_cdkey, token_name);
		CHAR_CHAT_DEBUG_watchevent( charaindex, token );
	}else{
		sprintf( token,"%s\n","ʧ�ܣ����ʺ������������޷������");
		CHAR_talkToCli(charaindex, -1, token, CHAR_COLORWHITE);
	}
}
/*void CHAR_CHAT_DEBUG_eventsetnow( int charaindex, char* message )
{
	int point;
	int shiftbit;
	int eventno=0;
	char token[256];
	int array;
	int shift;

	shiftbit=atoi(message);
	eventno=atoi(message);

	// -1���׷º�����绥ئ����ì����   
	if(shiftbit==-1) return;

	array = shiftbit / 32;
	shift = shiftbit % 32;

	point = CHAR_getInt(charaindex,CHAR_NOWEVENT+array);
	point = point | (1 << shift);
	CHAR_setInt(charaindex,CHAR_NOWEVENT+array,point);

	sprintf( token, "�趨�¼���%d���졣",eventno);
	CHAR_talkToCli( charaindex, -1,token, CHAR_COLORWHITE);
}*/
// shan 2001/12/18 End

// shan 2001/12/15 Begin   ��춸ı���࣬�ʽ�ԭ�ȵ�functionע��

void CHAR_CHAT_DEBUG_eventsetend( int charaindex, char* message )
{
#ifdef _ADD_NEWEVENT              // WON �����������
	int event_num = 8;
#else
 	int event_num = 6;
#endif

	int point;
	int shiftbit;
	int eventno = 0;
	char token[1024];
	int array;
	int shift;
	char token_cdkey[256]="", token_name[256]="", cdkey[CDKEYLEN];
	unsigned int max_user = 0;
	BOOL find = FALSE;
	BOOL ret1, ret2;
	int  charaindex_tmp = 0;
	int i;
	
	getStringFromIndexWithDelim( message, " ", 1, token, sizeof( token));
	ret1 = getStringFromIndexWithDelim( message, " ", 2, token_cdkey, sizeof( token_cdkey));
    ret2 = getStringFromIndexWithDelim( message, " ", 3, token_name, sizeof( token_name));
	shiftbit = atoi(token);
	eventno = atoi(token);

    if( shiftbit!=-1 && ret1==FALSE && ret2==FALSE ){
		charaindex_tmp = charaindex;
		find=TRUE;
	}
	else{
		if( shiftbit==-1 || strlen(token_cdkey)==0 || strlen(token_name)==0 ){
			sprintf( token, "%s", "��������ȷ����ȷ��ʽ��[eventsetend ������� �ʺ� ��������]");
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORWHITE);
			return; 
		}
		max_user = getFdnum();
		for( i=0; i<max_user; i++){
			char szName[256];
			int i_use;			

			i_use = CONNECT_getUse( i);
			if( i_use){
				CONNECT_getCdkey( i, cdkey, sizeof( cdkey));
				CONNECT_getCharname( i, szName, sizeof( szName));
				if( strcmp( cdkey, token_cdkey)==0 && strcmp( szName, token_name)==0){
					charaindex_tmp = CONNECT_getCharaindex( i);					
					find=TRUE;
				}
			}
		}
	}

	if( strcmp(token, "all")==0 && find ){
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT, -1);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT2, -1);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT3, -1);
#ifdef _NEWEVENT
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT4, -1);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT5, -1);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT6, -1);
#endif
#ifdef _ADD_NEWEVENT              // WON �����������
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT7, -1);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT8, -1);
#endif

		sprintf( token, "Success, set all endflag.." );
		CHAR_talkToCli( charaindex, -1,token, CHAR_COLORWHITE);
		return ;
	}

	if(find){
		array = shiftbit / 32;
		shift = shiftbit % 32;
		
		if( array>=event_num ){
			sprintf( token, "���󣡣������������������ѳ���  Χ(0~%d)��",32*event_num-1);
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORWHITE);
			return;
		}
		point = CHAR_getInt( charaindex_tmp, CHAR_ENDEVENT+array);
		point = point | (1<<shift);
		CHAR_setInt( charaindex_tmp, CHAR_ENDEVENT+array, point);
   
		sprintf( token, "Success!! Character Name:%s eventend_number:[%d].", CHAR_getChar( charaindex_tmp, CHAR_NAME), eventno);
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
        // display
		if( charaindex_tmp==charaindex )
			sprintf( token, "%s", "");
		else
			sprintf( token, "%s %s", token_cdkey, token_name);
		CHAR_CHAT_DEBUG_watchevent( charaindex, token );
	}else{
		sprintf( token,"%s\n","ʧ�ܣ����ʺ������������޷������");
		CHAR_talkToCli(charaindex, -1, token, CHAR_COLORYELLOW);
	}

}

void CHAR_CHAT_DEBUG_debug( int charaindex, char* message )
{
	char	msgbuf[256];
	BOOL	flg = isstring1or0( message);
	if( flg == TRUE ){
		CHAR_setWorkInt( charaindex, CHAR_WORKFLG,
			CHAR_getWorkInt( charaindex, CHAR_WORKFLG ) | WORKFLG_DEBUGMODE );
		CHAR_setWorkInt( charaindex, CHAR_WORKDEBUGMODE, TRUE);
	}else{
		CHAR_setWorkInt( charaindex, CHAR_WORKFLG,
			CHAR_getWorkInt( charaindex, CHAR_WORKFLG ) & ~WORKFLG_DEBUGMODE );
		CHAR_setWorkInt( charaindex, CHAR_WORKDEBUGMODE, FALSE);
	}
	CHAR_send_P_StatusString( charaindex, CHAR_P_STRING_DEBUGMODE);
	snprintf( msgbuf, sizeof( msgbuf), "���趨����ģʽΪ%s��",
		( flg == TRUE) ? "ON" : "OFF");

	CHAR_talkToCli( charaindex, -1, msgbuf, CHAR_COLORYELLOW);

}

void CHAR_CHAT_DEBUG_dp( int charindex , char *message )
{
	CHAR_setInt( charindex , CHAR_DUELPOINT, atoi( message ) );
	CHAR_send_P_StatusString( charindex ,CHAR_P_STRING_DUELPOINT );
}

#ifdef _EQUIT_ARRANGE
void CHAR_CHAT_DEBUG_arrange( int charindex , char *message )
{
	CHAR_setWorkInt( charindex, CHAR_WORKFIXARRANGE, atoi( message) );
	CHAR_complianceParameter( charindex );
	CHAR_talkToCli( charindex, -1, "�趨����", CHAR_COLORYELLOW);
}
#endif


#ifdef _EQUIT_SEQUENCE
void CHAR_CHAT_DEBUG_sequence( int charindex , char *message )
{
	CHAR_setWorkInt( charindex, CHAR_WORKFIXSEQUENCE, atoi( message) );
	CHAR_complianceParameter( charindex );
	CHAR_talkToCli( charindex, -1, "�趨˳��", CHAR_COLORYELLOW);
}
#endif

#define ITEMRESERVESTRING   "item"
#define POOLITEMRESERVESTRING   "poolitem"

void CHAR_CHAT_DEBUG_setmerge_main( int charaindex, char* message, int mode )
{
	int haveitemindex = atoi( message);
	int itemindex;
	
	if( !CHAR_CHECKITEMINDEX( charaindex, haveitemindex) ){
		CHAR_talkToCli( charaindex, -1, "�������֡�", CHAR_COLORWHITE);
		return;
	}
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX( itemindex)) {
		CHAR_talkToCli( charaindex, -1, "�ô��Ƿ�û���κε���,�鱨�����֡�", CHAR_COLORWHITE);
		return;
	}
	ITEM_setInt( itemindex, ITEM_MERGEFLG, mode);
	
	CHAR_talkToCli( charaindex, -1, "�����׷º뼰����ë�������ף�", CHAR_COLORWHITE);
	
	CHAR_sendItemDataOne( charaindex, haveitemindex);
}

void CHAR_CHAT_DEBUG_effect( int charaindex, char* message )
{
	int		floorid=-1, effectid=-1, level=0;
	char	buf[ 256]; 
	int		i;
    int     playernum = CHAR_getPlayerMaxNum();
	BOOL	flg = FALSE;
	
	getStringFromIndexWithDelim( message, " ", 1, buf, sizeof( buf));
	if( strcmp( buf, "alloff" ) == 0 ) {
		flg = TRUE;
	}else {
		floorid = atoi( buf);
		if( !getStringFromIndexWithDelim( message, " ", 2, buf, sizeof( buf))) {
			return;
		}
		effectid = atoi( buf);
		if( !getStringFromIndexWithDelim( message, " ", 3, buf, sizeof( buf))) {
			return;
		}
		level = atoi( buf);
	}
	for( i = 0 ; i < playernum; i ++) {
		if( CHAR_CHECKINDEX( i)) {
			if( CHAR_getInt( i, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER){
				if( flg) {
					int ef = CHAR_getWorkInt( i, CHAR_WORKEFFECT);
					if( ef != 0 ) {
						int fd = getfdFromCharaIndex( i);
						lssproto_EF_send( fd, 0, 0, "");
						CHAR_setWorkInt( i, CHAR_WORKEFFECT, 0);
					}
				}
				else if( CHAR_getInt( i, CHAR_FLOOR) == floorid ) {
					int fd = getfdFromCharaIndex( i);
					lssproto_EF_send( fd, effectid, level, "");
					if( level == 0 ) {
						CHAR_setWorkInt( i, CHAR_WORKEFFECT, 
										CHAR_getWorkInt( i, CHAR_WORKEFFECT) &
											~effectid);
					}
					else {
						CHAR_setWorkInt( i, CHAR_WORKEFFECT, 
										CHAR_getWorkInt( i, CHAR_WORKEFFECT) | 
											effectid);
					}
				}
			}
		}
	}
}

void CHAR_CHAT_DEBUG_setTrans( int charaindex, char *message)
{
	CHAR_setInt(charaindex, CHAR_TRANSEQUATION, 0);
	CHAR_talkToCli( charaindex, -1, "�趨ת��", CHAR_COLORWHITE);

	CHAR_setInt(charaindex, CHAR_TRANSMIGRATION, atoi( message ));
	CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charaindex, CHAR_WORKOBJINDEX ));
}

void CHAR_CHAT_DEBUG_shutup(int charindex ,char *message)
{
     char token[100];
     char token1[100];
     char cdkey[CDKEYLEN];
     char buf[256];
     char line[256];
     unsigned int MAX_USER=0,i;
     BOOL flg;
     easyGetTokenFromString( message , 1 , token, sizeof(token));
     easyGetTokenFromString( message , 2 , token1, sizeof(token1));
     if( strlen(message)==0){
         sprintf(buf,"%s","��������ȷ");
         CHAR_talkToCli( charindex, -1,buf, CHAR_COLORYELLOW);
         return;
     }
     if( strlen(token1)==0){
         sprintf(buf,"%s","��������ȷ--> ON OFF");
         CHAR_talkToCli( charindex, -1,buf, CHAR_COLORYELLOW);
         return;
     }
     flg = isstring1or0( token1);
     MAX_USER=getFdnum();
     for(i=0;i<MAX_USER;i++){
         int i_use;
         i_use=CONNECT_getUse(i);
         if(i_use){
            CONNECT_getCdkey( i, cdkey, sizeof( cdkey ));
            if(strcmp(token,cdkey)==0){
               int index = CONNECT_getCharaindex( i );
               if(flg){
                  CONNECT_set_shutup(i,TRUE);
                  //print("\n<Set TRUE");
                  snprintf( line, sizeof( line),
                            "�� name:%s account=%s����",
                            CHAR_getChar( index, CHAR_NAME),
                            cdkey);
                  CHAR_talkToCli( charindex, -1,line, CHAR_COLORYELLOW);
               }else{
                  CONNECT_set_shutup(i,FALSE);
                  //print("\n<Set FALSE");
                  snprintf( line, sizeof( line),
                            "�� name:%s account=%s����",
                            CHAR_getChar( index, CHAR_NAME),
                            cdkey);
                  CHAR_talkToCli( charindex, -1,line, CHAR_COLORYELLOW);
               }
             }
          }//if use
       }//for
}

void CHAR_CHAT_DEBUG_getuser(int charindex ,char *message) //ttom 14/11/2000
{	//  ..NAME|FLOOR|
	char buf1[256], name[256], token[256];
	int i, k, MyFloor=-1, colors = CHAR_COLORWHITE;
	int user[50];
	int MAXCHARA=0;
	BOOL FINDNPC = FALSE;
	BOOL MOREs = FALSE;

	memset( name, 0, sizeof( name) );
	MAXCHARA = CHAR_getPlayerMaxNum();
	if( getStringFromIndexWithDelim( message, " ", 1, name, sizeof( name)) == FALSE )	return;
	if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof( buf1)) != FALSE ){
		MyFloor = atoi( buf1);
	}
	if( getStringFromIndexWithDelim( message, " ", 3, buf1, sizeof( buf1)) != FALSE ){
		if( strstr( buf1, "npc") != NULL ){
			FINDNPC = TRUE;
			MAXCHARA = CHAR_getCharNum();
		}
	}
	k = 0;
	for( i=0; i<MAXCHARA && k<50; i++ ){
		if( !CHAR_CHECKINDEX( i) ) continue;
		if( FINDNPC == TRUE ){
			if( CHAR_getInt( i, CHAR_WHICHTYPE ) == CHAR_TYPEPLAYER  ||
				CHAR_getInt( i, CHAR_WHICHTYPE ) == CHAR_TYPEPET ) continue;
		}
		if( MyFloor != -1 && MyFloor != CHAR_getInt( i, CHAR_FLOOR) ) continue;

		if( strstr( CHAR_getChar( i, CHAR_OWNTITLE), name) != NULL ||
			strstr( CHAR_getChar( i, CHAR_NAME), name) != NULL ){
			user[k++] = i;
			if( k >= 10 )MOREs = TRUE;
		}
	}

	memset( token, 0, sizeof(token));
	strcpy( token, "MORE:");
	for( i=0; i<k; i++){//CHAR_COLORYELLOW
		int userindex = user[i];
		if( !CHAR_CHECKINDEX( userindex)) continue;
		if( !strcmp( CHAR_getUseName( userindex), name ) ||
			!strcmp( CHAR_getChar( userindex, CHAR_NAME), name ) ){
			colors = CHAR_COLORRED;
		}else if( strstr( CHAR_getChar( userindex, CHAR_OWNTITLE), name) != NULL ||
			strstr( CHAR_getChar( userindex, CHAR_NAME), name) != NULL ){
			if( MOREs == FALSE ){
				colors = CHAR_COLORWHITE;
			}else{
				char temp[256];
				sprintf( temp, "[%s]", CHAR_getChar( userindex, CHAR_NAME));
				if( (strlen(token)+strlen(temp)) >= sizeof(token) ){
					CHAR_talkToCli( charindex, -1 , token , CHAR_COLORWHITE);
					memset( token, 0, sizeof(token));
					strcpy( token, "MORE:");
				}
				strcat( token, temp);
				continue;
			}
		}
		sprintf( buf1, "NAME:%s UName:%s CDKey:%s Point:[%d,%d,%d]",
			CHAR_getChar( userindex, CHAR_NAME), CHAR_getUseName( userindex),
			CHAR_getChar( userindex, CHAR_CDKEY),
			CHAR_getInt( userindex, CHAR_FLOOR), CHAR_getInt( userindex, CHAR_X),
			CHAR_getInt( userindex, CHAR_Y) );
		CHAR_talkToCli( charindex, -1 , buf1 , colors);
	}
	if( MOREs == TRUE ){
		CHAR_talkToCli( charindex, -1 , token , CHAR_COLORWHITE);
	}
}

void CHAR_CHAT_DEBUG_waeikick( int charindex, char* message )
{
    char token[100];
    char cdkey[CDKEYLEN];
    char kcmsg[256],kctalk[256];
    char szName[256];
    char buf[256];
    BOOL find=FALSE;
    int i=0;
    unsigned int MAX_USER=0;
    int fd_charaindex;
    if( strlen(message)==0){
        sprintf(buf,"%s","��������ȷ-> �ʺ�");
        CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
        return;
    }
    easyGetTokenFromString( message , 1 , token, sizeof(token));
    if( strlen(token)==0){
        sprintf(buf,"%s","��������ȷ");
        CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
        return;
    }
    MAX_USER=getFdnum();
    for(i=0;i<MAX_USER;i++){
        int i_use;
        i_use=CONNECT_getUse(i);
        if(i_use){
           CONNECT_getCdkey( i, cdkey, sizeof( cdkey ));
           if(strcmp(cdkey,token)==0){
              fd_charaindex = CONNECT_getCharaindex( i );
              CONNECT_getCharname( i, szName, sizeof( szName ) );
              sprintf(kcmsg,"%s����ŷ�����",szName);
              CHAR_talkToCli(fd_charaindex, -1, "�������¶����ߡ�", CHAR_COLORYELLOW);
              CONNECT_setCloseRequest( i , 1 );
              print( "\nWAEIKICK cdkey=%s", cdkey);
              find=TRUE;
           }
        }
    }//for i
    if(find){
       sprintf(kctalk,"ϵͳ��");
       for(i=0;i<MAX_USER;i++){
           int i_use;
           i_use=CONNECT_getUse(i);
           if(i_use){
              fd_charaindex = CONNECT_getCharaindex( i );
              CHAR_talkToCli( fd_charaindex, -1 , kctalk , CHAR_COLORWHITE);
              CHAR_talkToCli( fd_charaindex, -1 , kcmsg , CHAR_COLORWHITE);
           }
       }//for i
    }//if find
}

void CHAR_CHAT_DEBUG_waeikickall( int charindex, char* message )
{    
    int i=0;
    unsigned int MAX_USER=0;
    
    MAX_USER=getFdnum();
    for(i=0;i<MAX_USER;i++){
        int i_use;
        i_use=CONNECT_getUse(i);
        if(i_use){
           CONNECT_setCloseRequest( i , 1 );
        }
    }//for i
}

void CHAR_CHAT_DEBUG_jail( int charindex, char* message )
{
     char token[100];
     char cdkey[CDKEYLEN];
     char kcmsg[256],kctalk[256];
     char szName[256];
     char buf[256];
     BOOL find=FALSE;
     int i=0;
     unsigned int MAX_USER=0;
     int fd_charaindex;
     Char *chwk;
                      
     if( strlen(message)==0){
         sprintf(buf,"%s","��������ȷ-> �ʺ�");
         CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
         return;
     }
     easyGetTokenFromString( message , 1 , token, sizeof(token));
     if( strlen(token)==0){
         sprintf(buf,"%s","��������ȷ");
         CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
         return;
     }
     MAX_USER=getFdnum();
     for(i=0;i<MAX_USER;i++){
         int i_use;
         i_use=CONNECT_getUse(i);
         if(i_use){
            CONNECT_getCdkey( i, cdkey, sizeof( cdkey ));
            if(strcmp(cdkey,token)==0){
               fd_charaindex = CONNECT_getCharaindex( i );
               CONNECT_getCharname( i, szName, sizeof( szName ) );
               sprintf(kcmsg,"%s����ؽѡ�",szName);
               CHAR_talkToCli(fd_charaindex, -1, "�������¶�������", CHAR_COLORYELLOW);
               CHAR_setInt(fd_charaindex,CHAR_FLOOR,117);
               CHAR_setInt(fd_charaindex,CHAR_X,225);
               CHAR_setInt(fd_charaindex,CHAR_Y,13);
               CHAR_warpToSpecificPoint(fd_charaindex,117,225,13);
               chwk = CHAR_getCharPointer( fd_charaindex);

			   // CoolFish: +1 2001/11/05
			   if (!chwk)	continue;
#ifdef _CHAR_POOLITEM
				if( CHAR_SaveDepotItem( fd_charaindex) == FALSE ){
					print("saveDepotItem:%d\n", fd_charaindex);
				}
#endif
#ifdef _CHAR_POOLPET
				if( CHAR_SaveDepotPet( fd_charaindex) == FALSE ){
					print("saveDepotPet:%d\n", fd_charaindex);
				}
#endif
				CHAR_charSaveFromConnectAndChar( i, chwk ,FALSE );
               print( "\nWAEI JAIL cdkey=%s", cdkey);
               find=TRUE;
            }
         }
     }//for i
      if(find){
         sprintf(kctalk,"ϵͳ��");
         for(i=0;i<MAX_USER;i++){
             int i_use;
             i_use=CONNECT_getUse(i);
             if(i_use){
                fd_charaindex = CONNECT_getCharaindex( i );
                CHAR_talkToCli( fd_charaindex, -1 , kctalk , CHAR_COLORWHITE);
                CHAR_talkToCli( fd_charaindex, -1 , kcmsg , CHAR_COLORWHITE);
             }
         }//for i
      }//if find
}
void CHAR_CHAT_DEBUG_shutupall(int charindex ,char *message)
{
     int fl,x,y,i,j,fd;
     char cdkey[CDKEYLEN];
     char line[256];
     OBJECT  object;
     fl=CHAR_getInt( charindex, CHAR_FLOOR );
     x=CHAR_getInt( charindex, CHAR_X );
     y=CHAR_getInt( charindex, CHAR_Y );
     for(i=y-2;i<=y+2;i++){
         for(j=x-2;j<=x+2;j++){
             for( object = MAP_getTopObj(fl,j,i) ; object ;object = NEXT_OBJECT(object ) ){
                  int o = GET_OBJINDEX(object);
                  if( OBJECT_getType(o) == OBJTYPE_CHARA ){
                      int chara_index=OBJECT_getIndex(o);
                      if( CHAR_getInt( chara_index, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER ) continue;
                      if( (i==y)&&(j==x)) continue;
                      if( ( fd = getfdFromCharaIndex(chara_index) ) != -1 ){
                            CONNECT_set_shutup(fd,TRUE);
                            CONNECT_getCdkey( fd, cdkey, sizeof( cdkey ));
                            snprintf( line, sizeof( line),
                                      "�� name:%s account=%s����",
                                      CHAR_getChar( chara_index, CHAR_NAME),
                                      cdkey);
                            CHAR_talkToCli( charindex, -1,line, CHAR_COLORWHITE);
                            CHAR_talkToCli(chara_index,-1,"����ħ������", CHAR_COLORWHITE);
                                                                                                                                  
                      }
                  }
            }//for object
        }//for j
    }//for i
}
void CHAR_CHAT_DEBUG_send(int charindex ,char *message)
{
     char buf[256];
     char token[100];
     char szName[256];
     char cdkey[CDKEYLEN];
     char line[256];
     int fl,x,y,i;
     unsigned int MAX_USER=0;
     MAX_USER=getFdnum();
     if( strlen(message)==0){
        sprintf(buf,"%s","��������ȷ-> FLOOR X Y �ʺ�");
        CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
        return;
     }
     easyGetTokenFromString( message , 1 , token, sizeof(token));
     if( strlen(token)==0){
         sprintf(buf,"%s","��������ȷ-->��ͼ���");
         CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
         return;
     }
     fl=atoi( token );
     easyGetTokenFromString( message , 2 , token, sizeof(token));
     if( strlen(token)==0){
         sprintf(buf,"%s","��������ȷ-->X���� ");
         CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
         return;
     }
     x = atoi( token );
     easyGetTokenFromString( message , 3 , token, sizeof(token));
     if( strlen(token)==0){
         sprintf(buf,"%s","��������ȷ-->Y���� ");
         CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
         return;
     }
     y = atoi( token );
     easyGetTokenFromString( message , 4 , token, sizeof(token));
     if( strlen(token)==0){
         sprintf(buf,"%s","��������ȷ-->�ʺ�");
         CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
         return;
     }
     for(i=0;i<MAX_USER;i++){                                                                               
         int i_use,fd_charaindex;
         i_use=CONNECT_getUse(i);
         if(i_use){
            CONNECT_getCharname( i, szName, sizeof( szName ) );
            CONNECT_getCdkey( i, cdkey, sizeof( cdkey ));
            if(strcmp(token,cdkey)==0){
               fd_charaindex = CONNECT_getCharaindex( i );
               CHAR_warpToSpecificPoint(fd_charaindex, fl, x, y);
               snprintf( line, sizeof( line),
                         "�� name:%s account=%s ���͵� FL=%d X=%d Y=%d",
                          szName,cdkey,fl,x,y);
                 CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );
                 CHAR_talkToCli( fd_charaindex, -1 ,"��.����ϵͳ����", CHAR_COLORWHITE );
            }                                  
         }
     }
}
//ttom end
void CHAR_CHAT_DEBUG_noenemy( int charaindex, char* message )
{
	char	msgbuf[256];
	BOOL	flg = isstring1or0( message);
	if( flg == TRUE ){
	  int fd=CHAR_getWorkInt( charaindex, CHAR_WORKFD);
	  setEqNoenemy(fd, 200);
	}else{
	  int fd=CHAR_getWorkInt( charaindex, CHAR_WORKFD);
	  setEqNoenemy(fd, 0);
	}
	snprintf( msgbuf, sizeof( msgbuf), "������ģʽ%s��",
		( flg == TRUE) ? "����" : "�ر�");

	CHAR_talkToCli( charaindex, -1, msgbuf, CHAR_COLORWHITE);
}

// Arminius 7.12 login announce
#define ANNOUNCEFILE	"./announce.txt"

void CHAR_CHAT_DEBUG_loginannounce( int charaindex, char* message )
{
  char buf[256];
  char cmd[256];
  char say[256];

  if (message == NULL)	return;
  if( strlen(message)==0){
    sprintf(buf,"%s","��������ȷ-> clear/add/send ����Сд");
    CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
    return;
  }
  easyGetTokenFromString(message, 1, cmd, sizeof(cmd));
  if (strcmp(cmd,"clear")==0) {
    FILE *f=fopen(ANNOUNCEFILE,"w");

    strcpy(buf, "ʯ��ʱ������\n");
    fwrite(buf, strlen(buf), 1, f);
    fclose(f);
    LoadAnnounce();
    CHAR_talkToCli(charaindex, -1, "OK", CHAR_COLORWHITE);
  } else if (strcmp(cmd,"add")==0) {
    FILE *f;

    easyGetTokenFromString(message, 2, say, sizeof(say));
    if (strlen(say)==0) {
      sprintf(buf,"%s","��������ȷ");
      CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
      return;
    }
  
    if (!(f=fopen(ANNOUNCEFILE,"a+"))) {
      f=fopen(ANNOUNCEFILE,"w");
      strcpy(buf, "ʯ��ʱ������\n");
      fwrite(buf, strlen(buf), 1, f);
    }
    fwrite(say, strlen(say), 1, f);
    fwrite("\n", sizeof(char), 1, f);
    fclose(f);
    LoadAnnounce();
    CHAR_talkToCli(charaindex, -1, "OK", CHAR_COLORWHITE);
  } else if (strcmp(cmd,"send")==0) {
    int i;
    
    for (i=0; i<CHAR_getPlayerMaxNum(); i++) AnnounceToPlayer(i);
    CHAR_talkToCli(charaindex, -1, "OK", CHAR_COLORWHITE);
  } else if (strcmp(cmd,"load")==0) {
    LoadAnnounce();
    CHAR_talkToCli(charaindex, -1, "OK, loginannounce loaded.", CHAR_COLORWHITE);
  } else {
    sprintf(buf,"%s","��������ȷ-> clear/add/send (����Сд)");
    CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
    return;
  }
}

void CHAR_CHAT_DEBUG_checklock( int charaindex, char* message )
{
  char cmd[256];
  if( strlen(message)==0){
    CHAR_talkToCli(charaindex, -1, "��������ȷ", CHAR_COLORWHITE);
    return;
  }
  easyGetTokenFromString( message, 1, cmd, sizeof(cmd));
  saacproto_ACLock_send( acfd, cmd, 2, getFdidFromCharaIndex( charaindex));
}

void CHAR_CHAT_DEBUG_unlock( int charaindex, char* message )
{
	char cmd[256];
#ifdef _WAEI_KICK
	int act=1;
#endif

  if( strlen(message)==0){
    CHAR_talkToCli(charaindex, -1, "��������ȷ", CHAR_COLORWHITE);
    return;
  }
  easyGetTokenFromString(message, 1, cmd, sizeof(cmd));
#ifdef _WAEI_KICK
  saacproto_ACKick_send( acfd, cmd, getFdidFromCharaIndex( charaindex), act);
#else
  saacproto_ACLock_send( acfd, cmd, 3, getFdidFromCharaIndex( charaindex));
#endif
}

void CHAR_CHAT_DEBUG_unlockserver( int charaindex, char* message )
{
  char cmd[256];

  if( strlen(message)==0){
    CHAR_talkToCli(charaindex, -1, "��������ȷ", CHAR_COLORWHITE);
    return;
  }
  easyGetTokenFromString(message, 1, cmd, sizeof(cmd));
  saacproto_ACLock_send( acfd, cmd, 4, getFdidFromCharaIndex( charaindex));
}

void CHAR_CHAT_DEBUG_fixfmdata(int charaindex, char *message)
{
  char szCmd[64],szData[64],szFamilyID[8],szID[64];
  int	i,index = -1,charindex = -1,iPlayerNum = CHAR_getPlayerMaxNum();

	if(message == NULL || getStringFromIndexWithDelim(message," ",1,szFamilyID,sizeof(szFamilyID)) == FALSE){
		CHAR_talkToCli(charaindex, -1, "��������ȷ [fixfmdata <����ID> <�ʺ�(or -1)> <cmd> <data>]",CHAR_COLORRED);
   	return;
  }
	if(message == NULL || getStringFromIndexWithDelim(message," ",2,szID,sizeof(szID)) == FALSE){
		CHAR_talkToCli(charaindex, -1, "��������ȷ [fixfmdata <����ID> <�ʺ�(or -1)> <cmd> <data>]",CHAR_COLORRED);
   	return;
  }
	if(message == NULL || getStringFromIndexWithDelim(message," ",3,szCmd,sizeof(szCmd)) == FALSE){
		CHAR_talkToCli(charaindex, -1, "��������ȷ [fixfmdata <����ID> <�ʺ�(or -1)> <cmd> <data>]",CHAR_COLORRED);
   	return;
  }
	if(message == NULL || getStringFromIndexWithDelim(message," ",4,szData,sizeof(szData)) == FALSE){
		CHAR_talkToCli(charaindex, -1, "��������ȷ [fixfmdata <����ID> <�ʺ�(or -1)> <cmd> <data>]",CHAR_COLORRED);
   	return;
  }
	index = atoi(szFamilyID);
	if(index < 1 || index > 1000){
		CHAR_talkToCli(charaindex,-1,"id ֵ����", CHAR_COLORRED);
		return;
	}
	index -= 1;
	if(szID[0] != '-' && szID[1] != '1'){
		for(i=0;i<iPlayerNum;i++){
			if(CHAR_getCharUse(i) != FALSE){
				if(strcmp(CHAR_getChar(i,CHAR_CDKEY),szID) == 0){
					charindex = i;
					break;
				}
			}
		}
		if(charindex == -1){
			CHAR_talkToCli(charaindex,-1,"�Ҳ������ʺŵ�����", CHAR_COLORRED);
			return;
		}
		if(CHAR_getInt(charindex,CHAR_FMINDEX) == -1){
			CHAR_talkToCli(charaindex,-1,"���ʺŵ�����û�м���", CHAR_COLORRED);
			return;
		}
		else{
			if(CHAR_getWorkInt(charindex,CHAR_WORKFMINDEXI) != index){
				CHAR_talkToCli(charaindex,-1,"���ʺŵ��������ID������ļ���ID����", CHAR_COLORRED);
				return;
			}
		}
	}

	print("GMFixData index:%d charindex:%d cmd:%s data:%s\n",index,charindex,szCmd,szData);
  saacproto_ACGMFixFMData_send(acfd,index,szID,szCmd,szData,getFdidFromCharaIndex(charaindex));
}

// WON ADD �����峤����
void CHAR_CHAT_DEBUG_fixfmleader(int charaindex, char *message)
{
   char	token[256], cmd[256], id[16];
   int	index, charindex, i, j, user_index=-1, flag;  
   extern struct  FMMEMBER_LIST memberlist[FAMILY_MAXNUM];  

   if (strlen(message) == 0)
   {
   		CHAR_talkToCli(charaindex, -1, "��������ȷ [fixfmleader �ʺ� 1]", CHAR_COLORWHITE);
		return;
   }

   easyGetTokenFromString(message, 1, token, sizeof(token));
   sprintf(id, token);			// id		
   
   easyGetTokenFromString(message, 2, token, sizeof(token));
   flag = atoi(token);			// flag ִ����꣬���� 1 ������

   for(i=0;i<getFdnum() ;i++){
	   if( CONNECT_getUse(i) ){
		    j = CONNECT_getCharaindex(i);			
			if( CHAR_getChar(j, CHAR_CDKEY)){
				if ( strcmp(id, CHAR_getChar(j, CHAR_CDKEY)) == 0 ){
					user_index = j;
					break;
				}  	 		
			}
	   }
   }

   if(user_index == -1 ){
   		CHAR_talkToCli(charaindex, -1, "��Ҳ�������!!", CHAR_COLORWHITE);	   
		return;
   }

   index = CHAR_getInt( user_index, CHAR_FMINDEX ) - 1;
   charindex = 0;

   if( !flag ){ 
		
		if(strcmp(memberlist[index].numberlistarray[0], "") == 0 ){
		   	CHAR_talkToCli(charaindex, -1, "�˼��������峤��!!", CHAR_COLORWHITE);	   
			return;
		}
		if(CHAR_getInt( user_index, CHAR_FMLEADERFLAG ) == FMMEMBER_LEADER ){
		   	CHAR_talkToCli(charaindex, -1, "���������������峤!!", CHAR_COLORWHITE);	   
			return;
		}
		saacproto_ACGMFixFMData_send(acfd, index, id, " "," ", getFdidFromCharaIndex(charaindex));
		return;
   }

   //charname
   strcpysafe( cmd, sizeof(cmd), CHAR_getChar( user_index, CHAR_NAME));
   saacproto_ACGMFixFMData_send(acfd, index, id, "charname", cmd, getFdidFromCharaIndex(charaindex));

   //charid 
   strcpysafe( cmd, sizeof(cmd), CHAR_getChar( user_index, CHAR_CDKEY));
   saacproto_ACGMFixFMData_send(acfd, index, id, "charid", cmd, getFdidFromCharaIndex(charaindex));
 
   //charlv
   sprintf( cmd, "%d", CHAR_getInt( user_index, CHAR_LV) );
   saacproto_ACGMFixFMData_send(acfd, index, id, "charlv", cmd, getFdidFromCharaIndex(charaindex));

   //charflag
   strcpysafe( cmd, sizeof(cmd), "3");
   saacproto_ACGMFixFMData_send(acfd, index, id, "charflag", cmd, getFdidFromCharaIndex(charaindex));
 
   //predeltime
   strcpysafe( cmd, sizeof(cmd), "0");
   saacproto_ACGMFixFMData_send(acfd, index, id, "predeltime", cmd, getFdidFromCharaIndex(charaindex));
 
   //popular
   sprintf( cmd, "%d", CHAR_getInt( user_index, CHAR_FAME) );
   saacproto_ACGMFixFMData_send(acfd, index, id, "popular", cmd, getFdidFromCharaIndex(charaindex));
 
   //eventflag
   strcpysafe( cmd, sizeof(cmd), "0");
   saacproto_ACGMFixFMData_send(acfd, index, id, "eventflag", cmd, getFdidFromCharaIndex(charaindex));
 
}


void CHAR_CHAT_DEBUG_manorpk(int charaindex, char *message)
{
	char buf[256];
	char cmd[256];
	int i,manorid;
	int char_max=CHAR_getCharNum();

	if (message == NULL)	return;
	if( strlen(message)==0){
		sprintf(buf,"%s","��������ȷ�� [manorpk <allpeace/allreset/clean> id]");
		CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
		return;
	}

	easyGetTokenFromString(message, 1, cmd, sizeof(cmd));
	if (strcmp(cmd,"allpeace")==0) {
#ifdef _NEW_MANOR_LAW
		struct tm tm1;

		memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));
#endif
		CHAR_talkToCli(charaindex, -1, "allpeace", CHAR_COLORWHITE);
		easyGetTokenFromString(message, 2, cmd, sizeof(cmd));
		manorid=atoi(cmd);
		if ((manorid<1)||(manorid>MANORNUM)) {
			sprintf(buf,"%s","��������ȷ�� id ������ 1~9 ֮�䡣");
			CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
			return;
		}
#ifdef _NEW_MANOR_LAW
		memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));
#endif
		for (i=0; i<char_max; i++) {
			if (CHAR_getCharUse(i)) {
				if (CHAR_getInt(i,CHAR_WHICHTYPE)==CHAR_TYPEMANORSCHEDULEMAN) {
					int fmpks_pos=CHAR_getWorkInt(i, CHAR_NPCWORKINT1)*MAX_SCHEDULE;
					fmpks[fmpks_pos+1].flag=FMPKS_FLAG_MANOR_BATTLEEND;
#ifdef _NEW_MANOR_LAW
					CHAR_setWorkInt(i,CHAR_NPCWORKINT6,tm1.tm_mday);
#endif
					sprintf(buf,"%d peace��", CHAR_getWorkInt(i, CHAR_NPCWORKINT2));
					CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
				}
			}
		}
	}else if (strcmp(cmd,"peace")==0) {
#ifdef _NEW_MANOR_LAW
		struct tm tm1;

		memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));
#endif
		easyGetTokenFromString(message, 2, cmd, sizeof(cmd));
		manorid=atoi(cmd);
		if ((manorid<1)||(manorid>MANORNUM)) {
			sprintf(buf,"%s","��������ȷ�� id ������ 1~9 ֮�䡣");
			CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
			return;
		}
		for (i=0; i<char_max; i++) {
			if (CHAR_getCharUse(i)) {
				if (CHAR_getInt(i,CHAR_WHICHTYPE)==CHAR_TYPEMANORSCHEDULEMAN) {
					if (CHAR_getWorkInt(i, CHAR_NPCWORKINT2)==manorid) {
						int fmpks_pos=CHAR_getWorkInt(i, CHAR_NPCWORKINT1)*MAX_SCHEDULE;
						fmpks[fmpks_pos+1].flag=FMPKS_FLAG_MANOR_BATTLEEND;
#ifdef _NEW_MANOR_LAW
						CHAR_setWorkInt(i,CHAR_NPCWORKINT6,tm1.tm_mday);
#endif
					}
				}
			}
		}
	}else if (strcmp(cmd,"reload")==0) {//type 1 fmdata 2 fmpoint 3 fmpklist
		int type=-1, data=-1;
		char fmlist[3][256]={"fmdata","fmpoint","fmpklist"};
		easyGetTokenFromString(message, 2, cmd, sizeof(cmd));
		for( i=0; i<3; i++){
			if( strstr( cmd, fmlist[i]) == NULL ) continue;
			type = i+1;
		}
		if( type == -1 ) return;
		easyGetTokenFromString(message, 3, cmd, sizeof(cmd));
		data = atoi( cmd);
		saacproto_ACreLoadFmData_send( acfd, type, data);
	}else {
		sprintf(buf,"%s","��������ȷ�� [manorpk <allpeace/peace> id]");
		CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
	}
}

void CHAR_CHAT_DEBUG_shutdown(int charindex , char *message )
{
//function was disabled
/*
	char buf[1];
	print(" GM_shutdown!!!! ");
	strcpy( buf, NULL);
*/
}

void CHAR_CHAT_DEBUG_watchevent( int charaindex, char* message )
{
#ifdef _ADD_NEWEVENT              // WON �����������
	int event_num = 8;
#else
	int event_num = 6;
#endif

	unsigned int max_user = 0;
	int  charaindex_tmp = 0;	
	int  count = 0, point, ckpoint, i, j;
	char token_cdkey[256], token_name[256], cdkey[CDKEYLEN];
	char buf[1024], buf1[64];
	BOOL find = FALSE;
	
	if( strlen(message)==0 ){
		charaindex_tmp = charaindex;
		find=TRUE;
	}else{
		getStringFromIndexWithDelim( message, " ", 1, token_cdkey, sizeof( token_cdkey));
        getStringFromIndexWithDelim( message, " ", 2, token_name, sizeof( token_name));
        if( strlen(token_cdkey)==0 || strlen(token_name)==0 ){
            sprintf(buf,"%s","��������ȷ����ȷ��ʽ��[wahctevent �ʺ� ��������]");
            CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORWHITE);
            return;
		}
		max_user = getFdnum();
		for( i=0; i<max_user; i++){
			char szName[256];
			int i_use;			

			i_use = CONNECT_getUse( i);
			if( i_use){
				CONNECT_getCdkey( i, cdkey, sizeof( cdkey));
				CONNECT_getCharname( i, szName, sizeof( szName));
				if( strcmp( cdkey, token_cdkey)==0 && strcmp( szName, token_name)==0){
					charaindex_tmp = CONNECT_getCharaindex( i);					
					find=TRUE;
				}
			}
		}
	}

	if(find){
		sprintf(buf,"Character Name:%s \n",CHAR_getChar( charaindex_tmp, CHAR_NAME));
		CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORYELLOW);
		
		sprintf(buf,"%s\n","End Event Flag:");
		for(i=0; i<event_num; i++){
			point = CHAR_getInt( charaindex_tmp, CHAR_ENDEVENT + i);			
			for(j=0; j<32; j++){
				ckpoint = point;
				if(ckpoint & (1 << j)){			
					sprintf(buf1,"[%d] ", i*32+j);
					strcat(buf, buf1);
					count++;
				}
				if((count%15)==0 && count!=0){
					strcat(buf, "\n");		
				}
			}		
		}	
		CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);

	    count = 0;
		sprintf(buf,"%s\n","Now Event Flag:");
		for(i=0; i<event_num; i++){
			point = CHAR_getInt( charaindex_tmp, CHAR_NOWEVENT + i);			
			for(j=0; j<32; j++){
				ckpoint = point;
				if(ckpoint & (1 << j)){			
					sprintf(buf1,"[%d] ", i*32+j);
					strcat(buf, buf1);
					count++;
				}
				if((count%15)==0 && count!=0){
					strcat(buf, "\n");		
				}
			}		
		}	
		CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
	}else{
		sprintf(buf,"%s\n","ʧ�ܣ����ʺ������������޷������");
		CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
	}
}

#ifdef _GMRELOAD
void CHAR_CHAT_DEBUG_gmreload( int charaindex, char* message )
{
	char buf[256];
	char id[256];
	char clevel[256];
	char charcdkey[256];
	int level = 0, i = 0, charlevel = 0;
	
	if (message == NULL)	return;
	if (strlen(message) == 0)
	{
		sprintf(buf,"%s","��������ȷ��[gmreload all] �� [gmreload id level(1~3)] level 3:����ʦ,2:�鳤��,1:һ��gm");
		CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORWHITE);
		return;
	}
	snprintf(charcdkey, sizeof(charcdkey), CHAR_getChar(charaindex, CHAR_CDKEY));
	for (i = 0; i < GMMAXNUM; i++)
	{
		if (strcmp(charcdkey, gminfo[i].cdkey) == 0)
		{
			charlevel = gminfo[i].level;
			break;
		}
	}
	easyGetTokenFromString(message, 1, id, sizeof(id));
	if (strcmp(id, "all") == 0)
	{
		if (charlevel != 3)
		{
			CHAR_talkToCli( charaindex, -1, "��û���޸ģǣ͵ȼ���Ȩ�ޣ�",  CHAR_COLORWHITE);
			return;
		}
		if (LoadGMSet(getGMSetfile()))
			CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ�ǣͻ����趨�ϣˣ�",  CHAR_COLORWHITE);
		else 
			CHAR_talkToCli( charaindex, -1, "�ٴζ�ȡ�ǣͻ����趨ʧ�ܣ�",  CHAR_COLORRED);
	}
	else
	{
		int changeflag = 0;
		char tmpbuf[256];
		easyGetTokenFromString(message, 2, clevel, sizeof(clevel));
		level = atoi(clevel);
		if (level > 4 || level < 1 || level > gminfo[i].level)
		{
			CHAR_talkToCli( charaindex, -1, "�޸�ʧ�ܣ��޷������ʺŵȼ��޸ĵı��Լ��ȼ��ߣ�level(1~3)",  CHAR_COLORWHITE);
			return;
		}
		for (i = 0; i < GMMAXNUM; i++)
		{
			if (strcmp(id, gminfo[i].cdkey) == 0)
			{
				if (gminfo[i].level > charlevel)
				{
					snprintf(tmpbuf, sizeof(tmpbuf), "�޸�ʧ�ܣ��޷��޸ĵȼ����Լ��ߵ��ʺţ�");
					CHAR_talkToCli(charaindex, -1, tmpbuf, CHAR_COLORWHITE);
					return;
				}
				gminfo[i].level = level;
				snprintf(tmpbuf, sizeof(tmpbuf), "�޸�%s�ȼ�Ϊ%d�ϣˣ�", gminfo[i].cdkey, gminfo[i].level);
				CHAR_talkToCli(charaindex, -1, tmpbuf, CHAR_COLORWHITE);
				changeflag = 1;
				break;
			}
		}
		if (changeflag == 0)
		{
			snprintf(tmpbuf, sizeof(tmpbuf), "�ǣ��б��в��޴��ʺţ��޸�ʧ�ܣ�");
			CHAR_talkToCli(charaindex, -1, tmpbuf, CHAR_COLORWHITE);
		}
	}
}
#endif

void CHAR_CHAT_DEBUG_checktrade( int charaindex, char*message )
{
#ifdef _ITEM_PILEFORTRADE
	if( !strcmp( message, "waei" ) ){
		TRADE_CheckTradeListUser();
	}
#endif
}

void CHAR_CHAT_DEBUG_showtemp( int charaindex, char*message )
{
#ifdef _NPC_SEPARATEDATA
	if( !CHAR_CHECKINDEX( charaindex) ) return;
	if( !strcmp( message, "waei" ) )
		CHAR_showTempInt( charaindex);
#endif
}

#ifdef _TEST_PETCREATE
int avg_num;
int m_cindex=-1;
typedef struct AllCreatPetMy
{
		char name[128];
		int petId;
		int char_lv;
		int char_hp;
		int char_vital;
		int char_str;
		int char_tgh;
		int char_dex;
		int vital;
		int str;
		int tgh;
		int dex;
		int fixstr;
		int fixtgh;
		int fixdex;
		int creat_num;
}AllCreatPet;
AllCreatPet AVGCREATPET[380]={
	{"",-1,0,0,0,0,0,0,0,0,0,0,0,0,0,-1},
};
void ResetAVGCREATPET()
{
	int i;
	for( i=0; i<arraysizeof( AVGCREATPET); i++ )	{
		AVGCREATPET[i].petId = -1;
		AVGCREATPET[i].creat_num = 0;
		AVGCREATPET[i].char_hp = 0 ;
		AVGCREATPET[i].char_vital = 0;
		AVGCREATPET[i].char_str = 0;
		AVGCREATPET[i].char_tgh = 0;
		AVGCREATPET[i].char_dex = 0;
		AVGCREATPET[i].vital = 0;
		AVGCREATPET[i].str = 0;
		AVGCREATPET[i].tgh = 0;
		AVGCREATPET[i].dex = 0;
	}
}

void CHAR_CHAT_DEBUG_setBattle( int charaindex, char *message )
{
#ifdef _DEATH_CONTENDAB
	char buf1[256];
	int ti,battleindex;

	if( getStringFromIndexWithDelim( message, " " , 1, buf1, sizeof( buf1)) ==FALSE )
		return;
	ti = atoi( buf1);
	if( getStringFromIndexWithDelim( message, " " , 2, buf1, sizeof( buf1)) ==FALSE )
		return;
	battleindex = atoi( buf1);

	ABATTLE_CheckBattlefromFl(charaindex,ti,battleindex);
#endif
}

void CHAR_CHAT_DEBUG_reloadpkteamlist( int charaindex, char *message)
{
#ifdef _DEATH_CONTEND				// (���ɿ�) ANDY ����
	ABATTLE_InitABattle(19);
	CHAR_talkToCli(charaindex, -1,"�ض����������б����", CHAR_COLORWHITE);
#endif
}

void CHAR_CHAT_DEBUG_createpet( int charaindex, char*message )
{
	char buff1[128];
	char buf1[5][128]={"","","","",""};
	int ID,i,Num=1;
	int pet_Id1, pet_Id2;
	int pet_Lv;
	int pet_Num;


	while( getStringFromIndexWithDelim( message, " " , Num, buff1, sizeof( buff1)) !=FALSE ){
		Num++;
		sprintf( buf1[ Num-2], "%s", buff1);
		if( !strcmp( buf1[ Num-2] , "\0") )
			return;
	}
	pet_Id1 = atoi( buf1[0] );
	pet_Id2 = atoi( buf1[1] );
	pet_Lv = atoi( buf1[2] );
	pet_Num = atoi( buf1[3] );
	if( pet_Id1 < 0 || pet_Id2 < 0 || pet_Lv < 0 || pet_Num < 0 ) {
		return;
	}
	if( pet_Num < 0 || pet_Num > 10000 )
		return;
	if( pet_Id1 < 0 || pet_Id2 < 0 || pet_Id1 > pet_Id2 )
		return;
  
	ResetAVGCREATPET();
	i=0;
	for( ID=pet_Id1 ; ID <= pet_Id2; ID++)    {
		AVGCREATPET[i].petId = ID;
		AVGCREATPET[i].char_lv = pet_Lv;
		AVGCREATPET[i].creat_num = pet_Num;
		AVGCREATPET[i+1].petId = ID;
		AVGCREATPET[i+1].char_lv = pet_Lv;
		AVGCREATPET[i+1].creat_num = pet_Num;
		print(" AVGCREATPET[%d].petId[%d]", i, ID);
		i+=2;
		if( i > arraysizeof( AVGCREATPET)-1 )
		break;
	}
	avg_num = pet_Num;

	{
		char buf[256];
		sprintf( buf,"CREAT[%d-%d] LV=%d,num=%d ", pet_Id1, pet_Id2, pet_Lv,pet_Num);
		CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
		m_cindex = charaindex;
	}
	return;
}
void DefPetNum( int flg, int petId)
{
	char buf[128];
	AVGCREATPET[flg].char_hp /= avg_num;
	AVGCREATPET[flg].char_vital /= avg_num;
	AVGCREATPET[flg].char_str /= avg_num;
	AVGCREATPET[flg].char_tgh /= avg_num;
	AVGCREATPET[flg].char_dex /= avg_num;
	AVGCREATPET[flg].vital /= avg_num;
	AVGCREATPET[flg].str /= avg_num;
	AVGCREATPET[flg].tgh /= avg_num;
	AVGCREATPET[flg].dex /= avg_num;
	AVGCREATPET[flg].fixstr /= avg_num;
	AVGCREATPET[flg].fixtgh /= avg_num;
	AVGCREATPET[flg].fixdex /= avg_num;

	AVGCREATPET[flg+1].char_hp /= avg_num;
	AVGCREATPET[flg+1].char_vital /= avg_num;
	AVGCREATPET[flg+1].char_str /= avg_num;
	AVGCREATPET[flg+1].char_tgh /= avg_num;
	AVGCREATPET[flg+1].char_dex /= avg_num;
	AVGCREATPET[flg+1].vital /= avg_num;
	AVGCREATPET[flg+1].str /= avg_num;
	AVGCREATPET[flg+1].tgh /= avg_num;
	AVGCREATPET[flg+1].dex /= avg_num;
	AVGCREATPET[flg+1].fixstr /= avg_num;
	AVGCREATPET[flg+1].fixtgh /= avg_num;
	AVGCREATPET[flg+1].fixdex /= avg_num;

	sprintf( buf,"%s_%d.log" ,"creatpetlog", petId );
	backupTempLogFile( buf, "creatpetlog" , AVGCREATPET[flg].creat_num );

	LogCreatPet(
			AVGCREATPET[flg].name,
			AVGCREATPET[flg].petId ,
			1, AVGCREATPET[flg].char_hp, 
			AVGCREATPET[flg].char_vital, AVGCREATPET[flg].char_str,
			AVGCREATPET[flg].char_tgh, AVGCREATPET[flg].char_dex, 
			AVGCREATPET[flg].vital, AVGCREATPET[flg].str,
			AVGCREATPET[flg].fixstr,AVGCREATPET[flg].fixtgh,
			AVGCREATPET[flg].tgh, AVGCREATPET[flg].dex,
			AVGCREATPET[flg].fixdex,
			avg_num, 0, 1 );
	LogCreatPet(
			AVGCREATPET[flg+1].name,
			AVGCREATPET[flg+1].petId ,
			AVGCREATPET[flg+1].char_lv, AVGCREATPET[flg+1].char_hp, 
			AVGCREATPET[flg+1].char_vital, AVGCREATPET[flg+1].char_str,
			AVGCREATPET[flg+1].char_tgh, AVGCREATPET[flg+1].char_dex, 
			AVGCREATPET[flg+1].vital, AVGCREATPET[flg+1].str,
			AVGCREATPET[flg+1].fixstr,AVGCREATPET[flg+1].fixtgh,
			AVGCREATPET[flg+1].tgh, AVGCREATPET[flg+1].dex,
			AVGCREATPET[flg+1].fixdex,
			avg_num, 0, 1 );

}

void TEST_CreatPet( )
{
	int floor=777;
	char buf[128];
	int petindex;
	int ret,pet_list;
	int k,lvup,i,j;
	int ID=-1;
	int pet_Num=40,struct_num=-1;
	int pet_Lv = -1;

	{
		if( m_cindex >= 0 )	{
		  char buf[256];
		  sprintf( buf,"CREAT Start !!");
		  CHAR_talkToCli( m_cindex, -1, buf, CHAR_COLORYELLOW);
		}
	}

	if( AVGCREATPET[0].creat_num <= 0 && AVGCREATPET[0].petId < 0 )	{
		if( m_cindex >= 0 )
			CHAR_talkToCli( m_cindex, -1, "FAIL !", CHAR_COLORYELLOW);
		m_cindex = -1;
		return;
	}else	{
		if( m_cindex >= 0 )
			CHAR_talkToCli( m_cindex, -1, "OK !", CHAR_COLORYELLOW);
		m_cindex = -1;
	}

	for( i=arraysizeof( AVGCREATPET)-2; i>=0; i-=2 )	{	//creat_num
		if( AVGCREATPET[i].petId >= 0 )	{
			if( AVGCREATPET[i].creat_num <= 0 )	{
				AVGCREATPET[  i].petId = -1;
				AVGCREATPET[i+1].petId = -1;
			}else	{
				ID = AVGCREATPET[i].petId;
				struct_num = i;
				pet_Lv = AVGCREATPET[i].char_lv;
				break;
			}
		}
	}
	if( ID < 0 && pet_Lv <= 0 && struct_num <= 0 )
		return;

	pet_list = ENEMY_getEnemyArrayFromId( ID);
	if( !ENEMY_CHECKINDEX( pet_list) ){
		AVGCREATPET[  struct_num].petId = -1;
		AVGCREATPET[struct_num+1].petId = -1;
		AVGCREATPET[struct_num].creat_num = 0;
		AVGCREATPET[struct_num+1].creat_num = 0;
		return;
	}

	//creat����
	for( j=0; j < pet_Num; j++ )     {
		if( AVGCREATPET[struct_num].creat_num <= 0 )	{
			DefPetNum( struct_num, AVGCREATPET[struct_num].petId);
			AVGCREATPET[struct_num].petId = -1;
			AVGCREATPET[struct_num+1].petId = -1;
			AVGCREATPET[struct_num].creat_num = 0;
			AVGCREATPET[struct_num+1].creat_num = 0;
			//����
			return;
		}
		ret = ENEMY_TEST_createPetIndex( pet_list );
		//ret = ENEMY_createPetFromEnemyIndex( charaindex, pet_list);
		if( CHAR_CHECKINDEX( ret ) == TRUE ){
			CHAR_setMaxExpFromLevel( ret, pet_Lv);
		}
		petindex = ret;
		//log
		CHAR_complianceParameter( petindex );
		lvup = CHAR_getInt( petindex, CHAR_ALLOCPOINT);
		LogCreatPet(	CHAR_getChar( petindex, CHAR_NAME ), CHAR_getInt( petindex, CHAR_PETID ),
					CHAR_getInt( petindex, CHAR_LV ), 
					CHAR_getInt( petindex, CHAR_HP ), 
					CHAR_getInt( petindex, CHAR_VITAL), CHAR_getInt( petindex, CHAR_STR),
					CHAR_getInt( petindex, CHAR_TOUGH), CHAR_getInt( petindex, CHAR_DEX), 
					(lvup >> 24 ), ((lvup >> 16)&0xff), ((lvup >> 8)&0xff), (lvup&0xff),
					CHAR_getWorkInt(petindex,CHAR_WORKFIXSTR),
					CHAR_getWorkInt(petindex,CHAR_WORKFIXTOUGH),
					CHAR_getWorkInt(petindex,CHAR_WORKFIXDEX),
					lvup, CHAR_getInt( petindex, CHAR_PETRANK ) , 0 );

		sprintf( buf,"%s", CHAR_getChar( petindex, CHAR_NAME ));
		memcpy( AVGCREATPET[struct_num].name, buf, sizeof( AVGCREATPET[struct_num].name)  );
		AVGCREATPET[struct_num].creat_num -=1;
		//��¼
		AVGCREATPET[struct_num].char_hp += CHAR_getInt( petindex, CHAR_HP );
		AVGCREATPET[struct_num].char_vital += CHAR_getInt( petindex, CHAR_VITAL);
		AVGCREATPET[struct_num].char_str += CHAR_getInt( petindex, CHAR_STR);
		AVGCREATPET[struct_num].char_tgh += CHAR_getInt( petindex, CHAR_TOUGH);
		AVGCREATPET[struct_num].char_dex += CHAR_getInt( petindex, CHAR_DEX);

		AVGCREATPET[struct_num].vital += (lvup >> 24 );
		AVGCREATPET[struct_num].str += ((lvup >> 16)&0xff);
		AVGCREATPET[struct_num].tgh += ((lvup >> 8)&0xff);
		AVGCREATPET[struct_num].dex += (lvup&0xff);

		AVGCREATPET[struct_num].fixstr = CHAR_getWorkInt(petindex,CHAR_WORKFIXSTR);
		AVGCREATPET[struct_num].fixtgh = CHAR_getWorkInt(petindex,CHAR_WORKFIXTOUGH);
		AVGCREATPET[struct_num].fixdex = CHAR_getWorkInt(petindex,CHAR_WORKFIXDEX);

		CHAR_PetTakeLevelUp( petindex, pet_Lv);//ǿ�Ƴ�������
		CHAR_setInt( petindex , CHAR_HP , CHAR_getWorkInt( petindex , CHAR_WORKMAXHP ) );

		//log
		lvup = CHAR_getInt( petindex, CHAR_ALLOCPOINT);
			LogCreatPet(	CHAR_getChar( petindex, CHAR_NAME ), CHAR_getInt( petindex, CHAR_PETID ),
					CHAR_getInt( petindex, CHAR_LV ),
					CHAR_getInt( petindex, CHAR_HP ), 
					CHAR_getInt( petindex, CHAR_VITAL), CHAR_getInt( petindex, CHAR_STR),
					CHAR_getInt( petindex, CHAR_TOUGH), CHAR_getInt( petindex, CHAR_DEX), 
					(lvup >> 24 ), ((lvup >> 16)&0xff), ((lvup >> 8)&0xff), (lvup&0xff),
					CHAR_getWorkInt(petindex,CHAR_WORKFIXSTR),
					CHAR_getWorkInt(petindex,CHAR_WORKFIXTOUGH),
					CHAR_getWorkInt(petindex,CHAR_WORKFIXDEX),
					lvup, CHAR_getInt( petindex, CHAR_PETRANK ) , 0);
		
		sprintf( buf,"%s", CHAR_getChar( petindex, CHAR_NAME ));
		memcpy( AVGCREATPET[struct_num+1].name, buf, sizeof( AVGCREATPET[struct_num+1].name)  );
		AVGCREATPET[struct_num+1].creat_num -=1;
		AVGCREATPET[struct_num + 1].char_hp += CHAR_getInt( petindex, CHAR_HP );
		AVGCREATPET[struct_num + 1].char_vital += CHAR_getInt( petindex, CHAR_VITAL);
		AVGCREATPET[struct_num + 1].char_str += CHAR_getInt( petindex, CHAR_STR);
		AVGCREATPET[struct_num + 1].char_tgh += CHAR_getInt( petindex, CHAR_TOUGH);
		AVGCREATPET[struct_num + 1].char_dex += CHAR_getInt( petindex, CHAR_DEX);

		AVGCREATPET[struct_num + 1].vital += (lvup >> 24 );
		AVGCREATPET[struct_num + 1].str += ((lvup >> 16)&0xff);
		AVGCREATPET[struct_num + 1].tgh += ((lvup >> 8)&0xff);
		AVGCREATPET[struct_num + 1].dex += (lvup&0xff);

		AVGCREATPET[struct_num + 1].fixstr = CHAR_getWorkInt(petindex,CHAR_WORKFIXSTR);
		AVGCREATPET[struct_num + 1].fixtgh = CHAR_getWorkInt(petindex,CHAR_WORKFIXTOUGH);
		AVGCREATPET[struct_num + 1].fixdex = CHAR_getWorkInt(petindex,CHAR_WORKFIXDEX);

		CHAR_PetTakeDrop( petindex, floor, -1, -1);	//ǿ�ƶ�������
		CHAR_setInt( petindex, CHAR_PUTPETTIME, NowTime.tv_sec);
	}

	sprintf( buf,"%s_%d.log" ,"creatpetlog", AVGCREATPET[struct_num].petId );
	backupTempLogFile( buf, "creatpetlog" , AVGCREATPET[struct_num].creat_num);

	for( k=20000; k>=0; k-- )	{
		if( CHAR_CHECKINDEX( k ) == FALSE )
			continue;
		if( CHAR_getInt ( k, CHAR_WHICHTYPE) != CHAR_TYPEPET	)
			continue;
		if( CHAR_getInt( k, CHAR_FLOOR ) == 777 )	{
			CHAR_CharaDelete( k);
			ITEM_endExistItemsOne( k);
		}
	}
}
#endif
void CHAR_CHAT_DEBUG_cleanfloor( int charaindex, char *message)
{
	char buf[256];
	int floor, objindex, itemnum=0,petnum=0,goldnum=0;
	int objmaxnum = OBJECT_getNum();

	floor = atoi( message);
	for( objindex=0; objindex<objmaxnum; objindex++)	{
		if( CHECKOBJECT( objindex ) == FALSE ) continue;
		if( OBJECT_getType( objindex) == OBJTYPE_CHARA ){
			int petindex;
			petindex = OBJECT_getIndex( objindex);
			if( CHAR_getInt( petindex, CHAR_WHICHTYPE) != CHAR_TYPEPET) continue;
			if( CHAR_getInt( petindex, CHAR_MAILMODE) != CHAR_PETMAIL_NONE) continue;
			if( CHAR_getWorkInt( petindex, CHAR_WORKPETFOLLOWMODE) == CHAR_PETFOLLOW_NOW ) continue;
			CHAR_CharaDelete( petindex);
			petnum++;
		}else if( OBJECT_getType( objindex) == OBJTYPE_ITEM ){
			int itemindex = OBJECT_getIndex(objindex);
			if( !ITEM_CHECKINDEX( itemindex) ) continue;
			ITEM_endExistItemsOne( itemindex);
			CHAR_ObjectDelete( objindex);
			itemnum++;
		}else if( OBJECT_getType( objindex) == OBJTYPE_GOLD ){
			CHAR_ObjectDelete( objindex);
			goldnum++;
		}
	}
	sprintf( buf, "��� ����:%dֻ������:%d����ʯ��:%d����", petnum, itemnum, goldnum);
	CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
	return;
}


void CHAR_CHAT_DEBUG_fixfmpk(int charaindex, char *message)
{
	int meindex, fmpks_pos;
	int ID = atoi( message);
#ifdef _NEW_MANOR_LAW
	struct tm tm1;
#endif

	meindex = NPC_getManorsmanListIndex( ID );

	if( !CHAR_CHECKINDEX( meindex ))return;
	fmpks_pos = CHAR_getWorkInt( meindex, CHAR_NPCWORKINT1)*MAX_SCHEDULE;

	fmpks[fmpks_pos].flag = FMPKS_FLAG_NONE;
    fmpks[fmpks_pos+1].dueltime = CHAR_getWorkInt(meindex, CHAR_NPCWORKINT4) + NowTime.tv_sec;

	fmpks[fmpks_pos+1].flag = FMPKS_FLAG_MANOR_PEACE_SAVE;
#ifndef _NEW_MANOR_LAW
	NPC_ManorSavePKSchedule( meindex, -1, FMPKS_FLAG_MANOR_PEACE_SAVE);
#else
	NPC_ManorSavePKSchedule( meindex, -1, FMPKS_FLAG_MANOR_PEACE_SAVE,fmpks[fmpks_pos+1].dueltime,tm1);
#endif
}

#ifdef _GAMBLE_BANK
void CHAR_CHAT_DEBUG_setgamblenum( int charaindex, char *message )
{
	int set_num=0;
	char buf[256];
	if( !strcmp( message, "\0") )
		return;
	set_num = atoi( message);
	if( set_num < 0 )	set_num = 0;
	if( set_num > 10000) set_num = 10000;
	CHAR_setInt( charaindex, CHAR_GAMBLENUM, set_num);

	sprintf( buf,"�������ֳ�������Ϊ%d�㡣", CHAR_getInt( charaindex, CHAR_GAMBLENUM));
	CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
	return;
}

#endif


void CHAR_CHAT_DEBUG_petlevelup( int charaindex, char* message )
{
	int level, petindex, mylevel,i,j,k;
	if( (level = atoi( message)) > 140 ) return;

	for( i=0; i<5; i++ )	{
		petindex = CHAR_getCharPet( charaindex, i);
		if( !CHAR_CHECKINDEX( petindex) )return;
		mylevel = CHAR_getInt( petindex, CHAR_LV);
		level = level-mylevel;
		for( k = 1; k < level; k ++ ){	//����
			CHAR_PetLevelUp( petindex );
			CHAR_PetAddVariableAi( petindex, AI_FIX_PETLEVELUP );
			CHAR_setInt( petindex, CHAR_LV, CHAR_getInt( petindex, CHAR_LV) +1 );
		}
		CHAR_complianceParameter( petindex );
		CHAR_setInt( petindex , CHAR_HP , CHAR_getWorkInt( petindex , CHAR_WORKMAXHP ) );
	}

	{
		char msgbuf[256];
		for(j = 0; j < CHAR_MAXPETHAVE; j++){
			petindex = CHAR_getCharPet(charaindex, j);
			if( !CHAR_CHECKINDEX( petindex) )
				continue;
			CHAR_complianceParameter( petindex );
			snprintf( msgbuf, sizeof( msgbuf ), "K%d", j );
			CHAR_sendStatusString( charaindex, msgbuf );
			snprintf( msgbuf, sizeof( msgbuf ), "W%d", j );
			CHAR_sendStatusString( charaindex, msgbuf );
		}
	}

}


void CHAR_CHAT_DEBUG_engineer( int charindex, char *message)
{
	int MaxGold;
	if( !CHAR_CHECKINDEX( charindex) )
		return;
	if( strstr( message, "waei") == NULL )
		return;
#ifdef _TRANS_6
	if( CHAR_getInt( charindex, CHAR_TRANSMIGRATION) >= 6 )
#else
	if( CHAR_getInt( charindex, CHAR_TRANSMIGRATION) >= 5 )
#endif
		return;
	MaxGold = CHAR_getMaxHaveGold( charindex);
	CHAR_setInt( charindex, CHAR_LV, 140);
	CHAR_setInt( charindex, CHAR_TRANSMIGRATION, 5);
	CHAR_setInt( charindex, CHAR_SKILLUPPOINT, 600);
	CHAR_setInt( charindex, CHAR_LEARNRIDE, 150);
	CHAR_setInt( charindex, CHAR_GOLD, MaxGold);

	CHAR_talkToCli( charindex, -1, message, CHAR_COLORYELLOW);
	{
		int petTemp[]={1610,-1,-1,-1,-1};
		int add_pet[]={-1,-1,-1,-1,-1};
		int petNum=0;
		int i,j;
		int k=0, petindex,enemyarray;
		j=1;
		for( i=0; i< arraysizeof(ridePetTable) ; i++ ){
			if( CHAR_getInt( charindex, CHAR_BASEBASEIMAGENUMBER) == ridePetTable[i].charNo ){
				petTemp[j]= ridePetTable[i].petId;
				j++;
				if( j >= arraysizeof( petTemp) )
					break;
			}
		}
		j=0;
		for( petNum=0; petNum<arraysizeof( petTemp); petNum++)	{
			enemyarray = ENEMY_getEnemyArrayFromId( petTemp[ petNum]); //�׻�
			petindex = ENEMY_createPetFromEnemyIndex( charindex, enemyarray);
			if( petindex == -1 ) {
				continue;
			}
			add_pet[j]=petindex;
			j++;
			for( k = 1; k < 120; k ++ ){	//����
				CHAR_PetLevelUp( petindex );
				CHAR_PetAddVariableAi( petindex, AI_FIX_PETLEVELUP );
				CHAR_setInt( petindex, CHAR_LV, CHAR_getInt( petindex, CHAR_LV) +1 );
			}
			CHAR_complianceParameter( petindex );
			CHAR_setInt( petindex , CHAR_HP , CHAR_getWorkInt( petindex , CHAR_WORKMAXHP ) );
		}
		//for( k=0;k<arraysizeof( add_pet); k++)	{
		//	CHAR_endCharOneArray( add_pet[k]);
		//}
	}
	CHAR_send_P_StatusString(  charindex, CHAR_P_STRING_DUELPOINT|
		CHAR_P_STRING_TRANSMIGRATION| CHAR_P_STRING_RIDEPET|
		CHAR_P_STRING_BASEBASEIMAGENUMBER| CHAR_P_STRING_GOLD|
		CHAR_P_STRING_EXP| CHAR_P_STRING_LV| CHAR_P_STRING_HP|CHAR_P_STRING_LEARNRIDE);
	CHAR_Skillupsend( charindex);
	{
		int j, petindex;
		char msgbuf[256];
		for(j = 0; j < CHAR_MAXPETHAVE; j++){
			petindex = CHAR_getCharPet(charindex, j);
			if( !CHAR_CHECKINDEX( petindex) )
				continue;
			CHAR_complianceParameter( petindex );
			snprintf( msgbuf, sizeof( msgbuf ), "K%d", j );
			CHAR_sendStatusString( charindex, msgbuf );
			snprintf( msgbuf, sizeof( msgbuf ), "W%d", j );
			CHAR_sendStatusString( charindex, msgbuf );
		}
	}
}
#ifdef _WAEI_KICK
void CHAR_CHAT_DEBUG_gmkick( int charindex, char* message)
{
	char cmd[256];
	char buf1[256];
	char gmset[][256]={"LSLOCK","KICK","DEUNLOCK","UNLOCKALL","LOCK","TYPE", "UNLOCK"};
	int act=1;

	if( getStringFromIndexWithDelim( message, " ", 1, buf1, sizeof( buf1)) == FALSE )	{
		CHAR_talkToCli(charindex, -1, "��������ȷ", CHAR_COLORYELLOW);
		return;
	}
	sprintf( cmd, "%s", buf1);
	if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof( buf1)))	{
		int i;
		for( i=0;i<arraysizeof( gmset); i++)	{
			if( !strcmp( buf1, gmset[i]) )	{
				break;
			}
		}
		if( i >= arraysizeof( gmset) )	{
			CHAR_talkToCli(charindex, -1, "��������ȷ", CHAR_COLORYELLOW);
			return;
		}
		act = i;
		// WON FIX 5 --> 6
		if( act < 0 || act > 6 ){
			CHAR_talkToCli(charindex, -1, "��������ȷ", CHAR_COLORYELLOW);
			return;
		}
	}
	saacproto_ACKick_send( acfd, cmd, getFdidFromCharaIndex( charindex), act);
}
#endif

void CHAR_CHAT_DEBUG_remserver( int charaindex, char *message )
{
#ifdef _M_SERVER
	if( !CHAR_CHECKINDEX( charaindex) )
		return;
	if( strstr( message, "on") != NULL )	{
		if (mfd < 0){
			mfd = connectmServer(getmservername(),getmserverport());
			if (mfd != -1){
				initConnectOne(mfd,NULL,0);
				CHAR_talkToCli(charaindex, -1, "Connect OK", CHAR_COLORYELLOW);
			}else{
				CHAR_talkToCli(charaindex, -1, "Connect FAIL", CHAR_COLORYELLOW);
			}
		}else	{
			CHAR_talkToCli(charaindex, -1, "Connect Be LINK", CHAR_COLORYELLOW);
		}
	}else if( strstr( message, "off") != NULL ){
		if (mfd >= 0){
 			CONNECT_endOne_debug(mfd);
			close(mfd);
			mfd = -1;
			CHAR_talkToCli(charaindex, -1, "Connect Be CLOSE", CHAR_COLORYELLOW);
		}else	{
			CHAR_talkToCli(charaindex, -1, "Connect Not Be LINK", CHAR_COLORYELLOW);
		}
	}
#endif
}
//ɥ־
void CHAR_CHAT_DEBUG_reloadmsip( int charaindex, char *message)
{
#ifdef _M_SERVER
	char buf1[256];
	memset( buf1, 0, sizeof( buf1));
	if( getStringFromIndexWithDelim( message, " ", 1, buf1, sizeof( buf1)) == FALSE )
		return;
	setmservername( buf1);
	if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof( buf1)) != FALSE ){
		setmserverport( atoi( buf1));
	}
	memset( buf1, 0, sizeof( buf1));
	sprintf( buf1, "MServer IP:%s PORT:%d", getmservername(), getmserverport() );
	CHAR_talkToCli( charaindex, -1, buf1, CHAR_COLORYELLOW);
#endif
}

// WON ADD ����ָ��
void CHAR_CHAT_DEBUG_crash(int charaindex , char *message)
{
	int fd;


	fd = CHAR_getWorkInt( charaindex, CHAR_WORKFD);	

	CHAR_Talk(  fd, charaindex, message, 1, 3 );

/*
	char msg[1];	
	printf("\n carsh GMSV !! \n");
	//sprintf( msg, "%s", CHAR_getChar( charaindex, CHAR_NAME ) );	
	sprintf( msg, "12345" );
*/
}


#ifdef _PETSKILL_SETDUCK
void CHAR_CHAT_DEBUG_SetDuck( int charaindex, char *message)
{
	CHAR_setWorkInt( charaindex, CHAR_MYSKILLDUCK, atoi( message));
	CHAR_setWorkInt( charaindex, CHAR_MYSKILLDUCKPOWER, 100 );
}
#endif

#ifdef _TYPE_TOXICATION
void CHAR_CHAT_DEBUG_Toxication( int charaindex, char *message)
{
	int fd = getfdFromCharaIndex( charaindex );
	if( strstr( message, "TRUE") != 0){
		CHAR_talkToCli( charaindex, -1, "�ж�", CHAR_COLORYELLOW);
		setToxication( fd, 1);
	}else	{
		setToxication( fd, 0);
	}

}
#endif

#ifdef _ACTION_BULLSCR
void CHAR_CHAT_DEBUG_getascore(int charindex, char *message)
{
	int witchi[5] = { -1,CHAR_ABULLSTART, CHAR_ABULLSCORE, CHAR_ABULLTIME, CHAR_ABULLSTARTTIME};
	char witchn[5][256]={"ȫ","�������","�����","ս���ۻ�ʱ��","��ʼʱ��"};
	char witchs[5][256]={"all","start","score","btime","stime"};
	int i;
	for( i=0; i<5; i++)	{
		if( strstr( message, witchs[i]) != 0 ){
			char buf[256];
			if( i == 0 ){
				sprintf( buf, "%s:%d %s:%d %s:%d %s:%d��",
					witchn[1], CHAR_getInt( charindex, witchi[1]),
					witchn[2], CHAR_getInt( charindex, witchi[2]),
					witchn[3], CHAR_getInt( charindex, witchi[3]),
					witchn[4], CHAR_getInt( charindex, witchi[4]));
			}else{
				if( i == 2 ){
					int batime = CHAR_getInt( charindex, witchi[i]);
					sprintf( buf, "%s : %d��%d�롣", witchn[i], batime/60, batime%60 );
				}else {
					sprintf( buf, "%s : %d��", witchn[i], CHAR_getInt( charindex, witchi[i]) );
				}
			}
			CHAR_talkToCli( charindex, -1, buf, CHAR_COLORYELLOW);
			break;
		}
	}
}

void CHAR_CHAT_DEBUG_setascore(int charindex, char *message)
{
	int witchi[3] = {CHAR_ABULLSTART,CHAR_ABULLSCORE,CHAR_ABULLSTARTTIME};
	char witchs[3][256]={"start","score","stime"};
	char buf1[256];
	memset( buf1, 0, sizeof( buf1));
	if( getStringFromIndexWithDelim( message, " ", 1, buf1, sizeof( buf1)) == FALSE )
		return;
	if( strstr( buf1, "clean") != 0 ){
	}else {
		int i;
		for( i=0; i<3; i++)	{
			if( strstr( buf1, witchs[i]) != 0 ){
				char buf2[256];
				int nums=0;
				char witchn[3][256]={"�������","�����","��ʼʱ��"};
				if( getStringFromIndexWithDelim( message, " ", 2, buf2, sizeof( buf2)) == FALSE )
					return;
				nums = atoi( buf2);
				if( i == 2 )	{
					sprintf( buf2, "%s�趨Ϊ��ǰ%d��", witchn[i], nums);
					nums = (nums * 24) * (60 * 60);
					nums = (int)time( NULL) - nums;
				}else{
					sprintf( buf2, "%s�趨Ϊ%d��", witchn[i], nums);
				}
				CHAR_setInt( charindex, witchi[i], nums);
				CHAR_talkToCli( charindex, -1, buf2, CHAR_COLORYELLOW);
				return;
			}
		}
	}
}
#endif


#ifdef _CHAR_POOLITEM
void CHAR_CHAT_DEBUG_saveditem(int charaindex, char *message)
{
	if( CHAR_SaveDepotItem( charaindex) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "����ʧ�ܣ�", CHAR_COLORYELLOW);
	}
}

void CHAR_CHAT_DEBUG_insertditem(int charaindex, char *message)
{
	if( CHAR_GetDepotItem( -1, charaindex) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "��ȡʧ�ܣ�", CHAR_COLORYELLOW);
	}
}

void CHAR_CHAT_DEBUG_ShowMyDepotItems( int charaindex, char *message )
{
	if( !CHAR_CheckDepotItem( charaindex) ){
		char token[256];
		sprintf( token, "��δȡ�òֿ⡣");
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
    	return;
	}
	CHAR_ShowMyDepotItems( charaindex);
}

void CHAR_CHAT_DEBUG_InSideMyDepotItems( int charaindex, char *message )
{
	int i, j, itemindex, count=0;
	char token[256];

	if( !CHAR_CheckDepotItem( charaindex) ){
		sprintf( token, "��δȡ�òֿ⡣");
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
    	return;
	}

	for( i=CHAR_STARTITEMARRAY; i<CHAR_MAXITEMHAVE; i++ ){
		itemindex = CHAR_getItemIndex( charaindex, i );
		if( !ITEM_CHECKINDEX( itemindex)) continue;
		for( j=0; j<CHAR_MAXDEPOTITEMHAVE; j++){
			if( ITEM_CHECKINDEX( CHAR_getDepotItemIndex( charaindex, j) ) ) continue;
			CHAR_setItemIndex( charaindex, i, -1);
			CHAR_sendItemDataOne( charaindex, i);
			CHAR_setDepotItemIndex( charaindex, j, itemindex);
			count++;
			break;
		}
	}
	sprintf( token, "ת��%d�����ߡ�", count);
	CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
}
#endif


#ifdef _CHAR_POOLPET
void CHAR_CHAT_DEBUG_savedpet(int charaindex, char *message)
{
	if( CHAR_SaveDepotPet( charaindex) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "����ʧ�ܣ�", CHAR_COLORYELLOW);
	}
}

void CHAR_CHAT_DEBUG_insertdpet(int charaindex, char *message)
{
	if( CHAR_GetDepotPet( -1, charaindex) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "��ȡʧ�ܣ�", CHAR_COLORYELLOW);
	}
}

void CHAR_CHAT_DEBUG_ShowMyDepotPets( int charaindex, char *message )
{
	if( !CHAR_CheckDepotPet( charaindex) ){
		char token[256];
		sprintf( token, "��δȡ�òֿ⡣");
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
    	return;
	}
	CHAR_ShowMyDepotPets( charaindex);
}

void CHAR_CHAT_DEBUG_InSideMyDepotPets( int charaindex, char *message )
{
	int i, j, petindex, count=0;
	char token[256];

	if( !CHAR_CheckDepotPet( charaindex) ){
		sprintf( token, "��δȡ�òֿ⡣");
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
    	return;
	}

	for( i=0; i<CHAR_MAXPETHAVE; i++ ){
		petindex = CHAR_getCharPet( charaindex, i );
		if( !CHAR_CHECKINDEX( petindex)) continue;
		for( j=0; j<CHAR_MAXDEPOTPETHAVE; j++){
			if( CHAR_CHECKINDEX( CHAR_getDepotPetIndex( charaindex, j) ) ) continue;
			CHAR_setCharPet( charaindex, i, -1);
			
			snprintf( token, sizeof( token ), "K%d", i );
			CHAR_sendStatusString( charaindex, token );
			snprintf( token, sizeof( token ), "W%d", i );
			CHAR_sendStatusString( charaindex, token );
			
			CHAR_setDepotPetIndex( charaindex, j, petindex);
			count++;
			break;
		}
	}
	sprintf( token, "ת��%d�����", count);
	CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
}
#endif



void CHAR_CHAT_DEBUG_showMem(int charaindex, char *message)
{
	char bufarg[256];
	showMem( bufarg);
	CHAR_talkToCli( charaindex, -1, bufarg, CHAR_COLORRED);
}

void CHAR_CHAT_DEBUG_cleanfreepet(int charaindex, char *message)
{	
	int objindex, delobjnum=0;
	int objmaxnum = OBJECT_getNum();

	for( objindex=0; objindex<objmaxnum; objindex++)	{
		int nums;
		if( CHECKOBJECT( objindex ) == FALSE ) continue;
		nums = PET_CleanPetdeletetime( objindex);
		delobjnum += nums;
	}
	if( delobjnum > 0 ){
		char buf[256];
		sprintf( buf, "���%d  �ó��", delobjnum);
		CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORRED);
	}
}

#ifdef _NEW_PLAYERGOLD
void CHAR_CHAT_DEBUG_acnewplayer(int charaindex, char *message)
{	
	char CdKey[256], UserName[256], buf[256];
	int RunType;

	memset( buf, 0, sizeof( buf));
	if( getStringFromIndexWithDelim( message, " ", 1, buf, sizeof( buf)) == FALSE )
		return;
	buf[ strlen( buf)+1]=0;
	memcpy( CdKey, buf, strlen( buf)+1);
	if( getStringFromIndexWithDelim( message, " ", 2, buf, sizeof( buf)) == FALSE )
		return;
	buf[ strlen( buf)+1]=0;
	memcpy( UserName, buf, strlen( buf)+1);
	if( getStringFromIndexWithDelim( message, " ", 3, buf, sizeof( buf)) == FALSE )
		return;
	buf[ strlen( buf)+1]=0;
	RunType = atoi( buf);
	if( RunType < 1 || RunType > 2 ){
		CHAR_talkToCli( charaindex, -1, "��������", CHAR_COLORRED);
		return;
	}
	saacproto_ACNEWPlayerList_send( acfd, CdKey, UserName, charaindex, RunType);
}

void CHAR_CHAT_DEBUG_loadnewplayer( int charaindex, char*message )
{
	saacproto_LoadNEWPlayer_send( acfd, charaindex, message);
}
#endif

#ifdef _SEND_EFFECT	   	        // WON ADD AC����ѩ���������Ч
void CHAR_CHAT_DEBUG_sendeffect( int charaindex, char *message)
{
	int i, j, effect, level;
	char buf[256];
	struct tm tm1;
	int playernum = CHAR_getPlayerMaxNum();
	int		player;
	int		efnum = 0;

	struct {
		int floor;
		int effect;
		BOOL on;
		BOOL off;
	} ef[2048];


	memcpy( &tm1, localtime( (time_t *)&NowTime.tv_sec), sizeof( tm1));

	memset( buf, 0, sizeof( buf));

	if( getStringFromIndexWithDelim( message, " ", 1, buf, sizeof( buf)) == FALSE ) return;
	effect = atoi( buf );			
	
	if( getStringFromIndexWithDelim( message, " ", 2, buf, sizeof( buf)) == FALSE ) return;
	level =	atoi( buf );

	for( i=0; i<=CHAR_effectnum; i++){
		if(CHAR_effect[i].floor){
			CHAR_effect[i].effect = effect;
			CHAR_effect[i].level = level;
			CHAR_effect[i].sendflg = FALSE;
			snprintf( CHAR_effect[i].month  , sizeof( CHAR_effect[i].month) , "%d" , tm1.tm_mon+1);
			snprintf( CHAR_effect[i].day    , sizeof( CHAR_effect[i].day)   , "%d" , tm1.tm_mday);
			snprintf( CHAR_effect[i].hour   , sizeof( CHAR_effect[i].hour)  , "%d" , tm1.tm_hour);
			snprintf( CHAR_effect[i].min    , sizeof( CHAR_effect[i].min)   , "%d" , tm1.tm_min);
			snprintf( CHAR_effect[i].expire , sizeof( CHAR_effect[i].expire), "300000" );
		}// if
	}// for i


	memset( &ef, 0, sizeof( ef));

	for( i = 0; i < CHAR_effectnum; i ++){
		if( !CHAR_effect[i].sendflg) {
			for( player = 0 ; player < playernum; player ++ ) {
				if( !CHAR_CHECKINDEX( player ) ) continue;
				if( CHAR_getInt( player, CHAR_FLOOR) == CHAR_effect[i].floor ) {
					int fd = getfdFromCharaIndex( player);
					lssproto_EF_send( fd, CHAR_effect[i].effect, CHAR_effect[i].level, "");
					if( CHAR_effect[i].level == 0 ) {
						CHAR_setWorkInt( player, CHAR_WORKEFFECT, 
										CHAR_getWorkInt( player, CHAR_WORKEFFECT) &
											~CHAR_effect[i].effect);
					}else {					
						CHAR_setWorkInt( player, CHAR_WORKEFFECT, 
										CHAR_getWorkInt( player, CHAR_WORKEFFECT) | 
											CHAR_effect[i].effect);
					}
				}
			}
			CHAR_effect[i].sendflg = TRUE;
		}

		for( j = 0; j < efnum; j ++ ) {
			if( ef[j].floor == CHAR_effect[i].floor && ef[j].effect == CHAR_effect[i].effect) 
			{
				ef[j].on = TRUE;
				break;
			}
		}
		if( j == efnum ) {
			ef[j].floor = CHAR_effect[i].floor;
			ef[j].effect = CHAR_effect[i].effect;
			ef[j].on = TRUE;
			efnum++;
			if( efnum >= arraysizeof( ef)) {
				print( "err buffer over %s:%d\n", __FILE__, __LINE__);
				efnum --;
			} 
		} 
	}   

	for( i = 0; i < efnum; i ++ ) {
		if( ef[i].on == FALSE && ef[i].off == TRUE ) {
			for( player = 0 ; player < playernum; player ++ ) {
				if( !CHAR_CHECKINDEX( player ) ) continue;
				if( CHAR_getInt( player, CHAR_FLOOR) == ef[i].floor ) {
					int fd = getfdFromCharaIndex( player);
					lssproto_EF_send( fd, 0, 0, "");
					CHAR_setWorkInt( player, CHAR_WORKEFFECT, 0);
				}
			}
		}
	}
}
#endif

void CHAR_CHAT_DEBUG_checktime( int charaindex, char *message)
{
	time_t newsec;
	char buf[256];
	char WKday[7][32]={"��","һ","��","��","��","��","��"};
	struct tm *p;


	newsec = time( NULL);
	p = localtime( &newsec);
//	newsec = localtime( &newsec);
	sprintf( buf, "���죺%d��%d��%d�� %s %d:%d:%d ...newsec:%ld��",
		(1900+p->tm_year), (1+p->tm_mon), p->tm_mday, WKday[p->tm_wday],
		p->tm_hour, p->tm_min, p->tm_sec, newsec );
	CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORRED);
	//p = (struct tm *)calloc( 1, sizeof( struct tm) );
	//if( p == NULL ) return;
	if( getStringFromIndexWithDelim( message, "/", 1, buf, sizeof( buf)) == FALSE ) return;
	print("ANDY 1.buf:%s\n", buf);
	p->tm_year = atoi( buf)-1900;
	if( getStringFromIndexWithDelim( message, "/", 2, buf, sizeof( buf)) == FALSE ) return;
	print("ANDY 2.buf:%s\n", buf);
	p->tm_mon = atoi( buf)-1;
	if( getStringFromIndexWithDelim( message, "/", 3, buf, sizeof( buf)) == FALSE ) return;
	print("ANDY 3.buf:%s\n", buf);
	p->tm_mday = atoi( buf);
	if( getStringFromIndexWithDelim( message, "/", 4, buf, sizeof( buf)) == FALSE ) return;
	p->tm_wday = atoi( buf);
	if( p->tm_wday < 0 || p->tm_wday >= 7 ) p->tm_wday = 0;
	if( getStringFromIndexWithDelim( message, "/", 5, buf, sizeof( buf)) == FALSE ) return;
	p->tm_hour = atoi( buf);
	if( getStringFromIndexWithDelim( message, "/", 6, buf, sizeof( buf)) == FALSE ) return;
	p->tm_min = atoi( buf);
	if( getStringFromIndexWithDelim( message, "/", 7, buf, sizeof( buf)) == FALSE ) return;
	p->tm_sec =  atoi( buf);

	newsec = mktime( p);
	sprintf( buf, "ȷ��1��%d��%d��%d�� %s %d:%d:%d ...newsec:%ld��",
		(1900+p->tm_year), (1+p->tm_mon), p->tm_mday, WKday[p->tm_wday],
		p->tm_hour, p->tm_min, p->tm_sec, newsec );
	CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORRED);

	p = localtime( &newsec);
	sprintf( buf, "ȷ��2��%d��%d��%d�� %s %d:%d:%d ...newsec:%ld..time:%d��",
		(1900+p->tm_year), (1+p->tm_mon), p->tm_mday, WKday[p->tm_wday],
		p->tm_hour, p->tm_min, p->tm_sec, newsec, (int)time(NULL));
	CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORRED);
}



#ifdef _ACTION_GMQUE
void CHAR_CHAT_DEBUG_cleanqute( int charaindex, char *message)
{
	char cdkey[256], name[256];
	int i;
	int playernum = CHAR_getPlayerMaxNum();

	memset( cdkey, 0, sizeof( cdkey) );
	memset( name, 0, sizeof( name) );

	if( getStringFromIndexWithDelim( message, " ", 1, cdkey, sizeof( cdkey)) == FALSE )	return;
	if( getStringFromIndexWithDelim( message, " ", 2, name, sizeof( name)) == FALSE )	return;

	for( i=0; i<playernum; i++ ){
		if( CHAR_getCharUse( i) == FALSE ) continue;
		if( !strcmp( CHAR_getChar( i, CHAR_CDKEY), cdkey) &&
			!strcmp( CHAR_getChar( i, CHAR_NAME), name) ){
			char token[256];
			
			CHAR_setChar( i, CHAR_GMQUESTR1, "");
			CHAR_setInt( i, CHAR_GMQUEFLG, 0 );
			CHAR_setInt( i, CHAR_GMQUENUMS, 0);

			sprintf( token,"���%s��Ǻš�", CHAR_getUseName(i) );
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORRED);
			sprintf( token,"%s�����Ļ�Ǻš�", CHAR_getUseName(charaindex) );
			CHAR_talkToCli( i, -1, token, CHAR_COLORRED);
			return;
		}
	}
	CHAR_talkToCli( charaindex, -1, "û���ҵ�����!!", CHAR_COLORRED);
}
#endif



void CHAR_CHAT_DEBUG_playerspread( int charaindex, char *message)
{
	int i,j;
	int playernum = CHAR_getPlayerMaxNum();
#define CHECKMAPNUMS 100
	struct {
		int floor;
		int nums;
	}MapArray[CHECKMAPNUMS];

	if(strstr( message, "waei") == NULL ) return;
	for( i=0; i<CHECKMAPNUMS; i++ ){
		MapArray[i].floor = -1;
		MapArray[i].nums = 0;
	}
	for( i=0; i<playernum; i++ ){
		int Myfloor, finds=0;
		if( CHAR_getCharUse( i) == FALSE ) continue;
		Myfloor = CHAR_getInt( i, CHAR_FLOOR);
		for( j=0; j<CHECKMAPNUMS; j++){
			if( MapArray[j].floor == -1 ) break;
			if( MapArray[j].floor == Myfloor ){
				MapArray[j].nums++;
				finds=1;
				break;
			}
		}
		if( finds == 0 && j < CHECKMAPNUMS ){
			MapArray[j].floor=Myfloor;
			MapArray[j].nums++;
		}else if( finds == 0 ){
			print( "ANDY MapArray full!!\n");
		}
	}
	print("\n");
	for( i=0; i<CHECKMAPNUMS; i++ ){
		if( MapArray[i].floor == -1 ) break;
		print( "ANDY MapArray[%d:%s]-%d\n",
			MapArray[i].floor,
			MAP_getFloorName( MapArray[i].floor), MapArray[i].nums );
	}
}


#ifdef _CHAR_PROFESSION			// WON ADD ����ְҵ

void CHAR_CHAT_DEBUG_set_regist( int charaindex, char*message )
{

#ifdef _MAGIC_RESIST_EQUIT		// WON ADD ְҵ����װ��
	char temp[128] = {0}, msg[128] = {0};
	int f = 0, i = 0, t = 0;

	getStringFromIndexWithDelim( message, " ", 1, temp, sizeof(temp) );
	f = atoi(temp);
	getStringFromIndexWithDelim( message, " ", 2, temp, sizeof(temp) );
	i = atoi(temp);
	getStringFromIndexWithDelim( message, " ", 3, temp, sizeof(temp) );	
	t = atoi(temp);

	CHAR_setWorkInt( charaindex, CHAR_WORK_F_SUIT, f );		
	CHAR_setWorkInt( charaindex, CHAR_WORK_I_SUIT, i );
	CHAR_setWorkInt( charaindex, CHAR_WORK_T_SUIT, t );

	sprintf(msg, "����(%d) ˮ����(%d) �翹��(%d)",
			CHAR_getWorkInt( charaindex, CHAR_WORK_F_SUIT),				
			CHAR_getWorkInt( charaindex, CHAR_WORK_I_SUIT),
			CHAR_getWorkInt( charaindex, CHAR_WORK_T_SUIT)
			);

	CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORYELLOW);
#endif
}

#ifdef _CHANNEL_MODIFY
extern int *piOccChannelMember;
#endif

void CHAR_CHAT_DEBUG_show_profession( int charaindex, char*message )
{
	char worker[10], value[10], type[10];
	char token[20], msg[512];
	int i, skillid, array;


	getStringFromIndexWithDelim( message, " ", 1, type, sizeof(type) );
	getStringFromIndexWithDelim( message, " ", 2, worker, sizeof(worker) );
	getStringFromIndexWithDelim( message, " ", 3, value, sizeof(value) );

	if( strcmp(type, "restart") == 0 ){	// �ض� profession.txt 
			CHAR_talkToCli( charaindex, -1, "�ض� profession.txt", CHAR_COLORRED);
			rePROFESSION_initSkill();
	}else if( strcmp(type, "") != 0 ){	// �޸�ְҵ����
		if( strcmp(worker, "") != 0 ){
			if( strcmp(value, "") != 0 ){
				CHAR_setInt(charaindex, PROFESSION_CLASS+atoi(worker), atoi(value) );
#ifdef _CHANNEL_MODIFY
				if(CHAR_getInt(charaindex,PROFESSION_CLASS) > 0){
					int i,pclass = CHAR_getInt(charaindex,PROFESSION_CLASS) - 1;
					for(i=0;i<getFdnum();i++){
						if(*(piOccChannelMember + (pclass * getFdnum()) + i) == -1){
							*(piOccChannelMember + (pclass * getFdnum()) + i) = charaindex;
							break;
						}
					}
				}
#endif
			}else
				CHAR_talkToCli( charaindex, -1, "��������ֵ", CHAR_COLORRED);
		}else 
			CHAR_talkToCli( charaindex, -1, "�������޸���Ŀ��1(ְҵ��) 2(ְҵ�ȼ�) 3(ְҵ����ֵ) 4(ʣ�ŵ���)", CHAR_COLORRED);
	}

	if(  strcmp(type, "1") != 0 ){
		memset(msg, 0, sizeof(msg));
		sprintf(msg, "ְҵ��(%d) ְҵ�ȼ�(%d) ʣ�ŵ���(%d)",
				CHAR_getInt(charaindex, PROFESSION_CLASS),
				CHAR_getInt(charaindex, PROFESSION_LEVEL),
				CHAR_getInt(charaindex, PROFESSION_SKILL_POINT)
				);

		CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORYELLOW);


		for( i = 0; i < CHAR_SKILLMAXHAVE; i ++ ){
			skillid = CHAR_getCharSkill( charaindex, i);
			if( skillid > 0 ){
				array = PROFESSION_SKILL_getskillArray( skillid );
				memset(token, 0, sizeof(token));
				sprintf( token, "P%d(%s) ", i+1, PROFESSION_SKILL_getChar(array, PROFESSION_SKILL_NAME) );
				if( strcmp(token, "") != 0) 
					strcat( msg, token );
			}
		}
		CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORRED);
	}


    CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charaindex , CHAR_WORKOBJINDEX ));

}
#endif

void CHAR_CHAT_DEBUG_samecode( int charaindex, char *message)
{
	char buf1[256], buf2[256];
	char cmd[3][256]={
		"pet", "item", "set"
	};
	int type=-1, j, total = 0;
	BOOL checkn = FALSE;
	int MAXPLAYER = CHAR_getPlayerMaxNum();
	int MAXITEM = ITEM_getITEM_itemnum();
	int MAXPET = CHAR_getPetMaxNum();

	if( getStringFromIndexWithDelim( message, " ", 1, buf1, sizeof( buf1) ) == FALSE )return;
	for( j=0; j<3; j++){
		if( strstr( buf1, cmd[j]) != NULL ){
			type = j;
			break;
		}
	}
	if( j >= 3 ){
		CHAR_talkToCli( charaindex, -1, "��������", CHAR_COLORRED);
		return;
	}

	switch( type){
	case 0:
		if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof( buf1) ) == TRUE ){
			checkn = TRUE;
		}
		for( j=MAXPLAYER; j<MAXPLAYER+MAXPET; j++ ){
			if( !CHAR_CHECKINDEX( j) ) continue;
			if( CHAR_getWorkInt( j, CHAR_WORKPLAYERINDEX) != 100000 ) continue;
			if( checkn == TRUE && strstr( CHAR_getUseName( j), buf1) != NULL ){
				sprintf( buf2, "Detain PET:%s[%s]-%s LV:%d",
					CHAR_getUseName( j), CHAR_getChar( j, CHAR_NAME),
					CHAR_getChar( j, CHAR_UNIQUECODE), CHAR_getInt( j, CHAR_LV) );
				CHAR_talkToCli( charaindex, -1, buf2, CHAR_COLORYELLOW);
				total++;
			}
		}
		sprintf( buf2, "total DetainPet:%d", total);
		CHAR_talkToCli( charaindex, -1, buf2, CHAR_COLORYELLOW);
		break;
	case 1:
		for( j=0; j<MAXITEM; j++ ){
			if( !ITEM_CHECKINDEX( j) ) continue;
			if( CHAR_getWorkInt( j, ITEM_WORKCHARAINDEX) != 100000 ) continue;

			sprintf( buf2, "Detain Item:%s-%s",
				ITEM_getChar( j, ITEM_NAME), ITEM_getChar( j, ITEM_UNIQUECODE) );
			CHAR_talkToCli( charaindex, -1, buf2, CHAR_COLORYELLOW);
					total++;
		}
		sprintf( buf2, "total DetainItem:%d", total);
		CHAR_talkToCli( charaindex, -1, buf2, CHAR_COLORYELLOW);
		break;
	case 2:
		if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof( buf1) ) == TRUE ){
			for( j=0; j<=CHAR_MAXITEMHAVE; j++){
				int itemindex = CHAR_getItemIndex( charaindex , j );
				if( !ITEM_CHECKINDEX( itemindex) ) continue;
				ITEM_setChar( itemindex, ITEM_UNIQUECODE, buf1);
			}
			for( j=0; j<CHAR_MAXPETHAVE; j++){
    			int petindex = CHAR_getCharPet( charaindex, j);
				if( !CHAR_CHECKINDEX( petindex) ) continue;
				CHAR_setChar( petindex, CHAR_UNIQUECODE, buf1);
			}
		}
		break;
	}


}

// Robin 2001/04/30
void CHAR_CHAT_DEBUG_silent(int charindex ,char *message)
{
     char token[100];
     char token1[100];
     char cdkey[CDKEYLEN];
     char buf[256];
     char line[256];
     unsigned int MAX_USER=0,i;
     //BOOL flg;
     int minu;
     easyGetTokenFromString( message , 1 , token, sizeof(token));
     easyGetTokenFromString( message , 2 , token1, sizeof(token1));
     if( (strlen(message)==0) || (strlen(token1)==0) ){
         sprintf(buf,"%s","��������ȷ--> �ʺ� ����");
         CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
         return;
     }
     //flg = isstring1or0( token1);
     minu = atoi( token1 );
     MAX_USER=getFdnum();
     for(i=0;i<MAX_USER;i++){
         int i_use;
         i_use=CONNECT_getUse(i);
         if(i_use){
             CONNECT_getCdkey( i, cdkey, sizeof( cdkey ));
             //print("\n<shut_up>--token=%s",token);
             if(strcmp(token,cdkey)==0){
             	int index = CONNECT_getCharaindex( i );
                //print("\n<shut_up>--find-->cdkey %s",cdkey);
                
                //CONNECT_set_shutup(i,TRUE);
                
                // Robin
                CHAR_setInt(index,CHAR_SILENT, minu*60);
                //CHAR_setInt(index,CHAR_SILENT, (int)NowTime.tv_sec +(minu*60) );
                {
                CHAR_setWorkInt( index, CHAR_WORKLOGINTIME, (int)NowTime.tv_sec);
                }
                //print(" set_silent:%s:%dmin ", cdkey, minu);
                
                //print("\n<Set TRUE");
                snprintf( line, sizeof( line),
               		"ϵͳ�� %s ����%d���ӡ�",
                	CHAR_getChar( index, CHAR_NAME),
                	minu );
                CHAR_talkToCli( charindex, -1,line, CHAR_COLORWHITE);
                CHAR_talkToCli( index, -1,line, CHAR_COLORWHITE);
             }
          }//if use
       }//for
}

void CHAR_CHAT_DEBUG_help( int charindex, char *message)
{
	char token[256], buf[256];
	char buf1[256], buf2[256]; 
	int i, GmLevel=0, type= 0;
	int MaxMagicNum = CHAR_getChatMagicFuncMaxNum();

	getStringFromIndexWithDelim( message, " ", 1, buf, sizeof( buf) );
	if( ( strlen( message) == 0 ) || ( strlen( buf) == 0 ) ){
		strcpy( buf, "[help ָ��/all]");
		CHAR_talkToCli( charindex, -1, buf, CHAR_COLORYELLOW);
		return;
	}

	if( !strcmp( buf, "all") ) type = 1;
	strcpy( token, "����GMָ��:");
	GmLevel = CHAR_getWorkInt( charindex, CHAR_WORKGMLEVEL);

	for( i=0; i<MaxMagicNum; i++){
		if( CHAR_getChatMagicFuncNameAndString( i, buf1, buf2, GmLevel, TRUE) != 1 ) continue;
		if( type == 1 ){
			sprintf( buf2, ", [%s ]", buf1);
			if( strlen( token) >= 220 ){
				CHAR_talkToCli( charindex, -1, token, CHAR_COLORYELLOW);	
				strcpy( token, "����GMָ��:");
			}
			strcat( token, buf2);
			continue;
		}else{
			if( strstr( buf1, buf) != NULL || 
				strstr( buf2, buf) != NULL ){
				sprintf( token, "GMָ��:[%s %s]", buf1, buf2);
				CHAR_talkToCli( charindex, -1, token, CHAR_COLORYELLOW);
			}
			continue;
		}
	}
	if( type == 1 )
		CHAR_talkToCli( charindex, -1, token, CHAR_COLORYELLOW);	
}

#ifdef _GM_IDENTIFY
void CHAR_CHAR_DEBUG_gmidentify( int charaindex, char* message )
{
    int i;
	int MAX_USER=getFdnum(); //�������
    CHAR_setChar(charaindex, CHAR_GMIDENTIFY, message); //�趨gm�ִ�
    CHAR_sendCToArroundCharacter(CHAR_getWorkInt( charaindex, CHAR_WORKOBJINDEX )); //���͸���Χ����
	//for(i=0;i<MAX_USER;i++){
      //  CHAR_talkToCli( i, -1, CHAR_getChar(charaindex, CHAR_GMIDENTIFY), CHAR_COLORWHITE);
    //}
}
#endif

#ifdef _EQUIT_NEGLECTGUARD
void CHAR_CHAT_DEBUG_setneguard( int charaindex, char* message )
{
	int num = atoi( message);
	CHAR_setWorkInt(  charaindex, CHAR_WORKNEGLECTGUARD, num);
}
#endif

#ifdef _DEATH_CONTEND
void CHAR_CHAT_DEBUG_updatepklist( int charaindex, char* message )
{
	char mycdkey[256], tocdkey[256], buf1[256];
	int mynum, tonum, winer;
	
	if( !CHAR_CHECKINDEX( charaindex) ) return;
	if( getStringFromIndexWithDelim( message, " ", 1, buf1, sizeof(buf1) ) == FALSE ) return;
	mynum = atoi( buf1);
	if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof(buf1) ) == FALSE ) return;
	tonum = atoi( buf1);
	if( getStringFromIndexWithDelim( message, " ", 3, buf1, sizeof(buf1) ) == FALSE ) return;
	winer = atoi( buf1);

	PKLIST_ShowPkListTeamData( );

	if( PKLIST_GetTeamLeaderCdkey( mynum, mycdkey) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "�޷��ҵ�����1", CHAR_COLORYELLOW);
		return;
	}
	if( PKLIST_GetTeamLeaderCdkey( tonum, tocdkey) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "�޷��ҵ�����2", CHAR_COLORYELLOW);
		return;
	}

	//andy_log
	print("PkListUpDate_send( %s, %s, %d, %d, %d) \n",
		mycdkey, tocdkey, mynum, tonum, winer );
	
	//saacproto_PkListUpDate_send( acfd , mycdkey, tocdkey, mynum, tonum, winer, 0);
	//CHAR_talkToCli( charaindex, -1, "Send !!", CHAR_COLORYELLOW);
	PKLIST_UpData(mycdkey,tocdkey,mynum,tonum,winer,0);
}
#endif



#ifdef _DEATH_FAMILY_GM_COMMAND	// WON ADD ����սGMָ��

void CHAR_CHAR_DEBUG_reloadfm( int charaindex, char* message )
{

	CHAR_talkToCli( charaindex, -1, "�ض�����������...", CHAR_COLORYELLOW);		
	saacproto_ReloadFamily_send( acfd, charaindex );
}

void CHAR_CHAR_DEBUG_fmpk( int charaindex, char* message )
{
	//������ ����1 ����2 ʱ�� ����
	static int fm1 = -1, fm2 = -1, time = -1, id = -1;
	char buf1[256] = {0};

	if( getStringFromIndexWithDelim( message, " ", 1, buf1, sizeof(buf1) ) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "[fmpk ����1 ����2 ʱ�� ����]",  CHAR_COLORYELLOW);
		return;
	}
	fm1 = atoi( buf1);
	if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof(buf1) ) == FALSE ) return;
	fm2 = atoi( buf1);
	if( getStringFromIndexWithDelim( message, " ", 3, buf1, sizeof(buf1) ) == FALSE ) return;
	time = atoi( buf1);	
	if( getStringFromIndexWithDelim( message, " ", 4, buf1, sizeof(buf1) ) == FALSE ) return;
	id = atoi( buf1 );

	saacproto_ACShowMemberList_2_send( acfd, charaindex, fm1, fm2, time, id );
}


void CHAR_CHAR_DEBUG_fmpk_clean( int charaindex, char* message )
{
	//���������� ʱ�� 
	int time = -1, id = -1;
	char buf1[256] = {0};
	int i, fmpks_pos = -1, hour = -1, min = -1;

	if( getStringFromIndexWithDelim( message, " ", 1, buf1, sizeof(buf1) ) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "[fmpk_clean ���� ʱ��]",  CHAR_COLORYELLOW);
		return;
	}
	id = atoi( buf1 );
	fmpks_pos = id * MAX_SCHEDULE;

	if( getStringFromIndexWithDelim( message, " ", 2, buf1, sizeof(buf1) ) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "[fmpk_clean ���� ʱ��]",  CHAR_COLORYELLOW);
		return;
	}
	if( strcmp( buf1, "all" ) == 0 ) time = 2500;
	else							 time = atoi( buf1);	

	hour = time / 100;
	min = time % 100;

	for( i=0; i<MAX_SCHEDULE; i++){		
		if( fmpks[fmpks_pos+i].dueltime == hour*100 || hour == 25){
			char msg[256] = {0};
					
			fmpks[fmpks_pos+i].host_index = -1;
			fmpks[fmpks_pos+i].guest_index = -1;
				
			strcpy( fmpks[fmpks_pos+i].host_name, "" );
			strcpy( fmpks[fmpks_pos+i].guest_name, "" );			

			fmpks[fmpks_pos+i].prepare_time = -1;
			fmpks[fmpks_pos+i].max_player = -1;
			fmpks[fmpks_pos+i].win = -1;			
			fmpks[fmpks_pos+i].flag = FMPKS_FLAG_NONE;
			fmpks[fmpks_pos+i].setting_timeout = -1; 

			sprintf( msg, "ID:%d TIME:%d ������ս������", id, time );
			CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORYELLOW );

			return;
		}
	}
}
#endif

#ifdef _NEW_MANOR_LAW
void CHAR_CHAT_DEBUG_set_momentum( int charaindex, char* message )
{
	char szMsg[32],szBuf[16],buf[128],szName[32];
	int i,id,fd,iPlayerNum = CHAR_getPlayerMaxNum();

	getStringFromIndexWithDelim(message," ",1,szName,sizeof(szName));
	getStringFromIndexWithDelim(message," ",2,szBuf,sizeof(szBuf));
	id = atoi(szBuf);
	for(i=0;i<iPlayerNum;i++){
		if(CHAR_getCharUse(i) != FALSE){
			if(!strcmp(CHAR_getChar(i,CHAR_NAME),szName)) break;
		}
	}
	if(i >= iPlayerNum){
		CHAR_talkToCli(charaindex,-1,"�Ҳ����������",CHAR_COLORYELLOW);
		return;
	}
	CHAR_setInt(i,CHAR_MOMENTUM,id);
	sprintf(szMsg,"Ŀǰ����ֵ: %d",CHAR_getInt(i,CHAR_MOMENTUM)/100);
	CHAR_talkToCli(charaindex,-1,szMsg,CHAR_COLORYELLOW);
	sprintf(buf,"%d",id);
	fd = getfdFromCharaIndex(i);
	saacproto_ACFixFMData_send(acfd,
		CHAR_getChar(i,CHAR_FMNAME),CHAR_getInt(i,CHAR_FMINDEX),CHAR_getWorkInt(i,CHAR_WORKFMINDEXI),
		FM_FIX_FMMOMENTUM,buf,"",CHAR_getWorkInt(i,CHAR_WORKFMCHARINDEX),CONNECT_getFdid(fd));
	return;
}

void CHAR_CHAT_DEBUG_set_manor_owner( int charindex, char* message )
{
	int i,index = -1,iUseFlag;
	char szId[8],szUseFlag[2],szFamilyNameOrID[64],szToken[4096],szFamilyIndex[8],szGetFamilyName[64];

	if(message == NULL || getStringFromIndexWithDelim(message," ",1,szId,sizeof(szId)) == FALSE){
		CHAR_talkToCli(charindex,-1, "��������ȷ [set_manor_owner ׯ԰id(1~9) ʹ�����(0:���� 1:id) �������ƻ�id]", CHAR_COLORRED);
		return;
	}
	if(atoi(szId) < 1 || atoi(szId) > MANORNUM){
		CHAR_talkToCli(charindex,-1, "ׯ԰IDֵ����ȷ(1~9)", CHAR_COLORRED);
		return;
	}
	if(getStringFromIndexWithDelim(message," ",2,szUseFlag,sizeof(szUseFlag)) == FALSE){
		CHAR_talkToCli(charindex,-1, "��������ȷ [set_manor_owner ׯ԰id(1~9) ʹ�����(0:���� 1:id) �������ƻ�id]", CHAR_COLORRED);
		return;
	}
	iUseFlag = atoi(szUseFlag);
	if(iUseFlag < 0 || iUseFlag > 1){
		CHAR_talkToCli(charindex,-1, "ʹ�����ֵ����ȷ(0:���� 1:id)", CHAR_COLORRED);
		return;
	}
	if(getStringFromIndexWithDelim(message," ",3,szFamilyNameOrID,sizeof(szFamilyNameOrID)) == FALSE){
		CHAR_talkToCli(charindex,-1, "��������ȷ [set_manor_owner ׯ԰id(1~9) ʹ�����(0:���� 1:id) �������ƻ�id]", CHAR_COLORRED);
		return;
	}
	if(iUseFlag == 1){
		// ����Ƿ�������
		for(i=0;i<sizeof(szFamilyNameOrID);i++){
			if(szFamilyNameOrID[i] == '\0') break;
			if(szFamilyNameOrID[i] > '9' || szFamilyNameOrID < '0'){
				CHAR_talkToCli(charindex,-1, "����id��ʽ����", CHAR_COLORRED);
				return;
			}
		}
		for(i=0;;i++){
			if(getStringFromIndexWithDelim(familyListBuf,"|",i,szToken,sizeof(szToken))){
				if((getStringFromIndexWithDelim(szToken," ",1,szFamilyIndex,sizeof(szFamilyIndex))) &&
					(getStringFromIndexWithDelim(szToken," ",2,szGetFamilyName,sizeof(szGetFamilyName)))){
					if(strcmp(szFamilyNameOrID,szFamilyIndex) == 0){
						index = atoi(szFamilyIndex) - 1;
						break;
					}
				}
			}
			else break;
		}
		if(index == -1){
			CHAR_talkToCli(charindex,-1,"�Ҳ����˼�������", CHAR_COLORRED);
			return;
		}
		sprintf(szToken,"ׯ԰ %s �趨�� %s ����",szId,szGetFamilyName);
		saacproto_ACFixFMPoint_send(acfd,szGetFamilyName,index+1,index,szGetFamilyName,index+1,index,atoi(szId));
	}
	else{
		for(i=0;;i++){
			if(getStringFromIndexWithDelim(familyListBuf,"|",i,szToken,sizeof(szToken))){
				if((getStringFromIndexWithDelim(szToken," ",1,szFamilyIndex,sizeof(szFamilyIndex))) &&
					(getStringFromIndexWithDelim(szToken," ",2,szGetFamilyName,sizeof(szGetFamilyName)))){
					if(strcmp(szFamilyNameOrID,szGetFamilyName) == 0){
						index = atoi(szFamilyIndex) - 1;
						break;
					}
				}
			}
			else break;
		}
	
		if(index == -1){
			CHAR_talkToCli(charindex,-1,"�Ҳ����˼�������", CHAR_COLORRED);
			return;
		}
		sprintf(szToken,"ׯ԰ %s �趨�� %s ����",szId,szFamilyNameOrID);
		saacproto_ACFixFMPoint_send(acfd,szFamilyNameOrID,index+1,index,szFamilyNameOrID,index+1,index,atoi(szId));
	}
	CHAR_talkToCli(charindex,-1,szToken,CHAR_COLORRED);
}

void CHAR_CHAT_DEBUG_set_schedule_time( int charindex, char* message )
{
	struct tm tm1;
	int i,char_max = CHAR_getCharNum();
	char szId[8],szTime[12],szMsg[128];

	if(message == NULL || getStringFromIndexWithDelim(message," ",1,szId,sizeof(szId)) == FALSE){
		CHAR_talkToCli(charindex,-1, "��������ȷ [set_schedule_time ׯ԰id(1~9) ʱ��(��)]", CHAR_COLORRED);
		return;
	}
	if(atoi(szId) < 1 || atoi(szId) > MANORNUM){
		CHAR_talkToCli(charindex,-1, "ׯ԰IDֵ����ȷ", CHAR_COLORRED);
		return;
	}
	if(getStringFromIndexWithDelim(message," ",2,szTime,sizeof(szTime)) == FALSE){
		CHAR_talkToCli(charindex,-1, "��������ȷ [set_schedule_time ׯ԰id(1~9) ʱ��(��)]", CHAR_COLORRED);
		return;
	}
	for(i=0;i<char_max;i++){
		if(CHAR_getCharUse(i) && (CHAR_getInt(i,CHAR_WHICHTYPE) == CHAR_TYPEMANORSCHEDULEMAN)){
			if(CHAR_getWorkInt(i,CHAR_NPCWORKINT2) == atoi(szId)){
				int fmpks_pos = CHAR_getWorkInt(i,CHAR_NPCWORKINT1) * MAX_SCHEDULE;

				memcpy(&tm1,localtime((time_t*)&NowTime.tv_sec),sizeof(tm1));
				fmpks[fmpks_pos+1].dueltime = NowTime.tv_sec + atoi(szTime) * 60;
				sprintf(szMsg,"ׯ԰ʱ���趨Ϊ %s ��",szTime);
				CHAR_talkToCli(charindex,-1,szMsg,CHAR_COLORRED);
				return;
			}
		}
	}
	CHAR_talkToCli(charindex,-1,"�Ҳ���ׯ԰npc",CHAR_COLORRED);
}

#endif


#ifdef _ANGEL_SUMMON
void CHAR_CHAT_DEBUG_angelinfo(int charindex ,char *message) // ��ʾMission����
{
	char cdkey[256], msg[1024], buf[256];
	int i, clean =FALSE;
	int MAXCHARA=0;
	char angelkey[256], angelname[256];
	char herokey[256], heroname[256];

	if( message[0] == NULL || getStringFromIndexWithDelim( message, " ", 1, cdkey, sizeof(cdkey) ) == FALSE ){
		CHAR_talkToCli( charindex, -1, "��������ȷ [angelinfo �ʺ�(all) (clean)]", CHAR_COLORRED);
		return;
	}
	if( getStringFromIndexWithDelim( message, " ", 2, buf, sizeof(buf) ) != FALSE ){
		//if( !strcmp( buf, "clean") )	clean =TRUE;
	}

	if( !strcmp( cdkey, "all") ) { // ��ʾȫ������
		for( i =0; i <MAXMISSIONTABLE; i++) {
			if( missiontable[i].angelinfo[0] == NULL )
				continue;
		
			sprintf( msg, "%d ʹ��:%s ����:%s MS:%d FLG:%d TIME:%d LT:%d ",
					i, missiontable[i].angelinfo, missiontable[i].heroinfo,
					missiontable[i].mission, missiontable[i].flag, missiontable[i].time, missiontable[i].limittime);
			if( clean) {
				saacproto_ACMissionTable_send( acfd, i, 3, "", "");
				strcat( msg, "...���!! ");
			}
			CHAR_talkToCli( charindex, -1, msg, CHAR_COLORYELLOW);
		}
	}
	else {
		for( i =0; i <MAXMISSIONTABLE; i++) {
			if( missiontable[i].angelinfo[0] == NULL )
				continue;
			getStringFromIndexWithDelim( missiontable[i].angelinfo, ":", 1, angelkey, sizeof(angelkey));
			getStringFromIndexWithDelim( missiontable[i].heroinfo, ":", 1, herokey, sizeof(herokey));

			if( !strcmp( angelkey, cdkey) || !strcmp( herokey, cdkey)) {
				sprintf( msg, "%d ʹ��:%s ����:%s MS:%d FLG:%d TIME:%d LT:%d ",
					i, missiontable[i].angelinfo, missiontable[i].heroinfo,
					missiontable[i].mission, missiontable[i].flag, missiontable[i].time, missiontable[i].limittime);
				CHAR_talkToCli( charindex, -1, msg, CHAR_COLORYELLOW);
				
				if( clean) {
					saacproto_ACMissionTable_send( acfd, i, 3, "", "");
				}
			}
		}
	}
	CHAR_talkToCli( charindex, -1, "....����", CHAR_COLORYELLOW);
}

void CHAR_CHAT_DEBUG_angelclean(int charindex ,char *message) // ���Mission����
{
	int mindex;
	char msg[1024];

	mindex = atoi( message);
	if( mindex <0 || mindex >=MAXMISSIONTABLE) {
		CHAR_talkToCli( charindex, -1, "��Ų���ȷ", CHAR_COLORYELLOW);
		return;
	}
	if( missiontable[mindex].flag == MISSION_NONE ) {
		CHAR_talkToCli( charindex, -1, "�޴�����", CHAR_COLORYELLOW);
		return;
	}
	
	saacproto_ACMissionTable_send( acfd, mindex, 3, "", "");

	sprintf( msg, " GMָ�� ����������� i:%d ʹ��:%s ����:%s m:%d flg:%d time:%d lt:%d ",
		mindex, missiontable[mindex].angelinfo, missiontable[mindex].heroinfo,
		missiontable[mindex].mission, missiontable[mindex].flag,
		missiontable[mindex].time, missiontable[mindex].limittime );
	CHAR_talkToCli( charindex, -1, msg, CHAR_COLORYELLOW);
	LogAngel( msg);
}

void CHAR_CHAT_DEBUG_angelcreate(int charindex ,char *message) // ����ʹ�����������
{
	char angelid[256], heroid[256], msg[1024], buf[256];
	int i, clean =FALSE;
	int MAXCHARA=0;
	int angelindex =-1, heroindex =-1, mission =-1;

	if( getStringFromIndexWithDelim( message, " ", 1, angelid, sizeof(angelid) ) == FALSE ){
		CHAR_talkToCli( charindex, -1, "��������ȷ [angelcreate ʹ���ʺ� �����ʺ� ������]", CHAR_COLORRED);
		return;
	}
	if( getStringFromIndexWithDelim( message, " ", 2, heroid, sizeof(heroid) ) == FALSE ){
		CHAR_talkToCli( charindex, -1, "��������ȷ [angelcreate ʹ���ʺ� �����ʺ� ������]", CHAR_COLORRED);
		return;
	}
	if( getStringFromIndexWithDelim( message, " ", 3, buf, sizeof(buf) ) == FALSE ){
		CHAR_talkToCli( charindex, -1, "��������ȷ [angelcreate ʹ���ʺ� �����ʺ� ������]", CHAR_COLORRED);
		return;
	}
	mission = atoi( buf);
	//if( mission <= 0 || mission >= MAXMISSION || missionlist[mission].id <= 0) {
	if( mission >= MAXMISSION ) {
		CHAR_talkToCli( charindex, -1, "�����Ų���ȷ", CHAR_COLORRED);
		return;
	}

	MAXCHARA = CHAR_getPlayerMaxNum();
	for( i=0; i<MAXCHARA; i++ ) {
		if( !CHAR_CHECKINDEX( i) ) continue;
		if( CHAR_getInt( i, CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER) continue;

		if( !strcmp( CHAR_getChar( i, CHAR_CDKEY), angelid) )
			angelindex = i;
		if( !strcmp( CHAR_getChar( i, CHAR_CDKEY), heroid) )
			heroindex = i;
		if( angelindex != -1 && heroindex != -1 )
			break;
	}
	if( angelindex == -1 || heroindex == -1 ) {
		CHAR_talkToCli( charindex, -1, "�Ҳ���ʹ�߻�����", CHAR_COLORRED);
		return;
	}

	if( checkIfAngel( angelindex) != -1 || checkIfAngel( heroindex) != -1 ) {
		CHAR_talkToCli( charindex, -1, "ʹ�߻������Ѿ���������", CHAR_COLORRED);
		return;
	}

	selectAngel( angelindex, heroindex, mission, TRUE);

}

void CHAR_CHAT_DEBUG_missionreload(int charindex ,char *message)
{
	print("Reading MissionList File...");
	CHAR_talkToCli( charindex, -1, "���¶�ȡʹ�������б�...", CHAR_COLORYELLOW);
	if( !LoadMissionList( ) ) {
		CHAR_talkToCli( charindex, -1, "��ȡʧ��", CHAR_COLORYELLOW);
		print("fail!!\n");
		return;
	}
	print("done\n");
	CHAR_talkToCli( charindex, -1, "��ȡ���", CHAR_COLORYELLOW);

}

#endif

void CHAR_CHAT_DEBUG_itemreload(int charindex ,char *message)
{
	print("Reading itemset File...");
	CHAR_talkToCli( charindex, -1, "���µ�������...", CHAR_COLORYELLOW);

	if(!ITEM_readItemConfFile( getItemfile()) ) {
		CHAR_talkToCli( charindex, -1, "��ȡʧ��", CHAR_COLORYELLOW);
	}
	print("done\n");
	CHAR_talkToCli( charindex, -1, "��ȡ���", CHAR_COLORYELLOW);
}


void CHAR_CHAT_DEBUG_skywalker(int charaindex ,char *message)
{
	if( !CHAR_CHECKINDEX( charaindex) )
		return;

	if( strstr( message, "on") != NULL )	{
		CHAR_talkToCli( charaindex, -1, "������ģʽ����", CHAR_COLORYELLOW);
		CHAR_setWorkInt( charaindex, CHAR_WORKSKYWALKER, TRUE);
	}
	if( strstr( message, "off") != NULL )	{
		CHAR_talkToCli( charaindex, -1, "������ģʽ�ر�", CHAR_COLORYELLOW);
		CHAR_setWorkInt( charaindex, CHAR_WORKSKYWALKER, FALSE);
	}

	CHAR_send_P_StatusString( charaindex, CHAR_P_STRING_SKYWALKER);
}

#ifdef _ITEM_ADDEXP // ��ʾ�ǹ�Ч��
void CHAR_CHAT_DEBUG_itemaddexp(int charaindex ,char *message)
{
	char msg[1024];
	int toindex = -1;

	if( !CHAR_CHECKINDEX( charaindex) )
		return;

	if( !strcmp( message, "") ) {
		toindex = charaindex;
	}
	else {
		int maxchara, i;
		maxchara = CHAR_getPlayerMaxNum();
		for( i=0; i<maxchara; i++ ) {
			if( !CHAR_CHECKINDEX( i) ) continue;
			if( CHAR_getInt( i, CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER) continue;

			if( !strcmp( CHAR_getChar( i, CHAR_CDKEY), message) ) {
				toindex = i;
				break;
			}
		}
		if( toindex == -1 ) {
			CHAR_talkToCli( charaindex,-1, "���޴���", CHAR_COLORRED);
			return;
		}
	}

	sprintf( msg, "�ǻ�֮��Ч��  %s��Ŀǰ����%d  %d��  ʱ��%d�֡�",
			CHAR_getChar( toindex, CHAR_NAME),
			CHAR_getWorkInt( toindex, CHAR_WORKITEM_ADDEXP)
					* (int)(CHAR_getWorkInt( toindex, CHAR_WORKITEM_ADDEXPTIME)/60),
			CHAR_getWorkInt( toindex, CHAR_WORKITEM_ADDEXP),
			(int)(CHAR_getWorkInt( toindex, CHAR_WORKITEM_ADDEXPTIME)/60) );

	CHAR_talkToCli( charaindex,-1, msg, CHAR_COLORRED);
}
#endif

#ifdef _DEF_FMFREETIME
void CHAR_CHAT_DEBUG_fmfreetime(int charaindex,char *message)
{
	char token_cdkey[256] , buf[256];
	int  i , Playernum = CHAR_getPlayerMaxNum();

	if( !CHAR_CHECKINDEX( charaindex) )
		return;
	getStringFromIndexWithDelim( message, " ", 1, token_cdkey, sizeof( token_cdkey));
	if( strlen(token_cdkey)==0){
        sprintf(buf,"%s","��������ȷ����ȷ��ʽ��[fmfreetime  �ʺ�]");
        CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
        return;
	}
	for( i=0;i<Playernum;i++ ){
		if( !CHAR_CHECKINDEX( i) ) continue;
		if( CHAR_getInt( i, CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER) continue;
		if(CHAR_getCharUse(i) != FALSE){
			if( !strcmp( CHAR_getChar( i, CHAR_CDKEY), token_cdkey ) ){
				CHAR_setInt( i , CHAR_FMTIMELIMIT, 0 );			
				break;				
			}
		}
	}
	if( i >= Playernum ){
		sprintf(buf,"%s\n","ʧ�ܣ����޴��ʺš�");
		CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORRED);	
	}else{
		if(!CHAR_getInt( i , CHAR_FMTIMELIMIT )){
			sprintf(buf,"%s\n","�趨��ɡ�");
			CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORYELLOW);	
		}		
	}
}
#endif

#ifdef _DEF_GETYOU 
void CHAR_CHAT_DEBUG_getyou(int charaindex,char *message)
{
	int floor,x,y,i,j,fd,area;
	char cdkey[CDKEYLEN],areabuf[5];
	char szName[256],buf[256];
    OBJECT  object;
	char line[256];
	int o,chara_index;
	int getnum = 0;

	getStringFromIndexWithDelim( message, " ", 1, areabuf , sizeof( areabuf ));
	area = atoi(areabuf);
	if( area <= 0 || area > 3 ){
        sprintf(buf,"%s","û������  Χ��  Χ�������ƣ�������1~3");
        CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
        return;
	}
	floor=CHAR_getInt( charaindex, CHAR_FLOOR );
    x=CHAR_getInt( charaindex, CHAR_X );
    y=CHAR_getInt( charaindex, CHAR_Y );
	for(i=y-area;i<=y+area;i++){
		for(j=x-area;j<=x+area;j++){
			for( object = MAP_getTopObj(floor,j,i) ; object ;object = NEXT_OBJECT(object ) ){
				o = GET_OBJINDEX(object);
				if( OBJECT_getType(o) == OBJTYPE_CHARA ){
					chara_index=OBJECT_getIndex(o);
					if( CHAR_getInt( chara_index, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER ) continue;
					if( chara_index == charaindex ) continue;  //�ҵ��Լ��� 
					if( ( fd = getfdFromCharaIndex(chara_index) ) != -1 ){				
						getnum++;
						CONNECT_getCharname( fd, szName, sizeof( szName ) );  //��������
						CONNECT_getCdkey( fd, cdkey, sizeof( cdkey ));		  //�ʺ�	
						//CHAR_getChar( chara_index, CHAR_NAME);  
						snprintf( line, sizeof( line), "��%d���ҵ�[%s]���ʺ���[%s]",getnum,szName,cdkey);
                        CHAR_talkToCli( charaindex, -1,line, CHAR_COLORRED);
						print("��%d���ҵ�[%s]���ʺ���[%s]\n",getnum,szName,cdkey);					
					}
				}
			}
		}
	}
	if(!getnum) CHAR_talkToCli( charaindex, -1, "  Χ��û����", CHAR_COLORYELLOW);
}
#endif

#ifdef _DEF_NEWSEND
void CHAR_CHAT_DEBUG_newsend(int charindex ,char *message)
{
    char buf[256];
	char token[100];
	char szName[256];
	char cdkey[CDKEYLEN];
	char line[256];
	char say[128];
	int fl,x,y,i;
	unsigned int MAX_USER=0;
	MAX_USER=getFdnum();
	if( strlen(message)==0){
		sprintf(buf,"%s","��������ȷ-> FLOOR X Y ��/Ҫ˵�Ļ�");
		CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	easyGetTokenFromString( message , 1 , token, sizeof(token));
	if( strlen(token)==0){
		sprintf(buf,"%s","��������ȷ-->��ͼ���");
		CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	fl=atoi( token );
	easyGetTokenFromString( message , 2 , token, sizeof(token));
	if( strlen(token)==0){
		sprintf(buf,"%s","��������ȷ-->X���� ");
		CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	x = atoi( token );
	easyGetTokenFromString( message , 3 , token, sizeof(token));
	if( strlen(token)==0){
		sprintf(buf,"%s","��������ȷ-->Y���� ");
		CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	y = atoi( token );
	easyGetTokenFromString( message , 4 , token, sizeof(token));
	if( strlen(token)==0){
		sprintf(buf,"%s","��������ȷ-->�ʺ�");
		CHAR_talkToCli( charindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	easyGetTokenFromString( message , 5 , say , sizeof(say));

	for(i=0;i<MAX_USER;i++){                                                                               
		int i_use,fd_charaindex;
		i_use=CONNECT_getUse(i);
		if(i_use){
			CONNECT_getCharname( i, szName, sizeof( szName ) );
			CONNECT_getCdkey( i, cdkey, sizeof( cdkey ));
			if(strcmp(token,cdkey)==0){
				fd_charaindex = CONNECT_getCharaindex( i );
				CHAR_warpToSpecificPoint(fd_charaindex, fl, x, y);
				snprintf( line, sizeof( line),
						 "�� name:%s account=%s ���͵� FL=%d X=%d Y=%d say:%s",
						  szName,cdkey,fl,x,y,say);
				CHAR_talkToCli( charindex, -1 , line , CHAR_COLORWHITE );
				if(strlen(say)){ 
					CHAR_talkToCli( fd_charaindex, -1 , say, CHAR_COLORYELLOW );
				}
			}                                  
        }
	}
}	
#endif 

#ifdef _DEF_SUPERSEND
void CHAR_CHAT_DEBUG_supersend(int charaindex ,char *message)
{
    char buf[256];
	char token[100];
	char name[256],szName[256];
	char cdkey[CDKEYLEN],cdkeytoken[CDKEYLEN];
	char line[256];
	char say[128];
	int tofl,toX,toY,i,j,k,area;
    OBJECT  object;
	int fl,x,y,fd,fd_charaindex; //�ʺ�����λ��
	unsigned int MAX_USER=0;
	int sendnum = 0;
	int o,chara_index;


	if( strlen(message)==0){
		sprintf(buf,"%s","��������ȷ-> FLOOR X Y �ʺ� ���� ��/Ҫ˵�Ļ�");
		CHAR_talkToCli( charaindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	easyGetTokenFromString( message , 1 , token, sizeof(token));
	if( strlen(token)==0){
		sprintf(buf,"%s","��������ȷ-->��ͼ���");
		CHAR_talkToCli( charaindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	tofl=atoi( token );
	easyGetTokenFromString( message , 2 , token, sizeof(token));
	if( strlen(token)==0){
		sprintf(buf,"%s","��������ȷ-->X���� ");
		CHAR_talkToCli( charaindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	toX = atoi( token );
	easyGetTokenFromString( message , 3 , token, sizeof(token));
	if( strlen(token)==0){
		sprintf(buf,"%s","��������ȷ-->Y���� ");
		CHAR_talkToCli( charaindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	toY = atoi( token );
	easyGetTokenFromString( message , 4 , cdkeytoken , sizeof(cdkeytoken));
	if( strlen(cdkeytoken)==0){
		sprintf(buf,"%s","��������ȷ-->�ʺ�");
		CHAR_talkToCli( charaindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	easyGetTokenFromString( message , 5 , token, sizeof(token));
	area = atoi( token );
	if( strlen(token)==0 ||	area <= 0 || area > 3 ){
		sprintf(buf,"%s","��������ȷ-->����(1~3)");
		CHAR_talkToCli( charaindex, -1,buf, CHAR_COLORWHITE);
		return;
	}
	easyGetTokenFromString( message , 6 , say , sizeof(say));  //˵��

	MAX_USER=getFdnum();
    for(k=0;k<MAX_USER;k++){
		int i_use;
        i_use=CONNECT_getUse(k);
        if(i_use){
			CONNECT_getCdkey( k, cdkey, sizeof( cdkey ));
            if(strcmp(cdkey,cdkeytoken)==0){
				fd_charaindex = CONNECT_getCharaindex( k );
				CONNECT_getCharname( k , name, sizeof( name ) );
				fl=CHAR_getInt( fd_charaindex, CHAR_FLOOR );
				x=CHAR_getInt( fd_charaindex, CHAR_X );
				y=CHAR_getInt( fd_charaindex, CHAR_Y );
				for(i=y-area;i<=y+area;i++){
					for(j=x-area;j<=x+area;j++){
						object = MAP_getTopObj(fl,j,i); 
						while(object){
							o = GET_OBJINDEX(object);
							if( OBJECT_getType(o) == OBJTYPE_CHARA ){
								chara_index=OBJECT_getIndex(o);
								if( CHAR_getInt( chara_index, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER ){
									//if( chara_index == charaindex ) continue;  //�ҵ��Լ��� 
									if( ( fd = getfdFromCharaIndex(chara_index) ) != -1 ){				
										sendnum++;
										CONNECT_getCharname( fd, szName, sizeof( szName ) );  //��������
										CHAR_warpToSpecificPoint( chara_index, tofl, toX, toY);
										snprintf(line,sizeof( line),"��%d������[%s]",sendnum,szName);
										CHAR_talkToCli( charaindex, -1,line, CHAR_COLORWHITE);
										if(strlen(say))
											CHAR_talkToCli( chara_index, -1 , say , CHAR_COLORYELLOW );										
										object = MAP_getTopObj(fl,j,i);
										continue;
									}
								}
							}
							object = NEXT_OBJECT(object);
						}
					}
				}
				if(sendnum<=1) CHAR_talkToCli( charaindex, -1, "  Χ��û�б�����", CHAR_COLORYELLOW);
				snprintf( line, sizeof( line),"�� name:%s account=%s ��Χ%d���ڵ���ҹ�%d�����͵� FL=%d X=%d Y=%d",
													  name,cdkey,area,sendnum,tofl,toX,toY);
				CHAR_talkToCli( charaindex, -1 , line , CHAR_COLORYELLOW );
				break;
			}
		}
	}
}
#endif

#ifdef _FONT_SIZE
void CHAR_CHAT_DEBUG_fsize( int charaindex, char* message )
{
	char buf1[256];
	CHAR_setWorkInt( charaindex, CHAR_WORKFONTSIZE, atoi( message ));
	//CHAR_send_P_StatusString( charaindex , CHAR_P_STRING_LV);
	sprintf( buf1, "˵�����ʹ�С��Ϊ%d��", atoi( message ));
	CHAR_talkToCli( charaindex, -1, buf1, CHAR_COLORYELLOW);
}
#endif

#ifdef _JOBDAILY
void CHAR_CHAT_DEBUG_rejobdaily(int charaindex ,char *message)
{
	print("Reading Jobdaily File...");
	CHAR_talkToCli( charaindex, -1, "���¶�ȡ������־�б�...", CHAR_COLORYELLOW);
	if(!LoadJobdailyfile()){
		CHAR_talkToCli( charaindex, -1, "��ȡʧ��", CHAR_COLORYELLOW);
		print("fail!!\n");
		return;
	}
	print("done\n");
	CHAR_talkToCli( charaindex, -1, "��ȡ���", CHAR_COLORYELLOW);
}
#endif

#ifdef _RACEMAN
extern int asktotal;

void CHAR_CHAT_DEBUG_changeask(int charaindex, char *message)
{
	char cdkey[64] , free[64] , buf[256] , item[64];
	int  i ,toindex=0, Playernum = CHAR_getPlayerMaxNum();
	int  itemindex,itemid;
	unsigned int askno,asklv,temp;
	int petNo,intNo,shift,base;

	easyGetTokenFromString( message , 1 , cdkey , sizeof(cdkey));
	if( strlen(cdkey)==0){
        sprintf(buf,"%s","û���ʺš���ȷ��ʽ��[changeask  �ʺ�  ��������  ��֤id 0/1]");
        CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
        return;
	}
	easyGetTokenFromString( message , 2 , item, sizeof(item));
	if( strlen(item)==0){
        sprintf(buf,"%s","û�г�֤id����ȷ��ʽ��[changeask  �ʺ�  ��֤id 0/1]");
        CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
        return;
	}
	easyGetTokenFromString( message , 3 , free , sizeof(free));
	if( strlen(free)==0 ){
        sprintf(buf,"%s","����Ĳ���������ȷ��ʽ��[changeask  �ʺ� ��֤id 0/1]");
        CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
        return;
	}


	for( i=0;i<Playernum;i++ ){
		if( !CHAR_CHECKINDEX( i) ) continue;
		if( CHAR_getInt( i, CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER) continue;
		if( CHAR_getCharUse(i) != FALSE){
			if( !strcmp( CHAR_getChar( i, CHAR_CDKEY), cdkey ) ){
					toindex = i;
					break;			
			}
		}
	}
	if(i>=Playernum){
        sprintf(buf,"%s","�Ҳ�������ˣ���ȷ���ʺ�");
        CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
        return;
	}
	//�ҵ���
	for( i = 0 ; i < CHAR_MAXITEMHAVE ; i++ ) {
		itemindex=CHAR_getItemIndex( toindex , i );
		if( ITEM_CHECKINDEX( itemindex) ) {
			itemid=ITEM_getInt( itemindex ,ITEM_ID );			
			if(itemid==atoi(item)){	//�ҵ��Գ�֤
				if(atoi(free)==1){
					//�趨���
					petNo = ( CHAR_getInt( toindex , CHAR_CHECKIN ) & 0x7FC00000 ) >> 22 ;	
					intNo = petNo / 32 ;	
					shift = petNo % 32 ;
					base  = 1 << shift;
					CHAR_setInt( toindex , CHAR_KINDCNT1 + intNo , CHAR_getInt( toindex , CHAR_KINDCNT1 + intNo ) | base );
					CHAR_setInt( toindex,CHAR_CATCHCNT5,CHAR_getInt( toindex,CHAR_CATCHCNT5 ) +1 );
				}
				//����һ��
				if(CHAR_getInt( toindex,CHAR_CATCHCNT5 ) >= asktotal ){
					sprintf( buf,"��ϲ��ﵽ�Գ�����ռ�Ŀ�꣬ף�����д�");
				}else{
					askno = Raceman_getokask( toindex , rand() % asktotal );
					asklv = asktable[askno].lowlv + rand()%((asktable[askno].highlv+1)-asktable[askno].lowlv);
					temp = (askno << 22) & 0x7FC00000;
					temp = temp | ((asklv << 14) & 0x003FC000);
					temp = temp | ((asktable[askno].bbi - 100000) & 0x00003FFF);
					CHAR_setInt(  toindex , CHAR_CHECKIN , temp );  		
					if( asktable[askno].highlv == 0 )
						sprintf( buf,"��ץ���ҵȼ�%d�����ϡ���%s������Ŀǰ��¼%d��", asktable[askno].lowlv , asktable[askno].petname, CHAR_getInt( toindex,CHAR_CATCHCNT5 ) );
					else
						sprintf( buf,"��ץ���ҡ�ֻ�ܡ��ȼ�%d��%s������Ŀǰ��¼%d��", asklv , asktable[askno].petname, CHAR_getInt( toindex,CHAR_CATCHCNT5 ) );
				}				
				ITEM_setChar( itemindex, ITEM_EFFECTSTRING, buf);
				CHAR_sendItemDataOne( toindex , i );
				break;
			}			
		}
	}
	if( i >= CHAR_MAXITEMHAVE ){
		sprintf(buf,"%s","�Ҳ����Գ�֤��");
        CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
        return;
	}else{
		sprintf(buf,"%s","�趨���");
        CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
	}
}
#endif
