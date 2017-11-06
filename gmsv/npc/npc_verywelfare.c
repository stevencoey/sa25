#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "configfile.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "npc_verywelfare.h"

#ifdef _NPC_VERYWELFARE

static void NPC_VeryWelfare_selectWindow(int meindex, int toindex, int num, int select);

//����ǳ��ÿ�NPC�ĳ�ʼ��
BOOL NPC_VeryWelfareInit( int meindex )
{
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPEVERYWELFARE);
    print("\n����:%s\n",CHAR_getChar( meindex, CHAR_NAME));
    return TRUE;
}

//�Ի�ʱ�Ĵ���
void NPC_VeryWelfareTalked( int meindex , int talkerindex , char *szMes ,int color )
{
    if( NPC_Util_isFaceToFace( talkerindex, meindex , 1 ) == FALSE )
        return;
    //��ʼ��ѡ����
	NPC_VeryWelfare_selectWindow( meindex, talkerindex, 0, -1);
}

static void NPC_VeryWelfare_selectWindow( int meindex, int toindex, int num,int select)
{
	int buttontype = 0,windowtype = 0,windowno = 0;
	char	buf[256];
    int fd = getfdFromCharaIndex( toindex);
    char token[256];
	if( fd == -1 )
        return;
	switch(num)
	{
	    case 0:
            sprintf(token,"%s","���Ƿǳ��ÿ�A��NPC���ҽ��ṩ��ÿ�A�ķ���");
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		    windowno = NPC_VERYWELFARE_START;
		    lssproto_WN_send( fd, windowtype, WINDOW_BUTTONTYPE_OK, windowno,	
			CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),token);
		    break;
	    case 1:
		    sprintf(token,"%s","2\n��ѡ�����µ�ѡ��\n\n����������\n����ħ����");
			windowtype = WINDOW_MESSAGETYPE_SELECT;
		    windowno = NPC_VERYWELFARE_NO1;
		    lssproto_WN_send( fd, windowtype, buttontype, windowno,	
			CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
			makeEscapeString( token, buf, sizeof(buf)));
		    break;
	    case 2:
		    sprintf(token,"%s","��Ҫ���������");
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		    windowno = NPC_VERYWELFARE_NO2;
		    lssproto_WN_send( fd, windowtype, buttontype, windowno,	
			CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
			makeEscapeString( token, buf, sizeof(buf)));
		    break;
		case 3:
		    sprintf(token,"%s","��Ҫ���ħ����");
			windowtype = WINDOW_MESSAGETYPE_MESSAGE;
		    windowno = NPC_VERYWELFARE_NO3;
		    lssproto_WN_send( fd, windowtype, buttontype, windowno,	
			CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX),
			makeEscapeString( token, buf, sizeof(buf)));
		    break;
    }

}

void NPC_VeryWelfareWindowTalked( int meindex, int talkerindex, 
		int seqno, int select, char *data)
{
		
	print("\nѡ:%d",select);
	//int datanum = atoi( data);
	switch( seqno){

	    case NPC_VERYWELFARE_START:
	  		NPC_VeryWelfare_selectWindow(meindex, talkerindex, 1, 0);
		    break;
	    case NPC_VERYWELFARE_NO1:
			NPC_VeryWelfare_selectWindow(meindex, talkerindex, 2, 1);
	  	    break;
	    case NPC_VERYWELFARE_NO2:
			NPC_VeryWelfare_selectWindow(meindex, talkerindex, 3, 1);
	  	    break;
	    default:
	  	    break;
	}
}             

#endif
