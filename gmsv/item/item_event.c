#include "version.h"
#include <string.h> /* strstr */
#include <ctype.h>  /* isdigit, isalnum */
#include "common.h"
#include "char.h"
#include "char_base.h"
#include "char_data.h"
#include "configfile.h"
#include "item.h"
#include "item_event.h"
#include "object.h"
#include "anim_tbl.h"
#include "magic.h"
#include "handletime.h"
#include "log.h"
#include "util.h"
#include "encount.h"
#include "battle.h"
#include "battle_item.h"
#include "lssproto_serv.h"
#include "net.h"
#include "pet.h"
#include "npcutil.h"
#include "enemy.h"
#ifdef _Item_ReLifeAct
#include "battle_magic.h"
#endif
#ifdef _MARKET_TRADE
#include "item_trade.h"
#endif
#ifdef _ITEM_WARP_FIX_BI
extern	tagRidePetTable	ridePetTable[122];
#endif
#include "pet_skill.h"

#ifdef _DEATH_CONTENDAB
#include "deathcontend.h"
#endif

#ifdef _CFREE_petskill
#include "npc_freepetskillshop.h"
#endif

#ifdef _PROFESSION_SKILL			// WON ADD ����ְҵ����
#include "profession_skill.h"
#endif

int ITEM_TimeDelCheck( int itemindex )
{	int icnt, jcnt, playernum;
	playernum = CHAR_getPlayerMaxNum();
	for( icnt = 0; icnt < playernum; icnt ++ ) {
		if( CHAR_CHECKINDEX( icnt ) ) {
			for( jcnt = 0; jcnt < CHAR_MAXITEMHAVE; jcnt ++ ) {
				if( CHAR_getItemIndex( icnt, jcnt ) == itemindex ) {
					print( "����Ч��ʱ���ѵ���������ֵ��ߴ���(%s)(%s)\n",
						CHAR_getUseName(icnt),ITEM_getAppropriateName(itemindex) );
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

int ITEM_eventDrop( int itemindex, int charaindex, int itemcharaindex )
{
	typedef void (*DROPF)(int,int);
	char szBuffer[256]="";
	DROPF dropfunc=NULL;

    if( ITEM_CHECKINDEX(itemindex) == FALSE )return -1;

	dropfunc=(DROPF)ITEM_getFunctionPointer( itemindex, ITEM_DROPFUNC );
	if( dropfunc ){
		dropfunc( charaindex, itemindex );
	}

    if( ITEM_getInt( itemindex, ITEM_VANISHATDROP) != 1 )return 0;
	snprintf( szBuffer, sizeof( szBuffer), "%s �����ˡ�",
		ITEM_getAppropriateName(itemindex) );
	CHAR_talkToCli( charaindex, -1, szBuffer, CHAR_COLORWHITE );

	{
		LogItem(
			CHAR_getChar( charaindex, CHAR_NAME ), /* ƽ�ҷ�   */
			CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
			itemindex,
#else
       		ITEM_getInt( itemindex, ITEM_ID ),  /* ʧ��  ة  į */
#endif
			"Drop&Delete(��������ʧ)",
			CHAR_getInt( charaindex,CHAR_FLOOR),
			CHAR_getInt( charaindex,CHAR_X ),
 	      	CHAR_getInt( charaindex,CHAR_Y ),
            ITEM_getChar( itemindex, ITEM_UNIQUECODE),
						ITEM_getChar( itemindex, ITEM_NAME),
						ITEM_getInt( itemindex, ITEM_ID)
		);
	}
	CHAR_setItemIndex( charaindex, itemcharaindex, -1);
	ITEM_endExistItemsOne( itemindex );
	return 1;
}

#undef  UNDEF
#define UNDEF (-1)

typedef struct {
    char* cmd;        /* ġ    ٯ   */
    char*   onmessage;    /*  ����������  ٯ      */
    char*   offmessage;    /*  ����������  ٯ      */
    int   element;    /* ���ޱ�ئ��ۢ�� */
    int   maxElement; /* elementƥ϶�ý�ľ��ۢ�ټ�    ��ë�ֹ�ۢ�� */
} ITEM_EFFECTPARAM;
static ITEM_EFFECTPARAM ITEM_restorableParam[] = {
    {"hp", "HP�ظ��ˡ�", "HP�����ˡ�",CHAR_HP, CHAR_WORKMAXHP},
    {"mp", "MP�ظ��ˡ�", "MP�����ˡ�",CHAR_MP, CHAR_WORKMAXMP},
};
static ITEM_EFFECTPARAM ITEM_statusParam[] = {
    {"po", CHAR_POISONSTRING, CHAR_RECOVERPOISONSTRING,
     CHAR_POISON,    UNDEF},

    {"pa", CHAR_PARALYSISSTRING, CHAR_RECOVERPARALYSISSTRING,
     CHAR_PARALYSIS, UNDEF},

    {"si", CHAR_SILENCESTRING, CHAR_RECOVERSILENCESTRING,
     CHAR_SLEEP,UNDEF},

    {"st", CHAR_STONESTRING, CHAR_RECOVERSTONESTRING, CHAR_STONE, UNDEF},

    {"da", CHAR_DARKNESSSTRING, CHAR_RECOVERDARKNESSSTRING,
     CHAR_DRUNK,UNDEF},

    {"co", CHAR_CONFUSIONSTRING,CHAR_RECOVERCONFUSIONSTRING,
     CHAR_CONFUSION, UNDEF},
};

/*#define LOCAL_DEBUG*/
#ifdef LOCAL_DEBUG
#define DOUTFILE  "doutfile"
#include <stdio.h>
#include <stdarg.h>
static int eprintf(char* format, ...){
    va_list arg;
    long len;
    FILE* fp;
    fp = fopen(DOUTFILE, "a");
    va_start(arg, format);
    len = vfprintf(stderr, format, arg);
    if(fp){ vfprintf(fp, format, arg); fclose(fp); }
    va_end(arg);
    return len;
}

#define fprint eprintf
#endif

static BOOL ITEM_isValidEffect(char* cmd, int value){
    int i;

    for(i=0; i<arraysizeof(ITEM_restorableParam); i++){
        if(! strcmp(cmd, ITEM_restorableParam[i].cmd)){
            return value <= 0 ? FALSE : TRUE;
        }
    }
    if(value < 0 ) return FALSE;
    for(i=0; i<arraysizeof(ITEM_statusParam); i++){
        if(! strcmp(cmd, ITEM_statusParam[i].cmd)){
            return TRUE;
        }
    }
    return FALSE;
}

#define ID_BUF_LEN_MAX  20
#define SEPARATORI       '|'
BOOL ITEM_MedicineInit(ITEM_Item* itm)
{
    char cmd[ID_BUF_LEN_MAX], arg[ID_BUF_LEN_MAX];
    int value;
    int effectCount = 0;
    char* p, * q;
    char* effectarg;

    effectarg = itm->string[ITEM_ARGUMENT].string;
    if(* effectarg == '\0') return TRUE;
    for(p=effectarg; *p != '\0'; ){
        int i;
        if(*p == SEPARATORI) p++;
        for(q=cmd, i=0; isalnum(*p) && i<ID_BUF_LEN_MAX; i++){
            *q++ = *p++;
        }
        *q = '\0';
        if(q == cmd || *p != SEPARATORI){
            fprint("ITEM_medicineInit: error(c)? invalid:%s(%d)\n",
                   effectarg, effectCount);
            return FALSE;
        }
        p++;
        for(q=arg, i=0; isdigit(*p) && i<ID_BUF_LEN_MAX; i++){
            *q++ = *p++;
        }
        *q = '\0';
        value = strtol(arg, & q, 10); /* strtol()��OK? */
        if(ITEM_isValidEffect(cmd, value)){
            effectCount++;
        }else{
            fprint("ITEM_medicineInit: error(v)? invalid:%s(%d)\n",
                   effectarg, effectCount);
            return FALSE;
        }
    }
    return (effectCount == 0) ? FALSE : TRUE;
}

static BOOL ITEM_medicineRaiseEffect(int charaindex, char* cmd,int value)
{
    int i;
    char ansmsg[256];
    for( i=0 ; i<arraysizeof(ITEM_restorableParam); i++){
        if( ! strcmp(cmd, ITEM_restorableParam[i].cmd) ){
#if 1
            int maxv,curv,amount;
            char*   onoroff=NULL;
            maxv=CHAR_getWorkInt( charaindex, ITEM_restorableParam[i].maxElement);
            curv=CHAR_getInt(charaindex,ITEM_restorableParam[i].element);
            amount = value;
            if( curv + amount < 0 ) amount = -curv;
            CHAR_setInt(charaindex,ITEM_restorableParam[i].element, min((curv+amount),maxv));
            if( amount > 0 )
                onoroff=ITEM_restorableParam[i].onmessage;
            else if( amount < 0 )
                onoroff=ITEM_restorableParam[i].offmessage;
            else
                return FALSE;
            strcpysafe( ansmsg, sizeof(ansmsg), onoroff );
            CHAR_talkToCli( charaindex, -1, ansmsg, CHAR_COLORWHITE);
            return TRUE;
#else
            if(value == ITEM_EFFECT_RESTORE_NORMAL){
                int maxv, curv, amount;
                maxv =CHAR_getWorkInt(charaindex, ITEM_restorableParam[i].maxElement);
                curv =CHAR_getInt(charaindex, ITEM_restorableParam[i].element);
                amount = 30;
                if(curv + amount < 0) amount = -curv;
                CHAR_setInt(charaindex, ITEM_restorableParam[i].element, min((curv+amount), maxv));
                snprintf( ansmsg, sizeof(ansmsg), "%s�ظ��ˡ�",
                          ITEM_restorableParam[i].onmessage );
                CHAR_talkToCli( charaindex, -1, ansmsg, CHAR_COLORWHITE);
                return TRUE;
            }else if(value == ITEM_EFFECT_RESTORE_COMPLETE){
                int maxv;
                maxv =CHAR_getWorkInt(charaindex, ITEM_restorableParam[i].maxElement);
                CHAR_setInt(charaindex, ITEM_restorableParam[i].element, maxv);
                snprintf( ansmsg, sizeof(ansmsg), "%s����ȫ�ظ���", ITEM_restorableParam[i].onmessage );
                CHAR_talkToCli( charaindex, -1, ansmsg, CHAR_COLORWHITE );
                return TRUE;
            }else
                return FALSE;
#endif

        }
    }
    if( value >= 0 ){
        int found = 0;
        for(i=0; i<arraysizeof(ITEM_statusParam); i++){
            if(! strcmp(cmd, ITEM_statusParam[i].cmd)){
                ansmsg[0] = '\0';
                if( value && CHAR_getInt(charaindex,ITEM_statusParam[i].element) < value ){
                    CHAR_setInt(charaindex, ITEM_statusParam[i].element, value);
                    strcpysafe( ansmsg, sizeof(ansmsg), ITEM_statusParam[i].onmessage );
                }else{
                    if( CHAR_getInt(charaindex, ITEM_statusParam[i].element ) ){ 
						strcpysafe( ansmsg, sizeof(ansmsg), ITEM_statusParam[i].offmessage );
                    }
                    CHAR_setInt(charaindex, ITEM_statusParam[i].element, 0 );
                }
                found = 1;
                if( ansmsg[0] != '\0' )
                    CHAR_talkToCli( charaindex,-1,ansmsg,CHAR_COLORWHITE);
            }
        }
        if(found){
            CHAR_sendCToArroundCharacter( CHAR_getWorkInt(charaindex,CHAR_WORKOBJINDEX) );
            return TRUE;
        }
    }
#undef ITEM_STATUSCHANGEVALUE
    strcpysafe( ansmsg, sizeof(ansmsg),"ʲ��Ҳû������");
    CHAR_talkToCli( charaindex, -1, ansmsg, CHAR_COLORWHITE );
    return FALSE;
}

void ITEM_MedicineUsed(int charaindex, int to_charaindex, int itemindex)
{
    int itemid;
    int usedf = 0;
    char cmd[ID_BUF_LEN_MAX], arg[ID_BUF_LEN_MAX];
    int value;
    char* p, * q;
    char* effectarg;
    char ansmsg[256];
    itemid = CHAR_getItemIndex(charaindex, itemindex);
    if(!ITEM_CHECKINDEX(itemid)) return;
    effectarg = ITEM_getChar(itemid, ITEM_ARGUMENT);
    snprintf( ansmsg, sizeof(ansmsg), "ץ����%s ��", ITEM_getChar(itemid, ITEM_NAME) );
    CHAR_talkToCli( charaindex, -1, ansmsg, CHAR_COLORWHITE );
    for( p=effectarg ; *p != '\0'; ){
        int i;
        if(*p == SEPARATORI) p++;
        for(q=cmd, i=0; isalnum(*p) && i<ID_BUF_LEN_MAX; i++){
            *q++ = *p++;
        }
        *q = '\0';
        if(q == cmd || *p != SEPARATORI) return;
        p++;
        for(q=arg, i=0; isdigit(*p) && i<ID_BUF_LEN_MAX; i++){
            *q++ = *p++;
        }
        *q = '\0';
        value = strtol(arg, & q, 10);
        if(ITEM_medicineRaiseEffect(charaindex, cmd, value)){
            usedf = 1;
        }
    }
    if(usedf){
		CHAR_DelItem( charaindex, itemindex);
        CHAR_sendStatusString(charaindex, "P");

    }else
        fprint("ITEM_medicineUsed: error? cannot be used.\n");
#undef ID_BUF_LEN_MAX
#undef SEPARATORI
}

void ITEM_SandClockDetach( int charaindex , int itemid )
{
    int     i;
    if( !ITEM_CHECKINDEX( itemid )) return;
    for( i=0 ; i<CHAR_MAXITEMHAVE ; i++ ){
        if( CHAR_getItemIndex(charaindex,i ) == itemid ){
			CHAR_DelItem( charaindex, i);
            CHAR_talkToCli( charaindex, -1, "һж��ɳ©������Ȼ���ˣ�", CHAR_COLORWHITE );
            print( "deleted sand clock!\n" );
            break;
        }
    }
}

void ITEM_SandClockLogin( int charaindex )
{
	int i;
	int dTime;

	for( i=0 ; i<CHAR_MAXITEMHAVE; i++ ){
		int itemindex = CHAR_getItemIndex(charaindex,i);
		if( ITEM_getInt( itemindex, ITEM_ID ) != 29 )continue;
		if( ITEM_getInt( itemindex, ITEM_VAR4 ) == 0 )continue;
		dTime = NowTime.tv_sec - ITEM_getInt( itemindex, ITEM_VAR4 );
		ITEM_setInt( itemindex, ITEM_VAR3, ITEM_getInt( itemindex, ITEM_VAR3 ) + dTime );
	}

}

void ITEM_SandClockLogout( int charaindex )
{
	int i;
	for( i=0 ; i<CHAR_MAXITEMHAVE; i++ ){
		int itemindex = CHAR_getItemIndex(charaindex,i);
		if( ITEM_getInt( itemindex, ITEM_ID ) != 29 )continue;
		ITEM_setInt( itemindex, ITEM_VAR4, NowTime.tv_sec );
	}
}

// Arminius 7.2: Ra's amulet , remove "static"
/*static*/
BOOL ITEM_getArgument( char* argument , char* entryname, char* buf , int buflen )
{
    int     i;
    char    dividedbypipeline[512];
    for( i=1;  ; i++ ){
        BOOL   ret;
		ret = getStringFromIndexWithDelim( argument, "|", i, dividedbypipeline,
                                           sizeof(dividedbypipeline) );
        if( ret == TRUE ){
            int     tworet=1;
            char    first[512];
            tworet &= getStringFromIndexWithDelim( dividedbypipeline, ":", 1, first,sizeof(first) );
            tworet &= getStringFromIndexWithDelim( dividedbypipeline, ":", 2, buf,buflen );
            if( tworet != 0 )
                if( strcasecmp( first, entryname ) == 0 )
                    return TRUE;
        }else
            break;
    }

    return FALSE;
}

void ITEM_addTitleAttach( int charaindex, int itemindex )
{
    char    titlenumstring[256];
    int     titleindex;

    if( ITEM_CHECKINDEX(itemindex) == FALSE )return;

    if( ITEM_getArgument( ITEM_getChar(itemindex,ITEM_ARGUMENT),"addt",
                          titlenumstring, sizeof( titlenumstring) ) == FALSE ){
        print( "Can't find \"addt\" entry: %s\n",
               ITEM_getChar(itemindex,ITEM_ARGUMENT));
        return;
    }
    titleindex = atoi( titlenumstring );
    TITLE_addtitle( charaindex, titleindex );
    CHAR_sendStatusString( charaindex, "T" );
}

void ITEM_delTitleDetach( int charaindex, int itemindex )
{
    char    titlenumstring[256];
    int     titleindex;

    if( ITEM_CHECKINDEX(itemindex) == FALSE )return;

    if( ITEM_getArgument( ITEM_getChar(itemindex,ITEM_ARGUMENT),"delt",
                          titlenumstring, sizeof( titlenumstring) ) == FALSE ){
        print( "Can't find \"delt\" entry: %s\n",
               ITEM_getChar(itemindex,ITEM_ARGUMENT));
        return;
    }
    titleindex = atoi( titlenumstring );
    TITLE_deltitle( charaindex, titleindex );
    CHAR_sendStatusString( charaindex, "T" );
}

void ITEM_DeleteByWatched(
	int myobjindex, int moveobjindex, CHAR_ACTION act,
    int x, int y, int dir, int* opt, int optlen )
{

	int		itemindex, moveindex;
	char szBuffer[256]="";
	itemindex = OBJECT_getIndex(myobjindex);
    if( !ITEM_CHECKINDEX( itemindex )) return;
	if( OBJECT_getType( moveobjindex ) == OBJTYPE_CHARA ){
		moveindex = OBJECT_getIndex(moveobjindex);
    	if( CHAR_getInt( moveindex , CHAR_WHICHTYPE ) == CHAR_TYPEPLAYER ){
    		snprintf( szBuffer, sizeof( szBuffer ), "%s",
    							ITEM_getAppropriateName(itemindex) );
            CHAR_talkToCli( moveindex	, -1, "%s �����ˡ�", CHAR_COLORWHITE );
    	}
    }

	ITEM_endExistItemsOne(itemindex);
	CHAR_ObjectDelete(myobjindex);

}

void ITEM_DeleteTimeWatched(
	int objindex, int moveobjindex, CHAR_ACTION act,
    int x, int y, int dir, int* opt, int optlen)
{
	int	itemindex;
	int itemputtime;

	if( !CHECKOBJECTUSE(objindex) ){
		return;
	}
	itemindex = OBJECT_getIndex(objindex);
	if(!ITEM_CHECKINDEX(itemindex)){
		return;
	}
	itemputtime=ITEM_getInt(itemindex,ITEM_PUTTIME);
	if( !ITEM_CHECKINDEX( itemindex )) return;
#ifdef _MARKET_TRADE
	if( ITEM_getWorkInt( itemindex, ITEM_WORKTRADETYPE) == TRADETYPE_SELL )	{
		return;
	}
#endif
	if( (int)NowTime.tv_sec > (int)( itemputtime + getItemdeletetime() ) ) {
		if( ITEM_TimeDelCheck( itemindex ) == FALSE ){
			return ;
		}
		{
			LogItem(
					"NULL",
					"NULL",
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
					itemindex,
#else
		       		ITEM_getInt( itemindex, ITEM_ID ),
#endif
					"TiemDelete",
					OBJECT_getFloor( objindex ),
					OBJECT_getX( objindex ),
					OBJECT_getY( objindex ),
					ITEM_getChar( itemindex, ITEM_UNIQUECODE),
						ITEM_getChar( itemindex, ITEM_NAME),
						ITEM_getInt( itemindex, ITEM_ID)
				);
		}
		ITEM_endExistItemsOne(itemindex);
		CHAR_ObjectDelete(objindex);
	}
}

void ITEM_useEffectTohelos( int charaindex, int to_charaindex, int haveitemindex)
{
	char	buf[64];
	char	msgbuf[64];
	int		ret;
	int		itemindex;
	int		cutrate, limitcount;
    int		per;
    int		sendcharaindex = charaindex;

    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

    CHAR_setItemIndex(charaindex, haveitemindex ,-1);
    CHAR_sendItemDataOne( charaindex, haveitemindex);
	ret = getStringFromIndexWithDelim( ITEM_getChar(itemindex, ITEM_ARGUMENT) ,
										"|", 1, buf, sizeof( buf));
	if( ret != TRUE ) {
		{
			LogItem(
				CHAR_getChar( charaindex, CHAR_NAME ),
				CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
				itemindex,
#else
       			ITEM_getInt( itemindex, ITEM_ID ),
#endif
				"FieldErrorUse",
		       	CHAR_getInt( charaindex,CHAR_FLOOR),
				CHAR_getInt( charaindex,CHAR_X ),
        		CHAR_getInt( charaindex,CHAR_Y ),
				ITEM_getChar( itemindex, ITEM_UNIQUECODE),
						ITEM_getChar( itemindex, ITEM_NAME),
						ITEM_getInt( itemindex, ITEM_ID)
			);
		}
	    ITEM_endExistItemsOne( itemindex);
		return;
	}
	cutrate = atoi( buf);
	if( cutrate < 0 ) cutrate = 0;
	ret = getStringFromIndexWithDelim( ITEM_getChar(itemindex, ITEM_ARGUMENT) ,
										"|", 2, buf, sizeof( buf));
	if( ret != TRUE ) {
		{
			LogItem(
				CHAR_getChar( charaindex, CHAR_NAME ),
				CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
				itemindex,
#else
       			ITEM_getInt( itemindex, ITEM_ID ),
#endif
				"FieldUse",
		       	CHAR_getInt( charaindex,CHAR_FLOOR),
				CHAR_getInt( charaindex,CHAR_X ),
        		CHAR_getInt( charaindex,CHAR_Y ),
				ITEM_getChar( itemindex, ITEM_UNIQUECODE),
						ITEM_getChar( itemindex, ITEM_NAME),
						ITEM_getInt( itemindex, ITEM_ID)
			);
		}
	    ITEM_endExistItemsOne( itemindex);
		return;
	}
	limitcount = atoi( buf);
	if( limitcount < 0) limitcount = 0;
	if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE) == CHAR_PARTY_CLIENT) {
		sendcharaindex = CHAR_getWorkInt( charaindex, CHAR_WORKPARTYINDEX1);
	}
	CHAR_setWorkInt( sendcharaindex, CHAR_WORK_TOHELOS_CUTRATE, cutrate);
	CHAR_setWorkInt( sendcharaindex, CHAR_WORK_TOHELOS_COUNT, limitcount);

    snprintf( msgbuf, sizeof(msgbuf),
              "ץ����%s ��", ITEM_getChar(itemindex, ITEM_NAME) );
    CHAR_talkToCli( charaindex, -1, msgbuf, CHAR_COLORWHITE );

    if( sendcharaindex != charaindex ) {
	    snprintf( msgbuf, sizeof(msgbuf),
	              "%s ץ���� %s�� ",
	              CHAR_getChar( charaindex, CHAR_NAME),
	              ITEM_getChar( itemindex, ITEM_NAME) );
	    CHAR_talkToCli( sendcharaindex, -1, msgbuf, CHAR_COLORWHITE );
	}

	{
		LogItem(
			CHAR_getChar( charaindex, CHAR_NAME ), 
			CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
			itemindex,
#else
       		ITEM_getInt( itemindex, ITEM_ID ),
#endif
			"FieldUse",
	       	CHAR_getInt( charaindex,CHAR_FLOOR),
			CHAR_getInt( charaindex,CHAR_X ),
        	CHAR_getInt( charaindex,CHAR_Y ),
			ITEM_getChar( itemindex, ITEM_UNIQUECODE),
						ITEM_getChar( itemindex, ITEM_NAME),
						ITEM_getInt( itemindex, ITEM_ID)
		);
	}
	ITEM_endExistItemsOne( itemindex);
	per = ENCOUNT_getEncountPercentMin( sendcharaindex,
									CHAR_getInt( sendcharaindex, CHAR_FLOOR),
									CHAR_getInt( sendcharaindex, CHAR_X),
									CHAR_getInt( sendcharaindex, CHAR_Y));
	if( per != -1) {
		CHAR_setWorkInt( sendcharaindex, CHAR_WORKENCOUNTPROBABILITY_MIN, per);
	}
	per = ENCOUNT_getEncountPercentMax( sendcharaindex,
									CHAR_getInt( sendcharaindex, CHAR_FLOOR),
									CHAR_getInt( sendcharaindex, CHAR_X),
									CHAR_getInt( sendcharaindex, CHAR_Y));
	if( per != -1) {
		CHAR_setWorkInt( sendcharaindex, CHAR_WORKENCOUNTPROBABILITY_MAX, per);
	}
	CHAR_sendStatusString( sendcharaindex, "E" );
}

void ITEM_dropMic( int charaindex , int itemindex )
{
    if( !ITEM_CHECKINDEX( itemindex )) return;

	CHAR_setWorkInt( charaindex, CHAR_WORKFLG,
		CHAR_getWorkInt( charaindex, CHAR_WORKFLG ) & ~WORKFLG_MICMODE );
}

void ITEM_useMic_Field( int charaindex, int to_charaindex, int haveitemindex )
{
	if( CHAR_getWorkInt( charaindex, CHAR_WORKFLG ) & WORKFLG_MICMODE ){
		CHAR_setWorkInt( charaindex, CHAR_WORKFLG,
			CHAR_getWorkInt( charaindex, CHAR_WORKFLG ) & ~WORKFLG_MICMODE );
		CHAR_talkToCli( charaindex, -1, "����˷��趨ΪOFF��", CHAR_COLORWHITE);
	}else{
		CHAR_setWorkInt( charaindex, CHAR_WORKFLG,
			CHAR_getWorkInt( charaindex, CHAR_WORKFLG ) | WORKFLG_MICMODE );
		CHAR_talkToCli( charaindex, -1, "����˷��趨ΪON��", CHAR_COLORWHITE);
	}
}
#if 1
char *aszHealStringByOwn[] = {
	"%s���;����ظ�%d",
	"%s�������ظ�%d",
	"%s����������%d",
	"%s���ҳ϶�����%d",
	""
};

char *aszDownStringByOwn[] = {
	"%s���;�������%d",
	"%s����������%d",
	"%s�������½�%d",
	"%s���ҳ϶��½�%d",
	""
};

char *aszHealStringByOther[] = {
	"����%s%s���;����ظ�%d",
	"����%s%s�������ظ�%d",
	"����%s%s����������%d",
	"����%s%s���ҳ϶�����%d",
	""
};

char *aszDownStringByOther[] = {
	"����%s%s���;�������%d",
	"����%s%s����������%d",
	"����%s%s����������%d",
	"����%s%s���ҳ϶ȼ���%d",
	""
};

char *aszKeyString[] = {"��", "��", "��", "��", ""};
int  aHealInt[] = { CHAR_HP, CHAR_MP, CHAR_CHARM, CHAR_VARIABLEAI, -1 };
int  aHealMaxWork[] = { CHAR_WORKMAXHP, CHAR_WORKMAXMP, -1, -1,  -1 };

void ITEM_useRecovery_Field(
	int charaindex,
	int toindex,
	int haveitemindex
){
	int work, workmax, workmin;
	int power[BD_KIND_END] = {0,0,0},
		prevhp = 0,
		workhp = 0,
		recovery[BD_KIND_END] = {0,0,0};
	int itemindex, kind = BD_KIND_HP, HealFlg = 0, j;
	char *p = NULL, *arg, msgbuf[256];
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if( !ITEM_CHECKINDEX(itemindex) ) return;
	if( CHAR_CHECKINDEX( toindex ) == FALSE )return ;
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );
	if( arg == NULL ) return;

#ifdef _ITEM_UNBECOMEPIG
    if( (p = strstr( arg, "����" )) != NULL ){
        if( CHAR_getInt( toindex, CHAR_BECOMEPIG ) > -1 ){
		    CHAR_setInt( toindex, CHAR_BECOMEPIG, -1 );
			CHAR_complianceParameter( toindex );
		    CHAR_sendCToArroundCharacter( CHAR_getWorkInt( toindex , CHAR_WORKOBJINDEX ));
		    CHAR_send_P_StatusString( toindex , CHAR_P_STRING_BASEBASEIMAGENUMBER);
		    CHAR_talkToCli( toindex,-1,"������ʧЧ�ˡ�",CHAR_COLORWHITE);
       		CHAR_DelItemMess( charaindex, haveitemindex, 0);
		}
		return;
	}
#endif
#ifdef _ITEM_LVUPUP
	if( (p = strstr( arg, "LVUPUP" ) ) != NULL ){
		if (CHAR_getInt(toindex, CHAR_WHICHTYPE)==CHAR_TYPEPET){
		    int pidx=0,lvup=0,lv=0;
			sprintf( msgbuf, "%s", p+7 );
            pidx = atoi( strtok( msgbuf, " " ) );
			if( pidx != CHAR_getInt( toindex, CHAR_PETID) || CHAR_getInt(toindex, CHAR_LIMITLEVEL) == 0 ){
			    CHAR_talkToCli( charaindex,-1,"�޷�ʹ��",CHAR_COLORWHITE);
				return;
			}
			lv = CHAR_getInt( toindex, CHAR_LV );
			if( lv < 10 || lv >= 140 ){
			    CHAR_talkToCli( charaindex,-1,"Ŀǰ�ȼ��޷�ʹ��",CHAR_COLORWHITE);
				return;
			}
			if( (p = strstr( arg, "��" )) != NULL )
					lvup=2;
		    if( (p = strstr( arg, "ˮ" )) != NULL )
					lvup=3;
            if( (p = strstr( arg, "��" )) != NULL )
					lvup=0;
			if( (p = strstr( arg, "��" )) != NULL )
					lvup=1;		    
			if( lv%4 != lvup || CHAR_getInt(toindex, CHAR_LIMITLEVEL)-lv >= 1 ){//
			    CHAR_talkToCli( charaindex,-1,"�Ƴ���ӡʧ��",CHAR_COLORWHITE);
			    CHAR_DelItemMess( charaindex, haveitemindex, 0);
				return;
			}
			CHAR_setInt(toindex, CHAR_LIMITLEVEL, CHAR_getInt(toindex, CHAR_LIMITLEVEL)+1);	
			CHAR_talkToCli( charaindex,-1,"��ӡħ���������ߵȼ�����",CHAR_COLORWHITE);    
			if( CHAR_getInt(toindex, CHAR_LIMITLEVEL) == 140 ){
				CHAR_setInt(toindex, CHAR_LIMITLEVEL, 0);
			    CHAR_talkToCli( charaindex,-1,"��ӡħ������",CHAR_COLORWHITE);    	
			}
			CHAR_DelItemMess( charaindex, haveitemindex, 0);
			CHAR_complianceParameter( toindex );
		    CHAR_sendCToArroundCharacter( CHAR_getWorkInt( toindex , CHAR_WORKOBJINDEX ));
		}
		return;
	}
	if( (p = strstr( arg, "UPUPLV" ) ) != NULL ){
		if (CHAR_getInt(toindex, CHAR_WHICHTYPE)==CHAR_TYPEPET){
		    int pidx=0,lvup=0,lv=0;
			sprintf( msgbuf, "%s", p+7 );
			pidx = atoi( strtok( msgbuf, " " ) );
			if( pidx != CHAR_getInt( toindex, CHAR_PETID) || CHAR_getInt(toindex, CHAR_LIMITLEVEL) == 0 ){
			    CHAR_talkToCli( charaindex,-1,"�޷�ʹ��",CHAR_COLORWHITE);
				return;
			}
			lv = CHAR_getInt( toindex, CHAR_LV );
			if( lv < 125 || lv > 140 ){
			    CHAR_talkToCli( charaindex,-1,"Ŀǰ�ȼ��޷�ʹ��",CHAR_COLORWHITE);
				return;
			}
			CHAR_setInt(toindex, CHAR_LIMITLEVEL, 0);
			CHAR_talkToCli( charaindex,-1,"��ӡħ������",CHAR_COLORWHITE);    	
			CHAR_DelItemMess( charaindex, haveitemindex, 0);
			CHAR_complianceParameter( toindex );
		    CHAR_sendCToArroundCharacter( CHAR_getWorkInt( toindex , CHAR_WORKOBJINDEX ));
		}
		return;
	}
#endif
#ifdef _ITEM_PROPERTY
    if( (p = strstr( arg, "PROPERTY" ) ) != NULL ){
		//print("��ˮ���:%s", arg );
		if (CHAR_getInt(toindex, CHAR_WHICHTYPE)!=CHAR_TYPEPLAYER)
            return;
		if( (p = strstr( arg, "+" ) ) != NULL ){//��ת���� ��ת
	        if( CHAR_getInt( toindex, CHAR_EARTHAT ) == 100 )
                CHAR_setInt( toindex, CHAR_EARTHAT, CHAR_getInt( toindex, CHAR_EARTHAT )-10 ),
			    CHAR_setInt( toindex, CHAR_WATERAT, CHAR_getInt( toindex, CHAR_WATERAT )+10 );
		    else if( CHAR_getInt( toindex, CHAR_WATERAT ) == 100 )
                CHAR_setInt( toindex, CHAR_WATERAT, CHAR_getInt( toindex, CHAR_WATERAT )-10 ),
			    CHAR_setInt( toindex, CHAR_FIREAT, CHAR_getInt( toindex, CHAR_FIREAT )+10 );
            else if( CHAR_getInt( toindex, CHAR_FIREAT ) == 100 )
                CHAR_setInt( toindex, CHAR_FIREAT, CHAR_getInt( toindex, CHAR_FIREAT )-10 ),
			    CHAR_setInt( toindex, CHAR_WINDAT, CHAR_getInt( toindex, CHAR_WINDAT )+10 );
            else if( CHAR_getInt( toindex, CHAR_WINDAT ) == 100 )
                CHAR_setInt( toindex, CHAR_WINDAT, CHAR_getInt( toindex, CHAR_WINDAT )-10 ),
			    CHAR_setInt( toindex, CHAR_EARTHAT, CHAR_getInt( toindex, CHAR_EARTHAT )+10 );
			else if( CHAR_getInt( toindex, CHAR_EARTHAT ) > 0 && CHAR_getInt( toindex, CHAR_WATERAT ) > 0 )
                CHAR_setInt( toindex, CHAR_EARTHAT, CHAR_getInt( toindex, CHAR_EARTHAT )-10 ),
			    CHAR_setInt( toindex, CHAR_WATERAT, CHAR_getInt( toindex, CHAR_WATERAT )+10 );
            else if( CHAR_getInt( toindex, CHAR_WATERAT ) > 0 && CHAR_getInt( toindex, CHAR_FIREAT ) > 0 )
                CHAR_setInt( toindex, CHAR_WATERAT, CHAR_getInt( toindex, CHAR_WATERAT )-10 ),
			    CHAR_setInt( toindex, CHAR_FIREAT, CHAR_getInt( toindex, CHAR_FIREAT )+10 );
		    else if( CHAR_getInt( toindex, CHAR_FIREAT ) > 0 && CHAR_getInt( toindex, CHAR_WINDAT ) > 0 )
                CHAR_setInt( toindex, CHAR_FIREAT, CHAR_getInt( toindex, CHAR_FIREAT )-10 ),
			    CHAR_setInt( toindex, CHAR_WINDAT, CHAR_getInt( toindex, CHAR_WINDAT )+10 );
            else if( CHAR_getInt( toindex, CHAR_WINDAT ) > 0 && CHAR_getInt( toindex, CHAR_EARTHAT ) > 0 )
                CHAR_setInt( toindex, CHAR_WINDAT, CHAR_getInt( toindex, CHAR_WINDAT )-10 ),
			    CHAR_setInt( toindex, CHAR_EARTHAT, CHAR_getInt( toindex, CHAR_EARTHAT )+10 );
		}
        if( (p = strstr( arg, "-" ) ) != NULL ){//��ת���� ��ת
			if( CHAR_getInt( toindex, CHAR_EARTHAT ) == 100 )
                CHAR_setInt( toindex, CHAR_EARTHAT, CHAR_getInt( toindex, CHAR_EARTHAT )-10 ),
			    CHAR_setInt( toindex, CHAR_WINDAT, CHAR_getInt( toindex, CHAR_WINDAT )+10 );
		    else if( CHAR_getInt( toindex, CHAR_WATERAT ) == 100 )
                CHAR_setInt( toindex, CHAR_WATERAT, CHAR_getInt( toindex, CHAR_WATERAT )-10 ),
			    CHAR_setInt( toindex, CHAR_EARTHAT, CHAR_getInt( toindex, CHAR_EARTHAT )+10 );
            else if( CHAR_getInt( toindex, CHAR_FIREAT ) == 100 )
                CHAR_setInt( toindex, CHAR_FIREAT, CHAR_getInt( toindex, CHAR_FIREAT )-10 ),
			    CHAR_setInt( toindex, CHAR_WATERAT, CHAR_getInt( toindex, CHAR_WATERAT )+10 );
            else if( CHAR_getInt( toindex, CHAR_WINDAT ) == 100 )
                CHAR_setInt( toindex, CHAR_WINDAT, CHAR_getInt( toindex, CHAR_WINDAT )-10 ),
			    CHAR_setInt( toindex, CHAR_FIREAT, CHAR_getInt( toindex, CHAR_FIREAT )+10 );
		    else if( CHAR_getInt( toindex, CHAR_EARTHAT ) > 0 && CHAR_getInt( toindex, CHAR_WATERAT ) > 0 )
                CHAR_setInt( toindex, CHAR_EARTHAT, CHAR_getInt( toindex, CHAR_EARTHAT )+10 ),
			    CHAR_setInt( toindex, CHAR_WATERAT, CHAR_getInt( toindex, CHAR_WATERAT )-10 );
            else if( CHAR_getInt( toindex, CHAR_WATERAT ) > 0 && CHAR_getInt( toindex, CHAR_FIREAT ) > 0 )
                CHAR_setInt( toindex, CHAR_WATERAT, CHAR_getInt( toindex, CHAR_WATERAT )+10 ),
			    CHAR_setInt( toindex, CHAR_FIREAT, CHAR_getInt( toindex, CHAR_FIREAT )-10 );
		    else if( CHAR_getInt( toindex, CHAR_FIREAT ) > 0 && CHAR_getInt( toindex, CHAR_WINDAT ) > 0 )
                CHAR_setInt( toindex, CHAR_FIREAT, CHAR_getInt( toindex, CHAR_FIREAT )+10 ),
			    CHAR_setInt( toindex, CHAR_WINDAT, CHAR_getInt( toindex, CHAR_WINDAT )-10 );
            else if( CHAR_getInt( toindex, CHAR_WINDAT ) > 0 && CHAR_getInt( toindex, CHAR_EARTHAT ) > 0 )
                CHAR_setInt( toindex, CHAR_WINDAT, CHAR_getInt( toindex, CHAR_WINDAT )+10 ),
			    CHAR_setInt( toindex, CHAR_EARTHAT, CHAR_getInt( toindex, CHAR_EARTHAT )-10 );
		}
		CHAR_DelItemMess( charaindex, haveitemindex, 0);
		CHAR_complianceParameter( toindex );
		CHAR_sendCToArroundCharacter( CHAR_getWorkInt( toindex , CHAR_WORKOBJINDEX ));
		CHAR_send_P_StatusString( toindex ,
					CHAR_P_STRING_EARTH	|
					CHAR_P_STRING_WATER	|
					CHAR_P_STRING_FIRE	|
					CHAR_P_STRING_WIND	
				);
		return;
	}
#endif
#ifdef _ITEM_ADDPETEXP
	if( (p = strstr( arg, "GETEXP" )) != NULL ){
		if (CHAR_getInt(toindex, CHAR_WHICHTYPE)==CHAR_TYPEPET){
			getStringFromIndexWithDelim( arg, "|", 2, msgbuf,sizeof( msgbuf));//������
			if( atoi(msgbuf) == CHAR_getInt( toindex, CHAR_PETID) ){
				getStringFromIndexWithDelim( arg, "|", 3, msgbuf,sizeof( msgbuf));//�ȼ�����(�ȼ������ſ���)
				if( CHAR_getInt( toindex, CHAR_LV ) >= atoi(msgbuf) ){
					getStringFromIndexWithDelim( arg, "|", 4, msgbuf,sizeof( msgbuf));//����ֵ����
					if( CHAR_getInt( toindex, CHAR_LV) < CHAR_MAXUPLEVEL ){
						int UpLevel = 0;
						CHAR_setWorkInt( toindex, CHAR_WORKGETEXP, atoi(msgbuf) );//�ش�CHAR_WORKGETEXP
						CHAR_AddMaxExp( toindex, CHAR_getWorkInt( toindex, CHAR_WORKGETEXP ) );
						sprintf( msgbuf,"��ʯͷ���治֪����ʲ�����Ŀ����ҿ�����(���Ӿ���%d)", CHAR_getWorkInt( toindex, CHAR_WORKGETEXP ) );
						CHAR_talkToCli( charaindex,-1,msgbuf,CHAR_COLORWHITE);  
						UpLevel = CHAR_LevelUpCheck( toindex , charaindex);
						if( UpLevel > 0 ){
							if( getBattleDebugMsg( ) != 0 ){
								snprintf( msgbuf, sizeof(msgbuf),
											"(%s) ������ %d",
											CHAR_getUseName( toindex ),
											CHAR_getInt( toindex, CHAR_LV ) );
							BATTLE_talkToCli( charaindex, msgbuf, CHAR_COLORYELLOW );
							}
						}
						for( j = 0; j < UpLevel; j ++ ){
							CHAR_PetLevelUp( toindex );
							CHAR_PetAddVariableAi( toindex, AI_FIX_PETLEVELUP );
						}
						CHAR_complianceParameter( toindex );
					}
					else{
						CHAR_talkToCli( charaindex,-1,"����ʹ��",CHAR_COLORWHITE);  
					}
					/*CHAR_send_P_StatusString(  charindex, CHAR_P_STRING_DUELPOINT|
					CHAR_P_STRING_TRANSMIGRATION| CHAR_P_STRING_RIDEPET|
					CHAR_P_STRING_BASEBASEIMAGENUMBER| CHAR_P_STRING_GOLD|
					CHAR_P_STRING_EXP| CHAR_P_STRING_LV| CHAR_P_STRING_HP|CHAR_P_STRING_LEARNRIDE);
				*/
				}
				else
					CHAR_talkToCli( charaindex,-1,"��ʯͷ....(�޷��б��޷����Ӿ���)",CHAR_COLORWHITE);  
			}
			CHAR_DelItemMess( charaindex, haveitemindex, 0);
			CHAR_complianceParameter( toindex );
			CHAR_sendCToArroundCharacter( CHAR_getWorkInt( toindex , CHAR_WORKOBJINDEX ));
			return;
		}
		else{
			CHAR_talkToCli( charaindex,-1,"����ʹ��",CHAR_COLORWHITE);  
		}
	}
#endif
	for( j = 0; j < BD_KIND_END; j ++ ){
		power[j] = 0;
		recovery[j] = 0;
	}
	if( (p = strstr( arg, "ȫ" )) != NULL ){
		HealFlg = ( 1 << BD_KIND_HP );
		if( CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPET ){
		}else{
			HealFlg |= ( 1 << BD_KIND_MP );
		}
		power[BD_KIND_HP] = 10000000;
		power[BD_KIND_MP] = 100;
	}
	kind = BD_KIND_HP;
	if( (p = strstr( arg, aszKeyString[kind] )) != NULL ){
		HealFlg |= ( 1 << kind );
		if( sscanf( p+2, "%d", &work ) != 1 ){
			power[kind] = 1;
		}else{
			power[kind] = RAND( (work*0.9), (work*1.1) );
		}
#ifndef _MAGIC_REHPAI
		power[kind] *= GetRecoveryRate( toindex );
#endif
	}
	kind = BD_KIND_MP;
	if( (p = strstr( arg, aszKeyString[kind] )) != NULL
	&&  CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER){
		HealFlg |= ( 1 << kind );
		if( sscanf( p+2, "%d", &work ) != 1 ){
			power[kind] = 1;
		}else{
			power[kind] = RAND( (work*0.9), (work*1.1) );
		}
	}
	kind = BD_KIND_AI;
	if( (p = strstr( arg, aszKeyString[kind] )) != NULL
		&& CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPET){
		HealFlg |= ( 1 << kind );
		if( sscanf( p+2, "%d", &work ) != 1 ){
			power[kind] = 1;
		}else{
			power[kind] = RAND( (work*0.9), (work*1.1) );
		}
		power[kind] *= 100;
	}
	kind = BD_KIND_CHARM;
	if( (p = strstr( arg, aszKeyString[kind] )) != NULL
	&& CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER	){
		HealFlg |= ( 1 << kind );
		if( sscanf( p+2, "%d", &work ) != 1 ){
			power[kind] = 1;
		}else{
			power[kind] = RAND( (work*0.9), (work*1.1) );
		}
	}
	if( HealFlg == 0 )return;
#ifdef _TYPE_TOXICATION
	if( CHAR_CanCureFlg( toindex, "HP") == FALSE )return;
#endif

	for( j = 0; j < BD_KIND_END; j ++ ){
		if( ( HealFlg & ( 1 << j ) ) == 0 )continue;
		prevhp = CHAR_getInt( toindex, aHealInt[j] );
		workhp = prevhp + (int)power[j];

		if( j == BD_KIND_CHARM ){
			workmax = 100;
			workmin = 0;
		}else
		if( j == BD_KIND_AI ){
			workmax = 10000;
			workmin = -10000;
		}else{
			workmax = CHAR_getWorkInt( toindex, aHealMaxWork[j] );
			workmin = 1;
		}
		workhp = min( workhp, workmax );
		workhp = max( workhp, workmin );
		CHAR_setInt( toindex, aHealInt[j], workhp );
		recovery[j] = workhp - prevhp;
		if( j == BD_KIND_AI ){
			recovery[j] *= 0.01;
		}
	}
	CHAR_complianceParameter( toindex );

	if( CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER) {
		if( charaindex != toindex ) {
			CHAR_send_P_StatusString( toindex, CHAR_P_STRING_HP|CHAR_P_STRING_MP|CHAR_P_STRING_CHARM);
		}
	}
	CHAR_send_P_StatusString( charaindex, CHAR_P_STRING_HP|CHAR_P_STRING_MP|CHAR_P_STRING_CHARM);
	if( CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER &&
		CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE) != CHAR_PARTY_NONE ){
		CHAR_PartyUpdate( toindex, CHAR_N_STRING_HP|CHAR_N_STRING_MP );
	}
	if( CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPET ){
		int		i;
		for( i = 0; i < CHAR_MAXPETHAVE; i ++ ) {
			int workindex = CHAR_getCharPet( charaindex, i );
			if( workindex == toindex ){
				CHAR_send_K_StatusString( charaindex, i, CHAR_K_STRING_HP|CHAR_K_STRING_AI);
			}
		}
	}


	for( j = 0; j < BD_KIND_END; j ++ ){
		if( ( HealFlg & ( 1 << j ) ) == 0 )continue;
		if( charaindex != toindex) {
			if( power[j] >= 0 ){
				snprintf( msgbuf, sizeof( msgbuf),
					aszHealStringByOwn[j],
					CHAR_getUseName( toindex ), recovery[j] );
			}else{
				snprintf( msgbuf, sizeof( msgbuf),
					aszDownStringByOwn[j],
					CHAR_getUseName( toindex ), -recovery[j] );
			}
			CHAR_talkToCli( charaindex, -1, msgbuf, CHAR_COLORWHITE);
			if( CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER) {
				if( power[j] >= 0 ){
					snprintf( msgbuf, sizeof( msgbuf),
						aszHealStringByOther[j],
						CHAR_getUseName( charaindex ),
						CHAR_getUseName( toindex ), recovery[j] );
				}else{
					snprintf( msgbuf, sizeof( msgbuf),
						aszDownStringByOther[j],
						CHAR_getUseName( charaindex ),
						CHAR_getUseName( toindex ), -recovery[j] );
				}
				CHAR_talkToCli( toindex, -1, msgbuf, CHAR_COLORWHITE);
			}
		}else {
			if( power[j] >= 0 ){
				snprintf( msgbuf, sizeof( msgbuf),
					aszHealStringByOwn[j],
					CHAR_getUseName( charaindex ), recovery[j] );
			}else{
				snprintf( msgbuf, sizeof( msgbuf),
					aszDownStringByOwn[j],
					CHAR_getUseName( charaindex ), -recovery[j] );
			}
			CHAR_talkToCli( charaindex, -1, msgbuf, CHAR_COLORWHITE);
		}
	}
	{
		LogItem(
			CHAR_getChar( charaindex, CHAR_NAME ),
			CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
			itemindex,
#else
       		ITEM_getInt( itemindex, ITEM_ID ),
#endif
			"FieldUse",
	       	CHAR_getInt( charaindex,CHAR_FLOOR),
			CHAR_getInt( charaindex,CHAR_X ),
        	CHAR_getInt( charaindex,CHAR_Y ),
			ITEM_getChar( itemindex, ITEM_UNIQUECODE),
						ITEM_getChar( itemindex, ITEM_NAME),
						ITEM_getInt( itemindex, ITEM_ID)
		);
	}
	CHAR_DelItemMess( charaindex, haveitemindex, 0);

}

#endif

void ITEM_useRecovery( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return ;
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
		if( battlemode ){
			ITEM_useRecovery_Battle( charaindex, toindex, haveitemindex );
		}else{
			ITEM_useRecovery_Field(	charaindex, toindex, haveitemindex );
		}

}

#ifdef _ITEM_MAGICRECOVERY
void ITEM_useMRecovery( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return ;
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
		if( battlemode ){
			ITEM_useMRecovery_Battle( charaindex, toindex, haveitemindex );
		}else{
//			ITEM_useRecovery_Field(	charaindex, toindex, haveitemindex );
		}

}

#endif

#ifdef _ITEM_USEMAGIC
void ITEM_useMagic( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return ;
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
		if( battlemode ){
			ITEM_useMagic_Battle( charaindex, toindex, haveitemindex );
		}
}
#endif

#ifdef _PET_LIMITLEVEL
void ITEM_useOtherEditBase( int charaindex, int toindex, int haveitemindex)
{
	int itemindex,i;
	int work[4];
	int num=-1,type;
	int LevelUpPoint,petrank;
	char buf1[256];
	char buf2[][32]={"�����ɳ���","�;����ɳ���","�ٶȳɳ���","�����ɳ���","����"};
	char buf3[][32]={"������","��Ϊ���","��Ϊ����"};
	if( !CHAR_CHECKINDEX( charaindex ) )  return;
	if( !CHAR_CHECKINDEX( toindex ) )  return; 
#define RAND(x,y)   ((x-1)+1+ (int)( (double)(y-(x-1))*rand()/(RAND_MAX+1.0)) )
		itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
		if( !ITEM_CHECKINDEX(itemindex) )  return;
		if( CHAR_getInt( toindex, CHAR_PETID) == 718 
#ifdef _PET_2LIMITLEVEL
			|| CHAR_getInt( toindex, CHAR_PETID) == 401 
#endif
			)	{

			int maxnums=50;

			if( CHAR_getInt( toindex,CHAR_LV ) < 74 )	{
				sprintf(buf1,"���ҵ��𣿺���������  ร����������˱仯��");
				CHAR_talkToCli( charaindex, toindex, buf1, CHAR_COLORWHITE);
				num = ITEM_MODIFYATTACK;
				LevelUpPoint = CHAR_getInt( toindex, CHAR_ALLOCPOINT );
				petrank = CHAR_getInt( toindex, CHAR_PETRANK );
				work[3] =(( LevelUpPoint >> 24 ) & 0xFF);
				work[0] = (( LevelUpPoint >> 16 ) & 0xFF);
				work[1] = (( LevelUpPoint >> 8 ) & 0xFF);
				work[2] = (( LevelUpPoint >> 0 ) & 0xFF);
				for( i=0; i<4; i++)	{
					type = ITEM_getInt( itemindex, (num + i));
					work[i] += type;
					strcpy( buf1,"\0");
					if( work[i] > maxnums )	{
						sprintf(buf1,"%s �Ѿ��ﵽ����ˡ�", buf2[i]);
						work[i] = maxnums;
					}else if( work[i] < 0 )	{
						sprintf(buf1,"%s �Ѿ�Ϊ���ˡ�", buf2[i]);
						work[i] = 0;
					}else	{
						if( type > 0 )	{
							if( type > 2 )
								sprintf(buf1,"%s %s %s", buf2[i], buf3[0], "��");
							else
								sprintf(buf1,"%s %s %s", buf2[i], buf3[1], "��");
						}else if( type < 0 ){
							sprintf(buf1,"%s %s %s", buf2[i], buf3[2], "��");	
						}
					}
					if( strcmp( buf1, "\0"))	{
						CHAR_talkToCli( charaindex, toindex, buf1, CHAR_COLORWHITE);
					}
				}
				
				LevelUpPoint = ( work[3]<< 24) + ( work[0]<< 16) + ( work[1]<< 8) + ( work[2]<< 0);
				CHAR_setInt( toindex, CHAR_ALLOCPOINT, LevelUpPoint);
				CHAR_setInt( toindex, CHAR_PETRANK, petrank);
				LogPetPointChange(
					CHAR_getChar( charaindex, CHAR_NAME ),
					CHAR_getChar( charaindex, CHAR_CDKEY ),
					CHAR_getChar( charaindex, CHAR_NAME),
					toindex, 4,
					CHAR_getInt( toindex, CHAR_LV),
					"item_use",
					CHAR_getInt( charaindex, CHAR_FLOOR),
					CHAR_getInt( charaindex, CHAR_X ),
					CHAR_getInt( charaindex, CHAR_Y )
					);
			}
				
		}else	{
			sprintf(buf1,"������");
		}
		CHAR_DelItem( charaindex, haveitemindex);
		return;
}
#endif

void ITEM_useStatusChange( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return ; //����  
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
	if( battlemode ){
		ITEM_useStatusChange_Battle( charaindex, toindex, haveitemindex );
	}else{
	}

}

void ITEM_useStatusRecovery( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return ; //����  
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
	if( battlemode ){
		ITEM_useStatusRecovery_Battle( charaindex, toindex, haveitemindex );
	}else{
	}
}

void ITEM_useMagicDef( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return;
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
	if( battlemode ){
		ITEM_useMagicDef_Battle( charaindex, toindex, haveitemindex );
	}else{
	}

}

void ITEM_useParamChange( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return ;
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
		if( battlemode ){
			ITEM_useParamChange_Battle( charaindex, toindex, haveitemindex );
		}
}

void ITEM_useFieldChange( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return ;
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
		if( battlemode ){
			ITEM_useFieldChange_Battle( charaindex, toindex, haveitemindex );
		}
}

void ITEM_useAttReverse( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return;
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
	if( battlemode ){
		ITEM_useAttReverse_Battle( charaindex, toindex, haveitemindex );
	}else{
	}
}

void ITEM_useMic( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return;
	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
		if( battlemode ){
		}else{
			ITEM_useMic_Field( charaindex, toindex, haveitemindex );
		}

}

void ITEM_useCaptureUp( int charaindex, int toindex, int haveitemindex)
{
	int battlemode;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return ; //����  

	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	if( battlemode == BATTLE_CHARMODE_INIT ){
	}else
	if( battlemode ){
		ITEM_useCaptureUp_Battle( charaindex, toindex, haveitemindex );
	}else{
	}

}

#ifdef _PETSKILL_CANNEDFOOD
static void ITEM_usePetSkillCanned_PrintWindow( int charaindex, int flg)
{
	int fd;
	char	message[256], buf[2048];
	if( !CHAR_CHECKINDEX( charaindex )) return;
	fd = getfdFromCharaIndex( charaindex);
	if( fd == - 1 ) return;

	memset( message, 0, sizeof(message));
	sprintf( message, "%d", flg);
	lssproto_WN_send( fd, WINDOWS_MESSAGETYPE_PETSKILLSHOW, 
					WINDOW_BUTTONTYPE_NONE,
					ITEM_WINDOWTYPE_SELECTPETSKILL_SELECT,
					-1,
					makeEscapeString( message, buf, sizeof( buf)));
}

void ITEM_usePetSkillCanned_WindowResult( int charaindex, int seqno, int select, char * data)
{
	int itemindex=-1, itemNo, petindex=-1, petNo;
	int SkillNo, SkillID;
	char buf1[256];
	char *skillarg=NULL;

	petNo = CHAR_getWorkInt( charaindex, CHAR_WORKRENAMEITEMNUM);
	itemNo = CHAR_getWorkInt( charaindex, CHAR_WORKRENAMEITEMINDEX);


	itemindex = CHAR_getItemIndex( charaindex, itemNo);
	if( !ITEM_CHECKINDEX( itemindex) )	return;
	petindex = CHAR_getCharPet( charaindex, petNo);
	if( !CHAR_CHECKINDEX(petindex) ) return;
	SkillNo = atoi( data);
	if( SkillNo < 0 || SkillNo >= CHAR_MAXPETSKILLHAVE ) return;

	skillarg = ITEM_getChar( itemindex, ITEM_ARGUMENT);
	SkillID = atoi( skillarg);

#ifdef _CFREE_petskill
	if( NPC_CHECKFREEPETSKILL( charaindex, petindex, SkillID) == FALSE ){
		CHAR_talkToCli( charaindex, -1, "�ó����޷�ѧϰ����ܣ�", CHAR_COLORYELLOW);
		return;
	}
#endif

	CHAR_setPetSkill( petindex, SkillNo, SkillID);
	memset( buf1, 0, sizeof(buf1));
	snprintf( buf1, sizeof( buf1 ), "W%d", petNo);
	CHAR_sendStatusString( charaindex, buf1 );
	CHAR_sendStatusString( charaindex, "P");
	{
		int skillarray = PETSKILL_getPetskillArray( SkillID);
		sprintf( buf1, "����%sѧϰ%s������ %s��ʧ�ˡ�",
			CHAR_getUseName( petindex), PETSKILL_getChar( skillarray, PETSKILL_NAME),
			ITEM_getChar( itemindex, ITEM_NAME) );
	}

	CHAR_talkToCli( charaindex, -1, buf1, CHAR_COLORYELLOW);
	CHAR_setItemIndex( charaindex, itemNo ,-1);
	CHAR_sendItemDataOne( charaindex, itemNo);
	ITEM_endExistItemsOne( itemindex);
}

#endif

static void ITEM_useRenameItem_PrintWindow( int charaindex, int page)
{
	int fd;
	int	pos = 0;
	int i;
	int btntype = WINDOW_BUTTONTYPE_CANCEL;
	char	message[1024];
	char	msgwk[1024];
	char	buf[2048];

	if( !CHAR_CHECKINDEX( charaindex )) return;
	if( page < 0 || page > 3 ) {
		print( "%s:%d err\n", __FILE__, __LINE__);
		return;
	}
	fd = getfdFromCharaIndex( charaindex);
	if( fd == - 1 ) return;
	
	snprintf( message, sizeof( message),
				"2\n   Ҫ����Ǹ���Ŀ��������\n"
				"                               Page:%d\n", page +1);
	
	for( i = page *5; i < page *5 +5; i ++ ) {
		int itemindex = CHAR_getItemIndex( charaindex, i);
		BOOL	flg = FALSE;
		while( 1 ) {
			char *cdkey;
			if( !ITEM_CHECKINDEX( itemindex)) break;
			cdkey = ITEM_getChar( itemindex, ITEM_CDKEY);
			if( !cdkey) {
				print( "%s:%d err\n", __FILE__, __LINE__);
				break;
			}
			if( ITEM_getInt( itemindex, ITEM_MERGEFLG) != 1) break;
			if( ITEM_getInt( itemindex, ITEM_TYPE) == ITEM_DISH) break;
			if( ITEM_getInt( itemindex, ITEM_CRUSHLEVEL) != 0 ) break;
			if( strlen( cdkey) != 0 ) {
				if( strcmp( cdkey, CHAR_getChar( charaindex, CHAR_CDKEY)) != 0 ) {
					break;
				}
			}
			flg = TRUE;
			break;
		}
		if( flg ) {
			char *nm = ITEM_getChar( itemindex, ITEM_SECRETNAME);
			char wk[256];
			if( pos +strlen( nm) +1 > sizeof( msgwk)) {
				print( "buffer over error %s:%d\n", __FILE__, __LINE__);
				break;
			}
			snprintf( wk, sizeof( wk), "%s\n", nm);
			strncpy( &msgwk[pos],  wk, sizeof( msgwk) -pos -1);
			pos += strlen( wk);
		}
		else {
			if( pos +2 > sizeof( msgwk)) {
				print( "buffer over error %s:%d\n", __FILE__, __LINE__);
				break;
			}
			strncpy( &msgwk[pos], "\n", sizeof( msgwk) -pos -1 );
			pos += 1;
		}
	}
	strcat( message, msgwk);
	switch( page){
	  case 0:
	  	btntype |= WINDOW_BUTTONTYPE_NEXT;
	  	break;
	  case 3:
	  	btntype |= WINDOW_BUTTONTYPE_PREV;
	  	break;
	  case 1:
	  case 2:
	  	btntype |= WINDOW_BUTTONTYPE_PREV|WINDOW_BUTTONTYPE_NEXT;
	  	break;
	}
	lssproto_WN_send( fd, WINDOW_MESSAGETYPE_SELECT, 
					btntype,
					CHAR_WINDOWTYPE_SELECTRENAMEITEM_PAGE1+page,
					-1,
					makeEscapeString( message, buf, sizeof(buf)));
}

void ITEM_useRenameItem( int charaindex, int toindex, int haveitemindex)
{

	ITEM_useRenameItem_PrintWindow( charaindex, 0);

	CHAR_setWorkInt( charaindex, CHAR_WORKRENAMEITEMNUM, -1);
	CHAR_setWorkInt( charaindex, CHAR_WORKRENAMEITEMINDEX, haveitemindex);
/*
	char buf[256];
	int itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX( itemindex) ) return;
	sprintf( buf, "%s�����ѱ�ȡ����", ITEM_getChar( itemindex, ITEM_NAME));
	CHAR_talkToCli( charaindex, -1, "�����ѱ�ȡ����", CHAR_COLORRED );

	{
		LogItem(
			CHAR_getChar( charaindex, CHAR_NAME ),
			CHAR_getChar( charaindex, CHAR_CDKEY ),
			itemindex,
			"ħ����DEL",
			CHAR_getInt( charaindex, CHAR_FLOOR),
			CHAR_getInt( charaindex, CHAR_X ),
 	      	CHAR_getInt( charaindex, CHAR_Y ),
	        ITEM_getChar( itemindex, ITEM_UNIQUECODE),
			ITEM_getChar( itemindex, ITEM_NAME),
			ITEM_getInt( itemindex, ITEM_ID)
		);
	}

	CHAR_setItemIndex( charaindex, haveitemindex, -1);
	CHAR_sendItemDataOne( charaindex, haveitemindex);
	ITEM_endExistItemsOne( itemindex );
*/
}

void ITEM_useRenameItem_WindowResult( int charaindex, int seqno, int select, char * data)
{
	int page = 0;
	int fd;
	if( select == WINDOW_BUTTONTYPE_CANCEL) return;

	fd = getfdFromCharaIndex( charaindex);
	if( fd == - 1 ) return;
	if( seqno != CHAR_WINDOWTYPE_SELECTRENAMEITEM_RENAME ) {
		if( select == WINDOW_BUTTONTYPE_NEXT ) page = 1;
		else if( select == WINDOW_BUTTONTYPE_PREV ) page = -1;
		if( select == WINDOW_BUTTONTYPE_NEXT || select == WINDOW_BUTTONTYPE_PREV ) {
			int winno = seqno - CHAR_WINDOWTYPE_SELECTRENAMEITEM_PAGE1;
			winno += page;
			if( winno < 0 ) winno = 0;
			if( winno > 3 ) winno = 3;
			ITEM_useRenameItem_PrintWindow( charaindex, winno);
		}else {
			char	message[1024];
			char	buf[2048];
			char	haveitemindex = CHAR_getWorkInt( charaindex, CHAR_WORKRENAMEITEMNUM);
			int		itemindex;
			if( haveitemindex == -1 ) {
				haveitemindex = (seqno - CHAR_WINDOWTYPE_SELECTRENAMEITEM_PAGE1) * 5 + 
								( atoi(data)-1);
				CHAR_setWorkInt( charaindex, CHAR_WORKRENAMEITEMNUM, haveitemindex);
			}
			itemindex = CHAR_getItemIndex( charaindex, haveitemindex);

			snprintf( message, sizeof( message),
							"%s ����Ҫ����\n"
							"����������\n"
							"ȫ��13����, ����26����",
							ITEM_getChar( itemindex, ITEM_NAME));
			
			
			lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGEANDLINEINPUT, 
							WINDOW_BUTTONTYPE_OKCANCEL,
							CHAR_WINDOWTYPE_SELECTRENAMEITEM_RENAME,
							-1,
							makeEscapeString( message, buf, sizeof(buf)));
			
		}
	}
	else {
		BOOL	flg = FALSE;
		char	message[1024];
		char	buf[2048];
		while( 1 ) {
			char	*p;
			if( strlen( data) > 26 || strlen( data) < 1) {
				if( strlen( data) > 26 ) {
					strcpy( message, "������������");
				}else {
					strcpy( message, "������һ����������");
				}
				break;
			}

            // WON ADD ����ħ���ʸ�������
			flg = TRUE;

			for( p = data; *p ; p ++) {
				if( *p == ' '){
					strcpy( message, "���ɿհ�");
					flg = FALSE;
					break;
				}
				if( strncmp( p, "��",2) == 0 ) {
					strcpy( message, "���ɿհ�");
					flg = FALSE;
					break;
				}
				if( *p == '|'){
					strcpy( message, "�������������");
					flg = FALSE;
					break;
				}
			}
	
			break;
		}


		if( !flg ) {
			lssproto_WN_send( fd, WINDOW_MESSAGETYPE_MESSAGE, 
							WINDOW_BUTTONTYPE_OK,
							CHAR_WINDOWTYPE_SELECTRENAMEITEM_RENAME_ATTENTION,
							-1,
							makeEscapeString( message, buf, sizeof(buf)));
		}
		else {
			char	haveitemindex = CHAR_getWorkInt( charaindex, CHAR_WORKRENAMEITEMNUM);
			int		itemindex;
			int		renameitemindex;
			int		renameitemhaveindex;
			int		remain;
			char msgbuf[128];

			itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
			if( !ITEM_CHECKINDEX( itemindex)) {
				print( "%s %d err\n", __FILE__, __LINE__);
				return;
			}
			ITEM_setChar( itemindex, ITEM_SECRETNAME, data);
			ITEM_setChar( itemindex, ITEM_CDKEY, 
						CHAR_getChar( charaindex, CHAR_CDKEY));
			CHAR_sendItemDataOne( charaindex, haveitemindex);
			snprintf( msgbuf, sizeof(msgbuf),"�� %s ������ %s ", 
					ITEM_getChar( itemindex, ITEM_NAME), data);
		    CHAR_talkToCli( charaindex, -1, msgbuf, CHAR_COLORYELLOW);
			renameitemhaveindex = CHAR_getWorkInt( charaindex, CHAR_WORKRENAMEITEMINDEX);
			renameitemindex = CHAR_getItemIndex( charaindex, renameitemhaveindex);
			if( !ITEM_CHECKINDEX( renameitemindex)) {
				print( "%s %d err\n", __FILE__, __LINE__);
				return;
			}
			remain = atoi( ITEM_getChar( renameitemindex, ITEM_ARGUMENT));
			if( remain != 0 ) {
				remain --;
				if( remain <= 0 ) {
					snprintf( msgbuf, sizeof(msgbuf),"%s ��ʧ��", 
							ITEM_getChar( renameitemindex, ITEM_NAME));
				    CHAR_talkToCli( charaindex, -1, msgbuf, CHAR_COLORYELLOW);
				    CHAR_setItemIndex( charaindex, renameitemhaveindex, -1);
				    CHAR_sendItemDataOne( charaindex, renameitemhaveindex);
					ITEM_endExistItemsOne( renameitemindex );
				}else {
					char buf[32];
					snprintf( buf, sizeof( buf),"%d", remain);
					ITEM_setChar( renameitemindex, ITEM_ARGUMENT, buf);
				}
			}
		}
	}
}

//-------------------------------------------------------------------------
//	���г�Ƿë  ���������ѣ�
//	  �������ݱ�ݷ¼�ĸة���  ��ë��̫��  ���  �  įë  �����£�
//-------------------------------------------------------------------------
void ITEM_dropDice( int charaindex, int itemindex)
{
	char *dicename[] = { "һ", "��",  "��", "��", "��", "��"};
	int  diceimagenumber[] = { 24298,24299,24300,24301,24302,24303};
	int r = RAND( 0,5);
	
	//   �  įë��  
	ITEM_setInt( itemindex, ITEM_VAR1, ITEM_getInt( itemindex, ITEM_BASEIMAGENUMBER));
	//   �  į��ޥ
	ITEM_setInt( itemindex, ITEM_BASEIMAGENUMBER, diceimagenumber[r]);
	//   ��ޥ
	ITEM_setChar( itemindex, ITEM_SECRETNAME, dicename[r]);
	
	// ���������ͷ���ʧ�����߼�˪����������ƥ�浤��ƥ�ݳ��ƥ��֧��ئ�У�
}
//-------------------------------------------------------------------------
//	���г�Ƿë  ���������ѣ�
//  �������ݱ��  ���  �  įë���  �ʣ�
//-------------------------------------------------------------------------
void ITEM_pickupDice( int charaindex, int itemindex)
{
	//   �  įë���  �ʣ�
	ITEM_setInt( itemindex,  ITEM_BASEIMAGENUMBER, ITEM_getInt( itemindex, ITEM_VAR1));
	//   ������  ��
	ITEM_setChar( itemindex, ITEM_SECRETNAME, ITEM_getChar( itemindex, ITEM_NAME));
}
enum {
	ITEM_LOTTERY_1ST,		// 1�
	ITEM_LOTTERY_2ND,
	ITEM_LOTTERY_3RD,
	ITEM_LOTTERY_4TH,
	ITEM_LOTTERY_5TH,		// 5�
	ITEM_LOTTERY_6TH,		// 6�
	ITEM_LOTTERY_NONE,		// ½ľ
	ITEM_LOTTERY_NUM,
};
//-------------------------------------------------------------------------
//	���������Ԫ������ľ���ݼ����ѣ�
//  ���ƥ��ٱ�ئ�¾�ë裻��£�
//  ��  ����Ʊ��ɧԻ��

#define PRE_6		(10000)
#define PRE_5		(1300 + PRE_6)
#define PRE_4		(600 + PRE_5)
#define PRE_3		(300 + PRE_4)
#define PRE_2		(8 + PRE_3)
#define PRE_1		(1 + PRE_2)


//#define PRE_5		(13000)
//#define PRE_4		(2300 + PRE_5)
//#define PRE_3		(540 + PRE_4)
//#define PRE_2		(8 + PRE_3)
//#define PRE_1		(1 + PRE_2)


//#define PRE_5		(16000)
//#define PRE_4		(2500 + PRE_5)
//#define PRE_3		(400 + PRE_4)
//#define PRE_2		(10 + PRE_3)
//#define PRE_1		(1 + PRE_2)
//#define PRE_5		(27000)
//#define PRE_4		(4000 + PRE_5)
//#define PRE_3		(400 + PRE_4)
//#define PRE_2		(10 + PRE_3)
//#define PRE_1		(1 + PRE_2)
//
//-------------------------------------------------------------------------
BOOL ITEM_initLottery(ITEM_Item* itm)
{
	int r = RAND( 0, 49999);
	int hit = ITEM_LOTTERY_NONE;		// 
	char result[7];		// ��  
	int countnum[6];
	int count;
	int i;
	int len;
	// ������Ȼ��¼�ƥ�۷���  �������羮����Իئ�ƽ�ľئ�з������
	if( itm->data[ITEM_VAR3] == 1 ) return TRUE;

	// �������£�
	if( r < PRE_6 ) hit = ITEM_LOTTERY_6TH;
	else if( r < PRE_5 ) hit = ITEM_LOTTERY_5TH;
	else if( r < PRE_4 ) hit = ITEM_LOTTERY_4TH;
	else if( r < PRE_3 ) hit = ITEM_LOTTERY_3RD;
	else if( r < PRE_2 ) hit = ITEM_LOTTERY_2ND;
	else if( r < PRE_1 ) hit = ITEM_LOTTERY_1ST;
	else hit = ITEM_LOTTERY_NONE;
	
	// ��  ë������£�
	count = 0;
	memset( countnum, 0, sizeof( countnum));
	if( hit != ITEM_LOTTERY_NONE ) {
		// �Իë������
		result[0] = result[1] = result[2] = hit+1;
		count = 3;
		countnum[hit] = 3;
	}
	while( count < 6 ) {
		int r = RAND( ITEM_LOTTERY_1ST, ITEM_LOTTERY_6TH);
		if( countnum[r] >= 2 ) continue;
		// 2/3�����м���  ƥ��ְ��½ľ��  ë�����������֧�£�
		// ���������
		if(      ( hit != ITEM_LOTTERY_NONE && count == 3 )
		     ||  ( hit == ITEM_LOTTERY_NONE && count == 0 ) )
		{
			if( RAND( 0,2)) {
				result[count] = result[count+1] = r+1;
				countnum[r] += 2;
				count += 2;
				continue;
			}
		}
		countnum[r] ++;
		result[count] = r+1;
		count++;
	}
	// ��  ë�������׻����£�
	// �Ի���ݷ�2/3����  ƥ���Ի��ٯë��    Ƿ���������£�
	// ��ƽ��ƽ���ë�ѵ��У�
	len = sizeof( result)-2;
	if( hit != ITEM_LOTTERY_NONE ) {
		if( RAND( 0,2) ) {
			// �Ի��ٯë��    Ƿ���
			char s = result[0];
			result[0] = result[5];
			result[5] = s;
			len = sizeof( result)-3;
		}
	}
	for( i = 0; i < 10; i ++) {
		int x = RAND( 0, len);
		int y = RAND( 0, len);
		int s;
		s = result[x];
		result[x] = result[y];
		result[y] = s;
	}
	//   ���׻���  ٯ  ���б��ƻ����ʣ�
	result[sizeof(result)-1] = '\0';
	itm->data[ITEM_VAR1] = hit;
	// ����������������
	itm->data[ITEM_VAR2] = 0;
	itm->data[ITEM_VAR3] = 1;
	memcpy( itm->string[ITEM_ARGUMENT].string, result, sizeof( result));
	
	return TRUE;
}
//-------------------------------------------------------------------------
//	���������Ԫë�������ݼ����ѣ�
//  ������������پ���ʧ��  ة��  �����£�
//-------------------------------------------------------------------------
void ITEM_useLottery( int charaindex, int toindex, int haveitemindex)
{
	int i,j;
	int itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	int count = ITEM_getInt( itemindex, ITEM_VAR2);
	int hit = ITEM_getInt( itemindex, ITEM_VAR1);
	char buff[1024];
	char num[6][3] = { {"��"},{"��"},{"��"},{"��"},{"��"}, {"��"}};
	char numbuff[128];
	char *n;
	int result;
	BOOL flg;
	if( !ITEM_CHECKINDEX( itemindex) ) return;
	if( count == 0 ) {
		ITEM_setChar( itemindex, ITEM_EFFECTSTRING, "");
	}
	// ���  �����÷���½ľ�ƾ�ؤԻ  ئ�м�ƥ�ݳ��ƥʧ��  ةë����
	else if( count == 6 ) {
        CHAR_setItemIndex( charaindex , haveitemindex, -1 );
		CHAR_sendItemDataOne( charaindex, haveitemindex);
        ITEM_endExistItemsOne( itemindex );
		return;
	}
	n = ITEM_getChar( itemindex, ITEM_ARGUMENT);
	// for debug
#if 0
    snprintf( buff, sizeof( buff), "١�%d ��  : %d%d%d%d%d%d",
    			hit,
    			(int)(n[0]+1),(int)(n[1]+1),
    			(int)(n[2]+1),(int)(n[3]+1),
    			(int)(n[4]+1),(int)(n[5]+1)
    			);
    CHAR_talkToCli( charaindex, -1,
                    buff, CHAR_COLORYELLOW );
#endif
	result = (int)n[count]-1;
	// ������Ʃ����
	flg = FALSE;
	for( i = 0; i < count+1 && flg == FALSE; i ++ ) {
		for( j = i+1; j < count+1 && flg == FALSE; j ++ ) {
			if( i != j ) {
				if( n[i] == n[j] ) {
					flg = TRUE;
				}
			}
		}
	}
	memset( numbuff, 0, sizeof( numbuff));
	memcpy( numbuff, ITEM_getChar( itemindex, ITEM_EFFECTSTRING), (count)*2);
	// ���  ٯ  �����  ë���ʣ�
	snprintf( buff, sizeof( buff), "%s%s", numbuff, num[result]);
	count ++;
	ITEM_setInt( itemindex, ITEM_VAR2, count);
	// ���  ƥ�Իئ���Իʧ��  ة��  �ʣ�½ľئ��½ľ��  ����
	// �ݼ�ĸƤ����������ƥ���ʣ�
	if( count >= 6 ) {
		// �Ի
		if( hit != ITEM_LOTTERY_NONE ) {
      		int newitemindex;
      		char strbuff[1024];
            char msgbuff[1024];
            // ���ƻ�
            CHAR_setItemIndex( charaindex , haveitemindex, -1 );
            ITEM_endExistItemsOne( itemindex );
			// 2729 - 2734 ��  ��Ԫ��1  6�
			newitemindex = ITEM_makeItemAndRegist( 2729 + hit);
            CHAR_setItemIndex( charaindex , haveitemindex, newitemindex );
			// ��  �������
			snprintf( strbuff, sizeof( strbuff), "%s                %s", buff, 
						ITEM_getChar( newitemindex, ITEM_EFFECTSTRING));
			ITEM_setChar( newitemindex, ITEM_EFFECTSTRING, strbuff);
			CHAR_sendItemDataOne( charaindex, haveitemindex);
            snprintf( msgbuff, sizeof( msgbuff), "���˵�%d��", hit+1 );
            CHAR_talkToCli( charaindex, -1,
                            msgbuff,
                            CHAR_COLORYELLOW );
		}
		// ½ľ
		else {
//            CHAR_setItemIndex( charaindex , haveitemindex, -1 );
			// ��  �������
      		char strbuff[1024];
			snprintf( strbuff, sizeof( strbuff), "%s                       û��,�´�����", buff);
			ITEM_setChar( itemindex, ITEM_EFFECTSTRING, strbuff);
			CHAR_sendItemDataOne( charaindex, haveitemindex);
//            ITEM_endExistItemsOne( itemindex );
//            CHAR_talkToCli( charaindex, -1,
//                            "����ľ��",
//                            CHAR_COLORWHITE );
		}
	}
	// ����Ԫ��  ���ݷ��ݷ�����ë˪Իئ���ʷ����
	else {
		char strbuff[1024];
		if( flg) {
			// ��ʸ������ң
			// ʧ��  ة��      ���춪�������ݨ����  ٯƥ��
			// ��ʸ����ë���Ȼ�ݱ���ƻ����£�
			int spc = 16 + ( 6-count)*2;
			char space[17];
			memset( space, ' ', spc);
			space[spc] = '\0';
			snprintf( strbuff, sizeof( strbuff), "%s%s������", buff, space);
		}
		else {
			strcpy( strbuff, buff);
		}
		ITEM_setChar( itemindex, ITEM_EFFECTSTRING, strbuff);
		CHAR_sendItemDataOne( charaindex, haveitemindex);
	}
}

void ITEM_WarpDelErrorItem( int charaindex )
{
	int j;
	for(j=0; j<CHAR_MAXITEMHAVE; j++){
		int itemindex=CHAR_getItemIndex(charaindex, j);
		if(ITEM_CHECKINDEX(itemindex)){
			int id=ITEM_getInt(itemindex, ITEM_ID);
			if(id==2609||id==2704){
				CHAR_setItemIndex(charaindex, j, -1);
				ITEM_endExistItemsOne(itemindex);
				CHAR_sendItemDataOne(charaindex, j);
			}
		}
	}
#ifdef _ITEM_WARP_FIX_BI
	if(!CHAR_getWorkInt( charaindex, CHAR_WORKITEMMETAMO))
		recoverbi(charaindex);
#endif
}

BOOL ITEM_WarpForAny(int charaindex, int haveitemindex, int ff, int fx, int fy,int flg)
{
	if( CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE ) != BATTLE_CHARMODE_NONE )
		return FALSE;
#ifdef _BAD_PLAYER             // WON ADD �ͻ����ȥ��
	if( (CHAR_getInt(charaindex,CHAR_FLOOR)==117)||(CHAR_getInt(charaindex,CHAR_FLOOR)==887) ){
#else
	if( CHAR_getInt( charaindex, CHAR_FLOOR) == 117 
#ifdef _ADD_DUNGEON            //׷�ӵ���
        || CHAR_getInt( charaindex,CHAR_FLOOR ) == 8513
#endif
		){
#endif
		CHAR_talkToCli(charaindex, -1, "�˴��޷�ʹ�á�", CHAR_COLORYELLOW);
		return FALSE;
	}
	if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE ) == CHAR_PARTY_LEADER ){
		int i;
		if( flg == 0 )	{//����
			CHAR_talkToCli(charaindex, -1, "ֻ�ܵ���ʹ�á�", CHAR_COLORYELLOW);
			return FALSE;
		}
		for( i = 0; i < CHAR_PARTYMAX; i ++ ){
			int subindex = CHAR_getWorkInt( charaindex, CHAR_WORKPARTYINDEX1+i );
			if( CHAR_CHECKINDEX( subindex ) == FALSE ) continue;
			CHAR_talkToCli( subindex, -1, "ȫ��˲����У�����", CHAR_COLORWHITE);
			ITEM_WarpDelErrorItem( subindex );
			CHAR_warpToSpecificPoint( subindex, ff, fx, fy );
		}
	}else if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE ) == CHAR_PARTY_CLIENT ){
		CHAR_talkToCli(charaindex, -1, "��Ա�޷�ʹ�á�", CHAR_COLORYELLOW);
		return FALSE;
	}else if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE ) == CHAR_PARTY_NONE ){
		ITEM_WarpDelErrorItem( charaindex );
		CHAR_warpToSpecificPoint( charaindex, ff, fx, fy);
	}
	return TRUE;
}
//andy_end

// Robin 0523
void ITEM_useWarp( int charaindex, int toindex, int haveitemindex )
{
	char *arg;
	int itemindex, warp_t, warp_fl, warp_x, warp_y;
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;

#ifdef _ITEM_CHECKWARES
	if( CHAR_CheckInItemForWares( charaindex, 0) == FALSE ){
		CHAR_talkToCli(charaindex, -1, "Я�������޷�ʹ�á�", CHAR_COLORYELLOW);
		return;
	}
#endif
#ifdef _BAD_PLAYER             // WON ADD �ͻ����ȥ��
	if( (CHAR_getInt(charaindex,CHAR_FLOOR)==117)||(CHAR_getInt(charaindex,CHAR_FLOOR)==887) ){
		CHAR_talkToCli(charaindex, -1, "�˴��޷�ʹ�á�", CHAR_COLORYELLOW);
		return;
	}
#endif
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );
	if( arg == NULL )	return;
	if( sscanf( arg, "%d %d %d %d", &warp_t, &warp_fl, &warp_x, &warp_y) != 4 )
		return;
	if( ITEM_WarpForAny(charaindex, haveitemindex, warp_fl, warp_x, warp_y, warp_t) == FALSE )
		return;

	CHAR_DelItem( charaindex, haveitemindex);
	CHAR_sendStatusString(charaindex, "P");
}

#ifdef _USEWARP_FORNUM
void ITEM_useWarpForNum( int charaindex, int toindex, int haveitemindex )
{
	char *arg;
	int flg, ff, fx, fy, itemindex, usenum=0, i;
	int Mf, Mx, My;
	int MapPoint[12]={
		100, 200, 300, 400, 700,
		701, 702, 703, 704, 705,
		707, 708 };
		char MapString[12][256]={
			"������˹","��³��","��³��","ɳķ��","����԰��",
			"�����½����","���˹����","�����½�Ϸ�","����ŷ����","�����½����",
			"�����½����","����������"};
	
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX( itemindex) ) return;
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );
	if( arg == NULL )	return;

	if( sscanf( arg, "%d %d %d %d", &flg, &ff, &fx, &fy) != 4 )
		return;
	Mf = CHAR_getInt( charaindex, CHAR_FLOOR );
	Mx = CHAR_getInt( charaindex, CHAR_X );
	My = CHAR_getInt( charaindex, CHAR_Y );

	usenum = ITEM_getInt( itemindex, ITEM_DAMAGEBREAK);
	for( i=0; i<12; i++)	{
		if( Mf == MapPoint[i] ){
			break;
		}
	}
#ifdef _ITEM_CHECKWARES
	if( CHAR_CheckInItemForWares( charaindex, 0) == FALSE ){
		CHAR_talkToCli(charaindex, -1, "Я�������޷�ʹ�á�", CHAR_COLORYELLOW);
		return;
	}
#endif
	if( --usenum <= 0 )	{
		CHAR_DelItem( charaindex, haveitemindex);
		CHAR_sendStatusString(charaindex, "P");
	}else	{
		char buf[256];
		// WON ADD
		if( i >= 12 ){
			CHAR_talkToCli(charaindex, -1, "�˴��޷�ʹ�á�", CHAR_COLORYELLOW);
			return;
		}

		memset( buf, 0, sizeof( buf));
		sprintf( buf, "%d %d %d %d", flg, Mf, Mx, My);
		ITEM_setChar(itemindex, ITEM_ARGUMENT, buf);
		ITEM_setInt( itemindex, ITEM_DAMAGEBREAK, usenum);
		sprintf( buf, "���м�¼��(%s,%d,%d)", MapString[i], Mx, My);
		ITEM_setChar( itemindex, ITEM_EFFECTSTRING, buf);
		CHAR_talkToCli(charaindex, -1, buf, CHAR_COLORYELLOW);
		CHAR_sendItemDataOne( charaindex, haveitemindex);
	}

	// WON ADD
	if( ITEM_WarpForAny(charaindex, haveitemindex, ff, fx, fy, flg) == FALSE )
		return;
}
#endif

// Robin 0707 petFollow
void ITEM_petFollow( int charaindex, int toindex, int haveitemindex )
{
	char *arg;
	int itemindex, followLv, haveindex, i;
	//print(" PetFollow_toindex:%d ", toindex);
	if( CHAR_getWorkInt( charaindex, CHAR_WORKPETFOLLOW ) != -1 ){
		if( CHAR_CHECKINDEX( CHAR_getWorkInt( charaindex, CHAR_WORKPETFOLLOW ) ) ){
			CHAR_talkToCli( charaindex, -1, "�����ջطų��ĳ��", CHAR_COLORWHITE );
			return;
		}
		CHAR_setWorkInt( charaindex, CHAR_WORKPETFOLLOW, -1);
	}
#ifdef _FIX_METAMORIDE
	if( CHAR_CHECKJOINENEMY( charaindex) == TRUE ){
		CHAR_talkToCli( charaindex, -1, "������޷�ʹ�ã�", CHAR_COLORWHITE );
		return;
	}
#endif

	if( CHAR_CHECKINDEX( toindex ) == FALSE )	return;
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );
	if( arg == NULL )return;

	if( sscanf( arg, "%d", &followLv) != 1 )
		return;
	if( CHAR_getInt( toindex, CHAR_LV ) > followLv ){
		CHAR_talkToCli( charaindex, -1, "���ߵĵȼ����㣡", CHAR_COLORWHITE );
		return;
	}
	if( CHAR_getWorkInt( toindex, CHAR_WORKFIXAI ) < 80 )
	{
		//CHAR_talkToCli( charaindex, -1, "������ҳ϶Ȳ��㣡", CHAR_COLORWHITE );
		//return;
	}	
	
	
	haveindex = -1;	
	for( i = 0; i < 5; i++ )
	{
		if( CHAR_getCharPet( charaindex, i) == toindex ) {
			haveindex = i;
			break;
		}
	}
	if( haveindex == -1) return;
	
	
	if( !PET_dropPetFollow( charaindex, haveindex, -1, -1, -1 ) ) {
		CHAR_talkToCli( charaindex, -1, "�������ʧ�ܣ�", CHAR_COLORWHITE );
		return;
	}

}

// Nuke start 0624: Hero's bless
void ITEM_useSkup( int charaindex, int toindex, int haveitemindex)
{
	int itemindex;
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;	
	CHAR_setInt(charaindex,CHAR_SKILLUPPOINT,
	CHAR_getInt(charaindex,CHAR_SKILLUPPOINT)+1);
	CHAR_Skillupsend(charaindex);
	CHAR_talkToCli(charaindex, -1, "����ܵ��Լ��������������ˡ�", CHAR_COLORWHITE);

	CHAR_DelItem( charaindex, haveitemindex);
}
// Nuke end
extern void setNoenemy();
// Nuke start 0626: Dragon's bless
void ITEM_useNoenemy( int charaindex, int toindex, int haveitemindex)
{
	int itemindex,fd;
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;	
	fd=CHAR_getWorkInt( charaindex, CHAR_WORKFD);
	setNoenemy(fd);
	CHAR_talkToCli(charaindex, -1, "����ܵ��ܱߵ�ɱ����ʧ�ˡ�", CHAR_COLORWHITE);
	CHAR_DelItem( charaindex, haveitemindex);

}
// Nuke end

// Arminius 7.2: Ra's amulet
void ITEM_equipNoenemy( int charaindex, int itemindex )
{
  char buf[4096];
  int evadelevel;
  int fl,fd;
  
  if( ITEM_CHECKINDEX(itemindex) == FALSE )return;

  if( ITEM_getArgument( ITEM_getChar(itemindex,ITEM_ARGUMENT),"noen", buf, sizeof(buf) )
      == FALSE ){
        return;
  }

  evadelevel=atoi(buf);
  fl=CHAR_getInt(charaindex, CHAR_FLOOR);
  fd=CHAR_getWorkInt( charaindex, CHAR_WORKFD);
  if (evadelevel>=200) {
    setEqNoenemy(fd, 200);
    CHAR_talkToCli(charaindex, -1, "һ������Ĺ�â������������١�", CHAR_COLORWHITE);
    return;
  } else if (evadelevel>=120) {
    setEqNoenemy(fd, 120);

	if ( (fl==100)||(fl==200)||(fl==300)||(fl==400)||(fl==500) ){
	  CHAR_talkToCli(charaindex, -1, "һ������Ĺ�â������������١�", CHAR_COLORWHITE);
      return;
    }
  } else if (evadelevel>=80) {
    setEqNoenemy(fd, 80);
	if ( (fl==100)||(fl==200)||(fl==300)||(fl==400) ){
	
      CHAR_talkToCli(charaindex, -1, "һ������Ĺ�â������������١�", CHAR_COLORWHITE);
      return;
    }
  } else if (evadelevel>=40) {
    setEqNoenemy(fd, 40);
    if ( (fl==100)||(fl==200) ) {
      CHAR_talkToCli(charaindex, -1, "һ������Ĺ�â������������١�", CHAR_COLORWHITE);
      return;
    }
  }
  CHAR_talkToCli(charaindex, -1, "ʲ����Ҳû�з�����", CHAR_COLORWHITE);
}

#ifdef _Item_MoonAct
void ITEM_randEnemyEquipOne( int charaindex, int toindex, int haveitemindex)
{
	int itemindex, RandNum=0;
	char buf[256];

	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX( itemindex)) return;

	if( ITEM_getArgument( ITEM_getChar( itemindex,ITEM_ARGUMENT), "rand", buf, sizeof(buf) ) == FALSE ){
		return;
	}

	if( (RandNum=atoi( buf)) > 0 ){
		int fd = CHAR_getWorkInt( charaindex, CHAR_WORKFD);
		setEqRandenemy(fd, RandNum);
		CHAR_talkToCli(charaindex, -1, "�����ʽ����ˡ�", CHAR_COLORWHITE);
		sprintf( buf, "���� %s��ʧ�ˡ�", ITEM_getChar( itemindex, ITEM_NAME) );
		CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
		CHAR_DelItem( charaindex, haveitemindex);
		return;
	}


}

void ITEM_randEnemyEquip( int charaindex, int itemindex)
{
  char buf[4096];
  int RandNum=0;
  int fd;
  
  if( ITEM_CHECKINDEX(itemindex) == FALSE )return;
  if( ITEM_getArgument( ITEM_getChar(itemindex,ITEM_ARGUMENT),"rand", buf, sizeof(buf) )
      == FALSE ){
        return;
  }

  RandNum=atoi( buf);
  fd=CHAR_getWorkInt( charaindex, CHAR_WORKFD);
  if (RandNum > 0 ) {
    setEqRandenemy(fd, RandNum);
    CHAR_talkToCli(charaindex, -1, "�����ʽ����ˡ�", CHAR_COLORWHITE);
    return;
  }

}
void ITEM_RerandEnemyEquip( int charaindex, int itemindex)
{
  int RandNum=0;
  int fd;
  
  if( ITEM_CHECKINDEX(itemindex) == FALSE )return;

  fd=CHAR_getWorkInt( charaindex, CHAR_WORKFD);
  RandNum = getEqRandenemy( fd);

  if (RandNum > 0 ) {
    clearEqRandenemy( fd);
    CHAR_talkToCli(charaindex, -1, "�����ʻظ���", CHAR_COLORWHITE);
    return;
  }
}
#endif

#ifdef _ITEM_WATERWORDSTATUS
void ITEM_WaterWordStatus( int charaindex, int toindex, int haveitemindex)
{
	int itemindex;
	char itemarg[256];

	if( !CHAR_CHECKINDEX( charaindex) ){
		return;
	}
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX( itemindex) ){
		return;
	}

	if( ITEM_getArgument( ITEM_getChar( itemindex, ITEM_ARGUMENT),"time", itemarg, sizeof(itemarg) ) != FALSE ){
		char token[256];
		int nums = CHAR_getWorkInt( charaindex, CHAR_WORKSTATUSWATER);
		if( nums < 0 ) nums = 0;
		if( nums > 0 ){
			CHAR_talkToCli( charaindex, -1, "ˮ�к���ʱ�䲻���ۻ���", CHAR_COLORYELLOW);
			return;
		}
		CHAR_setWorkInt( charaindex, CHAR_WORKSTATUSWATER, nums+atoi(itemarg));
		CHAR_setItemIndex(charaindex, haveitemindex ,-1);
		CHAR_sendItemDataOne( charaindex, haveitemindex);
		ITEM_endExistItemsOne(itemindex);

		sprintf( token, "ˮ�к���ʱ������%d�֣��ܼ�%d�֡�", atoi(itemarg),
			CHAR_getWorkInt( charaindex, CHAR_WORKSTATUSWATER));
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
	}else{
		CHAR_talkToCli(charaindex, -1, "ʲ����Ҳû������", CHAR_COLORYELLOW);
	}
}
#endif

#ifdef _CHIKULA_STONE
void ITEM_ChikulaStone( int charaindex, int toindex, int haveitemindex)
{
	int itemindex,fd;
	char itemarg[256];

	if( !CHAR_CHECKINDEX( charaindex) ) return;
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX( itemindex) ) return;

	fd = CHAR_getWorkInt( charaindex, CHAR_WORKFD);
	CHAR_setWorkInt( charaindex, CHAR_WORKCHIKULAHP, 0 );
	CHAR_setWorkInt( charaindex, CHAR_WORKCHIKULAMP, 0 );
	setChiStone( fd, 0);	//1hp 2mp

	if( ITEM_getArgument( ITEM_getChar( itemindex, ITEM_ARGUMENT),"hp", itemarg, sizeof(itemarg) ) != FALSE ){
		setChiStone( fd, 1);
		CHAR_setWorkInt( charaindex, CHAR_WORKCHIKULAHP, atoi( itemarg) );
	}else if( ITEM_getArgument( ITEM_getChar( itemindex, ITEM_ARGUMENT),"mp", itemarg, sizeof(itemarg) ) != FALSE ){
		setChiStone( fd, 2);
		CHAR_setWorkInt( charaindex, CHAR_WORKCHIKULAMP, atoi( itemarg) );
	}else{
	}

	CHAR_talkToCli(charaindex, -1, "�����������ף����", CHAR_COLORWHITE);
	CHAR_setItemIndex(charaindex, haveitemindex ,-1);
	CHAR_sendItemDataOne( charaindex, haveitemindex);
	ITEM_endExistItemsOne(itemindex);
}
#endif

#ifdef _ITEM_ORNAMENTS
void ITEM_PutOrnaments( int charaindex, int itemindex)
{
	char *arg=NULL;
	char itemname[256];
	int bbnums=0;
	if( !CHAR_CHECKINDEX( charaindex) ) return;
	if( !ITEM_CHECKINDEX( itemindex) ) return;
	arg = ITEM_getChar( itemindex, ITEM_ARGUMENT);
	if( arg == NULL ) return;//ITEM_BASEIMAGENUMBER
	bbnums = atoi( arg);
	ITEM_setInt( itemindex, ITEM_BASEIMAGENUMBER, bbnums);
	ITEM_setWorkInt( itemindex, ITEM_CANPICKUP, 1);
	
	sprintf( itemname,"%s%s%s",CHAR_getChar( charaindex, CHAR_NAME), "��", ITEM_getChar( itemindex, ITEM_SECRETNAME));
	ITEM_setChar( itemindex, ITEM_SECRETNAME, itemname);
}
#endif

#ifdef _SUIT_ITEM

/*
#ifdef _SUIT_TWFWENDUM
	#define MAX_SUITTYPE 18
#else
	#ifdef _SUIT_ADDENDUM
		#define MAX_SUITTYPE 14
	#else
		#ifdef _MAGIC_RESIST_EQUIT			// WON ADD ְҵ����װ��
			#define MAX_SUITTYPE 11
		#else
			#define MAX_SUITTYPE 8
		#endif//_MAGIC_RESIST_EQUIT
	#endif//_SUIT_ADDENDUM
#endif//_SUIT_TWFWENDUM
*/
void ITEM_CheckSuitEquip( int charaindex)
{
	int i, j, itemindex, defCode=-1, same=0;
	int nItem[CHAR_STARTITEMARRAY];
	int maxitem;
	struct tagIntSuit{
		char fun[256];
		int intfun;	//CHAR_getInt
	};
	struct tagIntSuit ListSuit[]={
		{"VIT",CHAR_WORKSUITVIT},
		{"FSTR",CHAR_WORKSUITMODSTR},
		{"MSTR",CHAR_WORKSUITSTR},
		{"MTGH",CHAR_WORKSUITTGH},
		{"MDEX",CHAR_WORKSUITDEX},	
		{"WAST",CHAR_WORKSTATUSWATER},
		{"HP", CHAR_WORKROUNDHP},
		{"MP", CHAR_WORKROUNDMP}
#ifdef _MAGIC_RESIST_EQUIT			// WON ADD ְҵ����װ��
	   ,{"FRES", CHAR_WORK_F_SUIT},
	   {"IRES", CHAR_WORK_I_SUIT},
		{"TRES", CHAR_WORK_T_SUIT}
#endif
#ifdef _SUIT_ADDENDUM
	   ,{"RESIST",CHAR_WORKRESIST},   //�쳣������
		{"COUNTER",CHAR_WORKCOUNTER}, //������
		{"M_POW",CHAR_WORKMPOWER}      //��ǿ��ʦ��ħ��
#endif
#ifdef _SUIT_TWFWENDUM
	   ,{"EARTH",CHAR_WORK_EA},   //��
	   {"WRITER",CHAR_WORK_WR},   //ˮ
	   {"FIRE",CHAR_WORK_FI},   //��
	   {"WIND",CHAR_WORK_WI}     //��
#endif
#ifdef _SUIT_ADDPART3
	   ,{"WDUCKPOWER",CHAR_WORKDUCKPOWER},//��װ�ر�
	   {"RENOCASE",CHAR_WORKRENOCAST}, //��Ĭ������
	   {"SUITSTRP",CHAR_WORKSUITSTR_P},//������ ��λΪ%
	   {"SUITTGH_P",CHAR_WORKSUITTGH_P},//������ ��λΪ%
	   {"SUITDEXP",CHAR_WORKSUITDEX_P}//������ ��λΪ%
#endif
#ifdef _SUIT_ADDPART4
   	   ,{"SUITPOISON",CHAR_SUITPOISON},//����װ��
	   {"M2_POW",CHAR_WORKMPOWER2},      //��ǿ��ʦ��ħ��,��M_POW������ͬ(����30%)
		{"UN_POW_M",CHAR_WORKUNMPOWER}		//�ֿ���ʦ��ħ��	
#endif
	};
	maxitem = sizeof(ListSuit)/sizeof(ListSuit[0]);
	CHAR_setWorkInt( charaindex, CHAR_WORKSUITITEM, 0);
	for( i=0; i<maxitem/*MAX_SUITTYPE*/; i++)	{
		CHAR_setWorkInt( charaindex, ListSuit[i].intfun, 0 );
	}
	j=0;
	for( i=0; i<CHAR_STARTITEMARRAY; i++){
		nItem[i] = -1;
		itemindex = CHAR_getItemIndex( charaindex ,i);
		if( !ITEM_CHECKINDEX( itemindex) ) continue;
		nItem[j++] = ITEM_getInt( itemindex, ITEM_SUITCODE);
	}
	for( i=0; i<j && defCode==-1; i++){
		int k;
		same = 0;
		if( nItem[i] <= 0 ) continue;
		for( k=(j-1); k>=0; k-- ){
			if( nItem[i] == nItem[k] ) same++;
		}
		if( same >= 3 && nItem[i] != 0 )defCode = nItem[i];
	}
	if( defCode == -1 ) return;
	CHAR_setWorkInt( charaindex, CHAR_WORKSUITITEM, defCode);
	for( i=0; i<CHAR_STARTITEMARRAY; i++){
		char *buf,*str, buf1[256];
		itemindex = CHAR_getItemIndex( charaindex ,i);
		if( !ITEM_CHECKINDEX( itemindex) ) continue;
		if( ITEM_getInt( itemindex, ITEM_SUITCODE) == defCode ){
			for( j=0; j<maxitem/*MAX_SUITTYPE*/; j++){
				buf = ITEM_getChar( itemindex, ITEM_ARGUMENT);
				if( strstr( buf, ListSuit[j].fun) == NULL ) continue;
				if( NPC_Util_GetStrFromStrWithDelim( buf, ListSuit[j].fun, buf1, sizeof( buf1)) == NULL )continue;
				CHAR_setWorkInt( charaindex, ListSuit[j].intfun, atoi( buf1));
			}
		}
	}
	CHAR_complianceParameter( charaindex );
}

void ITEM_suitEquip( int charaindex, int itemindex)
{
	ITEM_CheckSuitEquip( charaindex);
}

void ITEM_ResuitEquip( int charaindex, int itemindex)
{
	ITEM_CheckSuitEquip( charaindex);
}
#endif//_SUIT_ITEM

void ITEM_remNoenemy( int charaindex, int itemindex )
{
  int fd=CHAR_getWorkInt( charaindex, CHAR_WORKFD);
  int el=getEqNoenemy(fd);
  int fl=CHAR_getInt(charaindex, CHAR_FLOOR);

  if( ITEM_CHECKINDEX(itemindex) == FALSE )return;

  clearEqNoenemy(CHAR_getWorkInt(charaindex, CHAR_WORKFD));
  if (el>=200) {
    CHAR_talkToCli(charaindex, -1, "��������Ĺ�â��ʧ�ˡ�", CHAR_COLORWHITE);
    return;
  } else if (el>=120) {
    if ( (fl==100)||(fl==200)||(fl==300)||(fl==400)||(fl==500) ) {
      CHAR_talkToCli(charaindex, -1, "��������Ĺ�â��ʧ�ˡ�", CHAR_COLORWHITE);
      return;
    }
  } else if (el>=80) {
    if ( (fl==100)||(fl==200)||(fl==300)||(fl==400) ) {
      CHAR_talkToCli(charaindex, -1, "��������Ĺ�â��ʧ�ˡ�", CHAR_COLORWHITE);
      return;
    }
  } else if (el>=40) {
    if ( (fl==100)||(fl==200) ) {
      CHAR_talkToCli(charaindex, -1, "��������Ĺ�â��ʧ�ˡ�", CHAR_COLORWHITE);
      return;
    }
  }
  CHAR_talkToCli(charaindex, -1, "ʲ����Ҳû�з�����", CHAR_COLORWHITE);
}

extern void setStayEncount(int fd);
void ITEM_useEncounter( int charaindex, int toindex, int haveitemindex)
{
	int itemindex,fd;
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;	

	fd=CHAR_getWorkInt( charaindex, CHAR_WORKFD);
	setStayEncount(fd);
#ifdef _USER_CHARLOOPS
	{
		Char 	*ch;
		ch  = CHAR_getCharPointer( charaindex);
		if( ch == NULL ) return;
		strcpysafe( ch->charfunctable[CHAR_LOOPFUNCTEMP1].string,
			sizeof( ch->charfunctable[CHAR_LOOPFUNCTEMP1]), "CHAR_BattleStayLoop");//ս��
		CHAR_setInt( charaindex, CHAR_LOOPINTERVAL, 2500);
		CHAR_constructFunctable( charaindex);
	}
#endif
	CHAR_talkToCli(charaindex, -1, "����ܵ��ܱ�ͻȻ������ɱ����", CHAR_COLORYELLOW);
	CHAR_DelItem( charaindex, haveitemindex);

}

#ifdef _DEATH_CONTENDWATCH
void ITEM_useWatchBattle( int charaindex, int toindex, int haveitemindex)
{
	int itemindex,fd;
	int battleTi;
	char buf1[256];
	char *itemarg=NULL;
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;
	if( (itemarg = ITEM_getChar( itemindex, ITEM_ARGUMENT)) == NULL ) return;
	battleTi = atoi( itemarg);
	fd = CHAR_getWorkInt( charaindex, CHAR_WORKFD);

	{
		ArrangeBattleC *aBo=NULL;
		ArrangeBattleC *aB1=NULL;
		ArrangeBattleC *aB2=NULL;

		if( battleTi<0 || battleTi>MAXBATTLEPAGE ) return;
		if( (aBo=ArrangeBattleC_getInBattleArray( battleTi)) == NULL){
			sprintf( buf1, "Ŀǰ��%d��û�о�����", battleTi);
			CHAR_talkToCli(charaindex, -1, buf1, CHAR_COLORYELLOW);
			return;
		}

		if( (aB1=aBo->next[0]) == NULL ||
			(aB2=aBo->next[1]) == NULL ||
			!CHAR_CHECKINDEX( aB1->toindex) ||
			!CHAR_CHECKINDEX( aB2->toindex) ||
			CHAR_getWorkInt( aB1->toindex, CHAR_WORKBATTLEMODE) == BATTLE_CHARMODE_NONE ||
			CHAR_getWorkInt( aB2->toindex, CHAR_WORKBATTLEMODE) == BATTLE_CHARMODE_NONE ){

			sprintf( buf1, "Ŀǰ��%d��δ��ʼս����", battleTi);
			CHAR_talkToCli(charaindex, -1, buf1, CHAR_COLORYELLOW);
			return;
		}

		if( BATTLE_WatchEntry( charaindex, aB1->toindex) ){
			
		}else{
			sprintf( buf1, "��սʧ�ܣ�", battleTi);
			CHAR_talkToCli(charaindex, -1, buf1, CHAR_COLORYELLOW);
			return;
		}
	}

	sprintf( buf1, "���� %s��ʧ�ˡ�", ITEM_getChar( itemindex, ITEM_NAME) );
	CHAR_talkToCli(charaindex, -1, buf1, CHAR_COLORYELLOW);
	CHAR_DelItem( charaindex, haveitemindex);
}
#endif

#ifdef _Item_DeathAct
void ITEM_UseDeathCounter( int charaindex, int toindex, int haveitemindex)
{
	int itemindex,fd;
	int itemmaxuse=-1;
	char buf1[256];
	char *itemarg;
	char itemnumstr[32];
	int  okfloor = 0;
	BOOL Useflag=FALSE;
	int i = 1;

	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;

#ifdef _ITEM_STONE
	itemarg = ITEM_getChar(itemindex,ITEM_ARGUMENT);
	while(1){
		if( getStringFromIndexWithDelim( itemarg, "|", i , itemnumstr, sizeof( itemnumstr)) == FALSE )
			break;
		okfloor = atoi(itemnumstr);
		if( CHAR_getInt( charaindex, CHAR_FLOOR ) == okfloor ){
			Useflag = TRUE;
			break;
		}
		i++;
	}
	if(okfloor != 0){   // ��ֵ�ʯͷ
		itemmaxuse = ITEM_getInt( itemindex, ITEM_DAMAGEBREAK);	
		if( itemmaxuse != -1 )	{
			itemmaxuse--;
			ITEM_setInt( itemindex, ITEM_DAMAGEBREAK, itemmaxuse);
			if( itemmaxuse < 1 )	{
				sprintf( buf1, "%s��ʧ�ˡ�", ITEM_getChar( itemindex, ITEM_NAME) );
				CHAR_talkToCli(charaindex, -1, buf1, CHAR_COLORYELLOW);
				CHAR_DelItem( charaindex, haveitemindex);
				if(Useflag==FALSE) {
					CHAR_talkToCli(charaindex, -1, "û�з����κ����飡", CHAR_COLORYELLOW);
					return;
				}
			}else{
				sprintf( buf1, "ԭ�����У���ʹ�ô���ʣ��%d�Ρ�", itemmaxuse);	
				ITEM_setChar( itemindex, ITEM_EFFECTSTRING, buf1);
				CHAR_sendItemDataOne( charaindex, haveitemindex);
				if(Useflag==FALSE) {
					CHAR_talkToCli(charaindex, -1, "û�з����κ����飡", CHAR_COLORYELLOW);
					return;
				}
			}
			fd = CHAR_getWorkInt( charaindex, CHAR_WORKFD);
			setStayEncount(fd);
			CHAR_talkToCli(charaindex, -1, "���������������Ϣ��", CHAR_COLORYELLOW);
		}else{
			sprintf( buf1, "%s��ʧ�ˡ�", ITEM_getChar( itemindex, ITEM_NAME) );
			CHAR_talkToCli(charaindex, -1, buf1, CHAR_COLORYELLOW);
			CHAR_DelItem( charaindex, haveitemindex);
			if(Useflag==FALSE) {
				CHAR_talkToCli(charaindex, -1, "û�з����κ����飡", CHAR_COLORYELLOW);
				return;
			}
			fd = CHAR_getWorkInt( charaindex, CHAR_WORKFD);
			setStayEncount(fd);
			CHAR_talkToCli(charaindex, -1, "���������������Ϣ��", CHAR_COLORYELLOW);		
		}
	}else{		//��ħ��ʯ
#endif
#ifdef _ITEM_MAXUSERNUM
	itemmaxuse = ITEM_getInt( itemindex, ITEM_DAMAGEBREAK);
	if( itemmaxuse != -1 )	{
		itemmaxuse--;
		ITEM_setInt( itemindex, ITEM_DAMAGEBREAK, itemmaxuse);
		if( itemmaxuse < 1 )	{
			sprintf( buf1, "���� %s��ʧ�ˡ�", ITEM_getChar( itemindex, ITEM_NAME) );
			CHAR_talkToCli(charaindex, -1, buf1, CHAR_COLORYELLOW);
			CHAR_DelItem( charaindex, haveitemindex);
		}else{
			sprintf( buf1, "ԭ�����У���ʹ�ô���ʣ��%d�Ρ�", itemmaxuse);
			ITEM_setChar( itemindex, ITEM_EFFECTSTRING, buf1);
			CHAR_sendItemDataOne( charaindex, haveitemindex);
		}
		fd = CHAR_getWorkInt( charaindex, CHAR_WORKFD);
		setStayEncount(fd);
		CHAR_talkToCli(charaindex, -1, "����ܵ��ܱ�ͻȻ������ɱ����", CHAR_COLORYELLOW);
	}else{
		CHAR_DelItem( charaindex, haveitemindex);
		return;
	}
#else
	fd = CHAR_getWorkInt( charaindex, CHAR_WORKFD);
	setStayEncount(fd);
	sprintf( buf1, "���� %s��ʧ�ˡ�", ITEM_getChar( itemindex, ITEM_NAME) );
	CHAR_talkToCli(charaindex, -1, buf1, CHAR_COLORYELLOW);
	CHAR_DelItem( charaindex, haveitemindex);
#endif
#ifdef _ITEM_STONE
	}
#endif
#ifdef _USER_CHARLOOPS
	{
		Char 	*ch;
		ch  = CHAR_getCharPointer( charaindex);
		if( ch == NULL ) return;
		strcpysafe( ch->charfunctable[CHAR_LOOPFUNCTEMP1].string,
			sizeof( ch->charfunctable[CHAR_LOOPFUNCTEMP1]), "CHAR_BattleStayLoop");//ս��
		CHAR_setInt( charaindex, CHAR_LOOPINTERVAL, 2500);
		CHAR_constructFunctable( charaindex);
	}
#endif
}
#endif

#ifdef _CHRISTMAS_REDSOCKS
void ITEM_useMaxRedSocks( int charaindex, int toindex, int haveitemindex)
{
	int itemtimes = -1, itemindex;
	char *itemarg=NULL;
	int present[13]={ 13061, 13062, 13063, 13064, 13088, 13089, 13090, 13091, //1.
						14756, 17256,
					    13092,19692,20594};
	int nowtimes = time( NULL);

	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;

	itemarg = ITEM_getChar( itemindex, ITEM_ARGUMENT);
	if( itemarg == NULL ) {
		CHAR_talkToCli(charaindex, -1, "������Ч!", CHAR_COLORYELLOW);
		return;
	}
	itemtimes = atoi( itemarg);
	if( nowtimes >= itemtimes && nowtimes <= itemtimes+(60*60*24) ){ //����ʱ���ڿɻ�����
		int si=0, ret;
		char token[256];
		//ɾ��
		CHAR_setItemIndex(charaindex, haveitemindex ,-1);
		CHAR_sendItemDataOne( charaindex, haveitemindex);
		ITEM_endExistItemsOne(itemindex);
		itemindex = -1;
		//����
		si = rand()%100;
		if( si > 70 ){
			si = rand()%3+10;
		}else if( si > 60 ){
			si = rand()%2+8;
		}else {
			si = rand()%8;
		}
		itemindex = ITEM_makeItemAndRegist( present[ si]);
		if( !ITEM_CHECKINDEX( itemindex)){
			CHAR_talkToCli(charaindex, -1, "������Ч!", CHAR_COLORYELLOW);
			return;
		}
		ret = CHAR_addItemSpecificItemIndex( charaindex, itemindex);
		if( ret < 0 || ret >= CHAR_MAXITEMHAVE ) {
			ITEM_endExistItemsOne( itemindex);
			CHAR_talkToCli(charaindex, -1, "�����÷���λ����!", CHAR_COLORYELLOW);
			return;
		}
		sprintf( token,"�õ�%s",ITEM_getChar( itemindex, ITEM_NAME));
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW );
		CHAR_sendItemDataOne( charaindex, ret);
	}else{
		char token[256];
		if( nowtimes < itemtimes ){
			int days, hours, minute, second;
			int defTimes = itemtimes - nowtimes;

			days = defTimes/(24*60*60);
			defTimes = defTimes-( days*(24*60*60));
			hours = defTimes/(60*60);
			defTimes = defTimes-( hours*(60*60));
			minute = defTimes/60;
			defTimes = defTimes-( minute*60);
			second = defTimes;
			sprintf( token,"%s����%d��%dСʱ%d��%d��ſ�ʹ��!",
				ITEM_getChar( itemindex, ITEM_NAME), days, hours, minute, second);
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW );
		}else if( nowtimes > itemtimes+(60*60*24) ){
			sprintf( token,"%sʹ�������ѹ�!", ITEM_getChar( itemindex, ITEM_NAME));
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW );
		}
	}
}
#endif

#ifdef _CHRISTMAS_REDSOCKS_NEW
void ITEM_useMaxRedSocksNew( int charaindex, int toindex, int haveitemindex)
{
	int itemindex;
	char *itemarg=NULL;
	char itemnumstr[32];
	int itemnum=0;
	int present[20];
    int si=0, ret,i;
	char token[256];

	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;

	itemarg = ITEM_getChar( itemindex, ITEM_ARGUMENT);
	if( itemarg == NULL ){
		CHAR_talkToCli(charaindex, -1, "���Ǹ�����ʥ����!", CHAR_COLORYELLOW);
		return;
	}

	//��������
	if( getStringFromIndexWithDelim( itemarg, "|", 1, itemnumstr, sizeof( itemnumstr)) == FALSE )
		return;
    itemnum = atoi(itemnumstr);
	if( itemnum > 20 )
		itemnum = 20;
	for(i=0;i<itemnum;i++){
		if( getStringFromIndexWithDelim( itemarg, "|", 2+i, itemnumstr, sizeof( itemnumstr)) )
		    present[i] = atoi(itemnumstr);
	}
	
	//ɾ��
	CHAR_setItemIndex(charaindex, haveitemindex ,-1);
	CHAR_sendItemDataOne( charaindex, haveitemindex);
	ITEM_endExistItemsOne(itemindex);
	itemindex = -1;
	//����
	si = rand()%itemnum;

	itemindex = ITEM_makeItemAndRegist( present[ si]);
	if( !ITEM_CHECKINDEX( itemindex)){
		CHAR_talkToCli(charaindex, -1, "������Ч!", CHAR_COLORYELLOW);
		return;
	}
	ret = CHAR_addItemSpecificItemIndex( charaindex, itemindex);
	if( ret < 0 || ret >= CHAR_MAXITEMHAVE ) {
		ITEM_endExistItemsOne( itemindex);
		CHAR_talkToCli(charaindex, -1, "�����÷���λ����!", CHAR_COLORYELLOW);
		return;
	}
	sprintf( token,"�õ�%s",ITEM_getChar( itemindex, ITEM_NAME));
	CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW );
	CHAR_sendItemDataOne( charaindex, ret);

}
#endif

#ifdef _PETSKILL_CANNEDFOOD
void ITEM_useSkillCanned( int charaindex, int toindex, int itemNo)
{
	int itemindex;
	char buf1[256];
	itemindex = CHAR_getItemIndex( charaindex, itemNo);
	if(!ITEM_CHECKINDEX( itemindex)) return;
	if( !CHAR_CHECKINDEX( toindex) ) return;
	if( CHAR_getInt( toindex, CHAR_WHICHTYPE) == CHAR_TYPEPET) {
		int i, petNo=-1;
		for( i=0; i<CHAR_MAXPETHAVE; i++)	{
			if( toindex == CHAR_getCharPet( charaindex, i) ){
				petNo = i;
				break;
			}
		}
		if( petNo == -1 ){
			sprintf( buf1, "%s�����������ϡ�", CHAR_getChar( toindex, CHAR_NAME) );
			CHAR_talkToCli( charaindex, -1, buf1, CHAR_COLORYELLOW);
			return;
		}
		ITEM_usePetSkillCanned_PrintWindow( charaindex, petNo);
		CHAR_setWorkInt( charaindex, CHAR_WORKRENAMEITEMNUM, petNo);
		CHAR_setWorkInt( charaindex, CHAR_WORKRENAMEITEMINDEX, itemNo);
	}else{
		sprintf( buf1, "���� %s���޳���ʹ�á�", ITEM_getChar( itemindex, ITEM_NAME) );
		CHAR_talkToCli( charaindex, -1, buf1, CHAR_COLORYELLOW);
		return;
	}

}
#endif

#ifdef _ITEM_METAMO
void ITEM_metamo( int charaindex, int toindex, int haveitemindex )
{
	
	char *arg, msg[128];
	int itemindex, metamoTime, haveindex, battlemode, i;

	if( CHAR_CHECKINDEX( charaindex ) == FALSE )	return;
	//print(" PetMetamo_toindex:%d ", toindex);

	if( CHAR_getInt( charaindex, CHAR_RIDEPET ) != -1 ){
		CHAR_talkToCli( charaindex, -1, "�޷���������в��ܱ���", CHAR_COLORYELLOW );
		return;
	}
#ifdef _FIX_METAMORIDE
	if( CHAR_CHECKJOINENEMY( charaindex) == TRUE ){
		CHAR_talkToCli( charaindex, -1, "�޷���������в��ܱ���", CHAR_COLORYELLOW );
		return;
	}
#else
	if( CHAR_getInt( charaindex, CHAR_BASEIMAGENUMBER) == 100259 ){
		CHAR_talkToCli( charaindex, -1, "�޷���������в��ܱ���", CHAR_COLORYELLOW );
		return;
	}
#endif
#ifdef _PETSKILL_BECOMEPIG
    if( CHAR_getInt( charaindex, CHAR_BECOMEPIG) > -1 ){//���������
		CHAR_talkToCli( charaindex, -1, "�޷������������в��ܱ���", CHAR_COLORYELLOW );
	    return;
	}
#endif


#ifdef _FIXBUG_ATTACKBOW
	{
		int armindex = CHAR_getItemIndex( charaindex, CHAR_ARM);
		if( ITEM_CHECKINDEX( armindex) == TRUE )	{
			int armtype = BATTLE_GetWepon( charaindex);
			if( armtype == ITEM_BOW || armtype == ITEM_BOUNDTHROW ||
				armtype == ITEM_BREAKTHROW || armtype == ITEM_BOOMERANG)	{
				CHAR_talkToCli( charaindex, -1, "ʹ��Զ���������޷�����", CHAR_COLORYELLOW );
				return;
			}
		}
	}
#endif

	battlemode = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE );
	// ��  ����ݷ�  ������
	if( battlemode == BATTLE_CHARMODE_INIT ){
		return;
	}
	// ��    ��������
	if( IsBATTLING( charaindex ) == TRUE ){
		toindex = BATTLE_No2Index(CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX ), toindex );

	}
	
	if( CHAR_CHECKINDEX( toindex ) == FALSE )	return;

	haveindex = -1;	
	for( i = 0; i < 5; i++ ){
		if( CHAR_getCharPet( charaindex, i) == toindex ) {
			haveindex = i;
			break;
		}
	}
	if( haveindex == -1 && charaindex != toindex ){
		CHAR_talkToCli( charaindex, -1, "�޷�����ֻ�ܱ���Լ��ĳ��", CHAR_COLORYELLOW );
		return;
	}

	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;
	
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );
	if( arg == NULL )return;
	if( sscanf( arg, "%d", &metamoTime) != 1 )
		return;


	if( toindex != charaindex ){
		CHAR_setWorkInt( charaindex, CHAR_WORKITEMMETAMO, NowTime.tv_sec +metamoTime);
		sprintf( msg, "�����%s��", CHAR_getChar( toindex, CHAR_NAME) );
	}
	else {
		CHAR_setWorkInt( charaindex, CHAR_WORKITEMMETAMO, 0);
		sprintf( msg, "����Լ���");
	}
	CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORYELLOW );

	CHAR_setInt( charaindex, CHAR_BASEIMAGENUMBER, CHAR_getInt( toindex, CHAR_BASEBASEIMAGENUMBER) );
	CHAR_complianceParameter( charaindex);
	CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charaindex , CHAR_WORKOBJINDEX ));
	CHAR_send_P_StatusString( charaindex , CHAR_P_STRING_BASEBASEIMAGENUMBER);

	CHAR_DelItem( charaindex, haveitemindex);

}
#endif
#ifdef _ITEM_CRACKER
void ITEM_Cracker(int charaindex,int toindex,int haveitemindex)
{
	int battlemode;
	// �������Ƿ���Ч
	if(CHAR_CHECKINDEX(charaindex) == FALSE) return; //ʧ��
	battlemode = CHAR_getWorkInt(charaindex,CHAR_WORKBATTLEMODE);
	// ս����ʹ����Ч
	if(!battlemode) ITEM_useCracker_Effect(charaindex,toindex,haveitemindex);
	else CHAR_talkToCli(charaindex,-1,"ʲ��Ҳû������",CHAR_COLORWHITE);
}
#endif

void ITEM_AddPRSkillPercent( int charaindex,int toindex,int haveitemindex)
{
#ifdef _PROFESSION_SKILL			// WON ADD ����ְҵ����

	int MySKPercent=0, itemindex=-1, i;

	if( !CHAR_CHECKINDEX( charaindex) ) return;
	if( CHAR_getInt( charaindex, PROFESSION_CLASS ) == 0 ) return;

	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX( itemindex) )return;

	for( i=0; i<PROFESSION_MAX_LEVEL; i++ ){
		if( CHAR_getIntPSkill( charaindex, i, SKILL_IDENTITY) == -1 ){
			continue;
		}
		MySKPercent = CHAR_getIntPSkill( charaindex, i, SKILL_LEVEL)+10;
		if( MySKPercent > 100 ) MySKPercent = 100;
		CHAR_setIntPSkill( charaindex, i, SKILL_LEVEL, MySKPercent);
	}

    CHAR_setItemIndex( charaindex, haveitemindex ,-1);
	CHAR_sendItemDataOne( charaindex, haveitemindex);
	ITEM_endExistItemsOne( itemindex );
	CHAR_sendStatusString( charaindex , "S");
    CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charaindex , CHAR_WORKOBJINDEX ));
	CHAR_talkToCli( charaindex, -1, "����ְҵ����������������������", CHAR_COLORYELLOW);
#endif
}

void ITEM_AddPRSkillPoint( int charaindex,int toindex,int haveitemindex)
{
#ifdef _PROFESSION_SKILL			// WON ADD ����ְҵ����

	int MyPRpoint=0, itemindex=-1;
	if( !CHAR_CHECKINDEX( charaindex) ) return;
	if( CHAR_getInt( charaindex, PROFESSION_CLASS ) == 0 ) return;

	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX( itemindex) )return;

	MyPRpoint = CHAR_getInt( charaindex, PROFESSION_SKILL_POINT);
	CHAR_setInt( charaindex, PROFESSION_SKILL_POINT, MyPRpoint+1 );


    CHAR_setItemIndex( charaindex, haveitemindex ,-1);
	CHAR_sendItemDataOne( charaindex, haveitemindex);
	ITEM_endExistItemsOne( itemindex );

    CHAR_sendCToArroundCharacter( CHAR_getWorkInt( charaindex , CHAR_WORKOBJINDEX ));

	CHAR_talkToCli( charaindex, -1, "����һ��ְҵ���ܵ�����", CHAR_COLORYELLOW);
#endif
}

#ifdef _ITEM_ADDEXP	//vincent ��������
void ITEM_Addexp(int charaindex,int toindex,int haveitemindex)
{
	// �������Ƿ���Ч
	if(CHAR_CHECKINDEX(charaindex) == FALSE) return; //ʧ��
#if 1
	ITEM_useAddexp_Effect(charaindex,toindex,haveitemindex);
#else
	if( !CHAR_getWorkInt(charaindex,CHAR_WORKITEM_ADDEXP)){
		ITEM_useAddexp_Effect(charaindex,toindex,haveitemindex);
	}else{
		CHAR_talkToCli(charaindex,-1,"��ǰʹ��֮ҩЧ��Ȼ����",CHAR_COLORYELLOW);
	}
#endif
}
#endif

#ifdef _ITEM_REFRESH //vincent ����쳣״̬����
void ITEM_Refresh(int charaindex,int toindex,int haveitemindex)
{
	int battlemode,itemindex;
print("\nvincent--ITEM_Refresh");
		// �������Ƿ���Ч
	if(CHAR_CHECKINDEX(charaindex) == FALSE) 
	{
print("\nvincent--(charaindex) == FALSE");
		return; //ʧ��
	}
	itemindex = CHAR_getItemIndex(charaindex,haveitemindex);

	battlemode = CHAR_getWorkInt(charaindex,CHAR_WORKBATTLEMODE);
	if(battlemode)
	{
print("\nvincent--enter ITEM_useAddexp_Effect");
print("\nvincent-->charaindex:%d,toindex:%d",charaindex,toindex);
		ITEM_useRefresh_Effect(charaindex,toindex,haveitemindex);
	}
	else CHAR_talkToCli(charaindex,-1,"ʲ��Ҳû������",CHAR_COLORWHITE);

    /* ƽ�ҷ�����������    �����������ջ� */
    CHAR_setItemIndex(charaindex, haveitemindex ,-1);
	CHAR_sendItemDataOne( charaindex, haveitemindex);/* ʧ��  ة��ޥ */
	/* ���� */
	ITEM_endExistItemsOne( itemindex );
}
#endif
//Terry 2001/12/21
#ifdef _ITEM_FIRECRACKER
void ITEM_firecracker(int charaindex,int toindex,int haveitemindex)
{
	int battlemode;

	// �������Ƿ���Ч
	if(CHAR_CHECKINDEX(charaindex) == FALSE) return; //ʧ��

	battlemode = CHAR_getWorkInt(charaindex,CHAR_WORKBATTLEMODE);
	
	if( battlemode // ����Ƿ���ս����
#ifdef _PETSKILL_BECOMEPIG
	    && CHAR_getInt( charaindex, CHAR_BECOMEPIG) == -1 
#endif
		) 
		ITEM_useFirecracker_Battle(charaindex,toindex,haveitemindex);
	else 
		CHAR_talkToCli(charaindex,-1,"ʲ��Ҳû������",CHAR_COLORWHITE);
}
#endif
//Terry end


void ITEM_WearEquip( int charaindex, int itemindex)
{
	// WON ADD
//	if( ITEM_getInt(itemindex,ITEM_ID) == 20130 ){
		CHAR_setWorkInt( charaindex, CHAR_PickAllPet, TRUE);
///	}
	return;
}
void ITEM_ReWearEquip( int charaindex, int itemindex)
{
	CHAR_setWorkInt( charaindex, CHAR_PickAllPet, FALSE);
	return;
}


#ifdef _Item_ReLifeAct
void ITEM_DIErelife( int charaindex, int itemindex, int eqw)
{
	int ReceveEffect=-1;
	int toNo;
	int battleindex=-1;
	int attackNo=-1;
	int WORK_HP=1;
	char buf[256];
	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
	if( BATTLE_CHECKINDEX( battleindex ) == FALSE )	{
		print("\n battleindex =%d return", battleindex);
		return;
	}
	if( CHAR_CHECKINDEX( charaindex) == FALSE )
		return;
	if( ITEM_CHECKINDEX( itemindex) == FALSE )
		return;
#ifdef _DUMMYDIE
	if( CHAR_getFlg( charaindex, CHAR_ISDUMMYDIE) == FALSE ) {
#else
	if( CHAR_getFlg( charaindex, CHAR_ISDIE) == FALSE )	{
#endif
		print("\n [ %d, CHAR_ISDIE DUMMY FALSE] return !", charaindex);
		return;
	}

	if( ITEM_getArgument( ITEM_getChar(itemindex,ITEM_ARGUMENT),"HP", buf, sizeof(buf) )
		== FALSE ){
		WORK_HP = 1;
	}else	{
		if( !strcmp( buf, "FULL") )	{
			WORK_HP = CHAR_getWorkInt( charaindex, CHAR_WORKMAXHP );
		}else	{
			WORK_HP=atoi( buf);
		}
	}

	ReceveEffect = SPR_fukkatu3;
	toNo = BATTLE_Index2No( battleindex, charaindex );
	attackNo = -1;

	BATTLE_MultiReLife( battleindex, attackNo, toNo, WORK_HP, ReceveEffect );
	CHAR_setItemIndex( charaindex, eqw ,-1);
	ITEM_endExistItemsOne( itemindex);
	CHAR_sendItemDataOne( charaindex, eqw);
	return;
}
#endif

#ifdef _EQUIT_DEFMAGIC
void ITEM_MagicEquitWear( int charaindex, int itemindex)
{
	char buf[256];
	char *itemarg;
	char Free[][128]={"EA","WA","FI","WI","QU"};
	int index=0;
	int dMagic=0;
	itemarg = ITEM_getChar(itemindex,ITEM_ARGUMENT);
	memset( buf, 0, sizeof( char) *256);
	if( itemarg == NULL ) return;
	while( index < arraysizeof( Free) )	{
		if( ITEM_getArgument( itemarg, Free[ index], buf, sizeof(buf)) == TRUE )	{
			dMagic = atoi( buf);
			if( dMagic <= 100 && dMagic >= -100 )	{
				int def_magic = CHAR_getWorkInt( charaindex, CHAR_EQUITDEFMAGIC_E+index);
				CHAR_setWorkInt( charaindex, CHAR_EQUITDEFMAGIC_E+index, def_magic + dMagic);
			}
			dMagic = 0;
		}
		index++;
		if( (CHAR_EQUITDEFMAGIC_E+index) > CHAR_EQUITQUIMAGIC )
			break;
	}

	return;
}

void ITEM_MagicEquitReWear( int charaindex, int itemindex){
	char buf[256];
	char *itemarg;
	char Free[][128]={"EA","WA","FI","WI","QU"};
	int index=0;
	int dMagic=0;

	itemarg = ITEM_getChar(itemindex,ITEM_ARGUMENT);
	memset( buf, 0, sizeof( char) *256);
	if( itemarg == NULL ) return;
	while( index < arraysizeof( Free) )	{
		if( ITEM_getArgument( itemarg, Free[ index], buf, sizeof(buf)) == TRUE )	{
			dMagic = atoi( buf);
			if( dMagic <= 100 && dMagic >= -100 )	{
				int def_magic = CHAR_getWorkInt( charaindex, CHAR_EQUITDEFMAGIC_E+index);
				CHAR_setWorkInt( charaindex, CHAR_EQUITDEFMAGIC_E+index, def_magic - dMagic);
			}
			dMagic = 0;
		}
		index++;
		if( (CHAR_EQUITDEFMAGIC_E+index) > CHAR_EQUITQUIMAGIC )
			break;
	}

	return;	
}
#endif

#ifdef _EQUIT_RESIST
void ITEM_MagicResist( int charaindex, int itemindex)
{
	char *itemarg,*p=NULL;
	itemarg = ITEM_getChar(itemindex,ITEM_ARGUMENT);
	if( itemarg == NULL ) return;
	if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITFIRE, atoi( p+4 ) );
	}
	else if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITTHUNDER, atoi( p+4 ) );
	}
	else if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITICE, atoi( p+4 ) );
	}
	else if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITWEAKEN, atoi( p+4 ) );
	}
	else if( p = strstr( itemarg, "ħ��" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITBARRIER, atoi( p+4 ) );
	}
	else if( p = strstr( itemarg, "��Ĭ" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITNOCAST, atoi( p+4 ) );
	}
	else if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITFALLRIDE, atoi( p+4 ) );
	}

}

void ITEM_MagicReResist( int charaindex, int itemindex)
{
	char *itemarg,*p=NULL;
	itemarg = ITEM_getChar(itemindex,ITEM_ARGUMENT);
	if( itemarg == NULL ) return;
	if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITFIRE, 0 );
	}
	else if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITFIRE, 0 );
	}
	else if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITFIRE, 0 );
	}
	else if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITFIRE, 0 );
	}
	else if( p = strstr( itemarg, "ħ��" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITFIRE, 0 );
	}
	else if( p = strstr( itemarg, "��Ĭ" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITFIRE, 0 );
	}
	else if( p = strstr( itemarg, "����" ) ){
		CHAR_setWorkInt( charaindex, CHAR_WORKEQUITFIRE, 0 );
	}
}
#endif

#ifdef _MAGIC_RESIST_EQUIT			// WON ADD ְҵ����װ��    
void ITEM_P_MagicEquitWear( int charaindex, int itemindex )
{
	char buf[256] = {0};
	char *itemarg;
	char Free[][128]={"FR","IR","TR"};
	int dMagic=0;
	int i;
	
	itemarg = ITEM_getChar(itemindex,ITEM_ARGUMENT);
	if( itemarg == NULL ) return;

//	print("\n won test 10.0 ==> itemarg(%s)", itemarg );

	for( i=0; i<3; i++ ){
		if( ITEM_getArgument( itemarg, Free[ i ], buf, sizeof(buf)) == TRUE )	{
			dMagic = atoi( buf);
//			print("\n won test 10.1 ==> i(%d) dMagic(%d)", i, dMagic );

			if( dMagic <= 100 && dMagic >= -100 ){
				int def_magic = CHAR_getWorkInt( charaindex, CHAR_WORK_F_SUIT+i );
				CHAR_setWorkInt( charaindex, CHAR_WORK_F_SUIT+i, def_magic + dMagic);

//				print("\n won test 10.2 ==> i(%d)(%d)", i, CHAR_getWorkInt( charaindex, CHAR_WORK_F_SUIT+i ) );
			}
			dMagic = 0;
		}
	}

	return;
}


void ITEM_P_MagicEquitReWear( int charaindex, int itemindex )
{
	char buf[256] = {0};
	char *itemarg;
	char Free[][128]={"FR","IR","TR"};
	int dMagic=0;
	int i;

	itemarg = ITEM_getChar(itemindex,ITEM_ARGUMENT);
	if( itemarg == NULL ) return;

//	print("\n won test 10.0 ==> itemarg(%s)", itemarg );

	for( i=0; i<3; i++ ){
		if( ITEM_getArgument( itemarg, Free[ i ], buf, sizeof(buf)) == TRUE )	{
			dMagic = atoi( buf);
//			print("\n won test 10.1 ==> i(%d) dMagic(%d)", i, dMagic );

			if( dMagic <= 100 && dMagic >= -100 ){
				int def_magic = CHAR_getWorkInt( charaindex, CHAR_WORK_F_SUIT+i );
				CHAR_setWorkInt( charaindex, CHAR_WORK_F_SUIT+i, def_magic - dMagic);

//				print("\n won test 10.2 ==> i(%d)(%d)", i, CHAR_getWorkInt( charaindex, CHAR_WORK_F_SUIT+i ) );
			}
			dMagic = 0;
		}
	}

	return;

}
#endif



#ifdef _ITEM_WARP_FIX_BI
void recoverbi(int index)
{
	int eBbi=-1, eArm=-1, eNum=-1, eBi=-1, bi=-1;	

	bi   = CHAR_getInt( index, CHAR_BASEIMAGENUMBER);	
	eBbi = CHAR_getInt( index, CHAR_BASEBASEIMAGENUMBER);
	eArm = CHAR_getItemIndex( index, CHAR_ARM);
	eNum = ITEM_FIST;
	if(ITEM_CHECKINDEX(eArm))
		eNum = ITEM_getInt( eArm, ITEM_TYPE);
	
	eBi = CHAR_getNewImagenumberFromEquip( eBbi, eNum);	

	if(CHAR_getInt( index, CHAR_RIDEPET)!=-1)	     //���			
		CHAR_complianceParameter(index);
	else                                             //�����		
		if((eBi!=-1)&&(eBi!=bi))
			CHAR_setInt( index, CHAR_BASEIMAGENUMBER, eBi);
}
#endif

#ifdef _ITEM_TIME_LIMIT
void ITEM_TimeLimit( int charaindex)
{
    int  i      = 0;
	int  iid    = 0;
	long lTime  = 0;
	char buff[256];

	if( !CHAR_CHECKINDEX( charaindex) ) return;
	for( i=0; i < CHAR_MAXITEMHAVE ; i++ ){
		int itemindex = CHAR_getItemIndex( charaindex , i );
		if( !ITEM_CHECKINDEX( itemindex ) ) continue;
		lTime = ITEM_getWorkInt( itemindex, ITEM_WORKTIMELIMIT);
		if( ITEM_getInt( itemindex, ITEM_ID) == 20173 //ȼ�ջ��
			|| ITEM_getInt( itemindex, ITEM_ID) == 20704 ){
			if( lTime > 0 && NowTime.tv_sec > lTime ){
				iid = ITEM_getInt( itemindex, ITEM_ID) + 1;
				snprintf( buff, sizeof( buff), "%s��Ч������ʧ..", ITEM_getChar( itemindex, ITEM_NAME));
				CHAR_talkToCli( charaindex, -1, buff, CHAR_COLORGREEN);
				CHAR_DelItemMess( charaindex, i, 0);
				itemindex = ITEM_makeItemAndRegist( iid);
				if(itemindex!=-1){
					CHAR_setItemIndex( charaindex, i, itemindex);
					ITEM_setWorkInt( itemindex, ITEM_WORKOBJINDEX, -1);
					ITEM_setWorkInt( itemindex, ITEM_WORKCHARAINDEX, charaindex);
					CHAR_sendItemDataOne( charaindex, i);
				}
			}
		}
	}	

}
#endif

#ifdef _BLACK_MARKET
void ITEM_BM_Exchange( int charaindex, int iindex)
{
	BOOL pckConfirm=FALSE;
	int  cdnConfirm=0;
	int  i, j, fd, id, itemindex, gTmp;
	char sBuf[256]="";

	fd = getfdFromCharaIndex( charaindex);

	if(!CHAR_CHECKINDEX( charaindex)) return;    	
    if( CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE)!=BATTLE_CHARMODE_NONE) return;
	if( CHAR_getWorkInt( charaindex, CHAR_WORKPARTYMODE)!=CHAR_PARTY_NONE) return;
	if( CHAR_getFlg( charaindex, CHAR_ISTRADE)) return;
    if( iindex==-1) return;

	for( i=0; i<12; i++){
		if(iindex==CONNECT_get_BMList( getfdFromCharaIndex( charaindex), i)){
			pckConfirm = TRUE;
			break;
		}
	}

	if(pckConfirm){	    
		if(!BMItem[iindex].GCondition||BMItem[iindex].GCondition<CHAR_getInt( charaindex, CHAR_GOLD)){
			for( i=0; i<4; i++){
				if(!BMItem[iindex].iCondition[i])
					cdnConfirm = cdnConfirm | (1<<i);
			}
			for( i=0; i<CHAR_MAXITEMHAVE; i++){
				itemindex = CHAR_getItemIndex( charaindex, i);
				if(ITEM_CHECKINDEX( itemindex)){
					id = ITEM_getInt( itemindex, ITEM_ID);
					for( j=0; j<4; j++){
						if(!(cdnConfirm&(1<<j))){
							if(BMItem[iindex].iCondition[j]==id)
								cdnConfirm = cdnConfirm | (1<<j);
						}
					}
				}				
			}
			if(cdnConfirm==15){								
				for( i=0; i<4; i++){
					if(!BMItem[iindex].iCondition[i])
						cdnConfirm = cdnConfirm & ~(1<<i);					    
				}
				for( i=0; i<CHAR_MAXITEMHAVE; i++){							
					itemindex = CHAR_getItemIndex( charaindex, i);				    					
					if(ITEM_CHECKINDEX( itemindex)){					    
						id = ITEM_getInt( itemindex, ITEM_ID);						
						for( j=0; j<4; j++){
					     	if(cdnConfirm&(1<<j)){
								if(BMItem[iindex].iCondition[j]==id){
									// add del log
									CHAR_DelItem( charaindex, i);
									cdnConfirm = cdnConfirm & ~(1<<j);									
									break;
								}
							}
						}						
					}										
				}
				{					
					int ret, mId, rn;

					rn = RAND( 0, BMIMAX-1);					
					if(rn==0)
						mId = BMItem[iindex].iId[0][RAND( 0, 2)];
					else if((rn%10)==0)
						mId = BMItem[iindex].iId[1][RAND( 0, 2)];
		            else if((rn%3)==0)
						mId = BMItem[iindex].iId[2][RAND( 0, 2)];
		            else
						mId = BMItem[iindex].iId[3][RAND( 0, 2)];

					itemindex = ITEM_makeItemAndRegist( mId);
					if(itemindex==-1){
						sprintf( sBuf, "����ʧ�ܣ�����");
						CHAR_talkToCli( charaindex, -1, sBuf, CHAR_COLORRED);
						return;
					}
					ret = CHAR_addItemSpecificItemIndex( charaindex, itemindex);
					if(ret<0||ret>=CHAR_MAXITEMHAVE){
						print( "npc_exchange.c: ACCEPTadditem error itemindex[%d]\n", itemindex);
						ITEM_endExistItemsOne( itemindex);
						sprintf( sBuf, "����ʧ�ܣ�����");
						CHAR_talkToCli( charaindex, -1, sBuf, CHAR_COLORRED);
						return;
					}
					CHAR_sendItemDataOne( charaindex, ret);					
				}				
				gTmp = CHAR_getInt( charaindex, CHAR_GOLD);
				gTmp -= BMItem[iindex].GCondition;
				CHAR_setInt( charaindex, CHAR_GOLD, gTmp);
				CHAR_complianceParameter( charaindex);	
	            CHAR_send_P_StatusString( charaindex, CHAR_P_STRING_GOLD);
				
				sprintf( sBuf,"����ʯ�� %d", BMItem[iindex].GCondition);
                CHAR_talkToCli( charaindex, -1, sBuf, CHAR_COLORWHITE);
				// test  shan shan
				sprintf( sBuf, "�����������������°ɣ���");
				CHAR_talkToCli( charaindex, -1, sBuf, CHAR_COLORYELLOW);
				// add to Empty
				for( i=0; i<12; i++){
					if(BMSellList[i]==iindex){
						BMSellList[i] = RAND(0, BMINum-1);	
						break;
					}
				}
				for( i=0; i<12; i++)
					CONNECT_set_BMList( getfdFromCharaIndex( charaindex), i, -1);
				return;
			}
		}
	}
	sprintf( sBuf, "����ʧ�ܣ�����");
	CHAR_talkToCli( charaindex, -1, sBuf, CHAR_COLORRED);
}
#endif

#ifdef _ITEM_CONSTITUTION
void ITEM_Constitution( int charaindex, int toindex, int haveitemindex)
{
	char buf[256];
	char *itemarg;
	char Free[][128]={"VI","ST","TG","DE"};
	int index=0, FixPoint=0, itemindex;
	BOOL FIXs=FALSE;
	int AllPoint=0;

	if( CHAR_getInt( charaindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER )
		return;
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;

	itemarg = ITEM_getChar( itemindex, ITEM_ARGUMENT);
	if( itemarg == NULL ) return;
	memset( buf, 0, sizeof( char) *256);

	AllPoint = CHAR_getInt( charaindex, CHAR_VITAL)
		+CHAR_getInt( charaindex, CHAR_VITAL+1)
		+CHAR_getInt( charaindex, CHAR_VITAL+2)
		+CHAR_getInt( charaindex, CHAR_VITAL+3);

	while( index < arraysizeof( Free) )	{
		if( ITEM_getArgument( itemarg, Free[ index], buf, sizeof(buf)) == TRUE )	{
			FixPoint = atoi( buf);
			if( FixPoint <= 100 && FixPoint > 0 )	{
				int points = CHAR_getInt( charaindex, CHAR_VITAL+index);
				//Change Fix
				//if( points < (FixPoint*100) ) break;
				if( points < (FixPoint*100) || AllPoint-(FixPoint*100) <= 0 ) 
				{
					CHAR_talkToCli( charaindex, -1, "��Ʒ��Ч��", CHAR_COLORYELLOW);
					return;
				}
				CHAR_setInt( charaindex, CHAR_VITAL+index, (points-(FixPoint*100)));
				CHAR_setInt( charaindex, CHAR_SKILLUPPOINT, CHAR_getInt( charaindex, CHAR_SKILLUPPOINT) + FixPoint);
				FIXs = TRUE;
			}
			FixPoint = 0;
		}
		index++;
		if( (CHAR_VITAL+index) > CHAR_DEX )
			break;
	}
	CHAR_complianceParameter( charaindex);
	CHAR_send_P_StatusString(  charaindex,
		CHAR_P_STRING_MAXHP|CHAR_P_STRING_HP|CHAR_P_STRING_LV|CHAR_P_STRING_EXP|
		CHAR_P_STRING_ATK|CHAR_P_STRING_DEF|CHAR_P_STRING_QUICK|
		CHAR_P_STRING_VITAL|CHAR_P_STRING_STR|CHAR_P_STRING_TOUGH|CHAR_P_STRING_DEX);

	CHAR_Skillupsend( charaindex);
	memset( buf, 0, sizeof( buf));
	if( FIXs == TRUE )	{
		sprintf( buf, "%s", "�������е��仯��");
	}else	{
		sprintf( buf, "%s", "��Ʒ��Ч��");
	}
	CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);
	CHAR_DelItem( charaindex, haveitemindex);
}
#endif

#ifdef _NEW_RIDEPETS
void ITEM_useLearnRideCode( int charaindex, int toindex, int haveitemindex)
{//CHAR_LOWRIDEPETS
	int itemindex, i;
	char buf1[256];
	char *itemarg;
	typedef struct{
		char arg[256];
		int Code;
	}tagNewRideCode;
	tagNewRideCode NewRides[10]={
		{ "RIDE_PET0", RIDE_PET0}, { "RIDE_PET1", RIDE_PET1}, { "RIDE_PET2", RIDE_PET2},
		{ "RIDE_PET3", RIDE_PET3}, { "RIDE_PET4", RIDE_PET4}, { "RIDE_PET5", RIDE_PET5},
		{ "RIDE_PET6", RIDE_PET6}, { "RIDE_PET7", RIDE_PET7}, { "RIDE_PET8", RIDE_PET8},
		{ "RIDE_PET9", RIDE_PET9}
	};

	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;
	itemarg = ITEM_getChar( itemindex, ITEM_ARGUMENT);
	if( itemarg == NULL ) return;
	memset( buf1, 0, sizeof( buf1));

	if( getStringFromIndexWithDelim(itemarg,"|", 1, buf1, sizeof(buf1)) == FALSE )
		return;
		
	for( i=0; i<10; i++)	{
		if( !strcmp( NewRides[i].arg, buf1) )	{
			int LRCode = CHAR_getInt( charaindex, CHAR_LOWRIDEPETS);
			if( RIDE_PET0 == NewRides[i].Code 
#ifdef _TRANS_6
				&& CHAR_getInt( charaindex, CHAR_TRANSMIGRATION) < 5 
#else
				&& CHAR_getInt( charaindex, CHAR_TRANSMIGRATION) != 5 
#endif
				){
				char token[256];
				memset( token, 0, sizeof( token));
				sprintf( token, "������ת�����ϲ���ѧϰ�������");
				CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
				return;//��������ת
			}
			LRCode = LRCode|NewRides[i].Code;
			CHAR_setInt( charaindex, CHAR_LOWRIDEPETS, LRCode);
			memset( buf1, 0, sizeof( buf1));
			if( getStringFromIndexWithDelim(itemarg,"|", 2, buf1, sizeof(buf1)) != FALSE ){
				char token[256];
				memset( token, 0, sizeof( token));
				sprintf( token, "ѧϰ���µ���� (%s)��", buf1);
				CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
			}
			break;
		}
	}
	CHAR_DelItem( charaindex, haveitemindex);
	CHAR_sendStatusString( charaindex, "x");
}

#endif





#ifdef _ITEM_EDITBASES
void ITEM_useFusionEditBase( int charaindex, int toindex, int haveitemindex)
{
	int itemindex;
	int work[4]={0,0,0,0};
	int anhour = PETFEEDTIME;//����  ʳʱ��(��λ����)
	
	if( !CHAR_CHECKINDEX( charaindex ) )  return;
	if( !CHAR_CHECKINDEX( toindex ) )  return;
	if( CHAR_getInt( toindex, CHAR_WHICHTYPE) != CHAR_TYPEPET){
		CHAR_talkToCli( charaindex, -1, "��Ʒ���޳���ʹ�á�", CHAR_COLORYELLOW);
		return;
	}
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX(itemindex) )  return;
	if( CHAR_getInt( toindex, CHAR_FUSIONBEIT) == 1 &&
		CHAR_getInt( toindex, CHAR_FUSIONRAISE) > 0 ){//����Ƿ�Ϊ�ںϳ�

#ifdef _TEST_PETFUSIONTIME
#else
		int time_l;
		int nowTime; // Robin fix
		time_l = CHAR_getInt( toindex, CHAR_FUSIONTIMELIMIT);
		nowTime = (int)time(NULL);
#if 1 // ����ҩ
		{
			char *arg = NULL;
			int deltime;
			char msg[1024];
			arg = ITEM_getChar( itemindex, ITEM_ARGUMENT);
			if( arg != NULL && !strncmp( arg, "��", 2) ) {
				sscanf( arg, "�� %d", &deltime);
				time_l -= (deltime*60);
				CHAR_setInt( toindex, CHAR_FUSIONTIMELIMIT, time_l);
				CHAR_DelItem( charaindex, haveitemindex);
				sprintf( msg, "�ӿ���ﵰ�������ٶ� %d ���ӡ�", deltime);
				CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORYELLOW);
				if( (time_l + anhour) <= nowTime ) {
					int min, sec, deftime;
					deftime = nowTime - (time_l + anhour);
					min = deftime/60;
					sec = deftime%60;
					sprintf( msg, "���ﵰ�Ѿ�����  ʳ�ˣ�����  ʳʱ��%d��%d�롣", min, sec);
					CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORYELLOW);
					return;
				}
			}
		}
#endif
		if( (time_l + anhour) > nowTime ){ //���ڵ�ʱ��Ҫ����(time_l+anhour)�ſ���  ʳ
			char Mess1[256];
			int min, sec, deftime;
			//int nowTime = (int)time(NULL);
			deftime = (time_l + anhour) - nowTime;
			min = deftime/60;
			sec = deftime%60;
			sprintf( Mess1, "���ﵰ���%d��%d��ſ�  ʳ��", min, sec);
			CHAR_talkToCli( charaindex, -1, Mess1, CHAR_COLORYELLOW);
			return;
		}
#endif
//---------------------------------------------
		work[3] = CHAR_getInt( toindex, CHAR_EVOLUTIONBASEVTL);
		work[0] = CHAR_getInt( toindex, CHAR_EVOLUTIONBASESTR);	
		work[1] = CHAR_getInt( toindex, CHAR_EVOLUTIONBASETGH);	
		work[2] = CHAR_getInt( toindex, CHAR_EVOLUTIONBASEDEX);
		PET_showEditBaseMsg( charaindex, toindex, itemindex, work);
		CHAR_setInt( toindex, CHAR_EVOLUTIONBASEVTL, work[3]);
		CHAR_setInt( toindex, CHAR_EVOLUTIONBASESTR, work[0]);	
		CHAR_setInt( toindex, CHAR_EVOLUTIONBASETGH, work[1]);	
		CHAR_setInt( toindex, CHAR_EVOLUTIONBASEDEX, work[2]);
//---------------------------------------------
		LogPetPointChange(
			CHAR_getChar( charaindex, CHAR_NAME ),
			CHAR_getChar( charaindex, CHAR_CDKEY ),
			CHAR_getChar( charaindex, CHAR_NAME),
			toindex, 4,
			CHAR_getInt( toindex, CHAR_LV),
			"item_use",
			CHAR_getInt( charaindex, CHAR_FLOOR),
			CHAR_getInt( charaindex, CHAR_X ),
			CHAR_getInt( charaindex, CHAR_Y )
		);

		LogPetFeed(
			CHAR_getChar( charaindex, CHAR_NAME),
			CHAR_getChar( charaindex, CHAR_CDKEY),
			CHAR_getChar( toindex, CHAR_NAME),
			toindex,
			CHAR_getInt( toindex, CHAR_LV),
			ITEM_getChar( itemindex, CHAR_NAME), // Key
			CHAR_getInt( charaindex, CHAR_FLOOR),
			CHAR_getInt( charaindex, CHAR_X),
			CHAR_getInt( charaindex, CHAR_Y),
			CHAR_getChar( toindex, CHAR_UNIQUECODE) );

#ifdef _PET_EVOLUTION
		{
			int raise = CHAR_getInt( toindex, CHAR_FUSIONRAISE);
			CHAR_setInt( toindex, CHAR_FUSIONRAISE, --raise);
			CHAR_setInt( toindex, CHAR_FUSIONTIMELIMIT, nowTime);
			if( CHAR_getInt( toindex, CHAR_FUSIONRAISE) <= 0 ){//����
				char buf[256], buf1[256];
				int newindex;
				sprintf( buf, "����%s��������", CHAR_getUseName( toindex ));

				newindex = EVOLUTION_createPetFromEnemyIndex( charaindex, toindex, 1);
				if( !CHAR_CHECKINDEX( newindex) ){
					CHAR_talkToCli( charaindex, -1, "���������������", CHAR_COLORYELLOW);
					return;
				}
				sprintf( buf1, "��%s����", CHAR_getChar( newindex, CHAR_NAME));
				strcat( buf, buf1);
				CHAR_talkToCli( charaindex, -1, buf, CHAR_COLORYELLOW);

				LogPetFeed(
					CHAR_getChar( charaindex, CHAR_NAME),
					CHAR_getChar( charaindex, CHAR_CDKEY),
					CHAR_getChar( toindex, CHAR_NAME),
					toindex,
					CHAR_getInt( toindex, CHAR_LV),
					buf, // Key
					CHAR_getInt( charaindex, CHAR_FLOOR),
					CHAR_getInt( charaindex, CHAR_X),
					CHAR_getInt( charaindex, CHAR_Y),
					CHAR_getChar( toindex, CHAR_UNIQUECODE) );

			}
		}
#endif
	}else	{
		CHAR_talkToCli( charaindex, -1, "������", CHAR_COLORYELLOW);
	}
	CHAR_DelItem( charaindex, haveitemindex);
	return;
}
#endif

#ifdef _THROWITEM_ITEMS
static int Niceitem = 0;
	int golds[3]={ 10000, 20000, 50000};
	int items1[18] = { 13092, 13091, 20439, 20417, 1284, 20172, 18210, 19014, 18360, 18362, 18364,
						18359, 18356, 18357, 18510, 20418, 20419, 1452};
	int items2[11] = { 15842, 16136, 14334, 14034, 14634, 14934, 15534, 14934, 16432, 17057, 19695};

	int items3[10] = { 16014, 16314, 14515, 14215, 14815, 15115, 15715, 15295, 16552, 17157};

	int items4[18] = { 14516, 14513, 14216, 14213, 14816, 14813, 15116, 15716, 15415, 17360, 20279,
						20282, 20276, 20270, 20288, 20290, 20291, 20289};
	int items5[5] = { 20280, 20283, 20277, 20271, 20274};
	int items6[5] = { 20284, 20272, 20275, 20281, 20278};
void ITEM_ThrowItemBox( int charaindex, int toindex, int haveitemindex)
{
	int i, ret, Iindex, ItemID=-1, itemindex;
	char token[256];

	if( !CHAR_CHECKINDEX(charaindex) ) return;
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX(itemindex) )  return;

	if( Niceitem > 10 ) ret = rand()%920;
	else ret = rand()%1000;

	for( i = CHAR_STARTITEMARRAY ; i < CHAR_MAXITEMHAVE ; i++ ){
		Iindex = CHAR_getItemIndex( charaindex , i );
		if( !ITEM_CHECKINDEX(itemindex) ) continue;
		if( itemindex == Iindex ){
			CHAR_DelItem( charaindex, i);

			LogItem(
				CHAR_getChar( charaindex, CHAR_NAME ), CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD ��item��log������item����
				itemindex,
#else
				ITEM_getInt( itemindex, ITEM_ID),
#endif
				"WarpManDelItem",
				CHAR_getInt( charaindex, CHAR_FLOOR), CHAR_getInt( charaindex, CHAR_X ),
				CHAR_getInt( charaindex, CHAR_Y ), ITEM_getChar( itemindex, ITEM_UNIQUECODE),
				ITEM_getChar( itemindex, ITEM_NAME),
				ITEM_getInt( itemindex, ITEM_ID)
			);
			break;
		}
	}
	if( i >= CHAR_MAXITEMHAVE ){
		return;
	}

	if( ret <= 774 ){
		int Golds=0;
		Golds = golds[ RAND(0,2)];
		CHAR_AddGold( charaindex, Golds);
	}else {
		if( ret < 924 ){
			ItemID = items1[RAND(0,17)];
		}else if( ret < 964 ){
			ItemID = items2[RAND(0,10)];
		}else if( ret < 984 ){
			ItemID = items3[RAND(0,9)];
			Niceitem++;
		}else if( ret < 994 ){
			ItemID = items4[RAND(0,17)];
			Niceitem++;
		}else if( ret < 999 ){
			ItemID = items5[RAND(0,4)];
			Niceitem++;
		}else {
			ItemID = items6[RAND(0,4)];
			Niceitem++;
		}

		Iindex = ITEM_makeItemAndRegist( ItemID);
		if( !ITEM_CHECKINDEX( Iindex) ){
			return;
		}
		ret = CHAR_addItemSpecificItemIndex( charaindex, Iindex);
		if( ret < 0 || ret >= CHAR_MAXITEMHAVE ) {
			ITEM_endExistItemsOne( Iindex);
			return;
		}
		sprintf( token,"�õ�%s", ITEM_getChar( Iindex, ITEM_NAME));
		CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW );
		CHAR_sendItemDataOne( charaindex, ret);
	}
}
#endif

#ifdef _ITEM_LOVERPARTY
void ITEM_LoverSelectUser( int charaindex, int toindex, int haveitemindex)
{
	int itemindex, i;
	int playernum = CHAR_getPlayerMaxNum();

	if( !CHAR_CHECKINDEX( charaindex ) )  return;
	if( !CHAR_CHECKINDEX( toindex ) )  return;
	if( CHAR_getInt( toindex, CHAR_WHICHTYPE) != CHAR_TYPEPLAYER){
		CHAR_talkToCli( charaindex, -1, "��Ʒ��������ʹ�á�", CHAR_COLORYELLOW);
		return;
	}
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX(itemindex) )  return;
	if( ITEM_getInt( itemindex, ITEM_TYPE) != 16 ){
		ITEM_setInt( itemindex, ITEM_TYPE, 16);
		return;
	}
	if( !strcmp( ITEM_getChar( itemindex, ITEM_FORUSERNAME), "") ||
		!strcmp( ITEM_getChar( itemindex, ITEM_FORUSERCDKEY), "") ){//Ѱ���趨����
		if( charaindex == toindex ){
			CHAR_talkToCli( charaindex, -1, "����ѡ���Լ�ʹ�á�", CHAR_COLORYELLOW);
			return;
		}
		ITEM_setChar( itemindex, ITEM_FORUSERNAME, CHAR_getChar( toindex, CHAR_NAME) );
		ITEM_setChar( itemindex, ITEM_FORUSERCDKEY, CHAR_getChar( toindex, CHAR_CDKEY) );
		ITEM_setInt( itemindex, ITEM_TARGET, 0);
		{
			char token[256];
			sprintf( token, "%s(%s)",
				ITEM_getChar( itemindex, ITEM_SECRETNAME), CHAR_getChar( toindex, CHAR_NAME));
			ITEM_setChar( itemindex, ITEM_SECRETNAME, token);
			sprintf( token, "���Ͷ����趨Ϊ%s��", CHAR_getChar( toindex, CHAR_NAME));
			CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW);
		}
		CHAR_sendItemDataOne( charaindex, haveitemindex);
	}else{
		char buf1[256];
		//if( CHAR_getInt( charaindex, CHAR_FLOOR) == 117 || CHAR_getInt( charaindex, CHAR_FLOOR) == 887 ||
		//	CHAR_getInt( charaindex, CHAR_FLOOR) == 1042 || CHAR_getInt( charaindex, CHAR_FLOOR) == 2032 ||
		//	CHAR_getInt( charaindex, CHAR_FLOOR) == 3032 || CHAR_getInt( charaindex, CHAR_FLOOR) == 4032 ||
		//	CHAR_getInt( charaindex, CHAR_FLOOR) == 5032 || CHAR_getInt( charaindex, CHAR_FLOOR) == 6032 ||
		//	CHAR_getInt( charaindex, CHAR_FLOOR) == 7032 || CHAR_getInt( charaindex, CHAR_FLOOR) == 8032 ||
		//	CHAR_getInt( charaindex, CHAR_FLOOR) == 9032 ){ 
		if( checkUnlawWarpFloor( CHAR_getInt( charaindex, CHAR_FLOOR) ) ) {
			CHAR_talkToCli( charaindex, -1, "�������λ���޷����͡�", CHAR_COLORYELLOW );
			return;
		}

		for( i=0; i<playernum; i++)	{
			int itemmaxuse=0;
			if( !CHAR_CHECKINDEX( i) ) continue;
			if( !strcmp( ITEM_getChar( itemindex, ITEM_FORUSERNAME), CHAR_getChar( i, CHAR_NAME)) &&
				!strcmp( ITEM_getChar( itemindex, ITEM_FORUSERCDKEY), CHAR_getChar( i, CHAR_CDKEY)) ){
				int floor, x, y;
				char token[256];
				floor = CHAR_getInt( i, CHAR_FLOOR);
				x = CHAR_getInt( i, CHAR_X);
				y = CHAR_getInt( i, CHAR_Y);

				//if( floor == 887 || floor == 117 ||
				//	floor == 1042 || floor == 2032 || floor == 3032 || floor == 4032 ||
				//	floor == 5032 || floor == 6032 ||floor == 7032 || floor == 8032 || floor == 9032 ){ 
				if( checkUnlawWarpFloor( floor) ) {
					CHAR_talkToCli( charaindex, -1, "�������ڵط��޷����͡�", CHAR_COLORYELLOW );
					return;
				}

				CHAR_warpToSpecificPoint( charaindex, floor, x, y );
				sprintf( token, "%s���ɽ�ָ���͵����������", CHAR_getChar( charaindex, CHAR_NAME));
				CHAR_talkToCli( i, -1, token, CHAR_COLORYELLOW );
				sprintf( token, "���ɽ�ָ���͵�%s��ߡ�", CHAR_getChar( i, CHAR_NAME));
				CHAR_talkToCli( charaindex, -1, token, CHAR_COLORYELLOW );
				CHAR_DischargePartyNoMsg( charaindex);//��ɢ�Ŷ�
#ifdef _ITEM_MAXUSERNUM
				itemmaxuse = ITEM_getInt( itemindex, ITEM_DAMAGEBREAK);
#endif
				if( itemmaxuse != -1 )	{
					itemmaxuse--;
#ifdef _ITEM_MAXUSERNUM
					ITEM_setInt( itemindex, ITEM_DAMAGEBREAK, itemmaxuse);
#endif
					if( itemmaxuse < 1 )	{
						sprintf( buf1, "���� %s��ʧ�ˡ�", ITEM_getChar( itemindex, ITEM_NAME) );
						CHAR_talkToCli(charaindex, -1, buf1, CHAR_COLORYELLOW);
						CHAR_DelItem( charaindex, haveitemindex);
						return;
					}else{
						sprintf( buf1, "������Ŀ���������λ�ã���ʹ�ô���ʣ��%d�Ρ�", itemmaxuse);
						ITEM_setChar( itemindex, ITEM_EFFECTSTRING, buf1);
					}
				}else{
					itemmaxuse = 10;
					ITEM_setInt( itemindex, ITEM_DAMAGEBREAK, itemmaxuse);
					sprintf( buf1, "������Ŀ���������λ�ã���ʹ�ô���ʣ��%d�Ρ�", itemmaxuse);
					ITEM_setChar( itemindex, ITEM_EFFECTSTRING, buf1);
				}

				CHAR_DischargePartyNoMsg( charaindex);
				CHAR_complianceParameter( charaindex );
				CHAR_sendItemDataOne( charaindex, haveitemindex);
				return;
			}
		}
		CHAR_talkToCli( charaindex, -1, "�����ڴ��ŷ����������ϡ�", CHAR_COLORYELLOW );
	}
}
#endif

#ifdef _ANGEL_SUMMON
void ITEM_AngelToken( int charaindex, int toindex, int haveitemindex )
{
	
	Use_AngelToken( charaindex, toindex, haveitemindex );

}

void ITEM_HeroToken( int charaindex, int toindex, int haveitemindex )
{

	Use_HeroToken( charaindex, toindex, haveitemindex );

}
#endif

#ifdef _HALLOWEEN_EFFECT
void ITEM_MapEffect(int charaindex,int toindex,int haveitemindex)
{
	int itemindex,i,floor;
	char *pActionNumber,szMsg[128];

	if(CHAR_CHECKINDEX(charaindex) == FALSE )return ;

	itemindex = CHAR_getItemIndex(charaindex,haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;
	
	// �ҳ�������ڵĵ�ͼ���
	floor = CHAR_getInt(charaindex,CHAR_FLOOR);
	// �ҳ�����Ҫ�ŵ���Ч�ı��
	pActionNumber = ITEM_getChar(itemindex,ITEM_ARGUMENT);
	sprintf(szMsg,"%d 8 %s",floor,pActionNumber);
	// ִ��
	CHAR_CHAT_DEBUG_effect(charaindex,szMsg);
	CHAR_DelItemMess(charaindex,haveitemindex,0);
}
#endif

void ITEM_changePetOwner( int charaindex, int toindex, int haveitemindex)
{
	int itemindex, i;
	if( CHAR_CHECKINDEX( charaindex ) == FALSE )return ;
	if( CHAR_CHECKINDEX( toindex ) == FALSE )return ;

	//ITEM_useRecovery_Field(	charaindex, toindex, haveitemindex );

	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX(itemindex) ) return;
	
	if( CHAR_getInt( toindex, CHAR_WHICHTYPE) != CHAR_TYPEPET ) {
		CHAR_talkToCli( charaindex, -1, "��ֻ�����ڳ�������ม�", CHAR_COLORYELLOW );
		return;
	}

	if( !strcmp( CHAR_getChar( toindex, CHAR_NPCARGUMENT), "") || 
		!strcmp( CHAR_getChar( toindex, CHAR_NPCARGUMENT), CHAR_getChar( charaindex, CHAR_CDKEY))) {
		CHAR_talkToCli( charaindex, -1, "��ֻ���ﱾ��������ģ�������Ҫʹ�����ѽ��", CHAR_COLORYELLOW);
		return;
	}

	CHAR_setChar( toindex, CHAR_NPCARGUMENT, "");
	
	for( i =0; i < CHAR_MAXPETHAVE; i++)
		if( CHAR_getCharPet( charaindex, i) == toindex )
			CHAR_send_K_StatusString( charaindex, i,CHAR_K_STRING_NAME|CHAR_K_STRING_CHANGENAMEFLG);

	CHAR_talkToCli( charaindex, -1, "��������԰���ֻ����������ˡ�", CHAR_COLORYELLOW );

	CHAR_DelItemMess( charaindex, haveitemindex, 0);
}


#ifdef _DEL_DROP_GOLD
void GOLD_DeleteTimeCheckLoop( void)
{
	int amount;
	int objindex;
	int objmaxnum;
	
	objmaxnum = OBJECT_getNum();
	for( objindex=0; objindex<objmaxnum; objindex++)	{
		if( CHECKOBJECT( objindex ) == FALSE ) continue;
		if( OBJECT_getType( objindex) != OBJTYPE_GOLD) continue;
		
		amount = OBJECT_getIndex( objindex);
		if( amount >= 10000) continue;
		
		if( (int)NowTime.tv_sec > (int)( OBJECT_getTime( objindex) + getItemdeletetime() ) ) {
			
			LogStone(
				-1,
				"NULL",
				"NULL",
				amount,
				0,
				"Del(ɾ����ʱ��Ǯ)",
				OBJECT_getFloor( objindex ),
				OBJECT_getX( objindex ),
				OBJECT_getY( objindex )
				);
			
			CHAR_ObjectDelete(objindex);
		}
	}
}

void GOLD_DeleteTimeCheckOne( int objindex)
{
	int amount;
	
	if( CHECKOBJECT( objindex ) == FALSE ) return;
	if( OBJECT_getType( objindex) != OBJTYPE_GOLD) return;
	
	amount = OBJECT_getIndex( objindex);
	if( amount >= 10000) return;
	
	if( (int)NowTime.tv_sec > (int)( OBJECT_getTime( objindex) + getItemdeletetime() ) ) {
		
		LogStone(
			-1,
			"NULL",
			"NULL",
			amount,
			0,
			"Del(ɾ����ʱ��Ǯ)",
			OBJECT_getFloor( objindex ),
			OBJECT_getX( objindex ),
			OBJECT_getY( objindex )
			);
		
		CHAR_ObjectDelete(objindex);
	}

}

#endif

#ifdef _TIME_TICKET
void ITEM_timeticket( int charaindex, int toindex, int haveitemindex)
{
	ITEM_timeticketEx( charaindex, toindex, haveitemindex, 0);
}

void ITEM_timeticketEx( int charaindex, int toindex, int haveitemindex, int flag)
{
	int itemindex;
	int addtime;
	int nowtime = time(NULL);
	int tickettime;
	int lefttime;
	char msg[1024];
	
	if( !CHAR_CHECKINDEX( charaindex ) )  return;

	if( check_TimeTicketMap( CHAR_getInt( charaindex, CHAR_FLOOR)) == FALSE 
		&& flag == 0 ) {
		CHAR_talkToCli( charaindex, -1, "����ص㲻��ʹ�á�", CHAR_COLORYELLOW);
		return;
	}
	
	itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( !ITEM_CHECKINDEX(itemindex) )  return;

	tickettime = CHAR_getWorkInt( charaindex, CHAR_WORKTICKETTIME);
	if( tickettime > nowtime+20 ) {
		CHAR_talkToCli( charaindex, -1, "ʱ�����ʣ�£��������ڲſ�ʹ�á�", CHAR_COLORYELLOW);
		return;
	}
	// ��һ��ʹ�õĻ�
	if( tickettime == 0 ) {
		tickettime = nowtime;
		CHAR_setWorkInt( charaindex, CHAR_WORKTICKETTIMESTART, nowtime);
	}
	// ս�����ҳ���ʱ��ʱʹ��
	//if( CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEMODE) != BATTLE_CHARMODE_NONE 
	//		&& tickettime < nowtime ) {
		//tickettime = nowtime;
	//	sprintf(msg, "ʱ�䲻��%d�롣", nowtime - tickettime );
	//	CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORYELLOW);
	//}

	addtime = atoi( ITEM_getChar( itemindex, ITEM_ARGUMENT));
	tickettime += addtime;
	CHAR_setWorkInt( charaindex, CHAR_WORKTICKETTIME, tickettime );
	lefttime = tickettime - nowtime;
	if( lefttime > 0 )
		sprintf(msg, "ʱ������%d�룬��ʣ��%d��%d�롣", addtime, lefttime/60, lefttime%60 );
	else
		sprintf(msg, "ʱ������%d�룬������%d��%d�롣", addtime, (-lefttime)/60, (-lefttime)%60 );
	CHAR_talkToCli( charaindex, -1, msg, CHAR_COLORYELLOW);
	CHAR_DelItem( charaindex, haveitemindex);
}
#endif
