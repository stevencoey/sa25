#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/wait.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include "version.h"
#include "TCPIP.h"
#include "tools.h"
#include "acctsql.h"

static int WORK_TYPE = 0;
#ifdef _FIX_WORKS
#define ACADDRESS "127.0.0.1"
#define ACPORT 9400
static int afd;
int LoginToAC();
int Ac_message_accept( char *buf, int slen);

#endif

#define PROD_ID	"8904001"
unsigned long myIP;

int mainsockfd;
int port = -1;
int debug = 0;
int timeout = 7200;
char logdir[1024];

int csd=-1;
char WGSAddress[1024];
int WGSPort = -1;
#define BUF_SIZE 4096
int zap=0;
int additional_process=0;

#ifdef _ADD_AC_IP				 // WON ADD ��AC_IP��ͨѶЭ����
char AC_IP[20];
#endif

#ifdef _DYNAMIC_PORT
int wkport = -1;
#endif

void sigpipe(int i)
{
#ifdef _OFFICIAL_USE
	csd=0;
#endif
	signal(SIGPIPE,sigpipe);
}

void sigusr1(int i)
{
#ifdef _OFFICIAL_USE
	if (csd>0) close(csd);
	csd=TCPconnect(WGSAddress,WGSPort);
#endif
	signal(SIGUSR1,sigusr1);
}

void sigusr2(int i)
{
	zap=1-zap;
	additional_process=time(0)/256;
	signal(SIGUSR2,sigusr2);
}

void sigterm(int i)
{
#ifdef _ACNT_LOGIN
	acctsql_close();
#endif
	signal(SIGTERM,sigterm);
}

void writeLog( char *msg );

char *dummy[7][2]={
    {"SYSOP","GoodNight" },
    {"SYSOP1","Good1" },
    {"SYSOP2","Good2" },
    {"SYSOP3","cary" },
    {"SYSOP4","Good4" },
    {"SYSOP5","Good5" },
    {"SYSOP6","Good6" }};

#ifdef _OFFICIAL_USE
static int checkCSconnect( void)
{
	int num = 0;
	while( csd <= 0 && num < 10 ){
		csd = TCPconnect(WGSAddress,WGSPort);
		if( csd <= 0 ) sleep( 6);
		num++;
	}
	if( csd <= 0 ){
		char buf[256];
		sprintf( buf, "Can't Not Connect %s:%d !\n", WGSAddress, WGSPort);
		Log( buf);
		return -1;
	}
	return 1;
}
#endif

#ifdef _TIMEOUTKICK
static int doKN( char *id , char *pas , char *flag , char *process , char *deadline)
#else
static int doKN( char *id , char *pas , char *flag , char *process)
#endif
{
	int ret = -1;

	switch (*flag)
	{
	case '1':
	case '2':
	case '3':
	case '4':
		ret = 0;
		break;
#ifdef _ACNT_LOGIN
	case '9':
		if (acctsql_query (id, pas, process) == 1)
			ret = 0;
		break;
#endif
	}
	
	return ret;
}

/* offical use only
{
	char buffer[BUF_SIZE],idbuf[BUF_SIZE],pasbuf[BUF_SIZE],m[BUF_SIZE];
	char *ptr;
	int ret, x, baka;
	struct timeval tv;
	fd_set rfds;
	int iProcess;

	int length;

	if (zap) return(0);

	again:

	if (csd<=0) csd=TCPconnect(WGSAddress,WGSPort);

	if (csd<=0) { sleep(1); return(-1); }
	else {
		tv.tv_sec=0;
		tv.tv_usec=10000;
		FD_ZERO(&rfds);
		FD_SET(csd,&rfds);
		x=select(1024,&rfds,0,0,&tv);
		if (x<0) return(-100);
		if (x>0) {
			ret=read(csd,buffer,BUF_SIZE-8);
			
			if (ret<=0) { close(csd); sleep(1); csd=0; goto again; }
		}
		strcpy(idbuf,xchange(id));
		strcpy(pasbuf,xchange(pas));
		iProcess = atoi( process );

		switch (*flag) {
		case '1': // Login
		
#ifdef _FIX_MESSAGE                // WON ADD �޸ķ������		

#ifdef _ADD_AC_IP				 // WON ADD ��AC_IP��ͨѶЭ����
			sprintf(buffer,"&;5;6;%s;%s;0;%s;",idbuf,pasbuf, AC_IP);
#else
			sprintf(buffer,"&;5;6;%s;%s;0;",idbuf,pasbuf);
#endif

#else
			sprintf(buffer,"&;9999;5;J;%s;%s;",idbuf,pasbuf);
#endif
			baka=90000+bakayaro(id);
			Log( buffer);
			break;
		case '4': // Delete

#ifdef _FIX_MESSAGE                // WON ADD �޸ķ������		

#ifdef _ADD_AC_IP				 // WON ADD ��AC_IP��ͨѶЭ����
			sprintf(buffer,"&;5;6;%s;%s;0;%s;",idbuf,pasbuf, AC_IP);
#else
			sprintf(buffer,"&;5;6;%s;%s;0;",idbuf,pasbuf);
#endif

#else		
			sprintf(buffer,"&;9999;5;J;%s;%s;",idbuf,pasbuf);
#endif
			baka=90000+bakayaro(id);
			break;
		case '2': // Game Start
#ifdef _TIMEOUTKICK
		case '5': // recheck WGS deadline
#endif

#ifdef _FIX_MESSAGE                // WON ADD �޸ķ������		

#ifdef _ADD_AC_IP				 // WON ADD ��AC_IP��ͨѶЭ����
			sprintf(buffer,"&;4;6;%s;%s;%s;",idbuf,pasbuf, AC_IP);
#else
			sprintf(buffer,"&;4;6;%s;%s;",idbuf,pasbuf);
#endif

#else				
			sprintf(buffer,"&;9999;6;6;%s;%s;%d;",idbuf,pasbuf,iProcess+additional_process);
#endif
			Log( buffer);
			baka=10000+bakayaro(id);			
			break;
		case '3': // Game End
		
#ifdef _FIX_MESSAGE                // WON ADD �޸ķ������		

#ifdef _ADD_AC_IP				 // WON ADD ��AC_IP��ͨѶЭ����
			sprintf(buffer,"&;3;7;%s;%s;",idbuf, AC_IP);
#else
			sprintf(buffer,"&;3;7;%s;",idbuf);
#endif

#else				
			sprintf(buffer,"&;9999;5;7;%s;%d;",idbuf,iProcess+additional_process);
#endif
			Log( buffer);
			baka=20000+bakayaro(id);
			break;
		default:
			return(-100);
		}
		Log("<BW>");
		Log(buffer);
		ret=write(csd,buffer,strlen(buffer));
		Log("<AW>");

		// Write error
		if (ret<0) { close(csd); sleep(1); csd=0; goto again; }

		// Line busy
		if (ret==0) { sleep(1); return(-103); }

		tv.tv_sec=5;
		tv.tv_usec=0;
		FD_ZERO(&rfds);
		FD_SET(csd,&rfds);
		Log ("<BS>");
		x=select(1024,&rfds,0,0,&tv);
		Log ("<AS>");
		if (x<0) return(-101);
		if (x==0) return(-102);
		Log ("<BR>");
		ret=read(csd,buffer,BUF_SIZE-8);
		Log ("<AR>");
		snprintf( m, sizeof(m), "ret: %d ", ret );
		Log( m );
		// Read error
		if (ret<=0) { close(csd); sleep(1); csd=0; return(-104); }
		buffer[ret]=0; 

		snprintf( m,sizeof(m) ," WGS: %s\n",buffer);
		Log( m );

		strtok(buffer,";"); // Get &
		length = atoi( strtok(0,";")); // Get length
		ptr=strtok(0,";"); // Get Flag

		if (ptr==0) return(-105);

		if (ptr[0]=='K') {

			int payType;

#ifndef _FIX_MESSAGE                // WON ADD �޸ķ������	
			if (atoi(ptr+1)!=baka){
				char buff1[256];
				sprintf( buff1, "%d != %d\n", atoi(ptr+1), baka);
				Log(buff1);
				return(-100);
			}
#endif

#ifdef _TIMEOUTKICK

			payType = atoi( strtok(0,";"));
			
			if( *flag == '1' || *flag == '4' ) { // GameLogin or Char Delete
				strtok(0,";"); // get Point
			}
			
			if( strcmp( strtok(0,";"), idbuf)) {  // get PlayerAccount
				//Log(" �ش��ʺŲ���! ");
				//return(-106);
			}

			switch( payType) {
			case 1: // ������
				strcpy( deadline ,"0");
				Log(" ���� ");
				break;
			case 2: // ʱ����
				//strtok(0,";"); // get PlayerAccount
				strcpy( deadline ,strtok(0,";"));
				Log(" ʱ�� ");
				break;
			case 0:
				strcpy( deadline ,"0");
				Log(" ���� ");
				break;
			}

#endif

			switch (*flag) {
			case '1': // List
				Log("User Login\n");
				break;
			case '4': // Delete
				Log("User delete\n");
				break;
			case '2':
				Log("Game start\n");
				break;
			case '3':
				Log("Game end\n");	
				break;
#ifdef _TIMEOUTKICK
			case '5':
				Log("Deadline recheck\n");
				break;
#endif
			default:
				Log("Strange but passed\n");
				break;
			}
			return(0); // Authencation passed
		}

#ifdef _FIX_MESSAGE                // WON ADD �޸ķ������
		// �ش�������
		else if (ptr[0]=='E'){
			char err_code[10];
			int i, err_num=0, login_star=-1;
#ifdef _TIMEOUTKICK
			if( length != 3 )
				strtok(0,";"); // Get 1
			if( *flag != '1' && *flag != '4') {
				if( strcmp( strtok(0,";"), idbuf)) {  // get PlayerAccount
					//Log(" �ش��ʺŲ���! ");
					//return(-106);
				}
			}
			strcpy( deadline ,"0");
#endif
			ptr=strtok(0,";"); 
			
			for(i=0; i<9; i++){
				if (ptr[i] == ' ' )	break;
				err_code[i]=ptr[i];			
			}

			err_num=atoi(err_code);

			if( err_num == 203 ){			// 203 ��ͬ��ϵ
					login_star=atoi(ptr+i);
					Log("203");
					// �ش��ѵ������ϵ����
					if(login_star != -1)
						return(login_star+2030);
			}
			else if( err_num == 204 ){		// 204 ͬ��ϵ	
					Log("204");
					if( *flag == '5')
						Log(" ʱ��δ�� ");
					return(204);
			}else{
				char msg[256];
				sprintf( msg, "won test 1 ==> err_num(%d)", err_num);
				Log( msg );
				return(err_num);
			}

			
		}
#endif

		return(-106);
	}
}
*/


#ifndef _FIX_WORKS  
static int createMainSocket( int port )
{
    int s,r;
    struct sockaddr_in sin;

    s = socket( AF_INET , SOCK_STREAM , 0 );
    if( s < 0 ){
        fprintf( stderr, "socket error" );
        return s;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons( port );
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if( (r = bind( s , (struct sockaddr*) &sin , sizeof( sin )) ) <0 ){
        fprintf(stderr, "bind error" );
        return r;
    }
    listen( s , 5 );

    return s;

}
#endif

static void remove_r( char *s )
{
    int i;
    for(i=0;;i++){
        if(s[i]=='\r')s[i] =0;
        if( s[i]==0)break;
    }
}

static void easyGetTokenFromString( char *src,int count,char*output,int len )
{
    int i;
    int counter = 0;

    if( len <= 0 )return;

#define ISSPACETAB( c )   ( (c) == ' ' || (c) == '\t' )

    for(i=0;;i++){
        if( src[i]=='\0'){
            output[0] = '\0';
            return;
        }
        if( i > 0 && ! ISSPACETAB( src[i-1] ) &&
            ! ISSPACETAB(  src[i] ) ){
            continue;
        }

        if( ! ISSPACETAB( src[i]) ){
            counter++;
            if( counter == count){
                /* copy it */
                int j;
                for(j=0;j<len-1;j++){
                    if( src[i+j] == '\0' ||
                        ISSPACETAB( src[i+j] ) ){
                        break;
                    }
                    output[j]=src[i+j];
                }
                output[j]='\0';
                return;
            }
        }
    }
}

static void workerMainLoop( void )
{
    char linebuf[4096];
	int i_flag;//Arminitus && ttom 12/15
	time_t retime;

#ifndef _FIX_WORKS
	struct sockaddr_in sin;
	int addrlen = sizeof( sin );
	FILE *fp;
	int newsockfd  ;
    while(1){
        fd_set  rfds;
        struct timeval t;
        t.tv_sec = timeout;
        t.tv_usec = 0;
        FD_ZERO( &rfds );
        FD_SET( mainsockfd , &rfds );
        select( 1024 , &rfds , (fd_set*) NULL , (fd_set*)NULL , &t );
        if( FD_ISSET( mainsockfd , &rfds ) ){
            newsockfd = accept( mainsockfd , (struct sockaddr*) &sin , &addrlen );
            if( newsockfd < 0 ){
                fprintf( stderr,"accept error! \n");
            } else {
                memcpy(&myIP, &sin.sin_addr, sizeof(long));
                fprintf( stderr,"accepted. myIP=%ld\n", myIP );
            }
            break;
        } else {
            fprintf(stderr, "accept timed out." );
            exit(0);
        }
    }
    close(mainsockfd);
    {
        int flag = 1;
        int result = setsockopt( newsockfd, IPPROTO_TCP, TCP_NODELAY,
                                 (char*) &flag, sizeof(int));
        if( result != 0 ){
            fprintf( stderr , "nodelay error!\n" );
        }
    }
    fp = fdopen( newsockfd , "r+");
    if( fp == NULL ){
        fprintf( stderr,"fdopen error.abort" );
        return ;
    }
#endif

#ifdef _OFFICIAL_WGS
	//ȷ��CS������
	if( checkCSconnect() == -1 ) return;
#endif

#ifdef _FIX_WORKS
	if( !LoginToAC() )	//login to ac
		return;
	retime = time( NULL);
#ifdef _SQL_PING
	static time_t ping_time;
#endif
	while( WORK_TYPE!=-1)	{
#else
while( fgets( linebuf,sizeof(linebuf), fp)){
#endif
    	// Spock 2000/10/31
        char id[104] ,pas[1024] , flag[8] , process[16];
        // Spock end
        int knret;
#ifdef _TIMEOUTKICK
		char deadline[100] = "0"; // _TIMEOUTKICK
#endif
		usleep(10*1000);
#ifdef _SQL_PING
		if (ping_time < time(NULL)) {
			acctsql_ping();
			
			ping_time = time(NULL) + 10;
		}
#endif

#ifdef _FIX_WORKS
		if( (WORK_TYPE = Ac_message_accept( linebuf, sizeof( linebuf))) <= 0 )
			continue;
		retime = time( NULL);
#endif
		memset( id, 0, sizeof( id));
		memset( process, 0, sizeof( process));
		memset( flag, 0, sizeof( flag));

        /* chop */
        linebuf[strlen(linebuf)-1]=0;
        /* kill-\r */
        remove_r( linebuf );

        easyGetTokenFromString( linebuf , 1 , id , sizeof( id ));
        easyGetTokenFromString( linebuf , 2 , pas , sizeof( pas ));
        // Nuke +1
        easyGetTokenFromString( linebuf , 3 , flag, sizeof(flag));
        // Spock 2000/10/31
       	easyGetTokenFromString( linebuf , 4 , process , sizeof(process) );
        // Spock end
        
        // Nuke
        i_flag=0;          //Arminius && ttom
        i_flag=atoi(flag); //Arminius && ttom
        if((!process)||(!*process)){
           knret=-7;
        }else{
       		switch (i_flag) {  //Arminius && ttom

#ifdef _FIX_MESSAGE                // WON ADD �޸ķ������	

#ifdef _TIMEOUTKICK

			case  1: // List
#ifdef _AP_CHECK_3_TIMES           // WON ADD ������֤����
				knret = 0;
#else
				knret = doKN( id, pas, flag, "", deadline);
#endif
				break;
       		case  4: // Delete
				knret = doKN( id , pas , flag, "" , deadline);
				break;
			case  2: // Load
				knret = doKN( id , pas , flag, "" , deadline);
				break;
			case  3: // Save
				knret = doKN( id , pas, flag, "" , deadline);
				break;
			case  5: // recheck WGS
				knret = doKN( id , pas , flag, "" , deadline);
				break;
#ifdef _ACNT_LOGIN
			case  9:
				knret = doKN (id, pas, flag, process, deadline);
				break;
#endif
#else

			case  1: // List
#ifdef _AP_CHECK_3_TIMES           // WON ADD ������֤����
				knret = 0;
#else
				knret = doKN( id, pas, flag, "");
#endif
				break;
       		case  4: // Delete
				knret = doKN( id , pas , flag, "" );
				break;
			case  2: // Load
				knret = doKN( id , pas , flag, "" );
				break;
			case  3: // Save
					knret = doKN( id , pas, flag, "" );
				break;
#ifdef _ACNT_LOGIN
			case  9:
				knret = doKN (id, pas, flag, process);
				break;
#endif

#endif

#else


       		case  1: // List
				knret = doKN( id, pas, flag, "");
				break;
       		case  4: // Delete
				knret = doKN( id , pas , flag , "" );
				break;
			case  2: // Load
				knret = doKN( id , pas , flag , process );
				break;
			case  3: // Save
				knret = doKN( id , pas , flag , process );
				break;
#ifdef _ACNT_LOGIN
			case  9:
				knret = doKN (id, pas, flag, process);
				break;
#endif
#endif
			default:
				knret = -7;
			}
		}
#ifdef _FIX_WORKS
		{//andy_add
			int ret;
			char buf1[256];
#ifdef _TIMEOUTKICK
			sprintf( buf1, "%d %s \n", knret, deadline);
#else
			sprintf( buf1, "%d\n", knret);
#endif
			Log( buf1);
			if((ret = write( afd, buf1, sizeof( buf1))) < 0 )	{
				close( afd);
				afd = 0;
				break;
			}
		}
		memset( linebuf, 0, sizeof( linebuf));
#else
#ifdef _TIMEOUTKICK
		fprintf( fp , "%d %s \n", knret, deadline );
#else
		fprintf( fp , "%d\n", knret );
#endif
#endif
    }
#ifdef _FIX_WORKS
	if( afd > 0 ) close( afd);
	if( csd > 0 ) close( csd);
#else
    fclose( fp );
#endif

#ifdef _ACNT_LOGIN
	acctsql_close ();
#endif
	//if( AcLogFile != NULL)
	//	fclose( AcLogFile);
    Log( "acsv closed the connection. aborting.\n" );
}

static void parseOpt( int argc , char **argv )
{
    int c;
    int option_index;

#ifdef _ADD_AC_IP                // WON ADD �޸ķ������
	memset( AC_IP, -1 , sizeof( AC_IP) );
#endif

    if( argc == 1 ){
        fprintf( stderr , "Argument error: print usage by acwk -h\n" );
        exit(0);
    }
    
    while(1){
        static struct option long_options[] =
        {
            {"port" , 1, 0 ,'p'},
            {"debug" , 1, 0 , 'd'},
            {"help" , 0 , 0 , 'h'},
            {"logdir", 1, 0, 'l' },
            {"csaddr", 1, 0, 'a' },
            {"csport", 1, 0, 'c' },
#ifdef _DYNAMIC_PORT
			{"wkport", 1, 0, 'k'},
#endif
#ifdef _ADD_AC_IP                // WON ADD �޸ķ������
			{"acaddr", 1, 0, 'w' },
#endif
            {0,0,0,0}
        };

#ifdef _FIX_MESSAGE                // WON ADD �޸ķ������
#ifdef _DYNAMIC_PORT
		c = getopt_long (argc, argv, "p:dhl:a:c:w:k:", long_options, &option_index);
#else
        c = getopt_long (argc, argv, "p:dhl:a:c:w:", long_options, &option_index);
#endif
#else
#ifdef _DYNAMIC_PORT
		c = getopt_long (argc, argv, "p:dhl:a:c:k:", long_options, &option_index);
#else
        c = getopt_long (argc, argv, "p:dhl:a:c:", long_options, &option_index);
#endif
#endif

        if( c == -1 )break;

        switch(c ){
        case 'l':
            if( optarg){
                snprintf( logdir, sizeof( logdir), "%s", optarg );
            } else{
                fprintf( stderr , "need directory name after -l.\n" );
            }
            break;
        case 'p' :
            port = atoi( optarg );
            break;
		case 'k':
			wkport = atoi (optarg);
			break;
        case 'd':
            debug = 1;
            break;
        case 'h':
            fprintf( stderr,
                     "Usage: acwk [-d|--debug] [-p port|--port port] [-h|--help] [-l logdir]\n"
                     "            [-a CSAddress] [-c CSPort]\n"
                     "Default debug mode is 0.\n"
                     "\nCopyright 1999 Kengo Nakajima / Japan system supply\n"
                     );
            exit(0);
            break;
        // Spock 2000/12/5
        case 'a':
            strcpy( WGSAddress, optarg );
            break;
        case 'c':
            WGSPort = atoi( optarg );
            break;
        // Spock end
#ifdef _ADD_AC_IP				 // WON ADD ��AC_IP��ͨѶЭ����
		case 'w':
			strcpy( AC_IP, optarg ); 
			Log( AC_IP );
			break;
#endif
        }
    }
}

int main( int argc , char ** argv )
{

    // Spock +1 2000/12/5
    //WGSAddress[0] = 0;
	strcpy( WGSAddress, "127.0.0.1" );
	WGSPort = 9999;
	
    parseOpt(argc,argv);
#ifndef _FIX_WORKS
    if( port < 0 ){
        fprintf( stderr , "You have to specify port number by -p option.\n");
        exit(1);
    }
#endif
    // Spock 2000/12/5
    if( strlen(WGSAddress) < 8 || strlen(WGSAddress) > 16 ){
    	fprintf( stderr , "You have to specify CS IP address by -a option.\n");
    	exit(1);
    }
    if( WGSPort < 0 ){
    	fprintf( stderr , "You have to specify CS port by -c option.\n");
    	exit(1);
    }

#ifndef _FIX_WORKS    	
    fprintf( stderr , "\nport:[%d] debug:[%d] CS:[%s] CSPort:[%d]" ,
    	port , debug , WGSAddress , WGSPort );
    // Spock end
    if( (mainsockfd = createMainSocket( port ))<0){
        fprintf( stderr , "cannot create mainsocket: %s\n",
                 strerror( errno ));
        return 1;
    }
#else
    fprintf( stderr , "\ndebug:[%d] CS:[%s] CSPort:[%d]" ,
			 debug , WGSAddress , WGSPort );
#endif
// Nuke +1
    additional_process=time(0)/256;
    signal(SIGPIPE,sigpipe);
    signal(SIGUSR1,sigusr1);
    signal(SIGUSR2,sigusr2);
    signal(SIGTERM,sigterm);

#ifdef _ACNT_LOGIN
	acctsql_init ();
#endif

    workerMainLoop();
    return 0;
}

/*
static char * chop( char *src )
{
    int i;

    for(i=0;;i++){
        if( src[i] == '\n' ) src[i]= 0;
        if( src[i] == 0 )break;
    }
    return src;
}
*/

void writeLog( char *msg )
{
	char logbuf[BUF_SIZE];
    time_t t;
	if( sizeof( msg) > sizeof( logbuf))
		return;
	sprintf( logbuf, "%s", msg);
	Log( logbuf);
    t = time(NULL);
}

void Log( char *msg )
{
#ifdef _DEBUG_WORK
	FILE *AcLogFile=NULL;
	if( AcLogFile == NULL )	{
		char filename[1000];
		snprintf( filename, sizeof( filename), "%s/%d.worker" , logdir, port );
		AcLogFile = fopen( filename, "a+" );
		if(AcLogFile == NULL)return;
	}

    fprintf( AcLogFile, "%s" , msg );
	fclose( AcLogFile);
#else
	fprintf( stderr,"%s", msg);
#endif
}

#ifdef _FIX_WORKS
int LoginToAC()
{
	Log( "\n connect ac ");
	usleep( 80*10000);
	Log( ".");
#ifdef _DYNAMIC_PORT
	if (wkport == -1)
		wkport = ACPORT;
	afd = TCPconnect (ACADDRESS, wkport);
#else
	afd = TCPconnect( ACADDRESS, ACPORT);
#endif
	if( afd <= 0 )	{
		close( afd);
		return 0; 
	}
	Log( ".");
	usleep( 40*10000);
	return 1;
}
char	tmpbuf[4096];
int readsize;
struct timeval select_timeout;

int Ac_message_accept( char *buf, int slen)
{
    //int i;
    int sret;
    //int accepted = 0;
    struct timeval t;
	fd_set rfds, /*wfds,*/ efds;

	readsize = sizeof( tmpbuf);

	FD_ZERO( & rfds );
    //FD_ZERO( & wfds );
    FD_ZERO( & efds );    

	FD_SET( afd, & rfds );
	//FD_SET( afd, & wfds );
	FD_SET( afd, & efds );

	select_timeout.tv_sec = 0;
	select_timeout.tv_usec = 0;
    t = select_timeout;
    sret = select( 1024 , & rfds , (fd_set*)NULL, & efds , &t);
    if( ( afd >= 0 ) && FD_ISSET( afd , &rfds ) ){
		int rr;
		rr = read( afd , tmpbuf , readsize );
		if( rr > 0 )	{
			memcpy( buf, tmpbuf, slen);
		}else	{
			Log("..WORK ERROR\n");
			return -1;
		}
    }else	{
		return 0;
	}
	return 1;
}

#endif


