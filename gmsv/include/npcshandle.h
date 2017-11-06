#ifndef _NPCS_HANDLE_H
#define _NPCS_HANDLE_H
#include "version.h"
#include "util.h"

#ifdef _NPCSERVER_NEW
//�����ж�
BOOL NPCS_HandleCheckFreeMess( int npcobjindex, int charaindex, int charobjindex,
				char *CheckfreeMess);

BOOL NPCS_FreePassCheck( int charaindex, char *buf);

//�Ƚ��ж�
BOOL NPCS_FreeBigSmallCheck( int charaindex,char* buf);
//�����ж�
BOOL NPCS_ArgumentFreeCheck( int charaindex, char* Argument, int amount, int temp, int probjID);
//������С�ж�
BOOL NPCS_ArgumentBigSmallCheck( int point, int mypoint, int flg);
//ȡ��ָ��ID������	*reAmount ʣ����λ
int NPCS_getUserPetAmount( int charaindex, int objID, int flg);
//ȡ��ָ��ID������	*reAmount ʣ����λ
int NPCS_getUserItemAmount( int charaindex, int objID, int flg);
//ȡ���Ŷ�����
int NPCS_getUserPartyAmount( int charaindex );
BOOL NPCS_NpcstalkToCli( int charaindex,int npcobjindex, char* message, CHAR_COLOR color );
//�¼�����
BOOL NPCS_RunDoEventAction( int charaindex, char *buf1);
#endif
#endif
