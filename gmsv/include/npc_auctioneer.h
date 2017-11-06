#ifndef __NPC_AUCTIONEER_H__
#define __NPC_AUCTIONEER_H__

#ifdef _AUCTIONEER

// ��������������Ҫ�� saac/auction.h ����Ķ���һ��
#define MAX_AUCTION	100	// ����ί����
#define MAX_OVERDUE	500	// ���ڵ�ί�е������� (��С�趨=���ڱ�������*���ί����)
#define AUC_PET     1
#define AUC_ITEM    2

typedef struct tagAuctionTable {
	int  flag;		      // 0=(null) 1=ʹ���� 2=����
	char cdkey[32];		  // ί���� cdkey
	char customer[32];	// ί������������
	int itemtype;		    // 1=���� 2=���� otherwise=error
	char goods[4096];	  // ��Ʒ (���� or ����)
	char description[256];	// ����
  char listdata[512]; // ��ʾ�ڱ�������
	int price;		      // ���
  int onsaletime;
  int overduetime;
} AuctionTable;

extern AuctionTable onsale[MAX_AUCTION];	// �����еĵ��ߡ�����
//extern AuctionTable overdue[MAX_OVERDUE];	// ���ڵĵ��ߡ�����

BOOL NPC_AuctioneerInit(int meindex);
void NPC_AuctioneerTalked(int meindex, int talkerindex, char *msg, int color);
void NPC_AuctioneerWindowTalked(int meindex, int talkerindex, int seqno, int select, char *data);
void NPC_AuctioneerLoop(int meindex);

#endif /* _AUCTIONEER */

#endif 
