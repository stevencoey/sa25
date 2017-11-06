#ifndef _SERVICE_H
#define _SERVICE_H

typedef struct _NPC
{
  int use;// is using?
  int index; //npc index
  unsigned sqlindex; //index in database
  char gmsvname[32]; //gmsv name
  char username[32]; //usr name
  int  buyflag; //does the man have goods?
  char goodsname[32]; //goods name
  int  type; //goods type;
  int  amount; //goods amount;
  int  code; //goods code;
  char param[32]; //goods params;
}NPC, PNPC;

enum _AP_CLIENT_MESSAGE
  {
    Cli_CHECKPLAYER,  //Ҫ��AP������Ƿ��й����Ʒ��ȷ��  0
    AP_CHECKPLAYER,   //AP��Ӧ����Ƿ��й����Ʒ
    Cli_YES,          //���ȷ��Ҫ����Ʒ����
    Cli_CANCEL,       //֪ͨAP��lock��λ��Ϊ0             
    Cli_GET,          //֪ͨAP����Ʒ
    AP_GET,           //AP��Ӧ��Ʒ5
    Cli_CONNECT,      //�����Ƿ�������AP
    AP_CONNECT,       //AP��Ӧ
    AP_ERROR          //AP�д���
  };

enum _BUYORNOT
  {
    DIDNOT_BUY = 0,
    DID_BUY,
    ANOTHER_USING,
    ALL_TAKEN,
  };

enum _GOODS_TYPE
  {
    GOLD = 0,
    ITEM,
    PET,
  };

#define ARRY_NUM 128
extern NPC npc_arry[ARRY_NUM];

int init_service(void);
int do_service(int);

#endif
