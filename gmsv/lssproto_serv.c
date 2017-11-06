#include "version.h"
#include "autil.h"
#include "lssproto_serv.h"
#include "common.h"	// for StoneAge
#include "log.h"	// for StoneAge
#include "char.h"
#include <stdio.h>
#include <time.h>
#include <net.h>


void lssproto_SetServerLogFiles( char *r , char *w )
{
	lssproto_strcpysafe( lssproto_writelogfilename , w , sizeof(lssproto_writelogfilename ));
	lssproto_strcpysafe( lssproto_readlogfilename , r , sizeof(lssproto_readlogfilename ));
}
int lssproto_InitServer(	int (*writefunc)(int,char*,int) ,	int worksiz )
{
	int i;
	if( (void*)writefunc == NULL){lssproto.write_func = lssproto_default_write_wrap;} else {lssproto.write_func = writefunc;}
	lssproto_AllocateCommonWork(worksiz);
	lssproto_stringwrapper = (char**)calloc( 1,sizeof(char*) * MAXLSRPCARGS);
	if(lssproto_stringwrapper ==NULL)return -1;
	memset( lssproto_stringwrapper , 0, sizeof(char*)*MAXLSRPCARGS);
	for(i=0;i<MAXLSRPCARGS;i++){
		lssproto_stringwrapper[i] = (char*)calloc( 1, worksiz );
		if( lssproto_stringwrapper[i] == NULL){
			for(i=0;i<MAXLSRPCARGS;i++){free( lssproto_stringwrapper[i]);return -1;}
		}
	}
	lssproto.ret_work = (char*)calloc( 1,sizeof( worksiz ));
	if( lssproto.ret_work == NULL ){ return -1; }
	return 0;
}
void lssproto_CleanupServer( void )
{
	int i;
	free( lssproto.work );
	free( lssproto.arraywork);
	free( lssproto.escapework );
	free( lssproto.val_str);
	free( lssproto.token_list );
	for(i=0;i<MAXLSRPCARGS;i++){free( lssproto_stringwrapper[i]);}
	free( lssproto_stringwrapper );
	free( lssproto.ret_work );
}

#define DME() print("<DME(%d)%d:%d>",fd,__LINE__,func)

int lssproto_ServerDispatchMessage(int fd, char *encoded)
{
	int func,fieldcount;
	char raw[65500];

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	
	util_DecodeMessage(raw,encoded);

	if( !util_SplitMessage(raw,SEPARATOR) ){
		print("\nDME1:package=%s\n",raw);
		DME(); return -1;
	}
	if (!util_GetFunctionFromSlice(&func, &fieldcount)) {
		logHack(fd,HACK_GETFUNCFAIL);
		DME(); return -1;
	}
	
	if (func==LSSPROTO_W_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		char direction[65500];

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_destring(4, direction);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_W_recv(fd, x, y, direction);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_W2_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		char direction[65500];

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_destring(4, direction);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_W2_recv(fd, x, y, direction);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_EV_RECV) {
		int checksum=0, checksumrecv;
		int event;
		int seqno;
		int x;
		int y;
		int dir;

		checksum += util_deint(2, &event);
		checksum += util_deint(3, &seqno);
		checksum += util_deint(4, &x);
		checksum += util_deint(5, &y);
		checksum += util_deint(6, &dir);
		util_deint(7, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_EV_recv(fd, event, seqno, x, y, dir);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_DU_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_DU_recv(fd, x, y);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_EO_RECV) {
		int checksum=0, checksumrecv;
		int dummy;

		checksum += util_deint(2, &dummy);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_EO_recv(fd, dummy);
		util_DiscardMessage();
		return 0;
	}
     
	if (func==LSSPROTO_BU_RECV) {
		int checksum=0, checksumrecv;
		int dummy;

		checksum += util_deint(2, &dummy);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_BU_recv(fd, dummy);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_JB_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_JB_recv(fd, x, y);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_LB_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_LB_recv(fd, x, y);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_B_RECV) {
		int checksum=0, checksumrecv;
		char command[65500];

		checksum += util_destring(2, command);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}		
		lssproto_B_recv(fd, command);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_SKD_RECV) {
		int checksum=0, checksumrecv;
		int dir;
		int index;

		checksum += util_deint(2, &dir);
		checksum += util_deint(3, &index);
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_SKD_recv(fd, dir, index);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_ID_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int haveitemindex;
		int toindex;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &haveitemindex);
		checksum += util_deint(5, &toindex);
		util_deint(6, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_ID_recv(fd, x, y, haveitemindex, toindex);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_PI_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int dir;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &dir);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_PI_recv(fd, x, y, dir);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_DI_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int itemindex;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &itemindex);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_DI_recv(fd, x, y, itemindex);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_DG_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int amount;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &amount);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_DG_recv(fd, x, y, amount);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_DP_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int petindex;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &petindex);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_DP_recv(fd, x, y, petindex);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_MI_RECV) {
		int checksum=0, checksumrecv;
		int fromindex;
		int toindex;

		checksum += util_deint(2, &fromindex);
		checksum += util_deint(3, &toindex);
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_MI_recv(fd, fromindex, toindex);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_MSG_RECV) {
		int checksum=0, checksumrecv;
		int index;
		char message[65500];
		int color;

		checksum += util_deint(2, &index);
		checksum += util_destring(3, message);
		checksum += util_deint(4, &color);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_MSG_recv(fd, index, message, color);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_PMSG_RECV) {
		int checksum=0, checksumrecv;
		int index;
		int petindex;
		int itemindex;
		char message[65500];
		int color;

		checksum += util_deint(2, &index);
		checksum += util_deint(3, &petindex);
		checksum += util_deint(4, &itemindex);
		checksum += util_destring(5, message);
		checksum += util_deint(6, &color);
		util_deint(7, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_PMSG_recv(fd, index, petindex, itemindex, message, color);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_AB_RECV) {
		int checksum=0, checksumrecv;
		util_deint(2, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_AB_recv(fd);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_DAB_RECV) {
		int checksum=0, checksumrecv;
		int index;

		checksum += util_deint(2, &index);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_DAB_recv(fd, index);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_AAB_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_AAB_recv(fd, x, y);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_L_RECV) {
		int checksum=0, checksumrecv;
		int dir;

		checksum += util_deint(2, &dir);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_L_recv(fd, dir);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_TK_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		char message[65500];
		int color;
		int area;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_destring(4, message);
		checksum += util_deint(5, &color);
		checksum += util_deint(6, &area);
		util_deint(7, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_TK_recv(fd, x, y, message, color, area);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_M_RECV) {
		int checksum=0, checksumrecv;
		int fl;
		int x1;
		int y1;
		int x2;
		int y2;

		checksum += util_deint(2, &fl);
		checksum += util_deint(3, &x1);
		checksum += util_deint(4, &y1);
		checksum += util_deint(5, &x2);
		checksum += util_deint(6, &y2);
		util_deint(7, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_M_recv(fd, fl, x1, y1, x2, y2);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_C_RECV) {
		int checksum=0, checksumrecv;
		int index;

		checksum += util_deint(2, &index);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_C_recv(fd, index);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_S_RECV) {
		int checksum=0, checksumrecv;
		char category[65500];

		checksum += util_destring(2, category);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_S_recv(fd, category);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_FS_RECV) {
		int checksum=0, checksumrecv;
		int flg;

		checksum += util_deint(2, &flg);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_FS_recv(fd, flg);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_HL_RECV) {
		int checksum=0, checksumrecv;
		int flg;

		checksum += util_deint(2, &flg);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_HL_recv(fd, flg);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_PR_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int request;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &request);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_PR_recv(fd, x, y, request);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_KS_RECV) {
		int checksum=0, checksumrecv;
		int petarray;
		checksum += util_deint(2, &petarray);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_KS_recv(fd, petarray);
		util_DiscardMessage();
		return 0;
	}

#ifdef _STANDBYPET
	if (func==LSSPROTO_SPET_RECV) {
		int checksum=0, checksumrecv;
		int standbypet;
		checksum += util_deint(2, &standbypet);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_SPET_recv(fd, standbypet);
		util_DiscardMessage();
		return 0;
	}
#endif

#ifdef _RIGHTCLICK
	if (func==LSSPROTO_RCLICK_RECV) {
		int checksum=0, checksumrecv;
		int type;
		char data[1024];
		checksum += util_deint(2, &type);
		checksum += util_destring(3, data);
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_RCLICK_recv(fd, type, data);
		util_DiscardMessage();
		return 0;
	}
#endif

	if (func==LSSPROTO_AC_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int actionno;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &actionno);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_AC_recv(fd, x, y, actionno);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_MU_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int array;
		int toindex;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &array);
		checksum += util_deint(5, &toindex);
		util_deint(6, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_MU_recv(fd, x, y, array, toindex);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_PS_RECV) {
		int checksum=0, checksumrecv;
		int havepetindex;
		int havepetskill;
		int toindex;
		char data[65500];

		checksum += util_deint(2, &havepetindex);
		checksum += util_deint(3, &havepetskill);
		checksum += util_deint(4, &toindex);
		checksum += util_destring(5, data);
		util_deint(6, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_PS_recv(fd, havepetindex, havepetskill, toindex, data);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_ST_RECV) {
		int checksum=0, checksumrecv;
		int titleindex;

		checksum += util_deint(2, &titleindex);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_ST_recv(fd, titleindex);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_DT_RECV) {
		int checksum=0, checksumrecv;
		int titleindex;

		checksum += util_deint(2, &titleindex);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_DT_recv(fd, titleindex);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_FT_RECV) {
		int checksum=0, checksumrecv;
		char data[65500];

		checksum += util_destring(2, data);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_FT_recv(fd, data);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_SKUP_RECV) {
		int checksum=0, checksumrecv;
		int skillid;

		checksum += util_deint(2, &skillid);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_SKUP_recv(fd, skillid);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_KN_RECV) {
		int checksum=0, checksumrecv;
		int havepetindex;
		char data[65500];

		checksum += util_deint(2, &havepetindex);
		checksum += util_destring(3, data);
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_KN_recv(fd, havepetindex, data);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_WN_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int seqno;
		int objindex;
		int select;
		char data[65500];

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &seqno);
		checksum += util_deint(5, &objindex);
		checksum += util_deint(6, &select);
		checksum += util_destring(7, data);
		
		util_deint(8, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_WN_recv(fd, x, y, seqno, objindex, select, data);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_SP_RECV) {
		int checksum=0, checksumrecv;
		int x;
		int y;
		int dir;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &dir);
		util_deint(5, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_SP_recv(fd, x, y, dir);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_CLIENTLOGIN_RECV) {
		int checksum=0, checksumrecv;
		char cdkey[65500];
		char passwd[65500];
		
		strcpy(PersonalKey, _DEFAULT_PKEY);

		checksum += util_destring(2, cdkey);
		checksum += util_destring(3, passwd);
		
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		
		lssproto_ClientLogin_recv(fd, cdkey, passwd);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_CREATENEWCHAR_RECV) {
		int checksum=0, checksumrecv;
		int dataplacenum;
		char charname[65500];
		int imgno;
		int faceimgno;
		int vital;
		int str;
		int tgh;
		int dex;
		int earth;
		int water;
		int fire;
		int wind;
		int hometown;

		checksum += util_deint(2, &dataplacenum);
		checksum += util_destring(3, charname);
		checksum += util_deint(4, &imgno);
		checksum += util_deint(5, &faceimgno);
		checksum += util_deint(6, &vital);
		checksum += util_deint(7, &str);
		checksum += util_deint(8, &tgh);
		checksum += util_deint(9, &dex);
		checksum += util_deint(10, &earth);
		checksum += util_deint(11, &water);
		checksum += util_deint(12, &fire);
		checksum += util_deint(13, &wind);
		checksum += util_deint(14, &hometown);
		util_deint(15, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_CreateNewChar_recv(fd, dataplacenum, charname, imgno, faceimgno, vital, str, tgh, dex, earth, water, fire, wind, hometown);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_CHARDELETE_RECV) {
		int checksum=0, checksumrecv;
		char charname[65500];

		checksum += util_destring(2, charname);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_CharDelete_recv(fd, charname);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_CHARLOGIN_RECV) {
		int checksum=0, checksumrecv;
		char charname[65500];

		checksum += util_destring(2, charname);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_CharLogin_recv(fd, charname);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_CHARLIST_RECV) {
		int checksum=0, checksumrecv;
#ifdef _PKSEVER_VER
		int star = 0;
		util_deint(2, &checksumrecv);
		util_deint(3, &star);
#else
		util_deint(2, &checksumrecv);
#endif
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}

//#ifdef _PKSEVER_VER
//		lssproto_CharList_recv( fd, star);
//#else
		lssproto_CharList_recv( fd);
//#endif

		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_CHARLOGOUT_RECV) {
		int checksum=0, checksumrecv;
		int Flg=1;
#ifdef _CHAR_NEWLOGOUT
		checksum += util_deint(2, &Flg);
		util_deint(3, &checksumrecv);
#else
		util_deint(2, &checksumrecv);
#endif
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_CharLogout_recv(fd, Flg);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_PROCGET_RECV) {
		int checksum=0, checksumrecv;
		
		strcpy( PersonalKey, _DEFAULT_PKEY);

		util_deint(2, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_ProcGet_recv(fd);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_PLAYERNUMGET_RECV) {
		int checksum=0, checksumrecv;
		util_deint(2, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_PlayerNumGet_recv(fd);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_ECHO_RECV) {
		int checksum=0, checksumrecv;
		char test[65500];

		checksum += util_destring(2, test);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_Echo_recv(fd, test);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_SHUTDOWN_RECV) {
		int checksum=0, checksumrecv;
		char passwd[65500];
		int min;

		checksum += util_destring(2, passwd);
		checksum += util_deint(3, &min);
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_Shutdown_recv(fd, passwd, min);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_TD_RECV) {
		int checksum=0, checksumrecv;
		char message[65500];

		checksum += util_destring(2, message);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_TD_recv(fd, message);
		util_DiscardMessage();
		return 0;
	}

	if (func==LSSPROTO_FM_RECV) {
		int checksum=0, checksumrecv;
		char message[65500];

		checksum += util_destring(2, message);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_FM_recv(fd, message);
		util_DiscardMessage();
		return 0;
	}
    
	if (func==LSSPROTO_PETST_RECV) {
		int checksum=0, checksumrecv;
		int nPet;
		int sPet;		

		checksum += util_deint(2, &nPet);
		checksum += util_deint(3, &sPet);		
		util_deint(4, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}		
		lssproto_PETST_recv(fd, nPet, sPet);		
		util_DiscardMessage();
		return 0;
	}
    
	// _BLACK_MARKET
	if (func==LSSPROTO_BM_RECV) {
		int checksum=0, checksumrecv;
		int iindex;		

		checksum += util_deint(2, &iindex);		
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}		
		lssproto_BM_recv(fd, iindex);		
		util_DiscardMessage();
		return 0;
	}

#ifdef _FIX_DEL_MAP           // WON ADD ��ҳ��ͼ�ͼ���
	if (func==LSSPROTO_DM_RECV) {
	    char buffer[2];
	    buffer[0] = '\0';
		lssproto_DM_recv( fd );
		util_DiscardMessage();
		return 0;
	}

#endif

#ifdef _CHECK_GAMESPEED
	if (func==LSSPROTO_CS_RECV) {
	    char buffer[2];
	    buffer[0] = '\0';
		lssproto_CS_recv( fd );
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _TEAM_KICKPARTY
	if ( func == LSSPROTO_KTEAM_RECV ) {
		int checksum = 0, checksumrecv;
		int sindex;
		checksum += util_deint( 2, &sindex);
		util_deint( 3, &checksumrecv);
		if(checksum!=checksumrecv){
			util_DiscardMessage();
			logHack( fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_KTEAM_recv( fd, sindex);
		util_DiscardMessage();
		return 0;
	}
#endif

#ifdef _MIND_ICON
	if(func==LSSPROTO_MA_RECV){
		int checksum = 0, checksumrecv;
		int nMind;
		int x, y;
		
		checksum += util_deint( 2, &nMind);
		checksum += util_deint( 3, &x);
		checksum += util_deint( 4, &y);
		util_deint( 5, &checksumrecv);
		if(checksum!=checksumrecv){
			util_DiscardMessage();
			logHack( fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}		
		lssproto_MA_recv(fd, x, y, nMind);
		util_DiscardMessage();
		return 0;
	}
#endif

#ifdef _CHATROOMPROTOCOL			// (���ɿ�) Syu ADD ������Ƶ��
	if (func==LSSPROTO_CHATROOM_RECV) {
		int checksum=0, checksumrecv;
		char test[65500];

		checksum += util_destring(2, test);
		util_deint(3, &checksumrecv);
		if (checksum!=checksumrecv) {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_CHATROOM_recv(fd, test);
		util_DiscardMessage();
		return 0;
	}
#endif

#ifdef _NEWREQUESTPROTOCOL			// (���ɿ�) Syu ADD ����ProtocolҪ��ϸ��
	if ( func==LSSPROTO_RESIST_RECV ) {
	    char buffer[2];
	    buffer[0] = '\0';
		lssproto_RESIST_recv( fd );
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _OUTOFBATTLESKILL			// (���ɿ�) Syu ADD ��ս��ʱ����Protocol
	if(func==LSSPROTO_BATTLESKILL_RECV){
		int checksum = 0, checksumrecv;
		int iNum;
		
		checksum += util_deint( 2, &iNum);
		util_deint( 3, &checksumrecv);
		if(checksum!=checksumrecv){
			util_DiscardMessage();
			logHack( fd, HACK_CHECKSUMERROR);
			DME(); return -1;
		}		
		lssproto_BATTLESKILL_recv(fd, iNum);
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _STREET_VENDOR
	if(func == LSSPROTO_STREET_VENDOR_RECV){
		int checksum = 0,checksumrecv;
		char message[65500];

		checksum += util_destring(2,message);
		util_deint(3,&checksumrecv);
		if(checksum != checksumrecv){
			util_DiscardMessage();
			logHack(fd,HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_STREET_VENDOR_recv(fd,message);
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _JOBDAILY
	if(func == LSSPROTO_JOBDAILY_RECV){
		int checksum = 0,checksumrecv;
		char buffer[16384];

		buffer[0] = '\0';
		memset(buffer,0,16384); //kkkkkkkkk
		checksum += util_destring(2,buffer);
		util_deint(3,&checksumrecv);
		if(checksum != checksumrecv){
			util_DiscardMessage();
			logHack(fd,HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		print("\njobdaily:%s ",buffer);
		lssproto_JOBDAILY_recv(fd,buffer);
		util_DiscardMessage();
		return 0;

	}
#endif
#ifdef _TEACHER_SYSTEM
	if(func == LSSPROTO_TEACHER_SYSTEM_RECV){
		int checksum = 0,checksumrecv;
		char message[65500];

		checksum += util_destring(2,message);
		util_deint(3,&checksumrecv);
		if(checksum != checksumrecv){
			util_DiscardMessage();
			logHack(fd,HACK_CHECKSUMERROR);
			DME(); return -1;
		}
		lssproto_TEACHER_SYSTEM_recv(fd,message);
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _ADD_STATUS_2
	if(func == LSSPROTO_S2_RECV){
		return 0;
	}
#endif

	util_DiscardMessage();
	logHack(fd,HACK_NOTDISPATCHED);
	DME(); return -1;
}

/*
   servertoclient XYD( int x, int y, int dir );
 	Ǳ���  ��    ����������������  ë  Ʃ�������׻���������
*/
void lssproto_XYD_send(int fd,int x,int y,int dir)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, x);
	checksum += util_mkint(buffer, y);
	checksum += util_mkint(buffer, dir);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_XYD_SEND, buffer);
}

void lssproto_EV_send(int fd,int seqno,int result)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, seqno);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_EV_SEND, buffer);
}

void lssproto_EN_send(int fd,int result,int field)
{
	char buffer[65500];
	int checksum=0;

	//print(" EN_send ");

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, result);
	checksum += util_mkint(buffer, field);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_EN_SEND, buffer);
}


void lssproto_RS_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_RS_SEND, buffer);
}


void lssproto_RD_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_RD_SEND, buffer);
}

void lssproto_B_send(int fd,char* command)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, command);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_B_SEND, buffer);
}

void lssproto_I_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_I_SEND, buffer);
}

void lssproto_SI_send(int fd,int fromindex,int toindex)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, fromindex);
	checksum += util_mkint(buffer, toindex);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_SI_SEND, buffer);
}

void lssproto_MSG_send(int fd,int aindex,char* text,int color)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, aindex);
	checksum += util_mkstring(buffer, text);
	checksum += util_mkint(buffer, color);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_MSG_SEND, buffer);
}


void lssproto_PME_send(int fd,int objindex,int graphicsno,int x,int y,int dir,int flg,int no,char* cdata)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, objindex);
	checksum += util_mkint(buffer, graphicsno);
	checksum += util_mkint(buffer, x);
	checksum += util_mkint(buffer, y);
	checksum += util_mkint(buffer, dir);
	checksum += util_mkint(buffer, flg);
	checksum += util_mkint(buffer, no);
	checksum += util_mkstring(buffer, cdata);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PME_SEND, buffer);
}


void lssproto_AB_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_AB_SEND, buffer);
}


void lssproto_ABI_send(int fd,int num,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, num);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_ABI_SEND, buffer);
}

void lssproto_TK_send(int fd,int index,char* message,int color)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, index);
	checksum += util_mkstring(buffer, message);
	checksum += util_mkint(buffer, color);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_TK_SEND, buffer);
}

void lssproto_MC_send(int fd,int fl,int x1,int y1,int x2,int y2,int tilesum,int objsum,int eventsum,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, fl);
	checksum += util_mkint(buffer, x1);
	checksum += util_mkint(buffer, y1);
	checksum += util_mkint(buffer, x2);
	checksum += util_mkint(buffer, y2);
	checksum += util_mkint(buffer, tilesum);
	checksum += util_mkint(buffer, objsum);
	checksum += util_mkint(buffer, eventsum);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_MC_SEND, buffer);
}


void lssproto_M_send(int fd,int fl,int x1,int y1,int x2,int y2,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, fl);
	checksum += util_mkint(buffer, x1);
	checksum += util_mkint(buffer, y1);
	checksum += util_mkint(buffer, x2);
	checksum += util_mkint(buffer, y2);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_M_SEND, buffer);
}

/*
   servertoclient C( string data );
       (Characters)
       �ͷ���ʧ������ CA ��  ��������ƽ�ҷ±年�л�Ʃئ�������ݷ��� C 
       ƥ������ëۢ��ƥ���£��ӡ��ﷴ����������ئ�У�
       
 	string data
 		  ���ɡ�Ѩ������ ��Ʊ��3��  ����  ë���Ѩƥئ������
 	    �ּ�ƥؤ��.��ľ��ľ����  ��  �췴���ձ��׻���'|'ƥ�ʿ���ľ
 	    �����£�
 	    
 	      �׻���ƥ�ʿ���ľ�������ͼ���12�ۼ�����
 		  �����ͼ���    ��
 
 		  WHICHTYPE|CHARINDEX|X|Y|DIR|BASEIMG|LEVEL|NAMECOLOR|NAME|SELFTITLE|WALKABLE|HEIGHT|POPUPNAMECOLOR
 		  ����Ƥ����������ƽ�ҷ�������ƥؤ�����е�����ƥؤ�£�
 		  
 		  WHICHTYPE ���ݳ�ƽ�ҷ���������������������  ���ּ�
 		  ƥؤ�¾��۳�ľ���ӡ����ƥ����Ʊ��������������ľ�����£�
 		  
 		typedef enum
 		{
 		    CHAR_TYPENONE,          ��ƥ��ئ��
 		    CHAR_TYPEPLAYER,        ����������
 		    CHAR_TYPEENEMY,         ��
 		    CHAR_TYPEPET,           ʸ����
 		    CHAR_TYPEDOOR,          ��ʧ
 		    CHAR_TYPEBOX ,              
 		    CHAR_TYPEMSG ,          ��  
 		    CHAR_TYPEWARP ,         ������������
 		    CHAR_TYPESHOP ,         ��
 		    CHAR_TYPEHEALER ,       �ס��¡�
 		    CHAR_TYPEOLDMAN ,       Ӯ  
 		    CHAR_TYPEROOMADMIN,     ��  ǲ��
 		    CHAR_TYPETOWNPEOPLE,    ���м�����
 		    CHAR_TYPEDENGON,        ����  
 		    CHAR_TYPEADM,           ����  
 		    CHAR_TYPETEMPLE,        Temple master
 		    CHAR_TYPESTORYTELLER,   ��Ի  
 		    CHAR_TYPERANKING,       ��  ǲ�¼�ƽ����  ��      
 		    CHAR_TYPEOTHERNPC,      ����ְ�������ޱ�ئ��ئ��NPC
 		    CHAR_TYPEPRINTPASSMAN,  ��ʧ���ɵ�������  ������NPC
 		    CHAR_TYPENPCENEMY,      ���ó�
 		    CHAR_TYPEACTION,        ʧ���������  ɱ����NPC
 		    CHAR_TYPEWINDOWMAN,     ��ū������  ������NPC    ��������)
 		    CHAR_TYPESAVEPOINT,     ����Ƥ���̼���
 		    CHAR_TYPEWINDOWHEALER,  ���̼����������󼰼ס��¡�
 		    CHAR_TYPEITEMSHOP,	    ����
 		    CHAR_TYPESTONESHOP,	    ��  ��  ʸ������  ��  
 		    CHAR_TYPEDUELRANKING,   DUEL�¼�ƽ����NPC
 		    CHAR_TYPEWARPMAN,	    ������Ѩ��NPC
 		    CHAR_TYPEEVENT,	    ��ì����NPC
 		    CHAR_TYPEMIC,	    ��ì����NPC
 		    CHAR_TYPELUCKYMAN,	    ��ì����NPC
 		    CHAR_TYPEBUS,	    Ѩ��ƹ���ﵩ
 		    CHAR_TYPECHARM,	    ��ì����NPC
 		    CHAR_TYPENUM,
 		}CHAR_TYPE;
 		  
 		  ��ľ��ְ������������֧NPC֧��ƥؤ�£��ӡ������˪��
 		  �������巴έ����  ���年�л���  ˪�����£۹���
 		  Ի�ͷ���ʧ�����������ɸ�����ë����������������
 		  �ɸ�������̤��ľ����ئ��ƽ�ҷ�ë  �Ȼ����������ƻ���
 		  ���Ȼ��������е�����ƥؤ�£����ף�Ѩ�����С�ĩ��ëؤ
 		  �����������  ��������  ������  ���ı����л����£�
 		  �־��գ�Ѩ�����С�ĩ��ëؤ�����������  �����³��绥
 		    �ʽ�ľ�����������ӡ��������������ë    ������̫
 		  �ֽ�ئ��ľ��ئ��ئ�У� SELFTITLE�年�л�������������
 		  ƥؤ���׻���ë������ئ�з�����޵��������ľ�����£�
 		  �ͷ���ʧ������ʧ�����������Ѩ����(CA)��������ƥ��  
 		  ��ƥ  �����£�CHARINDEX�ӡ����  ��������ƽ�ҷ�ë  
 		  ��ƥ����  į��BASEIMG��  �����׻���  į��LEVEL��ƽ��
 		  �¼���ì��(0ئ��  ����ئ�У۳𼰰���NPCئ����������)
 		  WALKABLE��1�����幫����ëɧ�����³��绥ƥ���0ئ��ɧ
 		  �����³��绥ƥ��ئ�У�HEIGHT���ν�ë�ֹ��ּ�������ƥ
 		  ئ�м�����϶�ã�
 		  
 		  ƽ�ҷ���������  ����  ��į����<a href="#escaping">
 		  �޵��������ľئ��ľ��ئ��ئ�У�'|'ƥ�����ͼ�ë
 		  ��Ի���ƻ����վ޵�������ëݩ�����£۾޵�������������'
 		  |'���þ���  ٯ�����微���¼�ƥ��  �ٷ�ǩ�˱�'|'ë��
 		  ���������ƻ����У�˪������  �֣�  ����  ��įë�޵�
                   �������ƻ������׻���ƥ��ئ��ƥ����˪�����£�
                   ���ף�ʸ��������������  ��į��ҽ��Ի�彻���ǡ���ɬ��
                   ����ʸ������  󡻥˪���ľ�����£�
 		  
 	      �׻���ƥ�ʿ���ľ�������ͼ���6�ۼ�����
 		  �����ͼ���    ��
 		  INDEX|X|Y|BASEIMG|LEVEL|ITEM1LINEINFO
 		  ƥ��  ��  �л�����ʧ��  ة�年�л�����  ƥؤ�£�
 		  INDEX��ƽ�ҷ¼��̼����������羮����ئ��INDEXƥؤ�£�
 		  ����ʧ��  ةë�����ݱ��������£�X,Y��ʧ��  ة������
 		  �����  ��BASEIMG��  쫼�  į��ITEM1LINEINFO��1��
 		  info��  �������׻�����  ƥؤ�£�ʧ��  ة���̼�����  
 		  ��  ���ü���  ��  ��  ܷƥ�������£�ʧ��  ة�����ƻ�
 		  ��CA��  ئ�У�ITEM1LINEINFO��<a href="#escape">�޵�����
 		  ���ľ�£۳𼰾޵�������  ܷ��������  ë��徣�
 
 	      �׻���ƥ�ʿ���ľ�������ͼ���4�ۼ�����
 		  �����ͼ���    ��
 		  INDEX|X|Y|VALUE
 		  ƥ��  ��  �л��������ű年�л�����  ƥؤ�£�    ����
 		  �ͻ���ٯ��INDEX,X,Y ��ʧ��  ة��  Ԫ��VALUE ����ľ��
 		  �꼰�ھ����е���ƥؤ�£�ʧ��  ة�年�л�����  ƥؤ�£�
 	      �׻���ƥ�ʿ���ľ�������ͼ���1�ۼ�����
 		  INDEX
 		  ��ƽ�ҷ¼�C��������ľئ�У�
*/
void lssproto_C_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_C_SEND, buffer);
}

/*
   servertoclient CA( string data );
       (CharacterAction)
       έ����  ��������ƽ�ҷ¼�ʧ�����������ë��ޥ���£�
       �ӡ���������ͷ���ʧ��������  ����˪�����£�
       ��ƽ�ҷ¼�1ʧ������������˪����ӡ������ʧ�������ëƥ���·�
       ���������³����
       
 	string data
 	CHARINDEX|X|Y|ACTION|PARAM1|PARAM2|PARAM3|PARAM4|....ë��
 	    ��Ѩƥ�ʿ������ּ������£�PARAM��ʧ������������������
 	    ���о����ֳ���ئ�£۶�Ʊ��ʧ���������  ��X,Y����  ƥ��
 	    ��ʧ�������ƥ�֣�ؤ����������  ë϶�����£�
 	    ��  ٯ  ���޵��������ľئ�У�
 	  
 	        ACTION  PARAM1  PARAM2 
 	       PARAM3  PARAM4  
 	        Stand:0    ��0~7        
 	        Walk:1    ��0~7        
 	        Attack:2    ��0~7        
 	        Throw:3    ��0~7        
 	        Damage:4    ��0~7      
 	        Dead:5    ��0~7        
 	        UseMagic:6    ��0~7  
 	        UseItem:7    ��0~7  
 	        Effect:8    ��0~7  �ް�������  į  
 	        Down:10 (��ľ��)    ��0~7  
 	        Sit:11 (����)    ��0~7  
 	        Hand:12 (��ë����)    ��0~7  
 	        Pleasure:13 (����)    ��0~7  
 	        Angry:14 (����)    ��0~7  
 	        Sad:15 (  �Ƹ�)    ��0~7  
 	        Guard:16 (������)    ��0~7  
 	        actionwalk:17 (ʧ�������������)    ��0~7  
 	        nod:18 (��ئ����)    ��0~7  
 	        actionstand:19 (ʧ���������  �к̡���)    ��0~7  
 	        Battle:20 (��  ��  )    ��0~7  BattleNo(-1 ئ��  ������    SideNo  HelpNo  ��ئ�վ���ë����CA  ���ݨ�ئ�����ʣ����׷�  ��    
 	        Leader:21 (����ĸ����  )    ��0~7  0:  ������ 1:  ��  
 	        Watch:22 (��  ���)    ��0~7  0:  ������ 1:  ��  
 	        namecolor:23(  󡼰����  )    ��0~7    󡼰��  į  
 	        Turn:30(  ��  ��)    ��0~7    
 	        Warp:31(������)    ��0~7    
 	      
 	ACTION����������ƥ��    ������  ����������  ƥؤ�£�
*/
void lssproto_CA_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CA_SEND, buffer);
}

/*
   servertoclient CD( string data );
       (CharacterDelete)
 	data ����������  ,  ƥ����ľ���̼������͵���
 	  ����˪���ǻ���  �£�
       ��IDë  �Ȼ�����ƽ�ҷ��������������ݱ��ӡ��ﾮ���ͷ���ʧ������
       �������£�
*/
void lssproto_CD_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CD_SEND, buffer);
}

/*
   <a name="R">servertoclient R( string data );
       (Radar)
       ����ĸ����    ë�ͷ���ʧ������˪�����£��ͷ���ʧ��������˪��ë
       ۢ�����³��練ئ�У��ӡ���������ئ����������ƥ˪�����£�
       ��������10��ؤ���������羮��1�������羮��
 
 	string data
 	x ,y, kind ,���ͱ��ë'|'ƥ�ʿ��Ȼ�ئ�������ּ�ë
 	    ���ձ�'|'ƥئ�������ּ���
 	    x,y��ƽ�ҷ¾��ռ�姸���  ��kind����Ʊ���ƻ����Ѱ�ƥ��ئ��
 	    ��į������ĸ���嵤����  ����  ë϶�����£�˪��ľ�������ּ�
 	    ����  ������ľ��ƥ��������  ܷ�羮����ƽ��ƥ  ���ƣ���ľ��
 	    �ӡ�����������£�
 	 
 	        kind����        
 	        E  ��  
 	        P  ����������  
 	        S  ��  
 	        G  ����  
 	        I  ʧ��  ة(�۶�½)  
   
   data��  ��"12|22|E|13|24|P|14|28|P"
   ��  ٯ  ���޵��������ľئ�У�
*/
void lssproto_R_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_R_SEND, buffer);
}

/*
   servertoclient S( string data );
       (Status)
       ƽ�ҷ¼���  ������ë˪�����£�
       �������� ��  ������į  ٯ(��  ٯ)     ��ئ�Ȼ����£۹���Ի  �ټ�
       1  ٯëέľ���ϼ���  �����������£�    ��2  ٯ  ����ƥؤ�£�
           ����Ʊ�����ɡ�Ѩ���������׻������׻��߷�į'|'����������ƥؤ�£�
       2��  ���ϼ������ͼ���    ƥؤ�£�
       
 	P ���ɷ¶�����
 	    kubun hp maxhp mp maxmp str tough exp maxexp exp 
 	    level attackpower deffencepower 
 	    fixdex fixcharm fixluck fixfireat fixwaterat fixearthat fixwindat
 	    gold ���껯����
 	    ��į��index   � ���ñ�į 
 
 	    ���������� '|' ƥؤ�£�����  �����ñ�į����
 	    <a href="#escaping">�޵������������ּ�ë
 	    �׻���ƥ��ئ�����ּ���ɧ���ľ�£�
 	    ��ľ��ľ�������ᷴ��Ʊ��
 	    kubun ��໥  �Ȼ���������ɷ¶�����ë˪���
 	    �ᶯ������ݱ���������  �Ȼ������ּ����ɷ¶���������  ��  �Ȼ�˪���ľ�����£�
 	     ���ɣݨ⻥  ����hp��Maxhp��kubun ���ϼ������ͼ���  ����ľ�����£�
 	    
 	        kunun  int  
 	        Hp(    )hp  int  
 	        MaxHp  int  
 	        Mp(�����ɷ���)  int  
 	        MaxMp  int  
 	        Vital(    )  int  
 	        Str(    )  int  
 	        Tough(��ᨽ�)  int  
 	        Dex(�ٶƽ�)  int  
 	        Exp(������)exp  int  
 	        MaxExp(����ƥ��������)  int  
 	        Level(��ì��)  int  
 	        Attack(  ��  )  int  
      	        Defense(��    )  int  
 	        fQuick(  ���ٶƽ�)  int  
 	        fCharm(    )  int  
 	        fLuck(��)  int  
 	        fEarth(��)fixearthat  int  
 	        fWater(  )  int  
 	        fFire(��)  int  
 	        fWid(�)fixwindat  int  
 	        Gold(����)  int  
 	        ��į��index  int  
 	        ��ب�޻���̼���  int  
 	        ��Ϸ����  int  
 	          �    ٯ    
 	        ���ñ�į    ٯ    
 	    
 	    ��    ( char ��    ��������̤�� )
 	    
 	    P10|20|10|20|10|10|10|1|2|13|13|1|100|10|������|abc|def
 	
 	C ��  
 	    floor maxx maxy x y
 	    ë  | ƥ���Ȼ�˪�£�
 	    ��    ( char ��    ��������̤�� )
 	    
 	    C1024|100|100|10|20
 	    
 	    ������  ƥ�����׷�ʧID1024, ������100x100 X 10,Y 20�����
 	    Ƿ��ƽ�ҷ����������������е�    ��ئ�£۳�Ѩ��ƽ�ҷ¼���
 	    ��ʧ�ֿ��ʻ�����  ë�ͷ���ʧ���������³��绥ƥ���¼�����
 	    ��Ѩ������ƥؤ�£����弰�����׷�ʧ  ����  �ƾ�������ئ
 	    �У۷��Ȼ����ء�ة����ݣ�������ü����������������ئ��
 	    ��������Ѩ����ë�����Ȼ������а�ë˪����ئ�ʻ���ئ��ئ�У�
 	    
 	I ��ʧ��  ة
 	    (ʧ��  ة1)|(ʧ��  ة2)...(ʧ��  ةn)
 	    ʧ��  ة��    ����Ʊ��  ܷƥ���������£�
 	    ���繴��ʧ��  ة������ئ���ڶ�Ʊ��6���������ͼ���������ƥ
 	    ˪���ľ�£۷��繴���繴��    ����
 	    
 	      �|  �2|��  ����|ʧ��  ة��    |ʧ�߶�  į|���õ�  ����|���ø���|ʧ��  ة��ì��|�׷º�  
 	    
 	      ʧ��  ة  ��ġ  ��ì��ƥ��  ����  �ʽ�ľ�£�
 	        �2��ġ  ��ì��巽�Ȼ���ʧ��  ة��    ��  ��  �ã�
 		  �ͷ���ʧ������ʧ��  ة  ��2��  ��  ľ���ǣ�
 	      ��  ���ⷴ  󡼰��ë׸�¼���������
 	      ʧ��  ة��    ��
 	      ʧ�߶�  į����  �  į��
 	      ���õ�  �����練�ݳ�ʧ��  ة������ƥ���������ǻ���  ����  �Ȼ����£�
 	          ��ľ�����ӡ����ƥ����Ʊ��������������ľ�����£�
 	          
 			typedef enum
 			{
 				ITEM_FIELD_ALL,			���ͻ�������ƥ������
 				ITEM_FIELD_BATTLE,		��    ����
 				ITEM_FIELD_MAP,			ɧ��Ѩ����������
 
 			}ITEM_FIELDTYPE;
 	          
 	      ���ø����練�ݳ�ʧ��  ةë�������³��缰��  �¸��޻�  �Ȼ����£�
 	          �ӡ����ƥ����Ʊ��������������ľ�����£�
 	          
 			typedef enum
 			{
 				ITEM_TARGET_MYSELF,		���м���
 				ITEM_TARGET_OTHER,		ְ����  ����ֳ��)
 				ITEM_TARGET_ALLMYSIDE,		    ��  
 				ITEM_TARGET_ALLOTHERSIDE,	���촡��  
 				ITEM_TARGET_ALL,		�廯
 			}ITEM_TARGETTYPE;
 		
   	        <font size=+1>����ٯ��100ë�����������ƥ���·��ָ�����ئ�£�
 	      ʧ��  ة��ì��۳���ì�ﶯ������ƥئ������  ��  ئ�У�
 	      �׷º�  ������ئ�׷º�ë˪�����£۱�����������ɱ����Ʊ��ɧԻ��

 		      0bit    ʸ����������ƥ˪���  ئʧ��  ة����������˪���  �����ۨ߷���˪��ƥ��ئ�У�  
 		      1Bit    ��ʧ��  ة��������  �¾���������  
 		      2Bit    ��ʧ��  ة��    ����������1����    ƥؤ�£�  
 	    
 	    ���  ƥ��<a href="#escaping">�޵����������£�
 	    ��    ( char ��    ��������̤�� )
 	    
 	    Iؤ�� ئ ��|str+2|1|ؤ��\|��Ԫ|10|2|0
 	    
 	    ��  ���⼰������  �������Ѱ�ƥ
 	    
 	        1  ��  ����A  
 	        2  ��  ����B  	      
 	        3  ��  ����C  
 	        4  ��  ����D  
 	    
 	    �����£����ף�ʧ��  ة  ���뼰  �б年�л�����5�ۼ������ͼ���
 	    ��ƥ������Ի�׻��߻�ئ���Ϸ�����ƥ˪���ľ�����£�
 	
 	S �嵩ƽ��
 	    (��ƽ��0)|(��ƽ��1)|(��ƽ��2) ... (��ƽ��n)
 	    ��į��  Ԫƥ��ƽ�ҷ»��ֻ���    ��ƽ������ë˪�����£۹���
 	    Իئ���ּ��� ||| ��ئ�¼�ƥؤ�£�
 	    ���޼�������    ����
 	    ��ƽ�Ｐ��  �����|��ì��
 	    ƥؤ�£�
 	    ��    ( char ��    ��������̤�� )
 	    
 	    S10|2|20|2|||
 	
 	T ��į
 	    (��į0)|(��į1)|(��į2) ... (��įn)
 	    ��į���޼��������뼰������ '|' ��  �����¼�ƥ��  ��������
 	    ��ë    �ƻ���ƽ�Ҽ��ƻ���ئ��ئ�У۾�ئ����    ������˪��
 	    ���£�
 	    ���繴�����繴��    ����
 	      �
 	    ����ƥؤ�£�
 	    ��    ( ˪��ľ������ char ��    ��������ƥؤ�� )
 	    
 	    Tؤ��|����������
 	    
 	    ��ľ��<a href="#escaping">�޵����������£�
 
 	M   ���ɷ¶�����
 	    ƽ�ҷ�����������  �����������У�    ��  �ʽ�ľ���ּ�(�ػ�
 	    ��)�繫��ƥئ���ּ�(    ��ئ��)��ؤ�£���ľ������  ������
 	    ��  �ʽ�ľ����̫��  ����  ���ɷ¶�����ë˪�����¼���������
 	    ���������°�ū����ë  �ͱ����������ئ�¼�ƥ��  ��HP ,
 	    MP,EXP�����������إ�����ɷ¶�����˪����Ѩ����ë�������£�
 	    ��ľ����M��Ѩ����ƥؤ�£�2  ٯ  ���ϼ�  �������ͼ���HP��
 	    �ػ��� ,     �����ͼ���MP��  �������ͼ���EXPƥؤ�£�
 	    ��Ʊ����    ë�ƻ��ʣ�
 	    
 	    M54|210|8944909
 	    
 	    ��  ƥ��HP���ػ�����54��MP���ػ�����210����������8944909
 	    ��ئ�Ȼ����¼��֣۳�3��  ��  ��  ��  պ��������  ��ľ��
 	    �׻�    ������Ѩ��������׻�������Ƿ�ϳ�M��Ѩ���񼰾���
 	    Ի��P��Ѩ����ƥ���ɷ¶�����ë˪���ƻ��ַ��У��׷��ƣ�  ��
 	    ��������������ë�����׻��巴������Ѩ����ë�������绥��ٱ
 	    ��ľ�£�
 	    
         D ƽ�ҷ¼�ID
 	    ����̼��ݱ����˪��ƽ�ҷ¼�index�ݹ�ľ������ë˪�£�
 	    D1000|912766409�羮ƥ˪��ľ�£�
 	
 	E �޼�����������    Ʊ��)|(����)
 	    �޼���������  ë˪�����£۸�  ��n/100
 	    �ͷ���ʧ��������Ʊ�׼������յ��������ƻ��ݨ�������  ��+1���׸�  ƥ�޼������������¾�ë  Ӭ���£�
 	    �׷��ƣ����׼���ë����ئ�з��������£�
 	    �ݼԱ��ľ��˪��ľ��������ƥ�𼰸�  ƥ�޼���������ë��ң�����ǣ�
 	
 	J0   J6 ���������    
 	    ���������    ë˪�����£�
 	    0   6 ����ľ��ľʧ��  ة����  �����帲ɱ�ƻ����£���  ����  ����    ��  ���������    
 	    �ƾ���  ��  ئ��  �÷ֻ�����  �������б��廯����  ��������  ë˪��
 	    J0|kubun|mp|field|target|name|comment
 	    ���е����ɡ�Ѩ������ئ�Ȼ����£�
 	    kubun ������  ��ؤ�¾�  �о��ۨ߷���  �У۹�ľ���ϱ������ͼ�����ئ�Уۨ����ؤ�£�
 	    mp����  ��  ë  �ʣ�
 	    field����������ƥ�����¾����ӡ����ƥ����Ʊ��  ��������ľ�����£�
 	    
 		typedef enum
 		{
 			MAGIC_FIELD_ALL,		���ͻ�������ƥ������
 			MAGIC_FIELD_BATTLE,		��    ����
 			MAGIC_FIELD_MAP,		ɧ��Ѩ����������
 			
 		}MAGIC_FIELDTYPE;
 	    
 	    target����ľë���ޱ���  �¾����ӡ����ƥ����Ʊ��������������ľ�����£�
 	    
 		typedef enum
 		{
 			MAGIC_TARGET_MYSELF,		���м���
 			MAGIC_TARGET_OTHER,		ְ����  ����ֳ��)
 			MAGIC_TARGET_ALLMYSIDE,		    ��  
 			MAGIC_TARGET_ALLOTHERSIDE,	���촡��  
 			MAGIC_TARGET_ALL,		�廯
 			MAGIC_TARGET_NONE,		���ּ�  ��  ئ�У�  ��֧�׻�����
 			MAGIC_TARGET_OTHERWITHOUTMYSELF,ְ����  ����ֳ��ئ��)
 			MAGIC_TARGET_WITHOUTMYSELFANDPET, ������ʸ������½
 			MAGIC_TARGET_WHOLEOTHERSIDE,        ����������  
 		}MAGIC_TARGETTYPE;
 	    
 	    <font size=+1>����ٯ��100ë�����������ƥ���·��ָ�����ئ�£�
 	    
 	    name������  ��
 	    comment�������    ��
 	
 	N0    N3 ���޼��ɷ¶�����
 	    ���ޱ�ئ�Ȼ������м���  ë˪�����£�
 	    N0|kubun|level|charaindex|maxhp|hp|mp|name
 	    
 	    kubun ������  ��ؤ�¾�  �о��ۨ߷���  �У۹�ľ���ϱ������ͼ�����ئ�Уۨ����ɷ¶�������
 	    2��������ݱ���������  �Ȼ������ּ����ɷ¶�����  1bit    level 2bit   charaindexئ��  
 	    ��˪���ľ�£�
 	    charaindex �����޼�charaindex
 	    level�������м���ì���
 	    maxhp�������м�MAXHP
 	    hp�������м��ػ���HP
 	    mp�������м���  
 	    name �������м�  ��
 	
 	K0  K4 ��  �ɷ¶�����
 	    hp maxhp mp maxmp str tough exp 
 	    level attackpower deffencepower 
 	    fixdex fixcharm fixluck fixfireat fixwaterat fixearthat fixwindat
 	      � status
 	    K��  ��0  4ë϶���ƻ���������  ��ë϶�����³����
 	    �����ݼ������ͼ���0��  ���չ���ʸ����  ��  �����ɵ��ǣ�
 	    ؤ����������ƥؤ�£�1�������ɷ¶�������
 	    2��������ݱ���������  �Ȼ������ּ����ɷ¶�����  1bit    hp  2bit   maxhpئ��  
 	    ��˪���ľ�£�
 	    ���������� '|' ƥؤ�£�����  �����ñ�į����
 	    <a href="#escaping">�޵������������ּ�ë
 	    �׻���ƥ��ئ�����ּ���ɧ���ľ�£�
 	    ��ľ��ľ�������ᷴ��Ʊ��
 	    
 	        No.(  į)  int  
 	        islive(Ϸ��)  int  
 	        GRA(  �  į)  int  
 	        Hp(��ݼ  )  int  
 	        MaxHp(    ��ݼ  )  int  
 	        Mp  int  
 	        MapMp  int  
 	        Exp(������)  int  
 	        MaxExp(����ƥ��������)  int  
 	        Level(��ì��)  int  
 	        Attack(  ��  )  int  
 	        Defense(��    )  int  
 	        Quick(�ٶƽ�)  int  
 	        Ai(���պ)  int  
 	        fEarth(��)  int  
 	        fWater(  )  int  
 	        fFire(��)  int  
 	        fWid(�)  int  
 	        Slot(    ��  ��)  int  
 	          �  �ʸ��ڰ׷º�  int  
 	          �    ٯ    
 	        �����ǡ�ʸ����      ٯ    
 	    
 	      �  �ʸ��ڰ׷º��練�ݳ�ʸ������  �ë  ���ƻ����о����������׷º�ƥ��
 	    1 ����  ��    �ݨ߷���  ���ص���ئ�£�
 	    
 	    ��    ( char ��    ��������̤�� )
 	    
 	    P10|20|10|20|10|10|10|1|2|13|13|1|100|10|������|PC
 
 	W0  W4 ʸ������  ������
 	  W0|skillid|field|target|name|comment| x 7
 	  W0    W4 ����ľ��ľ��ʸ�����帲ɱ�ƻ����£�
 	  petskillid ����ʸ������  ��  į��pet_skillinfo.h��������ľ�����£�
 	  field ������  ������ƥ����ƥ���¾����ӡ����ƥ����Ʊ��������������ľ�����£�
 	  
 		typedef enum
 		{
 			PETSKILL_FIELD_ALL,		���ͻ�������ƥ������
 			PETSKILL_FIELD_BATTLE,		��    ����
 			PETSKILL_FIELD_MAP,		ɧ��Ѩ����������
 
 		}PETSKILL_FIELDTYPE;
 	  
 	  target ������  �����޻������е��ּ������ӡ����ƥ���ݼ�������������ľ�����£�
 	  
 		typedef enum
 		{
 			PETSKILL_TARGET_MYSELF,		���м���
 			PETSKILL_TARGET_OTHER,		ְ����  ����ֳ��)
 			PETSKILL_TARGET_ALLMYSIDE,	    ��  
 			PETSKILL_TARGET_ALLOTHERSIDE,	���촡��  
 			PETSKILL_TARGET_ALL,		�廯
 			PETSKILL_TARGET_NONE,		���ּ�  ��  ئ�У�  ��֧�׻�����
 			PETSKILL_TARGET_OTHERWITHOUTMYSELF,ְ����  ����ֳ��ئ��) 
 			PETSKILL_TARGET_WITHOUTMYSELFANDPET, ������ʸ������½
 		}PETSKILL_TARGETTYPE;
 	  
 	  name ��  ��  ��
 	  comment������  �帲����    ��
 	  target|name|comment| ��  ���Ѽ��Ш��ƥ˪��ľ�����£�
 	    ����ɱ�湴��ƥ��    ƥ  ��  �껯����  "|"���Ļ�����  ����
 	  ���ͷ���ʧ����ƥ�黧��  �������ǣ�
*/
void lssproto_S_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_S_SEND, buffer);
}

/*
   servertoclient D( int category , int dx , int dy , string data );
       (Display)
           ���Ͼ�  ������϶  ��
 
       
 	int category
 	��ë  �����¾���
 	    
 	        ��        
 	        1  ���ж�½��ܸ����ĸ�������data��  ٯ
 		    ��ئ���װ���  
 		    2  ���л�������ĸ�������data��  ٯ  ��
 		      ئ���װ�  
 		
 	int dx
 	Ѩ��ƽ�ҷ¾��ռ����������¸���  X����  ���巴��ì������  Ϸ
 	    ������  ���ͷ���ʧ�������𼰰������͸����ئ��  ë��ң�ƻ�
 	        ���£�
 	int dy
 	�¸���  Y
 	string data 
 	  ������    ��    ��category�巽�Ȼ�����£�
 	    ��  ٯ  ��<a href="#escaping">�޵��������ľئ��ľ��
 	    ئ��ئ�У�
*/
void lssproto_D_send(int fd,int category,int dx,int dy,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, category);
	checksum += util_mkint(buffer, dx);
	checksum += util_mkint(buffer, dy);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_D_SEND, buffer);
}

/*
   servertoclient FS( int flg);
       (FlgSet)PS����ɱ�ϣ����׷�����̼���ئ������м��������ƻ�˪��ľ�����£�
       
         int flg
          0 bit  0: ����Off                  1: ����On
                �ػ�  ����  1 bit  0: ��      ����off          1: ��      ����On    
              2 bit  0: DUEL off                 1: DUEL On
              3 bit  0:   ɧ����������ƹ����     1: �ɡ�  ū���ƾ�����������  ��ئ��ƹ����
              4 bit  0:   ͭ����OK               1:   ͭ������  
*/
void lssproto_FS_send(int fd,int flg)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, flg);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_FS_SEND, buffer);
}

/*
   servertoclient HL( int flg);
       (HeLp)HL����ɱ�ϣ����׷��ɡ�  ū�����޻���  ���ƾ���ƹ����ë  ������������˪��ľ�����£�
       
         int flg
          0: �ƾ���ƹ����Off                  1: �ƾ���ƹ����On
*/
void lssproto_HL_send(int fd,int flg)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, flg);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_HL_SEND, buffer);
}

/*
   servertoclient PR( int request, int result);
       (PartyRequest)����ۢ�Ƽ�ɱ�ϣ�PRë�ͷ���ʧ������˪�Ȼ���ئ�ʻ��ֳ�ľë���������ݻ�ؤ�£�
        �ɡ�  ū��  ��ݩ��  �ʻ�  ����  ئ�����ݣ�
       
       	int request
       	0: ���� 1:  ��
         int result
         0: ��   1: ��  
*/
void lssproto_PR_send(int fd,int request,int result)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, request);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PR_SEND, buffer);
}

#ifdef _PETS_SELECTCON
void lssproto_PETS_send(int fd,int petarray,int result)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, petarray);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PETST_SEND, buffer);
}
#endif

void lssproto_KS_send(int fd,int petarray,int result)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, petarray);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_KS_SEND, buffer);
}

void lssproto_SPET_send(int fd, int standbypet, int result)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, standbypet);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_SPET_SEND, buffer);
}

/*
   servertoclient PS( int result, int havepetindex, int havepetskill, int toindex);
       (PetSkill use result)ʸ������  ë�����������ͷ���ʧ������PS�帲ɱ�ƻ�˪��ľ�����£�
       result ��½���ͷ���ʧ������PS�帲ɱ�ƻ����£۰�ū�������������ƾ�����ľئ�У�
       
 	int result
 	�����0: ��   1:��  
         int havepetindex
         ��    ��ʸ�������������׾���
         int havepetskill
         ��    ��  ë�������׾���
 	int toindex
 	 ����  ܷë�������׾��۳�ľ����Ƥ��������֧ƽ�ҷ¼�indexƥ��ئ�У۶�Ʊ��  ��ئ�Ȼ����£�
 	
 	  ����    = 0
 	  ʸ����  = 1   5
 	  ����    = 6   10   S N ��0  4�帲ɱ�۷��з�����ֳ��ľ������  
 	
 	���޻��幻���羮�о���ئ�У��羮��������-1ƥ˪�����£�
*/
void lssproto_PS_send(int fd,int result,int havepetindex,int havepetskill,int toindex)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, result);
	checksum += util_mkint(buffer, havepetindex);
	checksum += util_mkint(buffer, havepetskill);
	checksum += util_mkint(buffer, toindex);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PS_SEND, buffer);
}

/*
   servertoclient SKUP( int point );
       (SKillUP)
       ��ƽ��ʧ����ƥ������ë�ӡ��ﻥɧ�����£����ʹ�������ľ�¾�ë϶�����£�
*/
void lssproto_SKUP_send(int fd,int point)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, point);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_SKUP_SEND, buffer);
}

/*
   servertoclient WN( int windowtype, int buttontype, int seqno, int objindex, string data );
       (Window)
       ��ū������ë  ���������ӡ�������ͷ���ʧ������ɧ�����£�
       
         int windowtype
         ��ū���������������������ئ��ֿ����ū������ë  �����¾��۳�ľ��buttontype ��׬��������ƥ
             ��ū��������������£�
             �ӡ����ƥ����Ʊ��  ��ɬ�ý�ľ�����£�
           
 		typedef enum
 		{
 			WINDOW_MESSAGETYPE_MESSAGE,			���������⼰��
 			WINDOW_MESSAGETYPE_MESSAGEANDLINEINPUT,		���������������    
 			WINDOW_MESSAGETYPE_SELECT,			��  ��ū������
 			WINDOW_MESSAGETYPE_PETSELECT,			ʸ������  ��ū������
 			WINDOW_MESSAGETYPE_PARTYSELECT,			���޼�  ��ū������
 			WINDOW_MESSAGETYPE_PETANDPARTYSELECT,		ʸ���������޼�  ��ū������
 			WINDOW_MESSAGETYPE_ITEMSHOPMENU,		���Ҽ�����ب�����̼�����
 			WINDOW_MWSSAGETYPE_ITEMSHOPYMAIN,		���Ҽ����̼����̼�����
 			WINDOW_MESSAGETYPE_LIMITITEMSHOPMAIN,		  ����Ի��  ʧ��  ة�м����̼����̼�����
 			WINDOW_MESSAGETYPE_PETSKILLSHOP,		ʸ������  �н������̼�����
 			WINDOW_MESSAGETYPE_WIDEMESSAGE,			���������⼰��    ����    
 			WINDOW_MESSAGETYPE_WIDEMESSAGEANDLINEINPUT,	���������������        ����    
 
 		}WINDOW_MESSAGETYPE;
 
         int buttontype
         ʾ��������ֿë϶�����£��ӡ����ƥ����Ʊ���ñ�������ľ�����£�
             
 		#define		WINDOW_BUTTONTYPE_NONE		(0)
 		#define		WINDOW_BUTTONTYPE_OK		(1 << 0)
 		#define		WINDOW_BUTTONTYPE_CANCEL	(1 << 1)
 		#define		WINDOW_BUTTONTYPE_YES		(1 << 2)
 		#define		WINDOW_BUTTONTYPE_NO		(1 << 3)
 		#define		WINDOW_BUTTONTYPE_PREV		(1 << 4)
 		#define		WINDOW_BUTTONTYPE_NEXT		(1 << 5)
             
             ��ľ�ռ�׬��������ƥ˪�����£�  ���ɣ�YESʾ������NOʾ������  �����ݷ�
             WINDOW_BUTTONTYPE_YES | WINDOW_BUTTONTYPE_NO   (=12)
             ƥ˪�£�
             
         int seqno
         ����ū��������  įë���ʣ��ӡ��������  ���£�
             �ͷ���ʧ������WN�廯����ū����������������ë߯�ʼ���ݳ�  įë������߯�����£�
             ��ľ�巽�Ȼ��ӡ����������NPC��������  ƥ����ū��������ë  Ӭ��  �·��������£�
         int objindex
         ����ū������ë������������NPCئ����index��ɡ  ��ľ�����£�
             �˵�  ة��������������������-1ئ����  �Ȼ����£�
             �ͷ���ʧ����������ū������      ��WN��������ƥ���Ѱ�ë��������߯����  �У�
         string data
         ����������    ë���ʣ�    ���޵����������£�"\n"ƥ������ݱ�����е���  �����£�
             ���ף��ͷ���ʧ��������ū������ƥ  ����  �²���ë�����׶˷���  ����ݱ���ľ�£�
             ��  �ü�ؤ����ū������ƥ����"\n"ƥ����ľ��  �ټ������ͼ������������⼰����
             ��ئԻ���ݱ嶪�������⼰�����ͼ���  �ټ������ͼ�ƥ϶�ý�ľ�����������
             ����ؤ�缰�����ͼ�  ���๴�ڹ�����  ����ئ�£�����󡾮�մͱ� 1 ����
               įë�Ի񲻯�ݼ�  ����������WNƥ��߯�ϼ��ݱ�߯�ʣ�
         string data(������  
         ����������    ë���ʣ�    ���޵����������£�"\n"ƥ������ݱ�����е���  �����£�
             ���ף��ͷ���ʧ��������ū������ƥ  ����  �²���ë�����׶˷���  ����ݱ���ľ�£�
             ������  ����    ��"  "ƥ����ľ�������ʣ�
    ��  ����
  	  ��  �°׷º�    ���ߡ�  �¨�    󡼰�������������׷º�  �����ߡ�����ئ�С�1    
 		�Ҽ�  �  ����������  �Ҷ���������  ���Ѽ�  ����������  ��ì����Իئ�ж���������  ��  ����������  
 		ʧ��  ة���������ж���������  ʧ��  ة      ����  ��ئ�а׷º�    ���¡���  ��ئ�С���    ʧ��  ة��ì��  
 		���    �  į        ʧ��  ة      ����  ��ئ�а׷º�  ʧ��  ة��ì��  ���    �  į      
       		
       		0|1|��ة�׻Ｐ  տ��|�����������У��ϼ���������|��������������ľ������|����  ۢ����|��ľë��  ���±巴��ì�ﻥ
       		��Իئ���������м�����|  �������Ϸ���|��������ʧ��  ة����������Ԫ����������  Ƥ�����¼�|
       		��|��ߨ�|���ߨߨ�|����  ��  ��\n���  Ъ|����|��|���|���ߨߨ�|    ��߼���\n  ݫ�巴����  \n����  
 
         ��  �£�
 		  ��  �°׷º�    ���ߡ�  �¨�    󡼰�������������׷º�  �����ߡ�����ئ�С�1    
 		�Ҽ�  �  ����������  �Ҷ���������  ���Ż��������б�ئ������������������|��  ����������  
 		ʧ��  ة      ľ��  ľئ�а׷º�  ���    �  į        ʧ��  ة      ľ��  ľئ�а׷º�  
 		���    �  į        
       
       		��|1|��ة�׻Ｐ  ���|�����������У��������£�  ��������������ľë  �Ȼ���ľ�£�|  �������Ϸ���|
       		��ľ��������Ż�����������������  Ƥ�����¼�|��|���|���ߨߨ�|
       		����  ��  ��\n���  Ъ|����|��|���|���ߨߨ�|    ��߼���\n  ݫ�巴����  \n����  
         ��ʧ��  ة  ���£�
 		�׷º�    ��=��    ؤ������  ����ľ�¾�  
 		�Ҽ�  �  
 		  ���¶���������  ��ľ����  ����ľئ�ж���������  ��  ����������  
 		  �    ����ľ�¾��׷º�  ��  ���ߣ��صڣ���    ���    �  į  ����̼�����  ʧ��  ة    į  �ྮ��  
 		  �    ����ľ�¾��׷º�  ��  ���ߣ��صڣ���    ���    �  į  ����̼�����  ʧ��  ة    į  �ྮ��  
         ��ʧ��  ة¦�����£�
 		�׷º�  ¦������=��    
 		�Ҽ�  �  ¦�����ʶ���������  ʧ��  ة����  ���ݼ�����������|��  ����������  
 		  �  ¦�������¾��������׷º�  ��  ���ߣ��صڣ���    ʧ��  ة��ì��  ���    �  į  ����̼�����  
 		  �  ¦�������¾��������׷º�  ��  ���ߣ��صڣ���    ʧ��  ة��ì��  ���    �  į  ����̼�����  
 
         �չ���ְ��
 		�Ҽ�  �  ����������
       
       		��ة�׻Ｐ  ���|�����������У��������£�
 	
         string data(ʸ������  �н�����  
         ����������    ë���ʣ�    ���޵����������£�"\n"ƥ������ݱ�����е���  �����£�
             ���ף��ͷ���ʧ��������ū������ƥ  ����  �²���ë�����׶˷���  ����ݱ���ľ�£�
             ������  ����    ��"  "ƥ����ľ�������ʣ�
 	   ��ʸ������  ��  ��
 		󡼰��  ��������������  ������0������ئ�С�1    �Ҽ�  �  ���̼�����������  
 		      ���                ���        
 	
 		1  ��ة�׻Ｐ  տ��  ������������������������������  ���Ⱦ�ƥ�ʷ�  
 		    ��������  500    ��  ��ߡ�    ��  ��  ��ߡ�          
 		�������  1000  ޥ����  έ��ؤ�¾���  ���    	
*/
void lssproto_WN_send(int fd,int windowtype,int buttontype,int seqno,int objindex,char* data)
{
	char buffer[65500];
	int checksum=0;

#ifdef _NO_WARP
	// shan hanjj add Begin
    CONNECT_set_seqno(fd,seqno);
    CONNECT_set_selectbutton(fd,buttontype);    
	// shan End
#endif

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, windowtype);
	checksum += util_mkint(buffer, buttontype);
	checksum += util_mkint(buffer, seqno);
	checksum += util_mkint(buffer, objindex);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_WN_SEND, buffer);
	
}

/*
   servertoclient EF( int effect, int level, string option );
       (EFfect)
       ʪ֧�ë��������Ի���ͷ���ʧ��������  ��ئ���  ��������˪�£�
       
         int effect
         ���  į�۹�ľ��ľë����ң�������    ���ּ����  ��ľ���ʣۨ�����������ʪ��������Ի�羮�����ʣ�
           
             1:�
             2:ʪ
             4:����ʪ    ��  
           
         int level
         ����۽�ۨ߷����ʣۨ�  �䷴  �ؼ��۽�ë  �ʣ�
         string option
         ��  ������
*/
/* Arminius
  �����������ӵĶ���
  int effect = 10   ������
      level dont care
      option = "pet1x|pet1y|dice1|pet2x|pet2y|dice2" (no makeEscapeString)
*/
void lssproto_EF_send(int fd,int effect,int level,char* option)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, effect);
	checksum += util_mkint(buffer, level);
	checksum += util_mkstring(buffer, option);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_EF_SEND, buffer);
}

/*
   servertoclient SE( int x, int y, int senumber, int sw );
       (SoundEffect)
 	�ͷ���ʧ������SEë  ���ʷ�����϶�����£�
       
         int x,y
         SEë  ���缰��  ��
         int senumber
             ��  į
         int sw
             ë  ���ʾ������¾���
           
             0:������
             1:  ����
*/
void lssproto_SE_send(int fd,int x,int y,int senumber,int sw)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, x);
	checksum += util_mkint(buffer, y);
	checksum += util_mkint(buffer, senumber);
	checksum += util_mkint(buffer, sw);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_SE_SEND, buffer);
}

/*
   servertoclient ClientLogin(string result);
       ClientLogin��߯�ϣ�
       
 	string result
 	"ok" ���е�  ٯ  �۳�  ٯ  ���޵��������ľئ�У�
*/
void lssproto_ClientLogin_send(int fd,char* result)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CLIENTLOGIN_SEND, buffer);
}

/*
   servertoclient CreateNewChar(string result,string data);
       CreateNewChar��߯�ϣ�
       
 	string result
 	"successful" �� "failed" ������ľ���۳�  ٯ  ���޵�����
 	    ����ئ�У�
 	string data
 	"failed" ���ݷ�    ë�������޼�έ������  ٯ
 	      ƥؤ�£�ʧ���������ӡ��ﾮ�ռ�߯�Ϲ�������ƥؤ�£�
 	    ��Ʊ��  ٯ  

 	    "failed bad parameter"

 	    ����������ƽ�ҷ��������������ü��ɷ¶�������  ��ë������
 	    �������е���ë��  ���£۳�ľ���ء�ة�ӡ��������  ���¶���
 	    ������ƥؤ�£۳�  ٯ  ���޵���������ئ�У�
*/
void lssproto_CreateNewChar_send(int fd,char* result,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, result);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CREATENEWCHAR_SEND, buffer);
}

/*
   servertoclient CharDelete(string result,string data);
       CharDelete ��߯�ϣ�
       
 	string result
 	"successful" �� "failed" ������ľ���۾޵���������ئ�У�
 	string data
 	"failed" ���ݷ�    ë�������޼�έ������  ٯ
 	      ƥؤ�£�ʧ���������ӡ��ﾮ�ռ�߯�Ϲ�������ƥؤ�£�
 	    ��������ë������ئ�м�ƥ���޵���������ئ�У�
*/
void lssproto_CharDelete_send(int fd,char* result,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, result);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CHARDELETE_SEND, buffer);
}

/*
   servertoclient CharLogin(string result,string data);
       CharaLogin��߯�ϣ�
       
 	string result
 	"successful" �� "failed" ������ľ���۾޵���������ئ�У�
 	string data
 	"failed" ���ݷ� ����    ��  ٯ  �۾޵���������ئ�У�
*/
void lssproto_CharLogin_send(int fd,char* result,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, result);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CHARLOGIN_SEND, buffer);
}

/*
   servertoclient CharList(string result,string data);
       CharList��߯�ϣ�
       
 	string result
 	"successful" �� "failed" ������ľ���۾޵���������ئ��
 	string data
 	result��"successful"���ݷ���ʧ���������ӡ�����������ľ��
 	    �������ͻ���ƽ�ҷ¼�  �����������ëë��ʸ����ƥ������
 	    ���ۼ�  ٯ  �۳�  ٯ  ë���Ȼ����¼�����ʧ���������ӡ����
 	    result �� "failed" ���ݷ�    ë�������޼�έ������  ٯ  ƥ
 	    ؤ�£���  ���ݼ������������  �췴��Ʊ������Իƥؤ�£�
 	    dataplace|faceimage|level|maxhp|atk|def|quick|charm|earth|water|fire|wind|logincount|name|place
 	    
 	      dataplace
 	      ����Ƥ����������    ���۳�ľƥƽ�ҷ�����������������    ��ë������£�
 	      faceimage
 	      ӿ��  �  į
 	      level
 	      ƽ�ҷ¼���ì��
 	      maxhp,atk,def,quick,charm
 	      ���ɷ¶�������
 	      earth.water,fire,wind
 	      �������
 	      logincount
 	      ����̼���������
 	      name
 	      ƽ�ҷ¼�  �
 	      place
 	      ƽ�ҷ¼���������
 	    
 	    "|" ƥ����ľ�����£� ��ľ��ľ����  ����<a
 	    href="#escaping">�޵��������ľ�����£۹���ؤ���׻���
 	    ƥ��ئ���£�
*/
void lssproto_CharList_send(int fd,char* result,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, result);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CHARLIST_SEND, buffer);
}

/*
   servertoclient CharLogout(string result , string data);
       Logout�帲����߯�ϣ�
       
 	string result
 	"successful" �� "failed" ������ľ���۾޵���������ئ�У�
 	string data
 	"failed" ���ݱ弰����  ��ؤԻ����  ��    (����)ë��������
 	    ��έ������  ٯ  ƥؤ�£۾޵���������ئ�У�
*/
void lssproto_CharLogout_send(int fd,char* result,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, result);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CHARLOGOUT_SEND, buffer);
}

/*
   servertoclient ProcGet( string data);
 	ProcGet��߯�ϣ�
       
 	string data
 	�޵����������£۱ء�ة�ӡ������    ��  ë˪�����£�    ��log/proc�����̻��̤��ľ��    ��  Ԫ��
*/
void lssproto_ProcGet_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");
	
	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PROCGET_SEND, buffer);
}

/*
   servertoclient PlayerNumGet( int logincount, int player);
 	PlayerNumGet��߯�ϣ�
       
 	int logincount,player
*/
void lssproto_PlayerNumGet_send(int fd,int logincount,int player)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, logincount);
	checksum += util_mkint(buffer, player);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PLAYERNUMGET_SEND, buffer);
}

/*
   servertoclient Echo( string test );
       Echo�帲����߯�ϣ�
       
 	string test
 	�������    ��ľ��  ٯ  �۾޵���������ئ�У�
*/
void lssproto_Echo_send(int fd,char* test)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, test);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_ECHO_SEND, buffer);
}

// CoolFish: Trade 2001/4/18

void lssproto_TD_send(int fd, int index, char* message)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_TD_SEND, buffer);
}
#ifdef _CHATROOMPROTOCOL			// (���ɿ�) Syu ADD ������Ƶ��
void lssproto_CHATROOM_send ( int fd , char* message )
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CHATROOM_SEND, buffer);
}
#endif
#ifdef _NEWREQUESTPROTOCOL			// (���ɿ�) Syu ADD ����ProtocolҪ��ϸ��
void lssproto_RESIST_send ( int fd , char* message )
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_RESIST_SEND, buffer);
}
#endif
#ifdef _OUTOFBATTLESKILL			// (���ɿ�) Syu ADD ��ս��ʱ����Protocol
void lssproto_BATTLESKILL_send ( int fd , char* message )
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_BATTLESKILL_SEND, buffer);
}
#endif
void lssproto_NU_send(int fd, int nu)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, nu);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_NU_SEND, buffer);
}


void lssproto_FM_send(int fd, char* message)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_FM_SEND, buffer);
}


void lssproto_WO_send(int fd,int effect)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, effect);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_WO_SEND, buffer);
}
#ifdef _ITEM_CRACKER
void lssproto_IC_send(int fd, int x, int y)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, x);
	checksum += util_mkint(buffer, y);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_IC_SEND, buffer);
}
#endif
#ifdef _MAGIC_NOCAST             // ����:��Ĭ
void lssproto_NC_send(int fd,int flg)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, flg);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_NC_SEND, buffer);
}
#endif

#ifdef _CHECK_GAMESPEED
void lssproto_CS_send( int fd, int deltimes)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint( buffer, deltimes);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CS_SEND, buffer);
}
#endif

#ifdef _STREET_VENDOR
void lssproto_STREET_VENDOR_send(int fd,char *message)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer,"");

	CONNECT_getCdkey(fd,PersonalKey,4096);
	strcat(PersonalKey,_RUNNING_KEY);

	checksum += util_mkstring(buffer,message);
	util_mkint(buffer,checksum);
	util_SendMesg(fd,LSSPROTO_STREET_VENDOR_SEND,buffer);
}
#endif

#ifdef _RIGHTCLICK
void lssproto_RCLICK_send(int fd, int type, char* data)
{
	char buffer[65500];
	int checksum=0;

	print("\n RCLICK_send( type=%d data=%s) ", type, data );

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkint(buffer, type);
	checksum += util_mkint(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_RCLICK_SEND, buffer);
}
#endif

#ifdef _JOBDAILY
void lssproto_JOBDAILY_send(int fd,char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer,"");

	CONNECT_getCdkey(fd,PersonalKey,4096);
	strcat(PersonalKey,_RUNNING_KEY);

	//print("tocli_data:%s ",data);
	checksum += util_mkstring(buffer,data);
	util_mkint(buffer,checksum);
	//print("tocli_buffer:%s ",buffer);
	util_SendMesg(fd,LSSPROTO_JOBDAILY_SEND,buffer);
}
#endif

#ifdef _TEACHER_SYSTEM
void lssproto_TEACHER_SYSTEM_send(int fd,char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer,"");

	CONNECT_getCdkey(fd,PersonalKey,4096);
	strcat(PersonalKey,_RUNNING_KEY);

	checksum += util_mkstring(buffer,data);
	util_mkint(buffer,checksum);
	util_SendMesg(fd,LSSPROTO_TEACHER_SYSTEM_SEND,buffer);
}
#endif

#ifdef _ADD_STATUS_2
void lssproto_S2_send(int fd,char* data)
{
	char buffer[65500];
	int checksum=0;

	strcpy(buffer,"");

	CONNECT_getCdkey( fd, PersonalKey, 4096);
	strcat(PersonalKey, _RUNNING_KEY);

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_S2_SEND, buffer);
}
#endif
