#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/connsix.h"

char wbuf[10] ;	

position_t prevPosition[2] = {{-1,-1},{-1,-1}};

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
		canConnect6(prevPosition); 
 
		if (prevPosition[0].x != -1) { // find 6 connection. WIN!
			hor1 = prevPosition[0].x ;
			ver1 = prevPosition[0].y ;
			hor2 = prevPosition[1].x ;
			ver2 = prevPosition[1].y; 
		} else { // else if 
			hor1 = (rand() % 19) + 'A' ;
			ver1 = (rand() % 19) + 1 ;
				
			hor2 = (rand() % 19) + 'A' ;
			ver2 = (rand() % 19) + 1 ;

			prevPosition[0].x = hor1;
			prevPosition[0].y = ver1;
			prevPosition[1].x = hor2;
			prevPosition[1].y = ver2;
		}
	
		snprintf(wbuf, 10, "%c%02d:%c%02d", hor1, ver1, hor2, ver2) ;

		char * rbuf = draw_and_read(wbuf) ;
		if (rbuf == 0x0) {
			printf("Error!\n") ;
			break ;
		}
		printf("Read %s from server.\n", rbuf) ;

		if (strcmp(rbuf, "WIN") == 0 || strcmp(rbuf, "LOSE") == 0 || strcmp(rbuf, "TIE") == 0)
			break ;
	}

	return 0 ;
}
