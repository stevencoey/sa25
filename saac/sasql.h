#ifndef _SASQL_H_
#define _SASQL_H_

#include "version.h"
#ifdef _SASQL
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

#include <errno.h>
#include <string.h>

int sasql_init( void );
void sasql_close( void );
int sasql_save_nm( int idx, char *acc, char *data );//��������, �ʺ�, ����
int sasql_save_opt( int idx, char *acc, char *data );
int sasql_save_int_info( int idx, char *acc, char *data );
int sasql_save_char_info( int idx, char *acc, char *data );
#endif

#endif
