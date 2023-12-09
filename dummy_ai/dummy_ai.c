#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/connsix.h"

char wbuf[10] ;	
extern int N;
// position_t prevPosition[2] = {{-1,-1},{-1,-1}};
// position_t oppsPosition[2] = {{-1,-1},{-1,-1}};

put_t prevPosition = {{-1,-1},{-1,-1}};
put_t oppsPosition = {{-1,-1},{-1,-1}};

int
main ()
{
	char ip[20] ;
	int port = 0 ;
	char color[10] ;
	printf("Enter ip: ") ;
	scanf("%s", ip) ;
	printf("Enter port number: ") ;
	scanf("%d", &port) ;
	printf("Enter player color: ") ;
	scanf("%s", color) ;

	char * redstones = lets_connect(ip, port, color) ;
	if (redstones == 0x0) {
		fprintf(stderr, "Error!\n") ;
		exit(EXIT_FAILURE) ;
	}
	printf("Received %s redstones.\n", redstones) ;

	char * first ;
	if (strcmp(color, "black") == 0) 
		first = draw_and_read("K10") ;
	else
		first = draw_and_read("") ;

	if (first == 0x0) {
		fprintf(stderr, "Error!\n") ;
		exit(EXIT_FAILURE) ;
	}
	printf("Read %s from server.\n", first) ;
	 
	char hor1 = '\0' ;
	char hor2 = '\0' ;
	int ver1 = 0 ;
	int ver2 = 0 ;
	srand(time(0x0)) ;

	while (1) {
		put_score_t nextPosition = decideNextStone(prevPosition, oppsPosition, strcmp(color, "black") == 0 ? 1 : 2, 0);

		prevPosition = nextPosition.put;

		hor1 = nextPosition.put.p1.x;
		ver1 = nextPosition.put.p1.y;
		hor2 = nextPosition.put.p2.x;
		ver2 = nextPosition.put.p2.y;

		if(hor1 >= 8)
			hor1 = hor1 + 'A' + 1;
		else
			hor1 = hor1 + 'A';
		if(hor2 >= 8)
			hor2 = hor2 + 'A' + 1;
		else
			hor2 = hor2 + 'A';
		ver1 = ver1 + 1;
		ver2 = ver2 + 1;	
		
		printf("\nhor1: %c, ver1: %d, hor2: %c, ver2: %d\n", hor1, ver1, hor2, ver2);
		snprintf(wbuf, 10, "%c%02d:%c%02d", hor1, ver1, hor2, ver2) ;

		char * rbuf = draw_and_read(wbuf) ;
		if (rbuf == 0x0) {
			printf("Error!\n") ;
			break ;
		}
		printf("Read %s from server2.\n", rbuf) ;


		if (strcmp(rbuf, "WIN") == 0 || strcmp(rbuf, "LOSE") == 0 || strcmp(rbuf, "TIE") == 0)
			break ;

		char *opps[2];
		char * _rbuf = strdup(rbuf) ;
		getOppsPosition(rbuf, &oppsPosition.p1.x, &oppsPosition.p1.y, &oppsPosition.p2.x, &oppsPosition.p2.y);
	}

	return 0 ;
}
