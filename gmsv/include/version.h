#ifndef __VERSION_H__
#define __VERSION_H__
#include "correct_bug.h"  //BUG FIX
#include "version_pk.h"   //PK SERVER

/*Server �汾����*/
#define _NEW_SERVER_
#define _SERVER_NUMS
#define SERVER_VERSION "v2.5"

#define UNIQUE_KEYCODEITEM	100
#define UNIQUE_KEYCODEPET	"i"

//#define _SA_VERSION_70				// (�ɿ���) WON ADD ʯ��ʱ��7.0 �İ汾����
#define _SA_VERSION_25
#define _DEFAULT_PKEY "cary"
#define _RUNNING_KEY  "12345678"

//#define _VERSION_20010911	        // (�ɿ���) 9.11 �����޸� 

// -------------------------------------------------------------------
//�¹��ܣ������ŷ���ר��	��Ա�����
//#define _TEST_PETCREATE             // (���ɿ�) ANDY ����GMָ��
//#define _LOGIN_ADDITEM			// (���ɿ�) ANDY ���������Ʒ
//#define _NEW_TESTSERVER			// (���ɿ�) ANDY ��Ҵ�������ʱ��������ת125�ȼ�����ֻ����
//#define _TEST_PETFUSIONTIME		// (���ɿ�) ANDY ���Գ����ں�  ʳʱ��ӿ�
/* -------------------------------------------------------------------
 * ר��������ׯ԰�����޸�	��Ա��С褡���ѫ��־ΰ��С��
 */

/*
�޸�װ԰������ʽ��
Account Server ��ʽ:
	acfamily.h	MAX_FMPOINTҪ�޸�
Account Server ��Ϸ����:
	saac/family/fmpoint/db_fmpoint������������ׯ԰���������趨
	ׯ԰����ͼ��|ׯ԰���������|ׯ԰���������|ׯ԰�峤ͼ��|ׯ԰�ڽ�֮��ׯ|ռ��ׯ԰֮���� index|ռ��ׯ԰֮���� name|ռ��ׯ԰֮�����ۺ�����ֵ
	���磺
		100|607|563|1041|1|3|shan|3132238
		200|73|588|2031|2|||
Game Server ��ʽ��
	version.h FAMILY_MAXHOME��MANORNUMҪ����
	family.c	 FM_PKFLOOR	������ս��ͼ��
				 FAMILY_RidePet() ��������峤��ͼ��
				 FAMILY_LeaderFunc() ����������뺯id
	npcutil.c addNpcFamilyTax function�������(ׯ԰ͼ��%100)��Ϊׯ԰˰�ղ���
Game Server ��Ϸ����:
	gmsv_v2/data/npc/family/manorsman.arg* id������
	gmsv_v2/data/npc/family/scheduleman.arg* id������
*/

//#define _FAMILY_MANORNUM_CHANGE	// CoolFish �����޸�װ԰����(4.0)
#ifdef _FAMILY_MANORNUM_CHANGE
#define FAMILY_MAXHOME			9	// (4.0) ����ݵ�
#define MANORNUM                9	// (4.0) ׯ԰����
#define FMPOINTNUM				9   // (4.0) �оݵ������������
#endif

#define _FMVER21		        // (�ɿ���) ����ְ�ƹ���  
#define _PERSONAL_FAME	    	// (�ɿ���) Arminius 8.30 �����������  
// -------------------------------------------------------------------
//ר����ʯ����Զ���		��Ա������������
#define _SERVICE     	    	// (�ɿ���) Terry 2001/10/03 ������Զ���
#define _EVERYONE_RIDE	    	// (�ɿ���) Robin 0924 ���ƽ��  
// -------------------------------------------------------------------
//ר��������������������	��Ա����־
// #define __ATTACK_MAGIC_MSG   // (������)
#define __ATTACK_MAGIC          // (�ɿ���)	�ṩս���еĹ���������
#define _ADD_ACTION             // (�ɿ���) һ��NPC����״�����趨

// -------------------------------------------------------------------
//ר����������߱���		��Ա��С�
#define _UNIQUE_P_I	        	// (�ɿ���) CoolFish 2001/10/11 

// -------------------------------------------------------------------
//ר�����°����ﴢ��pn �� ��������˷ 1018	��Ա����ѫ
#define _NEWSAVE                // (�ɿ���) Robin
#define _STORECHAR              // (�ɿ���) Robin
#define _DROPCHECK              // (�ɿ���) Robin ��ֹ�ظ�������
#define _DROPCHECK2             // (�ɿ���) Robin ��ֹ�ظ�������
// -------------------------------------------------------------------
//ר�� : ������	��Ա : ���
#define _PET_TRANS           	// (�ɿ���) ����ת������
#define _NEW_WARPMAN        	// (�ɿ���) NPC����WARP����
#define _PET_TALK	            // (�ɿ���) ����Ի�
#define _PET_LIMITLEVEL	        // (�ɿ���) ��������(����)
#define _MAP_NOEXIT	            // (�ɿ���) �����ͼ����ԭ��&ԭ�ǻ�ָ����
// -------------------------------------------------------------------
// ר����GMָ��㼶Reload		��Ա��С�
#define _GMRELOAD           	// (�ɿ���) CoolFish: 2001/11/12 
// -------------------------------------------------------------------
// ר���������½	��Ա�����������С�
//#define _GAMBLE_ROULETTE	    // (�ɿ���) ��� �ĳ�����	����С��  
//#define _FIX_GAMBLENUM          // (�ɿ���) �������� : ���߿ۻ��� 
//#define _GAMBLE_BANK	        // (�ɿ���) ���  �ĳ�����  
//#define _DROPSTAKENEW	        // (�ɿ���) CoolFish:  �ĳ���ע  
//#define _PETRACE	            // (�ɿ���) CoolFish: 2001/12/3 ���ﾺ�ٳ�  
#define _NEWEVENT               // (�ɿ���) CoolFish: 2001/12/5 �µ��������  
//#define _BIGSMALL             // (���ɿ�) Arminius 12.15 ���ӱȴ�С 
//#define _AUCTIONEER           // (���ɿ�) Arminius 12.20 ������  
// -------------------------------------------------------------------
//ר������������	��Ա������
//#define _ITEM_FIRECRACKER       // (�ɿ���) ս����ʹ�ñ��ڿ��԰���������
// -------------------------------------------------------------------
//�¹��ܣ�ׯ԰PK����(��綨��)		��Ա��־ΰ��־��
#define _MANOR_PKRULE           // (�ɿ���)
#define _MANOR_FIXFMPK          // (�ɿ���) WON ADD ����Լս�� GM ָ������õ�
// -------------------------------------------------------------------
// �¹��ܣ��¹���&���＼��		��Ա�����
//#define _PSKILL_FALLGROUND		// (�ɿ���) ���＼��  (������)
//#define _ENEMY_FALLGROUND		// (�ɿ���) NPC ����
//#define _BATTLE_NEWPOWER		// (�ɿ���)
//#define _Item_MoonAct	        // (�ɿ���) ������ػ�
//#define _Item_DeathAct          // (�ɿ���) ��ħ������
//#define _Item_PetAct            // (�ɿ���) ѱ�޽�ָ
#define _Item_ReLifeAct         // (�ɿ���) ��Ҫitemset1.txt ���ܿ�) ��������
#define _BATTLESTEAL_FIX        // (�ɿ���) ����͵��
#define _CFREE_petskill         // (�ɿ���) ��Ҫpetskill1.txt ���ܿ�) ��ѧϰ���＼�ܼ���
//#define _data_newreadfile       // (�ɿ���) �µ�server setup.cf ���� ���� itemset1.txt petskill1.txt �����µ�
//#define _ITEM_MAXUSERNUM        // (�ɿ���) �� itemset1.txt ITEM ʹ�ô���
// -------------------------------------------------------------------
//�¹��ܣ��������		��Ա�����
#define _PET_TALKPRO      		// (�ɿ���) ����Ի���ǿ	& �� END
//#define _PRO_NPCFIX	     		// (�ɿ���) NPC���ܼ�ǿ
//#define _PRO_BATTLEENEMYSKILL	// (�ɿ���) ս����ǿ	����NPC��AI����
//#define _NEW_WARPPOINT		    // (�ɿ���) ���͵���������ж�WARP
//#define _FIX_CHECKITEM	      	// (�ɿ���) ���߲����Ƿ����
// -------------------------------------------------------------------
//�¹��ܣ������	��Ա��С��,WON
//#define _ADD_ENCOUNT             // (�ɿ���) WON ���ӵ����������޼� 
//#define _WON_PET_MAIL_LOG		 // (�ɿ���) WON ��Ҽĳ��� LOG 
// -------------------------------------------------------------------
// �¹��ܣ���������	4.0	��Ա�����, ־ΰ , ���� , ־��
//#define _LOSE_FINCH_				// (���ɿ�) ANDY ��ȸ����
//#define _FIX_FAMILY_PK_LIMIT	    // (�ɿ���) WON ������սׯ԰����ǰʮ���Ϊǰ��ʮ��
//#define _ADD_POOL_ITEM				// (�ɿ���) WON ���ӿɼķŵĵ���
//#define _ITEM_METAMO                // (�ɿ���) shan metamo sprite. code:robin->shan
//#define _ENEMY_ATTACK_AI            // (�ɿ���) shan enemy attack mode. code:shan  
//#define _ITEM_TIME_LIMIT            // (�ɿ���) shan time limit of item. code:shan
//#define _MIND_ICON                  // (�ɿ���) shan show icon from char mind. code:shan
#define _BATTLENPC_WARP_PLAYER      // (�ɿ���) shan npc warp player when n round. code:shan
//#define _ITEM_WARP_FIX_BI           // (�ɿ���) shan fix bbi to bi. code:shan
//#define _SKILL_DAMAGETOHP			// (�ɿ���) ANDY ��Ѫ�� 
//#define _Skill_MPDAMAGE				// (�ɿ���) ANDY MP�˺�
#define _WAEI_KICK					// (�ɿ���) ANDY ����������
#define _BATTLE_TIMESPEED			// (�ɿ���) ANDY ս��ʱ�� ����(�������ؿ��������ٹ���)
//#define _NEW_RIDEPETS				// (�ɿ���) ANDY �����
//#define _FIXBUG_ATTACKBOW			// (�ɿ���) ANDY �������ʹ��Ͷ������bug
//#define _NPCCHANGE_PLAYERIMG		// (�ɿ���) ANDY NPC�ı����ͼ��, ǿ������,�����ͼ�� (����)
//#define _ITEM_ATTSKILLMAGIC			// (�ɿ���) ANDY ���߼���  
//#define _EQUIT_DEFMAGIC				// (�ɿ���) ANDY ��ħװ��
//#define _FIX_MAGICDAMAGE			// (�ɿ���) ANDY ����ħ����ʽ ���ڿ���
//#define _ITEM_CONSTITUTION			// (�ɿ���) ANDY ������������
//#define _MAGIC_REHPAI				// (�ɿ���) ANDY ��ѪAI
//#define _TRANSER_MAN				// (�ɿ���) ANDY ����ʦ
//#define _TAKE_ITEMDAMAGE			// (�ɿ���) ANDY ������	ITEMSET2
//#define _FIX_FIRECRACKER			// (�ɿ���) ANDY ���� ����
#define _FIX_ITEMPROB				// (�ɿ���) ANDY ���� ������
//#define _ADD_DEAMGEDEFC				// (�ɿ���) ANDY �����˺� & ���ӹ���	ITEMSET2
//#define _FIX_MAXCHARMP				// (�ɿ���) ANDY MP����
//#define _ITEM_INSLAY				// (�ɿ���) ANDY �ⱦʯ ����
#define _ITEMSET2_ITEM				// (�ɿ���) ANDY ITEMSET2
//#define _ITEM_NPCCHANGE				// (�ɿ���) ANDY ���� (����, ����)
//#define _PETSKILL_FIXITEM			// (�ɿ���) ANDY �޸� && ͬ������
#define _FIX_SETWORKINT				// (�ɿ���) ANDY (FIXWORKINT)
//#define _ITEM_ADDEXP				// (�ɿ���) vincent  ����:ָ��ʱ�������Ӿ���ֵ��������
//#define _ITEM_ADDEXP2				// �ǹ�ʱ������ۼ� Robin
//#define _FIX_METAMORIDE				// (�ɿ���) ANDY
//#define _M_SERVER					// (�ɿ���) GM SERVER (����&�������ؿ�)
//#define _FIX_MSERVER				// (�ɿ���) GM SERVER PROC (����&�������ؿ�)
//#define _CHECK_ITEMDAMAGE			// (�ɿ���) ����������	7/26
//#define _SKILL_WILDVIOLENT_ATT		// (�ɿ���) vincent  �輼:�񱩹���
//#define _MAGIC_WEAKEN				// (�ɿ���) vincent  ����:����
//#define _SKILL_WEAKEN				// (�ɿ���) vincent  �輼:����//��Ҫ��#define _MAGIC_WEAKEN
//#define _SKILL_SPEEDY_ATT			// (�ɿ���) vincent  �輼:���ٹ���
//#define _SKILL_GUARDBREAK2			// (�ɿ���) vincent  �輼:�Ƴ�����2
//#define _SKILL_SACRIFICE			// (�ɿ���) vincent  �輼:��Ԯ
//#define _PSKILL_MODIFY				// (�ɿ���) ANDY ����ǿ������
//#define _PSKILL_MDFYATTACK			// (�ɿ���) ANDY ����ת������
//#define _MAGIC_DEFMAGICATT			// (�ɿ���) ANDY ħ������
//#define _MAGIC_SUPERWALL			// (�ɿ���) ANDY ���ڷ���
//#define _OTHER_MAGICSTAUTS			// (�ɿ���) ANDY 
//#define _SKILL_TOOTH				// (�ɿ���) ANDY   ����
//#define _MAGIC_DEEPPOISON			// (�ɿ���) vincent  ����:�綾
//#define _MAGIC_BARRIER				// (�ɿ���) vincent  ����:ħ��
//#define _MAGIC_NOCAST				// (�ɿ���) vincent  ����:��Ĭ
//#define _ITEM_CRACKER               // (�ɿ���) vincent  ����:����//��Ҫ��#define _MIND_ICON
//#define _SKILL_DEEPPOISON			// (�ɿ���) vincent  �輼:�綾//��Ҫ��#define _MAGIC_DEEPPOISON
//#define _SKILL_BARRIER				// (�ɿ���) vincent  �輼:ħ��//��Ҫ��#define _MAGIC_BARRIER
//#define _SKILL_NOCAST				// (�ɿ���) vincent  �輼:��Ĭ//��Ҫ��#define _MAGIC_NOCAST
//#define _SKILL_ROAR					// (�ɿ���) vincent  �輼:���(������)
//#define _SKILL_REFRESH			    // (�ɿ���) vincent  �輼:����쳣״̬
//#define _ITEM_REFRESH               // (�ɿ���) vincent  ����쳣״̬����
//#define _MAGIC_TOCALL				// (�ɿ���) kjl		 �ٻ�	02/06/20 kjl

// -------------------------------------------------------------------
// �¹��ܣ�����������  ��Ա�����
#define _ITEMSET3_ITEM				// (�ɿ���) ANDY itemset3.txt
//#define _SUIT_ITEM					// (�ɿ���) ANDY (��װ) ������ ����itemset3.txt
//#define _PETSKILL_SETDUCK			// (�ɿ���) ANDY ���漼��
//#define _VARY_WOLF                  // (�ɿ���) pet skill : vary wolf. code:shan
// -------------------------------------------------------------------
// �¹��ܣ�ר��5.0		��Ա��
//#define _ITEMTBL_STAIC			// (���ɿ�) ANDY (��̬ITEMTBL)
//#define _USEWARP_FORNUM				// (�ɿ���) ANDY ������ë ������
//#define _IMPRECATE_ITEM				// (�ɿ���) ANDY ������� ������
//#define _MAGICPET_SKILL				// (�ɿ���) ANDY ħ���輼��
//#define _ITEM_CHECKWARES			// (�ɿ���) ANDY �������Ͳ���
//#define _NPC_REPLACEMENT			// (�ɿ���) ANDY �û�ս��npc
//#define _EMENY_CHANCEMAN			// (�ɿ���) ANDY EMENY ѡ��
//#define _TYPE_TOXICATION			// (�ɿ���) ANDY �ж��޷���Ѫ
//#define _PET_FUSION					// (�ɿ���) ANDY �����ں� enemybase1.txt
//#define _NPC_FUSION					// (�ɿ���) ANDY NPC�����ں�
//#define _ITEM_EDITBASES				// (�ɿ���) ANDY Ӫ����
//#define _PET_EVOLUTION				// (�ɿ���) ANDY ������

// -------------------------------------------------------------------
// �¹��ܣ�5.0׷�Ӳ���		��Ա��
#define _AVID_TRADETRYBUG			// (�ɿ���) ANDY ��ֹ�����ж���/���� ��(����)�ʼ� ʯ��
//#define _ASSESS_SYSEFFICACY			// (�ɿ���) ANDY ����ϵͳЧ�� 12/04 ����
//#define _ASSESS_SYSEFFICACY_SUB		// (�ɿ���) Robin ����ϵͳЧ�� ���Ӽ���ڻ�Ȧ
//#define _PREVENT_TEAMATTACK			// (�ɿ���) ANDY ��ֹͬ�ӻ��� 12/04 ����
//#define _ITEM_ORNAMENTS				// (�ɿ���) ANDY װ�ε���	12/04 ���� //���沿
//#define _CHIKULA_STONE				// (�ɿ���) ANDY �����֮ʯ 12/04 ���� //������ ���
//#define _SEND_EFFECT				// (�ɿ���) WON  AC����ѩ���������Ч 12/04 ����
#define _PETMAIL_DEFNUMS			// (�ɿ���) ANDY �����ʼ�����
#define _TEAM_KICKPARTY				// (�ɿ���) ANDY �ӳ�����(��ս��״̬)
//#define _PETSKILL_TIMID				// (�ɿ���) ANDY �輼-��ս
//#define _PETS_SELECTCON				// (�ɿ���) ANDY ���ﲻ�ɵ�ѡ��ս
//#define _CHRISTMAS_REDSOCKS			// (�ɿ���) ANDY ʥ������
#define _FIX_ARRAYBUG				// (�ɿ���) ANDY ����array ��λ
//#define _USER_CHARLOOPS				// (�ɿ���) ANDY �������LOOP
//#define _BATTLE_PROPERTY			// (�ɿ���) ANDY ս����������
//#define _PETSKILL_PROPERTY			// (�ɿ���) ANDY ����ħ�޳輼
//#define _ITEM_FIXALLBASE			// (�ɿ���) ANDY �޸�֮ʯ
//#define _ITEM_LOVERPARTY			// (�ɿ���) ANDY ���˽ڵ���
//#define _ITEM_FORUSERNAMES			// (�ɿ���) ANDY ����������
#define _BATTLECOMMAND_TIME			// (�ɿ���) ANDY ��ֹ��ҿ�����(�غ�ս��ʱ��)
//#define _NPCENEMY_ADDPOWER			// (�ɿ���) ANDY �޸Ĺ��������ֵ
//#define _BAD_PLAYER				// (���ɿ�) WON  �ͻ����ȥ��


// -------------------------------------------------------------------
// �¹��ܣ�ר��6.0		��Ա��
//
//#define _PETSKILL_CANNEDFOOD		// (�ɿ���) ANDY ���＼�ܹ�ͷ	for 6.0
//#define _TEST_DROPITEMS				// (�ɿ���) ANDY ���Զ���
#define _MAP_WARPPOINT				// (�ɿ���) ANDY Map WarpPoint
//#define _AUTO_ADDADDRESS			// (�ɿ���) ANDY �Զ�������Ƭ
//#define _DEFEND_TIMECHECK			// (�ɿ���) ANDY �ڲ�����ʱ��
//#define _TREASURE_BOX				// (�ɿ���) ANDY ����
//#define _BENEMY_EVENTRUN			// (�ɿ���) ANDY ս��npc������
//#define _BATTLE_ABDUCTII			// (�ɿ���) ANDY ��;ⷰ�2
//#define _BATTLE_LIGHTTAKE			// (�ɿ���) ANDY �ɹ���
//#define _BATTLE_ATTCRAZED			// (�ɿ���) ANDY �����
//#define _STATUS_WATERWORD			// (�ɿ���) ANDY ˮ����״̬
//#define _ITEM_WATERWORDSTATUS		// (�ɿ���) ANDY ˮ����״̬����
//#define _KILL_12_STOP_GMSV			// (�ɿ���) WON  ��sigusr2��ر�GMSV
//#define _DELBORNPLACE				// (�ɿ���) Syu  6.0 ͳһ��������ִ�
//#define _REEDIT_PETBBIS				// (�ɿ���) ANDY �������ﲨ��ͼ��
//#define _CAPTURE_FREES				// (�ɿ���) ANDY ��ץ����
//#define _THROWITEM_ITEMS			// (�ɿ���) �������н�Ʒ
//#define _OBJSEND_C					// (�ɿ���) ANDY _OBJSEND_C
//#define _ADD_SHOW_ITEMDAMAGE		// (�ɿ���) WON  ��ʾ��Ʒ�;ö�

//--------------------------------------------------------------------------
//ר�� 7.0	ְҵϵͳ	��Ա����� ־�� ��� ����

//#define _CHANGETRADERULE		    // (�ɿ���) Syu ADD ���׹����޶�
//#define _TRADE_SHOWHP				// (�ɿ���) Syu ADD ������ʾѪ��
//#define _SHOOTCHESTNUT				// (�ɿ���) Syu ADD �輼��������
//#define _SKILLLIMIT					// (�ɿ���) Syu ADD ���ù����ҷ�����
//#define _TRADESYSTEM2				// (�ɿ���) Syu ADD �½���ϵͳ
//#define _NPC_NOSAVEPOINT			// (�ɿ���) ANDY ��¼�㲻����������
#define _ACFMPK_LIST				// (�ɿ���) ANDY �����ս�б�����
#define _PETSKILL2_TXT				// (�ɿ���) ANDY petskill2.txt
//#define _PETSKILL_CHECKTYPE			// (�ɿ���) ANDY ���＼��ʹ��ʱ��
//#define _PETSKILL_TEAR				// (�ɿ���) ANDY �輼 ˺���˿�
//#define _NPC_MAKEPAIR				// (�ɿ���) ANDY NPC ���
//#define _ITEMSET4_TXT				// (�ɿ���) ANDY itemset4.txt
//#define _ITEM_PILENUMS				// (�ɿ���) ANDY �����زĶѵ� �� itemset4
//#define _ITEM_PILEFORTRADE			// (�ɿ���) ANDY ���׶ѵ�
//#define _ITEM_EQUITSPACE			// (�ɿ���) ANDY ��װ����λ
//#define _PET_LOSTPET				// (�ɿ���) ANDY ��ʧ������Ѱ�ƻ�����
//#define _ITEMSET5_TXT				// (�ɿ���) ANDY itemset5.txt
//#define _ITEMSET6_TXT				// (�ɿ���) ANDY itemset6.txt
//#define _EQUIT_ARRANGE				// (�ɿ���) ANDY ���� ��Ҫ itemset5.txt
//#define _EQUIT_SEQUENCE				// (�ɿ���) ANDY ����˳�� ��Ҫ itemset5.txt
//#define _EQUIT_ADDPILE				// (�ɿ���) ANDY ���ӿɶѵ��� ��Ҫ itemset5.txt
//#define _EQUIT_HITRIGHT				// (�ɿ���) ANDY ���� ��Ҫ itemset5.txt
//#define _EQUIT_NEGLECTGUARD			// (�ɿ���) ANDY ����Ŀ�������% ��Ҫ itemset6.txt
//#define _EQUIT_NEWGLOVE				// (�ɿ���) ANDY ������λ
//#define _FM_JOINLIMIT				// (�ɿ���) ANDY ���������������
//#define _FIX_MAX_GOLD				// (�ɿ���) WON ADD ���������Ǯ���� 
//#define _CK_ONLINE_PLAYER_COUNT		// (�ɿ���) WON ������������ (Ҫ���ac��������ؿ�)	
//#define _PET_SKILL_SARS				// (�ɿ���) WON ADD ��ɷ����
//#define _SONIC_ATTACK				// (�ɿ���) WON ADD ��������
//#define _NET_REDUCESEND				// (�ɿ���) ANDY ����DB���ϴ���
//#define _FEV_ADD_NEW_ITEM			// (�ɿ���) Change ���Ӹ����ؾ�
//#define _ALLDOMAN					// (�ɿ���) Syu ADD ���а�NPC
//#define _LOCKHELP_OK				// (�ɿ���) Syu ADD �������ɼ���ս��
//#define _TELLCHANNEL				// (�ɿ���) Syu ADD ����Ƶ��
// �¹��ܣ�ְҵ
//#define _NEWREQUESTPROTOCOL			// (�ɿ���) Syu ADD ����ProtocolҪ��ϸ��
//#define _OUTOFBATTLESKILL			// (�ɿ���) Syu ADD ��ս��ʱ����Protocol
//#define _CHAR_PROFESSION			// (�ɿ���) WON ADD ����ְҵ��λ
//#define _PROFESSION_SKILL			// (�ɿ���) WON ADD ����ְҵ����
//#define _NPC_WELFARE				// (�ɿ���) WON ADD ְҵNPC
//#define _PROFESSION_FIX_LEVEL		// (�ɿ���) WON ADD ����ְҵ����ֵ(����&�������ؿ�)
//#define _GM_SIGUSR2					// (�ɿ���) Change �´�sigusr2��ִ�� gm_sigusr_command�����ڵ�GMָ��,��û��ָ��͹ر�GMSV
#define _GM_BROADCAST				// (�ɿ���) WON ADD �ͷ�����ϵͳ
//#define _NPC_ADDLEVELUP				// (�ɿ���) ANDY NPC������ҵȼ�
#define _CHAR_FIXDATADEF			// (�ɿ���) ANDY �����������ϳ�ʼ
//ϵͳ����
#define _SIMPLIFY_ITEMSTRING		// (�ɿ���) ANDY �򻯵����ִ�
//#define _CHAR_NEWLOGOUT				// (�ɿ���) ANDY �ǳ��»���
//#define _CHATROOMPROTOCOL			// (�ɿ���) Syu ADD ������Ƶ�� 8/27

//#define _CHAR_POOLITEM				// (�ɿ���) ANDY ������ֿ߲⹲��
//#define _NPC_DEPOTITEM				// (�ɿ���) ANDY ������ֿ߲⹲��

#define _SIMPLIFY_PETSTRING			// (�ɿ���) ANDY �򻯳���浵�ִ�
#define _SIMPLIFY_ITEMSTRING2		// (�ɿ���) ANDY �򻯵����ִ�2
//#define _WOLF_TAKE_AXE				// (�ɿ���) WON ADD ץ˫ͷ�ǵ�����
#define _FIX_UNNECESSARY			// (�ɿ���) ANDY ��������Ҫ���ж�
//#define _ITEM_MAGICRECOVERY			// (�ɿ���) ANDY �¹⾵��
//9/17����
//#define _PETSKILL_GYRATE			// (�ɿ���) Change �輼:��������
//#define _PETSKILL_ACUPUNCTURE		// (�ɿ���) Change �輼:�����Ƥ (client�˵�_PETSKILL_ACUPUNCTUREҲҪ��)
//#define _PETSKILL_RETRACE			// (�ɿ���) Change �輼:׷������
//#define _PETSKILL_HECTOR			// (�ɿ���) Change �輼:����
//10/13����
//#define _PETSKILL_FIREKILL			// (�ɿ���) Change �輼:������ɱ
//#define _PETSKILL_DAMAGETOHP		// (�ɿ���) Change �輼:���¿���(��Ѫ���ı���) 
//#define _PETSKILL_BECOMEFOX         // (�ɿ���) Change �輼:�Ļ���
//10/22����
// �¹��ܣ��޸�WORKER(�¸��ѻ���)
#define _FIX_MESSAGE				// (�ɿ���) WON ADD �޸�WORKER������� (����&�������ؿ�)     

//11/12����
//#define _PETSKILL_SHOWMERCY         // (�ɿ�) Change �輼:��������
//#define _NPC_ActionFreeCmp          // (�ɿ�) Change ��NPC���������µıȽϷ�ʽ
//--------------------------------------------------------------------------


//11/26
//#define _NPC_ProfessionTrans      // (�ɿ�) Change ��ְҵNPC�����ж�ת��
//#define _NPC_NewDelPet            // (�ɿ�) Change ��NPC����ɾ������(ԭ��������,ֻ����warpmanʹ��ʱ���ɹ�,��������һ����һ��)
//#define _MUSEUM // ʯ�������  Robin 2003/11
	#ifdef _MUSEUM
	#define _DELBORNPLACE
	#endif
//#define _CHANGEGALAXY	// (�ɿ�) ��ϵ����Robin 2003/10
//#define _ALLDOMAN_DEBUG           // (�ɿ�) Change ��������server�����а���ʧbug
//20031217
//#define _CHRISTMAS_REDSOCKS_NEW	  // (�ɿ�) Change ��ʥ������
//#define _PETSKILL_COMBINED      // (�ɿ�) Change �輼:�ѵú�Ϳ [�ۺ�ħ��(����������ȡ��ħ��)]
//���˽ڻ
//#define _NPC_ADDWARPMAN1			// (�ɿ���) Change npcgen_warpman���ӹ���(�趨����������趨�����Ů����)
//#define _ITEM_QUITPARTY			// (�ɿ���) Change �����ɢʱ�ض����߼�����ʧ
//#define _ITEM_ADDEQUIPEXP       // (�ɿ�) Change װ���ᾭ��ֵ����
//2004/2/18
//#define _ITEM_LVUPUP            // (�ɿ�) Change ͻ�Ƴ���ȼ����Ƶ�ҩ(����ר��)
//2004/02/25
//#define _PETSKILL_BECOMEPIG     // (�ɿ�) Change �輼:������
//#define _ITEM_UNBECOMEPIG       // (�ɿ�) Change �������Ľ�ҩ
//#define _PET_TALKBBI            // (�ɿ�) Change �����ж�bbi
//#define _ITEM_USEMAGIC			// (�ɿ�) Change �ɵ���ʹ��ħ��
//2004/04/07
//#define _ITEM_PROPERTY          // (�ɿ�) Change �ı������Ե���
//#define _SUIT_ADDENDUM          // (�ɿ�) Change ����װ���ӹ��� (_SUIT_ITEMҪ��)
//2004/05/05
//#define _ITEM_CHECKDROPATLOGOUT   // (�ɿ�) Change ���ؼ�¼��ʱ�������޵ǳ�����ʧ����,����,���ɻؼ�¼��
#define _FM_MODIFY				// ���幦���޸�(����)
//#define _CHANNEL_MODIFY		// Ƶ����������(����)
//#define _STANDBYPET		// Robin ��������
//#define	_PROSKILL_OPTIMUM	// Robin ְҵ���ܴ�����ѻ�
#define	_PETSKILL_OPTIMUM	// Robin ���＼�ܴ�����ѻ�
#define _MAGIC_OPTIMUM		// Robin Magic table ��ѻ�
//#define _AC_PIORITY		// Nuke ���AC������Ȩ(�ɿ�)
//2004/05/12
//#define _CHAR_POOLPET				// (�ɿ���) Robin �������ֿ⹲��
//#define _NPC_DEPOTPET				// (�ɿ���) Robin �������ֿ⹲��
//#define _TIMEOUTKICK	// (�ɿ�) ʱ�������ˡ�Robin 2003/10
//#define _MAGIC_RESIST_EQUIT			// (�ɿ�) WON ADD ְҵ����װ��
//#define _LOG_OTHER // ����ѶϢר��LOG

//--------------------------------------------------------------------------
//����Ӫ������


// -------------------------------------------------------------------
//#define _NPC_QUIZSCOPE          // (���ɿ�) Change ��NPC�������  Χ����
//#define _ITEM_ADDCOMBO          // (���ɿ�) Change װ����ϻ�������(ս����Ҫ��2�������ϴ�������)δ���...
//#define _NPC_AIRDELITEM         // (���ɿ�) Change ���������ʱ���۳��ĵ���(���Ƿɻ�Ʊ��)
//#define _NPC_AIRLEVEL           // (���ɿ�) Change ��������յ������ȼ�����(�ȼ����߲�����)

//#define _ANDYLOG_TEST
//#define _NPC_WELFARE_2			// (���ɿ�) WON ADD ְҵNPC-2
//#define _UNIVERSE_CHATROOM			// (���ɿ�) ANDY ȫ��ϵ����
//#define _FIX_ALLDOMAN				// (���ɿ�) ANDY ����Ӣ��ս��
//������
//#define _SEND_PROF_DATA			// (���ɿ�) ANDY ADD ����ְҵ���¸�cli
//#define _AUCPROTOCOL				// (���ɿ�) Syu ADD ����Ƶ������Protocol
//#define _NPC_SELLSTH				// (���ɿ�) ANDY ����ϵͳ
//#define _NPC_SEPARATEDATA			// (���ɿ�) ANDY NPC ���Ϸ���

//------------------------------------------------------------------------
// ����Ϊ���Ṧ��
//------------------------------------------------------------------------
//#define fix_login_event			// (���ɿ�) WON ��������һ�ڵ����� (�������ؿ�)
//#define _FIX_DEL_MAP				// (���ɿ�) WON ��ҳ��ͼ�ͼ��� 
//#define _PETFIGHT					// (���ɿ�) ���ﾺ���� 
//#define _EXM_RANDITEM				// (���ɿ�) NPC (Exchangeman) ����Ҫ����Ʒ (��ȡ��)
//#define add_amulet_map			// (���ɿ�) WON ����̫�������ε�  Χ
//#define _ACFMPK_NOFREE			// (���ɿ�) ANDY ����������
//--------------------------------------------------------------------------
// 6.0׷�ӹ��� pile

//#define _IMPOROVE_ITEMTABLE		// (���ɿ�) ANDY ����ITEM TABLE
//#define _NPC_SHOPALTER01			// (���ɿ�) Change ����ϵͳ�޸�(���Client��Ϊͼ�ν����������޸�)
//#define _GM_IDENTIFY				// (���ɿ�) Change �Ƿ����GMͷ����ʶ���ִ���GMָ��
//#define _GSERVER_RUNTIME          // (���ɿ�) Change ����GSERVERִ�ж���ʱ���Mserver (MserverҪ���)
//#define _NPC_VERYWELFARE			// (���ɿ�) Change �ǳ��ÿ�A��NPC
//--------------------------------------------------------------------------
//#define _ADD_NEWEVENT             // (���ɿ�) WON �����������
//#define _BLACK_MARKET				// (���ɿ�) shan a black market. code:shan 
//#define _DYNAMIC_BUILDER_MAP		// (���ɿ�) shan (test)dynamic builder on the upper map. code:shan
//#define _TEST_ITEMMEMORY			// (���ɿ�) ANDY (����ITEMTABL)
//#define _LASTERR_FUNCTION			// (���ɿ�) ANDY (lastFunction)
//#define _MARKET_TRADE				// (���ɿ�) ANDY �����м�
//#define _PAUCTION_MAN				// (���ɿ�) ANDY 
//#define _SKILL_SELFEXPLODE		// (���ɿ�) vincent  �輼:�Ա�(ȱͼ)
//#define _ADD_FAMILY_TAX			// (���ɿ�) WON ����ׯ԰˰�� (Ҫ���ac��������ؿ�)
//#define _SYUTRY 
/* -------------------------------------------------------------------
 * �¹��ܣ�PK��ϵ 1121 ��Ա����ѫ,С��
 */
//#define _PKPETSHOP				// (���ɿ�)

// -------------------------------------------------------------------
//��������
//#define _CHECK_GAMESPEED			// (���ɿ�) ANDY ���ټ��
//#define _NPCSERVER_NEW			// (���ɿ�) ANDY NPC �ŷ���
//#define _OBJS_TALKFUN				// (���ɿ�) ANDY TALK ��Ѱ��
//#define _NEWOPEN_MAXEXP			// (���ɿ�) ANDY MAX����ֵ
//#define _MERGE_LOG				// (���ɿ�) ANDY �ϳ� LOG
//#define _CHECK_UCODE				// (���ɿ�) ANDY ���

//#define _DEATH_FAMILYCONTEND		// (���ɿ�) ANDY ����ս����

//#define _NEW_INSERVERPOINT		// (���ɿ�) ANDY ��������
//#define _DEATH_CONTENDWATCH		// (���ɿ�) ANDY �����ۿ�
//#define _DEATH_CONTENDTEST		// (���ɿ�) ANDY �������̼ӿ�

//#define _DEATH_FAMILY_GM_COMMAND	// (�ɿ�) WON ADD ����սGMָ��
//#define _DEATH_FAMILY_LOGIN_CHECK	// (�ɿ�) WON ADD ����ս������
//#define _DEATH_FAMILY_STRUCT		// (�ɿ�) WON ADD ����ս���ʤ������
//#define _DEATH_FAMILY_CANT_SCHEDULE // (�ɿ�) WON ADD ����ս��Ҳ�������Լս

//#define _ACTION_BULLSCR			// (���ɿ�) ANDY ��ţ&��ȸ ����� (7/30������ܹر�)
//#define _BULL_FIXPLAYER			// (���ɿ�) ANDY ����������ţ�����
//#define _BULL_CUTBULLSCR			// (���ɿ�) ANDY ��ֹ��ţ�����
//#define _ACTION_GMQUE				// (���ɿ�) ANDY GM����� ������
//#define _PET_FUSIONSHOWTIME		// (���ɿ�) ANDY ���ﵰ��ʱ��

//#define _NEW_PLAYERGOLD			// (�ɿ���) ANDY ���ʺŸ�Ǯ
// -------------------------------------------------------------------
//�����鶯��

//#define _FIX_TSKILLCAN			// (�ɿ���) ANDY ��������T����ͷ �Լ� ���T�ּ�(�ǻ���)
//#define _CHANGEGOATMETAMO			// (�ɿ���) Syu  �����޸�����ͼ

// -------------------------------------------------------------------
// �¹��ܣ�GM��Ƶ��������
//#define _GM_WARP_PLAYER			// (���ɿ�)	WON ������� (����&�������ؿ�)
//#define _RECAL_ASK_PLAYER			// (���ɿ�) WON Ҫ���������� (����&�������ؿ�)
//#define _RECAL_SEND_COUNT			// (���ɿ�) WON ����GS��Ѷ (����&�������ؿ�)
//#define _RECAL_SERVER_OFF			// (���ɿ�) WON MSERVER �ر� GS	(����&�������ؿ�)


#define	_ENEMYTEMP_OPTIMUM	// Robin EnemyBase table ��ѻ�
#define	_ENEMY_OPTIMUM		// Robin Enemy table ��ѻ�
//#define	_CHECK_BATTLETIME	// Robin ��¼ÿһս��ָ��ʱ��(������)
//#define	_CHECK_BATTLE_IO	// Robin ��¼ս���������(������)


//���̰�
//#define _BACK_VERSION	//���̰�
#define _REDHAT_V9 //(�ɿ�) Change ��REDHAT9.0�İ汾�±���ʱ��
//#define _UN_FMPOINT // (�ɿ�) Change �ص��������ݵ�
//#define _UN_FMMEMO //(�ɿ�)Change  �ص�����֮�����԰�
//--------------------------------------------------------------------------
//ר�� 7.5	������л�	��Ա����ѫ ���� С��
//#define _NPC_EXCHANGEMANTRANS   // (�ɿ�) Change ��exchangeman�����ж�ת��
//#define	_ANGEL_SUMMON	// Robin ��ʹ�ٻ�
	#ifdef _ANGEL_SUMMON
	#define _ADD_NEWEVENT
	#endif
// Terry define start -------------------------------------------------------------------
//#define _STREET_VENDOR		// ��̯����(����)
//#define _WATCH_EVENT			// ��ѯ������(����)
//#define _NEW_MANOR_LAW		// ��ׯ԰����(����)
//#define _MAP_TIME					// �����ͼ,�ᵹ��(����)
//#define _PETSKILL_LER			// �׶�����(����)
//#define _TEACHER_SYSTEM		// ��ʦϵͳ(������)
// Terry define end   -------------------------------------------------------------------
//#define _FIX_MAGIC_RESIST				// (�ɿ�) Change ְҵħ������
//#define _ADD_DUNGEON					// (�ɿ�) Change ׷�ӵ���
//#define _PROFESSION_ADDSKILL			// (�ɿ�) Change ׷��ְҵ����
//#define _ITEM_ADDPETEXP				// (�ɿ�) Change ���ӳ��ﾭ��ֵ����(�ɳԴ˵��߳���,ƽ�����ɻ�þ���ֵ)
//#define _TRANS_6						// (�ɿ�) Change ����6ת
//#define _PET_2TRANS					// (�ɿ�) Change ����2ת
//#define _PET_2LIMITLEVEL				// (�ɿ�) Change ��������2ת������
//#define _MAGICSTAUTS_RESIST			// (�ɿ�) Change (����)���Ծ���
//#define _EQUIT_RESIST					// (�ɿ�) Change ��һװ������(�ɿ���һ�����쳣״̬)�п��� �� �� ���� ħ�� ��Ĭ ����
//#define _SUIT_TWFWENDUM				// (�ɿ�) Change ��������װ���� (_SUIT_ITEMҪ��)
//#define _EXCHANGEMAN_REQUEST_DELPET	// (�ɿ�) Change ��exchangeman��TYPE:REQUEST����ɾ������
//#define _75_TEST						// (���ɿ�) 7.5����server
//#define _PK_SERVER //����pk�
//#define _DEATH_CONTEND			// (�ɿ�) ����pk� ANDY ����
//#define _DEATH_CONTENDAB			// (�ɿ�) ����pk� ANDY ��ʽ����
//--------------------------------------------------------------------------

//#define _ESCAPE_RESET			// ����������ʱ�������  Robin 2004/07/22
//#define _DEL_DROP_GOLD			// ɾ�����Ϲ�ʱ��ʯ��	Robin
//#define _ITEM_STONE				// cyg ��ֵ�ʯͷ
//#define _HELP_NEWHAND			// cyg �´���ɫ�������ֵ���)
//#define _DEF_FMFREETIME			// cyg GMָ�� �˳�����ʱ��������Ϊ0
#define _DEF_GETYOU				// cyg GMָ�� ��ѯ�Լ�����  Χ�ڵ�����ʺ�
#define _DEF_NEWSEND			// cyg GMָ�� �������ʱ�ɼӾ���Ҳ�ɲ���
#define _DEF_SUPERSEND			// cyg GMָ�� �ɴ�����ĳ�ʺ�Ϊ���ĸ���3�����������
//#define _SUIT_ADDPART3			// (�ɿ�) Change ��װ���ܵ����� �ر����% ��Ĭ�������% �����% �����% �����%
//#define _PETSKILL_2TIMID		// (�ɿ�) Change ��ս�ڶ���(��ʨŭ��)
//#define CAVALRY_DEBUG			// (��ʱ��) Change ���﹥��������
//#define _SHOW_FUSION			// (�ɿ�) Change �ںϳ�����CLIENT��ֻ��ʾת����,�޸�Ϊ�ںϳ���ʾΪ�ں�(client��ҲҪ��)
//#define _CONTRACT				// (�ɿ�) Robin ��Լ���� 
//#define _FONT_SIZE				// (�ɿ�) Robin ���ʹ�С����
//#define _PROSK99				// (�ɿ�) Change ���ְ���ȼ��Ͱ���
//#define _TIME_TICKET			// Robin ��ʱ�볡 ɭ��׽����
//#define _HALLOWEEN_EFFECT		// ��ʥ����Ч
//#define _ADD_STATUS_2			// ������������״̬�ڶ���
//#define _ADD_reITEM				// (�ɿ�) Change ��������λ�ո�
//#define _ADD_NOITEM_BATTLE		// ����NPC�����������û���߻����ս��
//#define _PETSKILL_BATTLE_MODEL	// ���＼��ս��ģ��
//#define _RACEMAN				// cyg �Գ�����npc
//#define _PETSKILL_ANTINTER		// (�ɿ�) Change �輼:��֮�� �ο�_PETSKILL_2TIMID
//#define _PETSKILL_REGRET		// (�ɿ�) Change �輼:����һ�� �ο�_SONIC_ATTACK
//#define _SUIT_ADDPART4			// (�ɿ�) Change ��װ���ܵ��ĵ� �ж�����% ��ǿ��ʦħ��(����30%) �ֿ���ʦ��ħ��

//#define _CHECK_ENEMY_PET		// (���ɿ�) Change �������Ƿ���в��ɵ�����Ľ�ɫ(enemy.txt��petflg��λΪ0����������������)
//#define _ITEM_TYPETABLE		// (���ɿ�) Change ׷��typetable
//#define _MERGE_NEW_8			// Robin 8.0�� �ºϳɹ���
//#define _RIGHTCLICK			// Robin �����Ҽ�����
//#define _JOBDAILY				// cyg ������־����
//#define	_ALCHEMIST			// Robin ��  ����
//#define _CHECK_ITEM_MODIFY	// ����쳣��ǿ�ĵ���
//#define _PETSKILL_EXPLODE		// (���ɿ�) Change �輼:���ѹ���

//use itemset3.txt database, just for old stoneage version
//can't be enabled together with _ITEM_MAXUSERNUM _TAKE_ITEMDAMAGE _ADD_DEAMGEDEFC _SUIT_ITEM
//_ITEM_INSLAY
#define _READ_ITEM3FORMAT
//any player can pickup others' pets
#define _PICKUP_ATWILL
//set how to add level exp by factor
#define _ADDEXP_ATWILL
#ifdef _ADDEXP_ATWILL
#define _ADDEXP_ATWILL_FACTOR 8
#endif
//set socket NON-BLOCK
#define _SOCKET_NONBLOCK
//nono pets will run away
#define _NONO_RUNAWAY
//account supports mysql
#define _ACNT_LOGIN
//set pet's hp str tgh dex by GM
#define _NEW_PETMAKE
//set pet's ai to 100
#define _SET_PETAI
//do not send map at walk
#define _MAPSENDMODE_OFF
//no forcing to pool item
#define _CHECK_FORCEPOOL
#ifdef _CHECK_FORCEPOOL
#define ITEM_SAMU_STONE_1 18538
#define ITEM_SAMU_STONE_2 18539
#define ITEM_SAMU_STONE_3 18540
#define ITEM_SAMU_STONE_4 18541
#define ITEM_SAMU_STONE_5 18542
#define ITEM_HERO_BLESS   18543
#endif
//change items in battle
#define _CHGITEM_INBATTLE
//dispatch package loop
#define _NETLOOP_PRO
//increase saac dispatch times
//#define _AC_PRO
#ifdef _AC_PRO
#define AC_PRO_TIMES 2
#endif
//client login interval
//#define _LOGIN_LIMIT
#ifdef _LOGIN_LIMIT
#define LOGIN_LIMIT_INTERVAL 2
#endif
//help rookies
#define _ROOKIE_GOLD
#ifdef _ROOKIE_GOLD
#define _ROOKIE_GOLD_AMOUNT 20000
#endif
//accept client new method
#define _NEW_ACPTCLI
//netloop_faster new method
#define _NEW_NETLOOP
//ac priority new method
#define _NEW_ACPRO

#endif
