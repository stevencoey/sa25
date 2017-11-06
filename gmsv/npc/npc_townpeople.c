#include "version.h"
#include "object.h"
#include "char_base.h"
#include "char.h"
#include "util.h"
#include "handletime.h"
#include "anim_tbl.h"
#include "npc_door.h"
#include "lssproto_serv.h"
#include "npcutil.h"


/*
 *  ���м�����̫��  by nakamura
 *  ������  ��TownPeople
 *    �ƾ�����ľ�������NPCARGUMENT��  ٯ  ë  �ƾ��껯�����б�  �ʣ�
 *
 *  ���ף�npcarg�� ���ϱ��з�,��  �������廥����ƥ����
 *  ��������  ʣ���Ѩë�����������ƻ�  �Ѽ�����������ë
 *  ̤�ʳ��绥ƥ������������¼�ĸةƥ����  �����������£�
 *  npcgen.perl ƥ���� MANƥؤ�£� MSG�����������ئԪ��ئؤ
 *
 */

/*
 *   �ƾ�����ľ�����巴NPCARGUMENTë��������  �ʣ�
 */
void NPC_TownPeopleTalked( int index, int talker, char *msg, int color )
{
	char arg[NPC_UTIL_GETARGSTR_BUFSIZE], token[NPC_UTIL_GETARGSTR_LINEMAX];
    int i, tokennum;

    /* 3��������  ����������߯������ */

	if( CHAR_getInt(talker,CHAR_WHICHTYPE) == CHAR_TYPEPLAYER 
        && NPC_Util_charIsInFrontOfChar( talker, index, 3 ) ){

        NPC_Util_GetArgStr( index, arg, sizeof( arg));

        tokennum = 1;
        /* ���Ѩƥ����ľ�������ͼ����ϳ�ؤ�¾������� */
        for( i=0;arg[i]!='\0';i++ ){
            if( arg[i] == ',' ) tokennum++;
        }

        /* �¼�ĸةƥ��ľë���¾�裻��������������ͼ�ë��Ի���� */
        getStringFromIndexWithDelim( arg,",",
                                     rand()%tokennum+1,token, sizeof(token));

        CHAR_talkToCli( talker, index, token, CHAR_COLORWHITE );
    }
}

/*
 * ��������£�
 */
BOOL NPC_TownPeopleInit( int meindex )
{

    //CHAR_setInt( meindex , CHAR_HP , 0 );
    //CHAR_setInt( meindex , CHAR_MP , 0 );
    //CHAR_setInt( meindex , CHAR_MAXMP , 0 );
    //CHAR_setInt( meindex , CHAR_STR , 0 );
    //CHAR_setInt( meindex , CHAR_TOUGH, 0 );
    //CHAR_setInt( meindex , CHAR_LV , 0 );

    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPETOWNPEOPLE );
    //CHAR_setFlg( meindex , CHAR_ISOVERED , 1 );
    //CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );  /*   ����ľئ�з��� */
    
    return TRUE;
}
