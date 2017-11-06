#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "version.h"
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "lssproto_serv.h"
#include "npc_stoneserviceman.h"
#include "readmap.h"
#include "battle.h"
#include "log.h"
#include "enemy.h"
#include "configfile.h"

#ifdef _SERVICE
#define LOOP_TIME 100		//0.1��(����loop function ��ʱ��)
#define MAX_AP_MSG 128		// ����AP����ѶϢ�������

// socket struct
struct socketaddr_in
{
  u_short sin_family;
  u_short sin_port;
  u_long  sin_addr;
  char    sin_zero[8];
};

// ����AP������ѶϢ
typedef struct _MSG
{
  int nAid;		// nAid => ��ˮ��*10 + ѶϢ����(_AP_CLIENT_MESSAGE)
  int nIndex;		// NPC��indexֵ
  int nLen;		// ѶϢ����
  LPList Data;		// ʵ������
  int bIsUse;		// ���ѶϢ�Ƿ���ʹ�ù�
}Msg,*pMsg;
Msg g_APMsg[MAX_AP_MSG];
        
// ����id        
enum _AP_CLIENT_MESSAGE
{
  Cli_CHECKPLAYER,  //Ҫ��AP������Ƿ��й����Ʒ��ȷ��  0
  AP_CHECKPLAYER,   //AP��Ӧ����Ƿ��й����Ʒ
  Cli_YES,          //���ȷ��Ҫ����Ʒ����
  Cli_CANCEL,	    //֪ͨAP��lock��λ��Ϊ0             
  Cli_GET,	    //֪ͨAP����Ʒ
  AP_GET,	    //AP��Ӧ��Ʒ			5
  Cli_CONNECT,      //�����Ƿ�������AP
  AP_CONNECT,       //AP��Ӧ
  AP_ERROR          //AP�д���				
};

// NPC�Ĺ�������
enum
{
  NPC_WORK_FLAG1   	 = CHAR_NPCWORKINT2,	//flag1
  NPC_WORK_FLAG2   	 = CHAR_NPCWORKINT3,	//flag2
  NPC_WORK_FLAG3   	 = CHAR_NPCWORKINT4,	//flag3
  NPC_WORK_TIMEOUT 	 = CHAR_NPCWORKINT5,	//time out ʱ��
  NPC_WORK_STATE   	 = CHAR_NPCWORKINT6,	//Ŀǰִ��״̬
  NPC_WORK_START   	 = CHAR_NPCWORKINT7,	//��ʼ��ʱflag
  NPC_WORK_LEAVE_COUNT   = CHAR_NPCWORKINT8,	//���talkʱ���ʱ
  NPC_WORK_TOINDEX 	 = CHAR_NPCWORKINT9,	//��ҵ�indexֵ
  NPC_WORK_SERIALNUM     = CHAR_NPCWORKINT10,   //��ˮ��
  NPC_WORK_ISUSE   	 = CHAR_NPCWORKINT11,	//ĿǰNPC�Ƿ��������ʹ��
};

// ִ�е�״̬
enum
{
  SERVICE_STATE_1,	// check ��û�к�AP������ 
  SERVICE_STATE_2,	// ��APȡ������������Ʒ
  SERVICE_STATE_3,	// ȡ��Ҫ����ҵĶ���������
  SERVICE_STATE_4	// end state
};
                                                                
extern int errno;

int g_nServiceSocket;
int g_EnableService = 0;
char g_ApID[32];
struct socketaddr_in server_sin;
struct sockaddr      temp;
unsigned short g_Port;
unsigned char msgbuf[65535];
unsigned char *rdata_pos = msgbuf;
unsigned char *rdata_size = msgbuf;
unsigned char *max_rdata = msgbuf + sizeof(msgbuf);

#define RFIFOSPACE (max_rdata - rdata_size)
#define RFIFOREST  (rdata_size - rdata_pos)

static void NPC_StoneServiceMan_selectWindow(int meindex,int toindex,int num,int select);
extern int GetSerialNum(void);
static int flag = 1;

/*********************************
* ��ʼ��
*********************************/
BOOL NPC_StoneServiceManInit(int meindex )
{
  char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];
  
  if(NPC_Util_GetArgStr(meindex,npcarg,sizeof(npcarg)) == NULL){
    print("StoneServiceMan:GetArgStrErr");
    return FALSE;
  }

  CHAR_setInt(meindex,CHAR_LOOPINTERVAL,LOOP_TIME);
  CHAR_setInt(meindex,CHAR_WHICHTYPE,CHAR_TYPESTONESERVICEMAN);
  CHAR_setWorkInt(meindex,NPC_WORK_FLAG1,1);
  CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
  CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
  CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
  CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_2);
  CHAR_setWorkInt(meindex,NPC_WORK_START,0);  
  CHAR_setWorkInt(meindex,NPC_WORK_TOINDEX,-1);
  CHAR_setWorkInt(meindex,NPC_WORK_ISUSE,0);
  CHAR_setWorkInt(meindex,NPC_WORK_LEAVE_COUNT,0);
  CHAR_setWorkInt(meindex,NPC_WORK_SERIALNUM,-1);

  g_EnableService = getEnableService();
  
  //���ߵ�AP��initʱֻ��һ��
  if(flag && g_EnableService){
    strcpy(g_ApID,getApID());
    g_Port = getApPort();
    bzero(g_APMsg,sizeof(g_APMsg));
    ConnectToAP();//���ߵ�AP
    flag = 0;
  }
  
  return TRUE;
}

void NPC_StoneServiceManTalked(int meindex,int talkerindex,char *szMes,int color)
{
  if(CHAR_getInt(talkerindex,CHAR_WHICHTYPE) != CHAR_TYPEPLAYER) return;
  if(NPC_Util_isFaceToFace(talkerindex,meindex,1) == FALSE) return;
  NPC_StoneServiceMan_selectWindow(meindex,talkerindex,0,-1);
}

static void NPC_StoneServiceMan_selectWindow(int meindex,int toindex,int num,int select)
{
  char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];
  char token[1024],buf[128],*pUserid;
  int buttontype = 0, windowtype = 0, windowno = 0,nAid,nLen;
  int fd = getfdFromCharaIndex(toindex);
  char Data[8][32],talk[128];
  int i,j,k,Ret;
  LPList DataList = NULL,Kill = NULL;

	
  if(fd == -1) 
  {
    print("getfd err\n");
    return;
  }
	
  if(NPC_Util_GetArgStr(meindex,npcarg,sizeof(npcarg)) == NULL)
  {
    print("GetArgStrErr");
    return ;
  }
  
  token[0] = '\0';

  //�����ʹ��ʯ������Ա�Ĺ���
  if(!g_EnableService)
  {
    sprintf(token,"\n ����ҵ���ڱ���������û�п�ͨ��");
    buttontype = WINDOW_BUTTONTYPE_OK;
    windowtype = WINDOW_MESSAGETYPE_MESSAGE;
    windowno = CHAR_WINDOWTYPE_SERVICE_WAIT;
    
    lssproto_WN_send(fd,windowtype,buttontype,windowno,
                     CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX),
		     token);    
    return;
  }
  
  //����������ǵ�һ����NPC����
  if(CHAR_getWorkInt(meindex,NPC_WORK_ISUSE) == 0 && 
     CHAR_getWorkInt(meindex,NPC_WORK_TOINDEX) == -1)
  {
    //��ס��һ����NPC��������˭
    CHAR_setWorkInt(meindex,NPC_WORK_TOINDEX,toindex);
    //�趨ΪNPC����ʹ����
    CHAR_setWorkInt(meindex,NPC_WORK_ISUSE,1);
  }
  //����ǵڶ����Ժ��NPC��������
  else if(CHAR_getWorkInt(meindex,NPC_WORK_ISUSE) == 1 &&
          CHAR_getWorkInt(meindex,NPC_WORK_TOINDEX) != toindex)
  {
    sprintf(token,"\n ���ڷ�����������У����Ե�...");
    buttontype = WINDOW_BUTTONTYPE_OK;
    windowtype = WINDOW_MESSAGETYPE_MESSAGE;
    windowno = CHAR_WINDOWTYPE_SERVICE_WAIT;
    
    lssproto_WN_send(fd,windowtype,buttontype,windowno,
                     CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX),
		     token);    
    return;
  }
  if(flag) 
  {
    ConnectToAP();
	flag = 0;
  }
  switch(num)
  {
  case 0:
  case 1:
    //ȡ����ҵ�id
    pUserid = CHAR_getChar(toindex,CHAR_CDKEY);
    if(CHAR_getWorkInt(meindex,NPC_WORK_FLAG2) == 1)
    {
	  if(pUserid == NULL || SendToAP(Cli_CHECKPLAYER,meindex,1,pUserid,NULL) == 0)
      {
        if(NPC_Util_GetStrFromStrWithDelim(npcarg,"SysBusyMsg",buf,sizeof(buf)) == NULL) return;
        sprintf(token,"\n %s",buf);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
        break;
      }
      CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,0);
      CHAR_setWorkInt(meindex,NPC_WORK_START,1);
    }
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_2)
    {
			// ��������time out
      if(CHAR_getWorkInt(meindex,NPC_WORK_TIMEOUT) >= 50)
      {
        close(g_nServiceSocket);
        flag = 1;
        sprintf(token,"\n ������ϵͳȡ������...");
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
				CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
				CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
				CHAR_setWorkInt(meindex,NPC_WORK_START,0);
        break;
      }
      if((Ret = RecvFromAP(&nAid,meindex,&nLen,&DataList)) == -1)
      {
        sprintf(token,"\n �ȴ���Ӧ...");
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
        break;
      }
      else 
      {
        for(i=0;i<nLen;i++)
        {
          strcpy(Data[i],DataList->Data);
          Kill = DataList;
          DataList = DataList->Next;
          if(Kill != NULL) free(Kill);
        }
        // ʹ�ù���ѶϢ���
        bzero(&g_APMsg[Ret],sizeof(Msg));
      }
      if(nAid == AP_CHECKPLAYER)
      {
        switch(atoi(Data[0]))
        {
        case 0: //û��
          if(NPC_Util_GetStrFromStrWithDelim(npcarg,"ErrMsg",buf,sizeof(buf)) == NULL) return;
          sprintf(token,"\n %s",buf);
          buttontype = WINDOW_BUTTONTYPE_OK;
          windowtype = WINDOW_MESSAGETYPE_MESSAGE;
          windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;     
        break;
        case 1: //����
          if(NPC_Util_GetStrFromStrWithDelim(npcarg,"MainMsg",buf,sizeof(buf)) == NULL) return;
          sprintf(token,
				  "5\n %s"
				  "\n ��Ҫ�� '%s'"
				  "\n �Ľ�Ʒ�������ɫ��\n"
				  "\n"
				  "\n                    �� ȷ  �� ��"
				  "\n"
				  "\n                    �� ȡ  �� ��",
				  buf,Data[1]);
          buttontype = WINDOW_BUTTONTYPE_NONE;
          windowtype = WINDOW_MESSAGETYPE_SELECT;
          windowno = CHAR_WINDOWTYPE_SERVICE_START; 
        break;
        //��һ����ɫ����ʹ����
        case 2:
          sprintf(token,"\n Ŀǰ������һ����ɫ������ȡ.");
          buttontype = WINDOW_BUTTONTYPE_OK;
          windowtype = WINDOW_MESSAGETYPE_MESSAGE;
          windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
          break;
		//��ҵĽ�Ʒ������
		case 3:
		  sprintf(token,"\n ���Ľ�Ʒ�Ѿ�������!!");
          buttontype = WINDOW_BUTTONTYPE_OK;
          windowtype = WINDOW_MESSAGETYPE_MESSAGE;
          windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
          break;
        break;
        }
      }
      else
      {
        sprintf(token,"\n ϵͳѶϢ����!!(%d)",nAid);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
      }
      CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
      CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
    }
    else
    {
      sprintf(token,"\n �ȴ���Ӧ...");
      buttontype = WINDOW_BUTTONTYPE_OK;
      windowtype = WINDOW_MESSAGETYPE_MESSAGE;
      windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
    }
  break;
  case 2:
    //ȡ����ҵ�id
    pUserid = CHAR_getChar(toindex,CHAR_CDKEY);
    if(CHAR_getWorkInt(meindex,NPC_WORK_FLAG3) == 1){
      if(pUserid == NULL || SendToAP(Cli_GET,meindex,1,pUserid,NULL) == 0){
        if(NPC_Util_GetStrFromStrWithDelim(npcarg,"SysBusyMsg",buf,sizeof(buf)) == NULL) return;
        sprintf(token,"\n %s",buf);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
        break;
      }
			CHAR_setWorkInt(meindex,NPC_WORK_START,1);
      CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,0);
    }
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_3){
			// ��������time out
      if(CHAR_getWorkInt(meindex,NPC_WORK_TIMEOUT) >= 50){
        close(g_nServiceSocket);
        flag = 1;
        sprintf(token,"\n ������ϵͳȡ������...");
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
				CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
				CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
				CHAR_setWorkInt(meindex,NPC_WORK_START,0);
        break;
      }
      if((Ret = RecvFromAP(&nAid,meindex,&nLen,&DataList)) == -1){
        sprintf(token,"\n �ȴ���Ӧ...");
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
        break;
      }
      else
      {
        for(i=0;i<nLen;i++)
        {
          strcpy(Data[i],DataList->Data);
          Kill = DataList;
          DataList = DataList->Next;
          if(Kill != NULL) free(Kill);
        }
        // ʹ�ù���ѶϢ���
        bzero(&g_APMsg[Ret],sizeof(Msg));
      }
      if(nAid == AP_GET)
      {
        //����Ǹ�Ǯ
        //ȷ�����������û�пյ���λ
        if(atoi(Data[2]) == 0){
          if(CHAR_getInt(toindex,CHAR_GOLD)+atoi(Data[1]) > CHAR_getMaxHaveGold(toindex) ){
            sprintf(token,"\n"
                          "\n �����ϵĿռ䲻���Է�����ô��Ǯ��\n");
            buttontype = WINDOW_BUTTONTYPE_OK;
            windowtype = WINDOW_MESSAGETYPE_MESSAGE;
            windowno = CHAR_WINDOWTYPE_SERVICE_EXIT; 
            break;
          }else{
			CHAR_AddGold( toindex, atoi(Data[1]) );

            CHAR_send_P_StatusString(toindex,CHAR_P_STRING_GOLD);
            LogService(CHAR_getChar(toindex,CHAR_NAME),
                       CHAR_getChar(toindex,CHAR_CDKEY),
                       atoi(Data[1]),
                       "Ǯ",
                       CHAR_getInt(toindex,CHAR_FLOOR),
                       CHAR_getInt(toindex,CHAR_X),
                       CHAR_getInt(toindex,CHAR_Y)
                      );
            sprintf(talk,"%sʯ���Ѿ������!!\n",Data[1]);
          }
        }
        //����Ǹ���Ʒ
        else if(atoi(Data[2]) == 1){
          //ȷ�����������û�пյ���λ
          int num = 0,itemindex = 0;

          for(i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++)
            if(CHAR_getItemIndex(toindex,i) != -1) num++;
          if(atoi(Data[1])+num > CHAR_MAXITEMNUM){
            sprintf(token,"\n"
                          "\n ��ĵ������ռ䲻����\n");
            buttontype = WINDOW_BUTTONTYPE_OK;
            windowtype = WINDOW_MESSAGETYPE_MESSAGE;
            windowno = CHAR_WINDOWTYPE_SERVICE_EXIT; 
            break;
          }
          //�пյ���λ
          for(j=0;j<atoi(Data[1]);j++){
            for(i=CHAR_STARTITEMARRAY;i<CHAR_MAXITEMHAVE;i++){
              if(CHAR_getItemIndex(toindex,i) == -1){
                itemindex = ITEM_makeItemAndRegist(atoi(Data[0]));
                CHAR_setItemIndex(toindex,i,itemindex);
                ITEM_setWorkInt(itemindex,ITEM_WORKOBJINDEX,-1);
                ITEM_setWorkInt(itemindex,ITEM_WORKCHARAINDEX,toindex);
                CHAR_sendItemDataOne(toindex,i);
                LogService(CHAR_getChar(toindex,CHAR_NAME),
                           CHAR_getChar(toindex,CHAR_CDKEY),
                           atoi(Data[0]),
                           "��Ʒ",
                           CHAR_getInt(toindex,CHAR_FLOOR),
                           CHAR_getInt(toindex,CHAR_X),
                           CHAR_getInt(toindex,CHAR_Y)
                          );
                break;
              }
            }
          }
          sprintf(talk,"%s %s���Ѿ������!!\n",ITEM_getAppropriateName(itemindex),Data[1]);
        }
        //����Ǹ�����
        else if(atoi(Data[2]) == 2)
        {
          //ȷ�����������û�пյ���λ
          int num = 0,ret = 0;
          
          for(i=0;i<CHAR_MAXPETHAVE;i++)
            if(CHAR_getCharPet(toindex,i) != -1) num++;
          if(atoi(Data[1])+num > CHAR_MAXPETHAVE)
          {
            sprintf(token,"\n"
                          "\n ��ĳ������ռ䲻����\n");
            buttontype = WINDOW_BUTTONTYPE_OK;
            windowtype = WINDOW_MESSAGETYPE_MESSAGE;
            windowno = CHAR_WINDOWTYPE_SERVICE_EXIT; 
            break;
          }
          //�пյ���λ
          for(j=0;j<atoi(Data[1]);j++)
          {
            for(i=0;i<CHAR_MAXPETHAVE;i++)
            {
              if(CHAR_getCharPet(toindex,i) == -1)
              {
                int enemynum,count;
                char msgbuf[64];

                enemynum = ENEMY_getEnemyNum();
                for(k=0;k<enemynum;k++)
                {
                  if(ENEMY_getInt(k,ENEMY_ID) == atoi(Data[0])) break;
                }
                if(k == enemynum) break;
#ifdef _NEW_PETMAKE
                ret = ENEMY_createPetFromEnemyIndex_new(toindex, k, atoi(Data[4]), atoi(Data[5]), atoi(Data[6]), atoi(Data[7]));
				if(atoi(Data[3]) > 0)
				{
					CHAR_setInt( ret, CHAR_LV, atoi(Data[3]) );
				}
#else
				ret = ENEMY_createPetFromEnemyIndex(toindex,k);
#endif
                CHAR_setCharPet(toindex,i,ret);
                snprintf(msgbuf,sizeof(msgbuf),"K%d",i);
                CHAR_sendStatusString(toindex,msgbuf);
                snprintf(msgbuf,sizeof(msgbuf),"W%d",i);
                CHAR_sendStatusString(toindex,msgbuf);

				for(count=0;count<CHAR_MAXPETHAVE;count++)
				{
					if(CHAR_getCharPet(toindex,count) == ret)
					{	
						CHAR_send_K_StatusString(toindex,count,CHAR_K_STRING_HP|CHAR_K_STRING_AI);
					}
				}
				//CHAR_setInt(petindex,CHAR_VARIABLEAI,CHAR_MAXVARIABLEAI);
                LogService(CHAR_getChar(toindex,CHAR_NAME),
                           CHAR_getChar(toindex,CHAR_CDKEY),
                           atoi(Data[0]),
                           "����",
                           CHAR_getInt(toindex,CHAR_FLOOR),
                           CHAR_getInt(toindex,CHAR_X),
                           CHAR_getInt(toindex,CHAR_Y)
                          );
                break;
              }
            }
          }
          sprintf(talk,"%s %sֻ�Ѿ������!!\n",CHAR_getChar(ret,CHAR_NAME),Data[1]);
        }
        //������ϴ浵
        CHAR_charSaveFromConnect(CHAR_getWorkInt(toindex,CHAR_WORKFD),0);
       if(pUserid == NULL || SendToAP(Cli_YES,meindex,1,pUserid,NULL) == 0)
       {
          if(NPC_Util_GetStrFromStrWithDelim(npcarg,"SysBusyMsg",buf,sizeof(buf)) == NULL) return;
          sprintf(token,"\n %s",buf);
          buttontype = WINDOW_BUTTONTYPE_OK;
          windowtype = WINDOW_MESSAGETYPE_MESSAGE;
          windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
		  LogService(CHAR_getChar(toindex,CHAR_NAME),
 					 CHAR_getChar(toindex,CHAR_CDKEY),
					 -1,
					 "�������ȡ���ﵫCli_YESû�͵�AP",
					 CHAR_getInt(toindex,CHAR_FLOOR),
					 CHAR_getInt(toindex,CHAR_X),
					 CHAR_getInt(toindex,CHAR_Y)
					 );
          break;
        }
        if(NPC_Util_GetStrFromStrWithDelim(npcarg , "ExitMsg" , buf , sizeof(buf)) == NULL) return;
        sprintf(token,"%s %s",talk,buf);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
        CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
        CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
        CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_4);
      }
      else 
      {
        sprintf(token,"\n ϵͳѶϢ����!!(%d)",nAid);
        buttontype = WINDOW_BUTTONTYPE_OK;
        windowtype = WINDOW_MESSAGETYPE_MESSAGE;
        windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
        break;
      }
    }
    else
    {
      sprintf(token,"\n �ȴ���Ӧ...");
      buttontype = WINDOW_BUTTONTYPE_OK;
      windowtype = WINDOW_MESSAGETYPE_MESSAGE;
      windowno = CHAR_WINDOWTYPE_SERVICE_CONTINUE;
    }
  break;
  case 3:
    pUserid = CHAR_getChar(toindex,CHAR_CDKEY);
    if(NPC_Util_GetStrFromStrWithDelim(npcarg,"CancelMsg",buf,sizeof(buf)) == NULL) return;
    sprintf(token,"\n %s",buf);
    buttontype = WINDOW_BUTTONTYPE_OK;
    windowtype = WINDOW_MESSAGETYPE_MESSAGE;
    windowno = CHAR_WINDOWTYPE_SERVICE_EXIT;
  break;
  }
  
  lssproto_WN_send(fd,windowtype,buttontype,windowno,
		   CHAR_getWorkInt(meindex,CHAR_WORKOBJINDEX),
		   token);

}

void NPC_StoneServiceManLoop(int meindex)
{
  int nLeaveCount,nTimeout;
  fd_set readfds;
  struct timeval tm;
  LPList kill = NULL;
	//static unsigned int dwLifeTime = 0;
  
  if(g_EnableService)
  {
    // �����Һ�NPC�����˾Ϳ�ʼ����
    if(CHAR_getWorkInt(meindex,NPC_WORK_ISUSE) == 1)
    {
      nLeaveCount = CHAR_getWorkInt(meindex,NPC_WORK_LEAVE_COUNT);
      // �����ҳ���ʱ��û��ɶ���(Ҳ�п��������û�а���ok���뿪�Ի���)
      // ��ʱҪ��NPC�Ĺ���������Ϊ���������һ�����talk
      if(nLeaveCount > getLoopTime())
      {
        int  talkerindex,i;
        char *pUserid;
       
        talkerindex = CHAR_getWorkInt(meindex,NPC_WORK_TOINDEX);
        pUserid = CHAR_getChar(talkerindex,CHAR_CDKEY);
        if(pUserid != NULL) SendToAP(Cli_CANCEL,meindex,1,pUserid,NULL);
        CHAR_setWorkInt(meindex,NPC_WORK_FLAG1,1);
        CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
        CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
        CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_2);
        CHAR_setWorkInt(meindex,NPC_WORK_START,0);
        CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
        CHAR_setWorkInt(meindex,NPC_WORK_TOINDEX,-1);
        CHAR_setWorkInt(meindex,NPC_WORK_ISUSE,0);
        CHAR_setWorkInt(meindex,NPC_WORK_LEAVE_COUNT,0);
        CHAR_setWorkInt(meindex,NPC_WORK_SERIALNUM,-1);
        //��֮ǰ�յ��й����NPC��ѶϢbuf���
        for(i=0;i<MAX_AP_MSG;i++)
        {
          // ���ѶϢ����û�б�ʹ��
          if(g_APMsg[i].bIsUse == 1)
          {
            // ����ǲ����͸����NPC��ѶϢ
            if(g_APMsg[i].nIndex == meindex)
            {
              // ʹ�ù���ѶϢ���
              while(g_APMsg[i].Data != NULL)
              {
                kill = g_APMsg[i].Data;
                g_APMsg[i].Data = g_APMsg[i].Data->Next;
                if(kill != NULL) free(kill);
              }
              bzero(&g_APMsg[i],sizeof(Msg));
            }
          }
        }
        print("Player leave NPC\n");
        return;
      }
      // δ����ʱ�������
      CHAR_setWorkInt(meindex,NPC_WORK_LEAVE_COUNT,++nLeaveCount);
    }

		/*if(dwLifeTime > 3000) 
		{
			SendToAP(Cli_CONNECT,meindex,0,NULL,NULL);
			dwLifeTime = 0;
			print("\nI'm life!!!!!");
		}
		else dwLifeTime++;*/
		 // NPC_WORK_STARTΪ1ʱ��ʱ����,��������Ϊtime out
    if(CHAR_getWorkInt(meindex,NPC_WORK_START) == 1)
    {
			tm.tv_sec = 0;
			tm.tv_usec = 0;
			FD_ZERO(&readfds);
			FD_SET(g_nServiceSocket,&readfds);
			// ���AP��û����ѶϢ����
			if(!select(g_nServiceSocket+1,&readfds,NULL,NULL,&tm))
			{
				nTimeout = CHAR_getWorkInt(meindex,NPC_WORK_TIMEOUT);
                CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,++nTimeout);
				return;
			}
			print("\nAP send message.");
			
			// ����ѶϢ��������ѶϢ
			switch(GetMsg(meindex))
			{
				//���ѶϢ���Ǹ����NPC
			case  0: 
				print("Msg not send to this npc!!\n");
				nTimeout = CHAR_getWorkInt(meindex,NPC_WORK_TIMEOUT);
				CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,++nTimeout);
				return;
			case -1: break; //����connect to ap
			case -2: print("Msg buf is full!!\n"); break; 	 // ѶϢbuffer�ù���
			case -3: print("Msg data is wrong!!\n"); break;  // ѶϢ���ݴ���
			case -4: print("calloc fail!!\n"); break;        // ���������ô�ʧ��
			case -5: print("\nAP send get I'm life"); break; // AP�յ�Cli_CONNECT�Ļ�Ӧ
			}
			CHAR_setWorkInt(meindex,NPC_WORK_START,0);
		}
  }
}

void NPC_StoneServiceManWindowTalked(int meindex,int talkerindex,int seqno,int select,char *data)
{
  int datanum = -1,i;
  char *pUserid;
  LPList Kill = NULL;

  makeStringFromEscaped(data);
  datanum = atoi( data);
  switch(seqno)
  {
  case CHAR_WINDOWTYPE_SERVICE_START:
    if(datanum == 1) 
    {
      CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_3);
      NPC_StoneServiceMan_selectWindow(meindex,talkerindex,2,-1);
    }
    else 
    if(datanum == 3) NPC_StoneServiceMan_selectWindow(meindex,talkerindex,3,-1);
  break;
  case CHAR_WINDOWTYPE_SERVICE_EXIT:
    pUserid = CHAR_getChar(talkerindex,CHAR_CDKEY);
    if(pUserid != NULL) SendToAP(Cli_CANCEL,meindex,1,pUserid,NULL);
    CHAR_setWorkInt(meindex,NPC_WORK_FLAG1,1);
    CHAR_setWorkInt(meindex,NPC_WORK_FLAG2,1);
    CHAR_setWorkInt(meindex,NPC_WORK_FLAG3,1);
    CHAR_setWorkInt(meindex,NPC_WORK_STATE,SERVICE_STATE_2);
    CHAR_setWorkInt(meindex,NPC_WORK_START,0);
    CHAR_setWorkInt(meindex,NPC_WORK_TIMEOUT,0);
    CHAR_setWorkInt(meindex,NPC_WORK_TOINDEX,-1);
    CHAR_setWorkInt(meindex,NPC_WORK_ISUSE,0);
    CHAR_setWorkInt(meindex,NPC_WORK_LEAVE_COUNT,0);
    CHAR_setWorkInt(meindex,NPC_WORK_SERIALNUM,-1);
    //��֮ǰ�յ��й����NPC��ѶϢbuf���
    for(i=0;i<MAX_AP_MSG;i++)
    {
      // ���ѶϢ����û�б�ʹ��
      if(g_APMsg[i].bIsUse == 1)
      {
        // ����ǲ����͸����NPC��ѶϢ
        if(g_APMsg[i].nIndex == meindex)
        {
          // ʹ�ù���ѶϢ���
          while(g_APMsg[i].Data != NULL)
          {
            Kill = g_APMsg[i].Data;
			if(g_APMsg[i].Data->Next != NULL) 
			{
				g_APMsg[i].Data = g_APMsg[i].Data->Next;
				if(Kill != NULL) free(Kill);
			}
			else
			{
				if(Kill != NULL) free(Kill);
				break;
			}
          }
          bzero(&g_APMsg[i],sizeof(Msg));
        }
      }
    }
  break;
  case CHAR_WINDOWTYPE_SERVICE_CONTINUE:
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_1)
    {
      NPC_StoneServiceMan_selectWindow(meindex,talkerindex,0,-1);
    }
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_2)
    {
      NPC_StoneServiceMan_selectWindow(meindex,talkerindex,1,-1);
    }
    if(CHAR_getWorkInt(meindex,NPC_WORK_STATE) == SERVICE_STATE_3)
    {
      NPC_StoneServiceMan_selectWindow(meindex,talkerindex,2,-1);
    }
  break;
  case CHAR_WINDOWTYPE_SERVICE_WAIT:
  break;
  default:
    print("ERROR type\n");
  break;
  }
}

//��ѶϢ��AP

//nAid (Cli_????) nIndex (npc Index) nLen (�������ϱ���) Data1 (user cdkey)
int SendToAP(int nAid,int nIndex,int nLen,void *Data1,void *Data2)
{
  char		buf[128];
  char		*pData[2];
  int		i,j,bufnum;
  fd_set	fdset;
  struct  timeval tmv;
  int     ret;
  
 // Syu ADD ����ʯ������Ա��ϵȷ��
  char *sags= "sa1.82";
 /*
  if( strcmp ( getAccountservername() , "210.64.50.31") == 0 ) sags = "sags31"; //����
  if( strcmp ( getAccountservername() , "210.64.50.41") == 0 ) sags = "sags41"; //����
  if( strcmp ( getAccountservername() , "210.64.50.51") == 0 ) sags = "sags51"; //̫��
  if( strcmp ( getAccountservername() , "210.64.50.61") == 0 ) sags = "sags61"; //��΢
  if( strcmp ( getAccountservername() , "210.64.50.71") == 0 ) sags = "sags71"; //����
  if( strcmp ( getAccountservername() , "210.64.50.81") == 0 ) sags = "sags81"; //����
  if( strcmp ( getAccountservername() , "210.64.50.91") == 0 ) sags = "sags91"; //��ӥ
  if( strcmp ( getAccountservername() , "210.64.50.101") == 0 ) sags = "sags101"; //��·��ͥ
  if( strcmp ( getAccountservername() , "210.64.50.106") == 0 ) sags = "sags106"; //����԰
  if( strcmp ( getAccountservername() , "210.64.50.111") == 0 ) sags = "sags111"; //ʥ��
  if( strcmp ( getAccountservername() , "152.104.1.206") == 0 ) sags = "hkgs206"; //���
  if( strcmp ( getAccountservername() , "202.134.122.130") == 0 ) sags = "hkgs130"; //�½�
*/
  
  bzero(buf,sizeof(buf));
  pData[0] = (char*)Data1;
  pData[1] = (char*)Data2;
  
  // ����Ϊ����
  // ȡ����ˮ��
  nAid = GetSerialNum()*10+nAid;
// Syu ADD ����ʯ������Ա��ϵȷ��
  nLen=2;	

  sprintf(buf,"&;%d;%d;%d;",nAid,nIndex,nLen);
  CHAR_setWorkInt(nIndex,NPC_WORK_SERIALNUM,nAid/10);
  if(nLen > 0)
  {
 // Syu ADD ����ʯ������Ա��ϵȷ��
    for(j = 0 ; j < 1 ; j++)
    {
      for(i = strlen(buf) ; i < 128 ; i++)
      {
      	if(*pData[j] == '\0')
      	  break;
      	else if(*pData[j] == ';')
      	{
      	  buf[i] = '\\';
      	  buf[++i] = ';';
      	}
      	else if(*pData[j] == '\\')
      	{
      	  buf[i] = '\\';
      	  buf[++i] = '\\';
      	}
      	else if(*pData[j] == '&')
      	{
      	  buf[i] = '\\';
      	  buf[++i] = '&';
      	}
      	else buf[i] = *pData[j];
      	pData[j]++;
      }
// Syu ADD ����ʯ������Ա��ϵȷ��
	  strcat(buf,";");
	  strcat(buf,sags);

      buf[strlen(buf)] = ';';

    }
  }
  else strcat(buf,"0;0;");


  memset(&buf[strlen(buf)],'&',sizeof(buf)-strlen(buf));

  // �����·�ɲ����Դ���ѶϢ
  FD_ZERO(&fdset);
  FD_SET(g_nServiceSocket,&fdset);
  tmv.tv_sec = tmv.tv_usec = 0;	
  ret = select(g_nServiceSocket + 1,(fd_set*)NULL,&fdset,(fd_set*)NULL,&tmv);
  print("ret1=%d ",ret);


  if(ret > 0)
	{
		// ��ѶϢ��AP
		if((bufnum = write(g_nServiceSocket,buf,sizeof(buf))) < 0)
		{
			//������ʧȥ����
			print("reconnect to server!\n");
			close(g_nServiceSocket);
			flag = 1;
			return 0;
		}
		buf[sizeof(buf) - 1] = '\0';
		print("\n**SendToAP**:[%s]\n",buf);
		print("Send %d bytes to AP.\n",bufnum);

	}
	else
	{
		print("reconnect to server!\n");
		close(g_nServiceSocket);
		flag = 1;
			
		return 0;
	}
	
  return 1;
}

//�յ�AP��Ӧ��ѶϢ
int RecvFromAP(int *nAid,int nIndex,int *nLen,LPList *Data)
{
  int  i;
  
  for(i=0;i<MAX_AP_MSG;i++)
  {
    // ���ѶϢ����û�б�ʹ��
    if(g_APMsg[i].bIsUse == 1)
    {
      // ����ǲ����͸����NPC��ѶϢ
      if(g_APMsg[i].nIndex == nIndex)
      {
        *nAid = g_APMsg[i].nAid;
        //�����ˮ�ŶԲ���
        if((*nAid)/10 == CHAR_getWorkInt(nIndex,NPC_WORK_SERIALNUM))
        {
          *nAid = *nAid%10;
          *nLen = g_APMsg[i].nLen;
          *Data = g_APMsg[i].Data;
          
          return i;
        }
      }
    }
  }
  return -1;
}

int GetMsg(int meindex)
{
  char buf[128];
  char temp[3][128];
  char *p;
  int  i = 0,j;
  pMsg Msg = NULL;
  LPList tempList = NULL;
  int readnum;
  
  bzero(buf,sizeof(buf));
  // ����AP������ѶϢ
  if((readnum = read(g_nServiceSocket, rdata_size, RFIFOSPACE)) <= 0)
  {
    print("can't read message!\n");
    //����AP
	close(g_nServiceSocket);
    flag = 1;
    return -1;
  }

  rdata_size += readnum;
  
  if(RFIFOREST >= sizeof(buf))
    {
      memcpy(buf, rdata_pos, sizeof(buf));
      
      rdata_pos += sizeof(buf);
      if(rdata_pos == rdata_size)
	{
	  rdata_pos = msgbuf;
	  rdata_size = msgbuf;
	}
      if((rdata_pos - msgbuf) * 4 > sizeof(msgbuf))
	{
	  memmove(msgbuf, rdata_pos, RFIFOREST);
	  rdata_size = msgbuf + RFIFOREST;
	  rdata_pos = msgbuf;
	}
    }
  else
    return 0;

  buf[sizeof(buf) - 1] = '\0';
  print("**GetFromAP**:[%s]\n",buf);
  
  // ȡ��һ��û�ù���ѶϢ������
  for(j=0;j<MAX_AP_MSG;j++)
  {
    if(g_APMsg[j].bIsUse == 0) 
    {
      Msg = &g_APMsg[j];
      if((Msg->Data = (LPList)calloc( 1,sizeof(List))) != NULL)
      {
        Msg->Data->Next = NULL;
        tempList = Msg->Data;
        break;
      }
      else return -4;
    }
  }
  // ����MAX_AP_MSG��ʾѶϢ��������
  if(j == MAX_AP_MSG) return -2;
	Msg->bIsUse = 1;
  
  // ����Ϊ����
  if(buf[i] == '&')
  {
    if(buf[++i] == ';')
    {
      bzero(temp[0] , sizeof(temp[0]));
      bzero(temp[1] , sizeof(temp[1]));
      bzero(temp[2] , sizeof(temp[2]));
      for(j = 0 ; j < 3 ; j++)
      {
        p = strchr(&buf[i+1] , ';');
        strncpy(temp[j] , &buf[i+1] , p - (&buf[i+1]));
        i += strlen(temp[j]) + 1;
      }
      Msg->nAid   = atoi(temp[0]);
      Msg->nIndex = atoi(temp[1]);
      Msg->nLen   = atoi(temp[2]);
	  // ����յ�����AP_CONNECT�������ѶϢ
	  if(Msg->nAid%10 == AP_CONNECT)
		{
			free(Msg->Data);
			bzero(Msg,sizeof(Msg));
			return -5;
		}
      j = 0;
      
      bzero(temp[0] , sizeof(temp[0]));
      
      if(Msg->nLen > 0)
      {
        while(buf[++i] != '\0')
        {
          switch(buf[i])
          {
          case '\\':
            switch(buf[i + 1])
            {
            case ';':
            case '&':
            case '\\':
              temp[0][j++] = buf[++i];
              break;
            default:
              temp[0][j++] = buf[i++];
              break;
            }
          break;
          case ';':
            j = 0;
            strcpy(tempList->Data,temp[0]);
            tempList->Next = (LPList)calloc( 1,sizeof(List));
            tempList = tempList->Next;
            bzero(temp[0],sizeof(temp[0]));
          break;
          case '&':
            buf[i--] = '\0';
          break;
          default:
            temp[0][j++] = buf[i];
          break;
          }
        }
        tempList->Next = NULL;
        // ������ѶϢ�Ǹ����NPC return 1
        if(Msg->nIndex == meindex)
			return 1;
        else
			return 0;
      }
    }
  }
  else return -3;
  
  return 1;
}

int ConnectToAP(void)
{
  //make socket and connect to ap
  if((g_nServiceSocket = socket(PF_INET,SOCK_STREAM,0)) < 0)
  //if((g_nServiceSocket = socket(PF_INET,SOCK_DGRAM,0)) < 0)
  {
    print("can't create socket!\n");
    return 0;
  }
  bzero((char*)&server_sin , sizeof(server_sin));
  server_sin.sin_family = AF_INET;
  server_sin.sin_port = htons(g_Port);
  server_sin.sin_addr = inet_addr(g_ApID);
  memcpy(&temp , &server_sin , sizeof(server_sin));
  if(connect(g_nServiceSocket,&temp,sizeof(temp)) < 0)
  {
//	print("connect server error: %s\n" , sys_errlist[errno]);
	print("connect server error !!\n");
    print("ip:%s,port:%d\n",g_ApID,g_Port);
    return 0;
  }
  else print("connect AP server success!\n");

  memset(msgbuf, 0, sizeof(msgbuf));
  rdata_pos = msgbuf;
  rdata_size = msgbuf;
  
  return 1;
}

//ȡ����ˮ��
int GetSerialNum(void)
{
  static int i = 0;
  
  if(++i > 65535)
  {
    i = 0;
  }
  return i;
}

#endif
