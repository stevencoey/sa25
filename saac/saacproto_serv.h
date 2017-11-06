/* output by ../../bin/lsgen.perl 0.41 ( 1998 May)
 * made Thu Feb 17 17:30:27 2000
 * user ringo
 * host indy
 * file /opt/ringo/chsa/src/saac/./saacproto_serv.h
 * util ./saacproto_util.c , ./saacproto_util.h
 * src  /opt/ringo/chsa/src/saac/../../doc/saacproto.html
 */
#ifndef _SAACPROTOSERV_H_
#define _SAACPROTOSERV_H_
#include "version.h"
#include "saacproto_util.h"

#ifdef MAXLSRPCARGS
#if ( MAXLSRPCARGS <= ( 7 + 1 )  )
#undef MAXLSRPCARGS
#define MAXLSRPCARGS ( 7 + 1 ) 
#endif
#else
#define MAXLSRPCARGS ( 7 + 1 ) 
#endif
#ifdef _ALLDOMAN					// Syu ADD ���а�NPC
void saacproto_UpdataStele_recv( int fd , char *cdkey , char *name , char *title , int level , int trns , int time , int floor) ;
void saacproto_UpdataStele_send( int fd , char *data ) ;
void saacproto_S_UpdataStele_send( int fd , char *ocdkey , char *oname , char *ncdkey , 
					  char *nname , char *title , int level , int trns , int floor ) ;

#endif

void saacproto_ACServerLogin_recv( int fd,char* servername,char* serverpas ) ;
void saacproto_ACServerLogin_send( int fd,char* result,char* data ) ;
void saacproto_ACServerLogout_recv( int fd ) ;

#ifdef _ACNT_LOGIN
void saacproto_ACQueryAccount_recv (int ti, char *id, char *pas, int mesgid, int servid);
void saacproto_ACQueryAccount_send (int fd, char *result, char *output, int id);

#endif

void saacproto_ACCharList_recv( int ti,char* id,char* pas , int mesgid);

void saacproto_ACCharList_send( int fd,char* result,char* output,int id ) ; /* ../../doc/saacproto.html line 124 */
void saacproto_ACCharLoad_recv( int fd,char* id,char* pas,char* charname,int lock,char* opt,int mesgid ) ; /* ../../doc/saacproto.html line 148 */
// CoolFish: 2001/10/16
#ifdef _NewSave
void saacproto_ACCharLoad_send( int fd,char* result,char* data,int id,int charindex ) ; /* ../../doc/saacproto.html line 175 */
void saacproto_ACCharSave_recv( int fd,char* id,char* charname,char* opt,char* charinfo,int unlock,int mesgid,int charindex ) ; /* ../../doc/saacproto.html line 191 */
#else
void saacproto_ACCharLoad_send( int fd,char* result,char* data,int id ) ; /* ../../doc/saacproto.html line 175 */
void saacproto_ACCharSave_recv( int fd,char* id,char* charname,char* opt,char* charinfo,int unlock,int mesgid ) ; /* ../../doc/saacproto.html line 191 */
#endif
void saacproto_ACCharSave_send( int fd,char* result,char* data,int id ) ; /* ../../doc/saacproto.html line 214 */

#ifdef _RELOGIN
void saacproto_ACCharLoadII_recv( int fd,char* id,char* pas,char* charname,int lock,char* opt,int mesgid ,int gs1,int gs2 ) ; /* ../../doc/saacproto.html line 148 */
void saacproto_ACCharLoadII_send( int fd,char* result,char* data,int id,int charindex,int gs1,int gs2  ) ; /* ../../doc/saacproto.html line 175 */
void saacproto_ACCharSaveII_recv( int fd,char* id,char* charname,char* opt,char* charinfo,int unlock,int mesgid,int charindex,int gs1,int gs2  ) ; /* ../../doc/saacproto.html line 191 */
void saacproto_ACCharSaveII_send( int fd,char* result,char* data,int id,int gs1,int gs2  ) ; /* ../../doc/saacproto.html line 214 */
#endif

void saacproto_ACCharDelete_recv( int fd,char* id,char* passwd,char* charname,char* option,int mesgid ) ; /* ../../doc/saacproto.html line 231 */
void saacproto_ACCharDelete_send( int fd,char* result,char* data,int id ) ; /* ../../doc/saacproto.html line 256 */
void saacproto_ACLock_recv( int fd,char* id,int lock,int mesgid ) ; /* ../../doc/saacproto.html line 271 */
void saacproto_ACLock_send( int fd,char* result,char* data,int id ) ; /* ../../doc/saacproto.html line 290 */
void saacproto_ACUCheck_recv( int fd,char* mem_id,int status ) ; /* ../../doc/saacproto.html line 304 */
void saacproto_ACUCheck_send( int fd,char* mem_id ) ; /* ../../doc/saacproto.html line 318 */
void saacproto_DBUpdateEntryString_recv( int fd,char* table,char* key,char* value,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 329 */
void saacproto_DBUpdateEntryString_send( int fd,char* result,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 344 */
void saacproto_DBDeleteEntryString_recv( int fd,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 357 */
void saacproto_DBDeleteEntryString_send( int fd,char* result,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 369 */
void saacproto_DBGetEntryString_recv( int fd,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 384 */
void saacproto_DBGetEntryString_send( int fd,char* result,char* value,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 396 */
void saacproto_DBUpdateEntryInt_recv( int fd,char* table,char* key,int value,char* info,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 413 */
void saacproto_DBUpdateEntryInt_send( int fd,char* result,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 435 */
void saacproto_DBGetEntryRank_recv( int fd,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 449 */
void saacproto_DBGetEntryRank_send( int fd,char* result,int rank,int count,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 463 */
void saacproto_DBDeleteEntryInt_recv( int fd,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 482 */
void saacproto_DBDeleteEntryInt_send( int fd,char* result,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 494 */
void saacproto_DBGetEntryInt_recv( int fd,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 508 */
void saacproto_DBGetEntryInt_send( int fd,char* result,int value,char* table,char* key,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 520 */
void saacproto_DBGetEntryByRank_recv( int fd,char* table,int rank_start,int rank_end,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 537 */
void saacproto_DBGetEntryByRank_send( int fd,char* result,char* list,char* table,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 552 */
void saacproto_DBGetEntryByCount_recv( int fd,char* table,int count_start,int num,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 571 */
void saacproto_DBGetEntryByCount_send( int fd,char* result,char* list,char* table,int count_start,int msgid,int msgid2 ) ; /* ../../doc/saacproto.html line 583 */
void saacproto_Broadcast_recv( int fd,char* id,char* charname,char* message,int flag ) ; /* ../../doc/saacproto.html line 598 */
void saacproto_Broadcast_send( int fd,char* id,char* charname,char* message ) ; /* ../../doc/saacproto.html line 611 */
void saacproto_Message_recv( int fd,char* id_from,char* charname_from,char* id_to,char* charname_to,char* message,int option ) ; /* ../../doc/saacproto.html line 624 */
void saacproto_Message_send( int fd,char* id_from,char* charname_from,char* id_to,char* charname_to,char* message,int option,int mesgid ) ; /* ../../doc/saacproto.html line 642 */
void saacproto_MessageAck_recv( int fd,char* id,char* charname,char* result,int mesgid ) ; /* ../../doc/saacproto.html line 661 */
void saacproto_MessageFlush_recv( int fd,char* id,char* charname ) ; /* ../../doc/saacproto.html line 680 */
int saacproto_InitServer(int (*writefunc)(int,char*,int) ,	int worksiz );
void saacproto_SetServerLogFiles( char *read , char *write );
void saacproto_CleanupServer( void );
//int saacproto_ServerDispatchMessage( int fd, char *line );
int saacproto_ServerDispatchMessage( int fd , char *encoded, char *debugfun);

// CoolFish: Family 2001/5/9
// ��������
#ifdef _PERSONAL_FAME
void saacproto_ACAddFM_recv(int fd, char *fmname, char *fmleadername,
	char *fmleaderid, int fmleaderlv, char *petname, char *petarrt,
	char *fmrule, int fmsprite, int fmleadergrano, int fame, int charfdid);
#else
void saacproto_ACAddFM_recv(int fd, char *fmname, char *fmleadername,
	char *fmleaderid, int fmleaderlv, char *petname, char *petarrt,
	char *fmrule, int fmsprite, int fmleadergrano, int charfdid);
#endif
void saacproto_ACAddFM_send(int fd, char *result, int fmindex, int index,
	int charfdid);
// �������
// #ifdef _PERSONAL_FAME	// Arminius: �����������
void saacproto_ACJoinFM_recv(int fd, char *fmname, int fmindex, char *charname,
	char *charid, int charlv, int index, int fame, int charfdid);
// #else
// void saacproto_ACJoinFM_recv(int fd, char *fmname, int fmindex, char *charname,
//	char *charid, int charlv, int index, int charfdid);
// #endif
void saacproto_ACJoinFM_send(int fd, char *result, int recv, int charfdid);
// �˳�����
void saacproto_ACLeaveFM_recv(int fd, char *fmname, int fmindex, char *charname,
	char *charid, int index, int charfdid);
void saacproto_ACLeaveFM_send(int fd, char *result, int resultflag, int charfdid);
// ��ɢ����
#ifdef _LEADERFUNCHECK
void saacproto_ACDelFM_recv(int fd, char *fmname, int fmindex, int index, char *charname,
	char *charid, int charfdid);
#else
void saacproto_ACDelFM_recv(int fd, char *fmname, int fmindex, int index, int charfdid);
#endif
void saacproto_ACDelFM_send(int fd, char *result, int charfdid);
// �г������б�
void saacproto_ACShowFMList_recv(int fd);
void saacproto_ACShowFMList_send(int fd, char *result, int num, char *data);
// �г���Ա�б�
void saacproto_ACShowMemberList_recv(int fd, int index);
void saacproto_ACShowMemberList_send(int fd, char *result, int index, int num,
	int acceptflag, int fmjoinnum, char *data);
// �г�������ϸ����
void saacproto_ACFMDetail_recv(int fd, char *fmname, int fmindex,
	int index,  int charfdid);
void saacproto_ACFMDetail_send(int fd, char *result, char *data, int charfdid);
// �г��������԰�
void saacproto_ACFMReadMemo_recv(int fd, int index);
void saacproto_ACFMReadMemo_send(int fd, char *result, int index, int num,
	int dataindex, char *data);
// д��������԰�
void saacproto_ACFMWriteMemo_recv(int fd, char *fmname, int fmindex,
	char *data, int index);
void saacproto_ACFMWriteMemo_send(int fd, char *result, int index);
// ���login
#ifdef _FM_MODIFY
void saacproto_ACFMCharLogin_recv(int fd, char *fmname, int fmindex,
		char *charname, char *charid, int charlv, int eventflag, int charfdid,int gsnum);
#else
	#ifdef _FMVER21
	void saacproto_ACFMCharLogin_recv(int fd, char *fmname, int fmindex,
		char *charname, char *charid, int charlv, int eventflag, int charfdid);
	#else
	void saacproto_ACFMCharLogin_recv(int fd, char *fmname, int fmindex,
		char *charname, char *charid, int charlv, int charfdid);
	#endif
#endif
#ifdef _PERSONAL_FAME   // Arminius: �����������
void saacproto_ACFMCharLogin_send(int fd, char *result, int index, int floor,
	int fmpopular, int joinflag, int fmsetupflag, int flag, int charindex,
	int charfame, int charfdid
#ifdef _NEW_MANOR_LAW
	,int momentum
#endif
	);
#else
void saacproto_ACFMCharLogin_send(int fd, char *result, int index, int floor,
	int fmpopular, int joinflag, int fmsetupflag, int flag, int charindex,
	int charfdid);
#endif

// ���logout
void saacproto_ACFMCharLogout_recv(int fd, char *fmname, int fmindex,
	char *charname, char *charid, int charlv, int index, int charfdid);
void saacproto_ACFMCharLogout_send(int fd, char *result, int charfdid);
// �峤��˳�Ա������塢�޸ļ����Աְλ
#ifdef _FMVER21
void saacproto_ACMemberJoinFM_recv(int fd, char* fmname, int fmindex,
	char* charname, int charindex, int index, int result, int meindex,
	int charfdid);
#else
void saacproto_ACMemberJoinFM_recv(int fd, char* fmname, int fmindex,
	char* charname, int charindex, int index, int result, int charfdid);
#endif
void saacproto_ACMemberJoinFM_send(int fd, char* result, int charfdid);
// �峤��˳�Ա�뿪����
#ifdef _FMVER21
void saacproto_ACMemberLeaveFM_recv(int fd, char* fmname, int fmindex,
	char* charname, int charindex, int index, int meindex, int charfdid);
#else
void saacproto_ACMemberLeaveFM_recv(int fd, char* fmname, int fmindex,
	char* charname, int charindex, int index, int charfdid);
#endif
void saacproto_ACMemberLeaveFM_send(int fd, char* result, int charfdid);
// �г�����ݵ�
void saacproto_ACFMPointList_recv(int fd);
void saacproto_ACFMPointList_send(int fd, char *result, char *data);

#ifdef _ADD_FAMILY_TAX  // WON ADD ����ׯ԰˰��
void saacproto_GS_ACK_TAX_recv(void);
void saacproto_GS_ACK_TAX_send(int fd, int fm_tax, int fm_point);
void saacproto_ACFMSetTAX_recv(int fd, int fm_tax, int index, int fmindex);
void saacproto_ACFMSetTAX_send(int fd, char *result, int fm_tax, int fmpoint, int index);
#endif

#ifdef _CK_ONLINE_PLAYER_COUNT    // WON ADD ������������
void saacproto_GS_ACK_PLAYER_COUNT_recv( int fd,  int playcount );
#endif

// �������ݵ�
void saacproto_ACSetFMPoint_recv(int fd, char *fmname, int fmindex, int index,
	int fmpointindex, int fl, int x, int y, int charfdid);
void saacproto_ACSetFMPoint_send(int fd, char *result, int r, int charfdid);
// �趨����ݵ�
void saacproto_ACFixFMPoint_recv(int fd, char *winfmname, int winfmindex,
	int winindex, char *losefmname, int losefmindex, int loseindex,
	int village);
void saacproto_ACFixFMPoint_send(int fd, char *result, int r);
// �峤����ϵ�����Ա�㲥
// kindflag 1:�峤�㲥 2:���屻ϵͳɾ�� 3:ϵͳ֪ͨѶϢ
void saacproto_ACFMAnnounce_recv(int fd, char *fmname, int fmindex, int index,
	char *data, int color);
void saacproto_ACFMAnnounce_send(int fd, char *result, char *fmname,
	int fmindex, int index, int kindflag, char *data, int color);
// �г��������а�
void saacproto_ACShowTopFMList_recv(int fd, int kindflag);
void saacproto_ACShowTopFMList_send(int fd, char *result, int kindflag, int num, char *data);
// ������������(�Ƿ����ճ�Ա)
// kindflag 1:�Ƿ������ļ�����Ա 2:����У����� 3:�����ػ��� 4:������ּ
//          5:��ɢ����ʱ�� 6:������� 7:����ð������ 8:������������
//          9:����ϳɡ��ӹ����� 10:������������
void saacproto_ACFixFMData_recv(int fd, char *fmname, int fmindex, int index,
        int kindflag, char *data1, char *data2, int charindex, int charfdid);
#ifdef _FAMILYBANKSTONELOG
void saacproto_ACgetFMBankgold_recv(int fd, char *fmname, int fmindex, int index,
        int charindex, int charfdid);

void saacproto_ACgetFMBankgold_send(int fd, int charfdid,int r);

#endif
void saacproto_ACFixFMData_send(int fd, char *result, int kindflag,
	char *data1, char *data2, int charfdid);
// �У�����¼�������
void saacproto_ACFixFMPK_recv(int fd, char *winfmname, int winfmindex,
	int winindex, char *losefmname, int losefmindex, int loseindex);
void saacproto_ACFixFMPK_send(int fd, char *result, int data, int winindex,
	int loseindex);
// ȡ�ü�������
// kindflag 1:��������
void saacproto_ACGetFMData_recv(int fd, char *fmname, int fmindex, int index,
	int kindflag, int charfdid);
void saacproto_ACGetFMData_send(int fd, char *result, int kindflag, int data,
	int charfdid);
// �ǣ�������������
void saacproto_ACGMFixFMData_recv(int fd, int fmindex, char *charid,
	char *cmd, char *data, int charfdid);
void saacproto_ACGMFixFMData_send(int fd, char *result, char *fmname, int charfdid);
// �����ս�ų�
void saacproto_ACFMClearPK_send(int fd, char *result, char *fmname, int fmindex,
	int index);
// CoolFish end

#ifdef _AC_SEND_FM_PK		 // WON ADD ׯ԰��ս�б�����AC
void saacproto_ACLoadFmPk_send(int fd, char *data);
#ifdef _ACFMPK_LIST//ACSendFmPk
void saacproto_ACSendFmPk_send( int fd, int userindex, int flg);
#endif
#endif

#ifdef _RECAL_BACKUP_PLAYER			// WON ���ݹ���
void saacproto_ACRecalPlayer_send(int fd, char *uid, int userfdid, int GmCliId, char *id, int char_num, int date, char *char_data, int backup_flag);
#endif

// Arminius 8.1 manor pk
void saacproto_ACManorPKAck_recv(int fd, char *data);
void saacproto_ACManorPKAck_send(int fd, char *data);
#ifdef _CHECKFMSCHEDULE	 // Syu ADD ͸��ACȷ�ϼ�����ս�ų�
void saacproto_ACCheckSchedule_recv(int fd,char *data);
void saacproto_ACCheckSchedule_send(int fd,char *data);
#endif
#ifdef _AUCTIONEER
void saacproto_ACAuctionSold_recv(int fd, char *data);
void saacproto_ACAuctionSold_send(int fd, char *data);
#endif

void saacproto_ACreLoadFmData_recv(int fd, int type, int data);

#ifdef _AC_SEND_FM_PK		 // WON ADD ׯ԰��ս�б�����AC
void saacproto_ACLoadFmPk_recv(int fd, int fmpks_pos);
#ifdef _ACFMPK_LIST
void saacproto_ACSendFmPk_recv(int fd, int fmpks_pos, int userindex, int flg, char *data);
#else
void saacproto_ACSendFmPk_recv(int fd, int fmpks_pos, char *data);
#endif
#endif

#ifdef _WAEI_KICK
void saacproto_ACKick_recv( int ti , char* id, int lock, int mesgid );
void saacproto_ACKick_send( int fd, int act, char* data, int id );
#endif
#ifdef _PAUCTION_MAN
void saacproto_ACItemAuction_recv( int fd, char *ITEMNAME, char *data, int itemID, int ret, int flg);
void saacproto_ACItemAuction_send( int fd, char *ITEMNAME, char* data, int itemID, int ret, int flg);
#endif
#ifdef _FIX_WORKS
void saacproto_WORKLogin_recv( int fd , char* workname, char* workpass, int work_port );
#endif

#ifdef _NEW_PLAYERGOLD
void saacproto_ACNewPlayerList_recv( int fd, char *CdKey, char *UserName, int UserIndex, int RunType);
void saacproto_ACNewPlayerList_send( int fd, char *CdKey, char* UserName, int UserIndex, int RunType);
void saacproto_LoadNEWPlayer_recv( int ti , int charaindex, char* filename);
void saacproto_LoadNEWPlayer_send( int fd, int charaindex, char* data);
#endif


#ifdef _DEATH_CONTEND
void saacproto_PKLISTGETDATA_recv( int fd,int start, int count);
void saacproto_PKLISTGETDATA_send( int fd, char *ret, int endti, char *data);

void saacproto_PKLISTDELDATA_send( int fd, int ti);

void saacproto_PKLISTUPDATE_recv( int fd, int menum, char *mecdkey, int tonum, char *tocdkey, int winer, int flg);
//void saacproto_PKLISTUPDATE_send( int fd, char *ret, int mesgid, int charindex, char *data);
void saacproto_PKLISTCHARTS_recv( int fd, int type, int flg);
void saacproto_PKLISTCHARTS_send( int fd, char *data, int type, int flg );
#endif

#ifdef _SEND_EFFECT		   // WON ADD ����ѩ���������Ч
void saacproto_SendEffect_send(int fd, char *effect);
#endif

#ifdef _GM_BROADCAST					// WON ADD �ͷ�����ϵͳ
void saacproto_GmBroadcast_send(int fd, int time, int loop, int wait, char *post_msg);
#endif

#ifdef _DEFEND_NEWUSERS
void saacproto_DefendList_recv( int fd, int type, char *data);
void saacproto_DefendList_send( int fd, int type, char *data);

void saacproto_CheckDefendScore_recv( int fd, int userindex, char *cdkey, char *name);
void saacproto_CheckDefendScore_send( int fd, char *fromname, int userindex, int score);
#endif

#ifdef _CHAR_POOLITEM
void saacproto_ACCharInsertPoolItem_recv( int fd, char *cdkey, int userindex, int clifdid, char *Pooldataarg);

void saacproto_ACCharSavePoolItem_recv( int fd, char *cdkey, int userindex, int clifdid, char *Pooldataarg);
void saacproto_ACCharSavePoolItem_send( int fd, char *Acces, char *Pooldataarg, int clifdid );
void saacproto_ACCharGetPoolItem_recv( int fd, char *cdkey, int userindex, int clifdid, int npcid);
void saacproto_ACCharGetPoolItem_send( int fd, char *Acces, char *Pooldataarg, int clifdid, int npcid);
#endif


#ifdef _CHAR_POOLPET
void saacproto_ACCharInsertPoolPet_recv( int fd, char *cdkey, int userindex, int clifdid, char *Pooldataarg);

void saacproto_ACCharSavePoolPet_recv( int fd, char *cdkey, int userindex, int clifdid, char *Pooldataarg);
void saacproto_ACCharSavePoolPet_send( int fd, char *Acces, char *Pooldataarg, int clifdid );
void saacproto_ACCharGetPoolPet_recv( int fd, char *cdkey, int userindex, int clifdid, int npcid);
void saacproto_ACCharGetPoolPet_send( int fd, char *Acces, char *Pooldataarg, int clifdid, int npcid);
#endif



#ifdef _DEATH_FAMILY_LOGIN_CHECK   // WON ADD ����ս������
void saacproto_ACFMCharLogin2_recv( int fd, char *char_id, char *char_name, int charindex );
void saacproto_ACFMCharLogin2_send( int fd, char *result, int charindex, char *data );
#endif


#ifdef _DEATH_FAMILY_GM_COMMAND	// WON ADD ����սGMָ��
void saacproto_ACRELOADFM_send( int fd, int charindex );
void saacproto_ACSHOWMEMBERLIST_2_send( int fd, int charindex, int fm1, char *fm1_name, int fm2, char *fm2_name, int time, int id );
#endif

#ifdef _DEATH_FAMILY_STRUCT		// WON ADD ����ս���ʤ������
void saacproto_ACFMPKSTRUCT_send( int fd, char *data );
#endif

#ifdef _UNIVERSE_CHATROOM
void saacproto_ACUniChatroom_recv( int fd, char *cdkey, char *data, int userindex, int clifdid );
void saacproto_ACUniChatroom_send( int fd, char *cdkey, char *result, char *data, int userindex, int clifdid );
#endif


#ifdef _ANGEL_SUMMON
void saacproto_ACMissionTable_recv( int fd, int num, int type, char *data, char* angelinfo);
void saacproto_ACMissionTable_send( int fd, int num, int type, char *data, char* angelinfo);
#endif

#ifdef _TEACHER_SYSTEM
void saacproto_ACCheckCharacterOnLine_send( int fd, int charaindex, int iOnline,char *data,int flag);
void saacproto_ACCheckCharacterOnLine_recv( int fd, int charaindex, char *id, char *name, int flag);
#endif

#ifdef _RACEMAN
void saacproto_ACRaceRecordandSort_send( int fd, int charaindex , int racetype , char *data );
void saacproto_ACRaceRecordandSort_recv( int fd, int charaindex, char *code , char *id, char *name , int racetype, int catchcnt, int ranknum );
void saacproto_ACRaceRecordfmdo_recv( int fd , int charaindex , int fmid , int bbi , char *unicode , char *petname  );
#endif

#endif                	
/* end of the generated server header code */


