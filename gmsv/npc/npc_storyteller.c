#include "version.h"
#include "char.h"

/*


  ����Ի�ͣ�

  ��������  ƥ��2��  �����������

 NPCARGUMENT:  0�������ճ������0��1�������ճ������1����Ի  ��  

 ���ط�3��ؤԻ���ͱ��׵����£�

 ����0�� ʧ����������۳�����ƥtalk��ľ�����
 ���������ף����ͻ��ͷ����о����֣����ƻ���  ������Ի��Ԫ���ۣ�ƥ
 ����1��ࢵ��
 ����1��talk��ľ���գ�
 ����ئ�׻����߸�ئ���ɣ�������微�������  ئ����ë���繴�� �����귽����
        ��������ë���߸꾮? yes/no��yesƥ����2�ߣ�noƥ
        ��ƥ����������Ԫ���ۣ�ƥ����0��
        
 ����2�� talk��ľ���գ�
 ����ئ�ױ���  �����ǣ����ͻ��ͣ�ë��ج���ף�  ��������  �ئ�ּ����ƻ�
        ��Ϸ  婱��������ƣ۽�����Ԫ���ۣ�
        �����Ȼ���ëӼ���ƣ����ǰ׷º�ëܰ�����£�
        ƥ����0�������£�



 
 Talked:

 if(     �巴ئ�ƻ�����1�����Ȼ����¾���){
   ����0�������£�
 }
   
 switch(����){
 case 0:  ��  ������1�ߣ�break;
 case 1:  ��  ������2�ߣ�break;
 case 2:  ��  ������0�ߣ�break;
 } 

 }

 
 
 

 



 */
BOOL NPC_StoryTellerInit( int meindex )
{
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPESTORYTELLER );
    CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );
    CHAR_setFlg( meindex , CHAR_ISOVERED , 0 );

    return TRUE;
}


void NPC_StoryTellerTalked( int meindex , int talker , char *msg , int col )
{
    
}
