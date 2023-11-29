#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "../include/connsix.h"
#include "socket.h"

/* static data structures */
static int sock_fd ;
static char * bufptr ;

typedef enum _status {
	EMPTY,
	BLACK,
	WHITE,
	RED,
	CANDIDATE,
} status_t ;

static status_t board[19][19] ; 

static status_t player_color ;
static status_t opponent_color ;
static position_t prevPosision[2];
static int first_turn ;

typedef enum _errcode {
	BADCOORD,
	NOTEMPTY,
	BADINPUT,
	GOOD,
} errcode_t ;

static const char * err_str[3] = {
	"BADCOORD",
	"NOTEMPTY",
	"BADINPUT",
} ;
/* static data structures */

/* static functions */
static void
print_board() {
	char visual[] = "*@OX" ;
	printf("   ABCDEFGHJKLMNOPQRST\n") ;
	for (int ver = 19; ver > 0; ver--) {
		printf("%2d ", ver) ;
		for (int hor = 0; hor < 19; hor++)
			printf("%c", visual[board[ver-1][hor]]) ;
		printf(" %d\n", ver) ;
	}
	printf("   ABCDEFGHJKLMNOPQRST\n") ;
}


void  
getOppsPosition (char * stone, int * hor1, int * ver1, int * hor2, int * ver2)
{
	char *opps[2];
	char * _stone = strdup(stone) ;

	 opps[0] = strtok(_stone, ":");
	 opps[1] = strtok(NULL, ":");

	char a1 = '\0' ;
	char a2 = '\0' ;
	int _hor1 = -1 ;
	int _ver1 = -1 ;
	int _hor2 = -1 ;
	int _ver2 = -1 ;
	int n = 0 ;


	sscanf(opps[0], "%c%2d%n", &a1, &_ver1, &n);
	sscanf(opps[1], "%c%2d%n", &a2, &_ver2, &n);

	if ('a' <= a1 && a1 <= 'h') {
		_hor1 = a1 - 'a' ;
	} else if ('A' <= a1 && a1 <= 'H') {
		_hor1 = a1 - 'A' ;
	} else if ('j' <= a1 && a1 <= 't') {
		_hor1 = a1 - 'a' - 1 ;
	} else if ('J' <= a1 && a1 <= 'T') {
		_hor1 = a1 - 'A' - 1 ;
	} 
	
	if (0 < _ver1 && _ver1 <= 19) {
		_ver1 -= 1 ;
	} 

	if ('a' <= a2 && a2 <= 'h') {
		_hor2 = a2 - 'a' ;
	} else if ('A' <= a2 && a2 <= 'H') {
		_hor2 = a2 - 'A' ;
	} else if ('j' <= a2 && a2 <= 't') {
		_hor2 = a2 - 'a' - 1 ;
	} else if ('J' <= a2 && a2 <= 'T') {
		_hor2 = a2 - 'A' - 1 ;
	} 
	
	if (0 < _ver2 && _ver2 <= 19) {
		_ver2 -= 1 ;
	} 

	
	*hor1 = _hor1 ;
	*ver1 = _ver1 ;

	*hor2 = _hor2 ;
	*ver2 = _ver2 ;

}

static errcode_t 
parse (char * stone, int * hor, int * ver)
{
	char a = '\0' ;
	int _hor = -1 ;
	int _ver = -1 ;
	int n = 0 ;
	if (sscanf(stone, "%c%2d%n", &a, &_ver, &n) != 2 || stone[n] != '\0')
		return BADINPUT ;

	if ('a' <= a && a <= 'h') {
		_hor = a - 'a' ;
	} else if ('A' <= a && a <= 'H') {
		_hor = a - 'A' ;
	} else if ('j' <= a && a <= 't') {
		_hor = a - 'a' - 1 ;
	} else if ('J' <= a && a <= 'T') {
		_hor = a - 'A' - 1 ;
	} else {
		return BADCOORD ;
	}
	
	if (0 < _ver && _ver <= 19) {
		_ver -= 1 ;
	} else {
		return BADCOORD ;
	}
	
	*hor = _hor ;
	*ver = _ver ;

	return GOOD ;	
}

static int
set_redstones (char * redstones)
{
	char * _redstones = strdup(redstones) ;
	if (_redstones == 0x0) {
		return 1 ;
	}
	char * stone = strtok(_redstones, ":") ;
	while (stone != 0x0) {

		int hor = -1 ;
		int ver = -1 ;
		errcode_t err = parse(stone, &hor, &ver) ;
		if (err != GOOD) {
			return 1 ;
		}
		
		if (board[ver][hor] == EMPTY) {
			board[ver][hor] = RED ;
		} else {
			return 1 ;
		}

		stone = strtok(0x0, ":") ;
	} 
	free(_redstones) ;

#ifdef PRINT
	print_board() ;
#endif

	return 0 ;
}

static errcode_t 
update_board(char * stones, status_t color)
{
	char * _stones = strdup(stones) ;
	if (_stones == 0x0) {
		return BADINPUT ;
	}

	char * stone[2] ;
	stone[0] = strtok(_stones, ":") ;
	if (stone[0] == 0x0) {
		return BADINPUT ;
	}
	stone[1] = strtok(0x0, ":") ;
	if (stone[1] == 0x0) {
		return BADINPUT ;
	}
	char * tok = strtok(0x0, ":") ;
	if (tok != 0x0) {
		return BADINPUT ;
	}
		
	for (int i = 0; i < 2; i++) {
		int hor = -1 ;
		int ver = -1 ;
		errcode_t err = parse(stone[i], &hor, &ver) ;
		if (err != GOOD) {
			return err ;
		}

		if (board[ver][hor] == EMPTY) {
			board[ver][hor] = color ;
		} else {
			return NOTEMPTY ;
		}
	}
	free(_stones) ;	

	return GOOD ;
}

static void
strict_format (char * stones) {
	char hor1 = '\0' ;
	char hor2 = '\0' ;
	int ver1 = -1 ;
	int ver2 = -1 ;

	int n = 0 ;
	if (sscanf(stones, "%c%2d:%c%2d%n", &hor1, &ver1, &hor2, &ver2, &n) != 4 || stones[n] != '\0')
		return ;
	
	if ('a' <= hor1)
		hor1 = hor1 - 'a' + 'A' ;
	if ('a' <= hor2)
		hor2 = hor2 - 'a' + 'A' ;

	sprintf(stones, "%c%02d:%c%02d", hor1, ver1, hor2, ver2) ;
	
	return ;
}

void
cleanup (void)
{
	close(sock_fd) ;
}
/* static functions */

/* API functions */
char *
lets_connect (char * ip, int port, char * color)
{
	if (strcmp(color, "black") == 0) {
		player_color = BLACK ;
		opponent_color = WHITE ;
	} else if (strcmp(color, "white") == 0) {
		player_color = WHITE ;
		opponent_color = BLACK ;
	} else {
		return 0x0 ;
	}
	
	for (int i = 0; i < 19; i++)
		for (int j = 0; j < 19; j++)
			board[i][j] = EMPTY ;

	first_turn = 1 ;

	struct sockaddr_in serv_addr ;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		return 0x0 ;
	} else {
		atexit(cleanup) ;
	}

	int optval = 1 ;
	if (setsockopt(sock_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) < 0) {
		return 0x0 ;
	}

	memset(&serv_addr, 0, sizeof(serv_addr)) ;
	serv_addr.sin_family = AF_INET ;
	serv_addr.sin_port = htons(port) ;
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		return 0x0 ;
	}

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		return 0x0 ;
	}

	bufptr = recv_msg(sock_fd) ;
	if (bufptr == 0x0) {
		return 0x0 ;
	}
	
	if (set_redstones(bufptr) != 0) {
		return 0x0 ;
	}

#ifdef PRINT	
	print_board() ;
#endif
	
	strict_format(bufptr) ;
	return bufptr ;
}

int
isEmpty(int x, int y)
{
	printf("%d ", board[y][x]);
	if(board[y][x] == 0)
		return 1;
	return 0;
}
void
canConnect6(position_t prevPosition[])
{
	if(prevPosition[0].x == -1){
		printf("dadasd");
		return;
	}
	position_t dir[4] = {{0,1}, {1,1}, {1,0}, {1,-1}};
	position_t position[4][11];
	int window[4][11];
	for(int i=0; i<2; i++){
		int x = prevPosition[i].x;
		int y = prevPosition[i].y;
		
		for(int j=0; j<4; j++){
			for(int k=-5; k<=5; k++){
				if(x-(dir[j].x)*k > 18 || y-(dir[j].y)*k > 18){
					window[j][k+5] = -1;
				}
				else{
					window[j][k+5] = board[y-(dir[j].y)*k][x-(dir[j].x)*k];	
				}
				position[j][k+5].x = x-(dir[j].x)*k;
				position[j][k+5].y = y-(dir[j].y)*k;
			}
			for(int k=0; k<6; k++){
				int check = 0;
				int empty_cnt = 0;
				int player_stone = 0;
				position_t empty_position[2];
				for(int l=0; l<6; l++){
					if(window[j][k+l] == -1){
						break;
					}
					else if(window[j][k+l] == 0){
						if(empty_cnt < 2){
							empty_position[empty_cnt].x = position[j][k+l].x;
							empty_position[empty_cnt].y = position[j][k+l].y;
						}
						empty_cnt++;
						continue;
					}
					else if(window[j][k+l] == player_color){
						player_stone++;
					}
					else{
						//if(k+l < 6)
						//	k = k+l;
						check = 1;
						player_stone = 0;
						break; 
					}					
				}

				if(player_stone >= 4 && check == 0){
					if(empty_cnt == 2){ // 6 connection exist with 2 empty space
						prevPosition[0].x = empty_position[0].x;
						prevPosition[0].y = empty_position[0].y;
						prevPosition[1].x = empty_position[1].x;
						prevPosition[1].y = empty_position[1].y;
						return;		
					}
					else{ // 6 connection exist with 1 empty space
						prevPosition[0].x = empty_position[0].x;
						prevPosition[0].y = empty_position[0].y;
						
						for(int m=0; m<19; m++){
							for(int n=0; n<19; n++){
								if(board[m][n] == EMPTY){
									prevPosition[1].x = n;
									prevPosition[1].y = m;
									return;
								}
							}
						}
					}
				}
			}
		}
		
	}
	prevPosition[0].x = -1; // There are no possible 6 connection
	prevPosition[0].y = -1;
	prevPosition[1].x = -1;
	prevPosition[1].y = -1;
	return;
}

int checkNo6(position_t candidate) {
	position_t dir[4] = {{0,1}, {1,1}, {1,0}, {1,-1}};
	position_t position[4][11];
	int window[4][11];

	int x = candidate.x;
	int y = candidate.y;
	
	for(int j=0; j<4; j++){
		for(int k=-5; k<=5; k++){
			if (x-(dir[j].x)*k > 18 || y-(dir[j].y)*k > 18){
				window[j][k+5] = -1;
			}
			else {
				window[j][k+5] = board[y-(dir[j].y)*k][x-(dir[j].x)*k];	
			}
			position[j][k+5].x = x-(dir[j].x)*k;
			position[j][k+5].y = y-(dir[j].y)*k;
		}
		for(int k=0; k<6; k++){
			int check = 0;
			int empty_cnt = 0;
			int opponent_stone = 0;
			
			for(int l=0; l<6; l++){
				if(window[j][k+l] == -1){
					break;
				}
				else if(window[j][k+l] == 0){
					empty_cnt++;
				}
				else if (window[j][k+l] == opponent_color){
					opponent_stone++;
				}
				else {
					check = 1;
					opponent_stone = 0;
					break; 
				}
			}

			if (opponent_stone >= 4 && check == 0) {
				return 0;
			}
		}
	}
	return 1;
}


void checkBlocked(position_t candidate[], int candidateindex, position_t newPosition[]) {

	printf("checkBlocked");
	for (int i=0; i<candidateindex; i++) {
		printf("candidate: %d : %d\n", candidate[i].x, candidate[i].y);
	}
	candidateindex /= 2;

	if(candidateindex == 0){
		newPosition[0].x = -1;
		newPosition[0].y = -1;
		newPosition[1].x = -1;
		newPosition[1].y = -1;
		return;
	}
	else if(candidateindex == 1){
		newPosition[0].x = candidate[0].x;
		newPosition[0].y = candidate[0].y;
		newPosition[1].x = -1;
		newPosition[1].y = -1;
		return;
	}
	else if (candidateindex == 2){
		newPosition[0].x = candidate[0].x;
		newPosition[0].y = candidate[0].y;
		newPosition[1].x = candidate[1].x;
		newPosition[1].y = candidate[1].y;
		return;
	}
	else if (candidateindex == 3){

		position_t a,b,c;
		for(int i=0; i<candidateindex; i++){
			for(int j=i+1; j<candidateindex; j++){
				a = candidate[i];
				b = candidate[j];
				c = candidate[3-i-j];
				board[a.y][a.x] = player_color;
				board[b.y][b.x] = player_color;

				if(checkNo6(c)){
					newPosition[0].x = a.x;
					newPosition[0].y = a.y;
					newPosition[1].x = b.x;
					newPosition[1].y = b.y;

					board[a.y][a.x] = EMPTY;
					board[b.y][b.x] = EMPTY;
					return;
				}
				else{
					board[a.y][a.x] = EMPTY;
					board[b.y][b.x] = EMPTY;
				}
			}
		}

		newPosition[0].x = -1;
		newPosition[0].y = -1;
		newPosition[1].x = -1;
		newPosition[1].y = -1;
		return;
	}
	else if (candidateindex == 4) {
		position_t a,b,c,d;
		for(int i=0; i<candidateindex; i++){
			for(int j=i+1; j<candidateindex; j++){
				a = candidate[i];
				b = candidate[j];
				for(int k=j+1; k<candidateindex; k++){
					c = candidate[k];
					d = candidate[6-i-j-k];
					board[a.y][a.x] = player_color;
					board[b.y][b.x] = player_color;

					if(checkNo6(c) && checkNo6(d)){
						newPosition[0].x = a.x;
						newPosition[0].y = a.y;
						newPosition[1].x = b.x;
						newPosition[1].y = b.y;

						board[a.y][a.x] = EMPTY;
						board[b.y][b.x] = EMPTY;
						return;
					}
					else{
						board[a.y][a.x] = EMPTY;
						board[b.y][b.x] = EMPTY;
					}
				}
			}
		}	
	}
	newPosition[0].x = -1;
	newPosition[0].y = -1;
	newPosition[1].x = -1;
	newPosition[1].y = -1;
	return ;
}

void
blockConnect6(position_t newPosition[], position_t oppsPosition[])
{
	if(oppsPosition[0].x == -1){
		printf("dadasd");
		return;
	}

	printf("blockConnect6");
	position_t dir[4] = {{0,1}, {1,1}, {1,0}, {1,-1}};
	position_t position[4][11];
	int window[4][11];

	position_t candidateBlocks[4];
	int candidateCount = 0;
	int candidateIndex = 0;

	for(int i=0; i<2; i++){
		
		int x = oppsPosition[i].x;
		int y = oppsPosition[i].y;

		printf("x: %d, y: %d\n", x, y);

		for(int j=0; j<4; j++){
			for(int k=-5; k<=5; k++){
				if(x-(dir[j].x)*k > 18 || y-(dir[j].y)*k > 18){
					window[j][k+5] = -1;
				}
				else{
					window[j][k+5] = board[y-(dir[j].y)*k][x-(dir[j].x)*k];	
				}
				position[j][k+5].x = x-(dir[j].x)*k;
				position[j][k+5].y = y-(dir[j].y)*k;
			}

			for(int k=0; k<6; k++){
				int check = 0;
				int empty_cnt = 0;
				int opps_stone = 0;
				position_t empty_position[2];
				position_t pos[2];
				for(int l=0; l<6; l++){
					// printf("widow: %d : %d\n", window[j][k+l], opponent_color);
					if(window[j][k+l] == -1){
						break;
					}
					else if(window[j][k+l] == 0){
						if(empty_cnt < 2){
							empty_position[empty_cnt].x = position[j][k+l].x;
							empty_position[empty_cnt].y = position[j][k+l].y;
							pos[empty_cnt].x = j;
							pos[empty_cnt].y = k+l; 
						}
						empty_cnt++;
						continue;
					}
					else if (window[j][k+l] == opponent_color){
						opps_stone++;
						// printf("opps_stone: %d\n", opps_stone);
					}
					else{
						check = 1;
						opps_stone = 0;
						break; 
					}					
				}
			
				if(opps_stone >= 4 && check == 0){
					if (candidateCount >= 2) {
						newPosition[0].x = -1; // There are no possible 6 connection
						newPosition[0].y = -1;
						newPosition[1].x = -1;
						newPosition[1].y = -1;
					}
		
					if(empty_cnt == 2){ // 6 connection exist with 2 empty space
						window[pos[0].x][pos[0].y] = CANDIDATE;
						window[pos[1].x][pos[1].y] = CANDIDATE;

						candidateBlocks[candidateIndex].x = empty_position[0].x;
						candidateBlocks[candidateIndex].y = empty_position[0].y;
						candidateIndex++;
						candidateBlocks[candidateIndex].x = empty_position[1].x;
						candidateBlocks[candidateIndex].y = empty_position[1].y;
						candidateIndex++;	

					}
					else{ // 6 connection exist with 1 empty space
						window[pos[0].x][pos[0].y] = CANDIDATE;

						candidateBlocks[candidateIndex].x = empty_position[0].x;
						candidateBlocks[candidateIndex].y = empty_position[0].y;
						candidateIndex++;
					}
					
					candidateCount++;
				}
			}
		}
	}

	checkBlocked(candidateBlocks, candidateIndex, newPosition);
	return;

}

char *
draw_and_read (char * draw)
{
	if (first_turn) {
		first_turn = 0 ;
		if (player_color == BLACK && (strcmp(draw, "K10") == 0 || strcmp(draw, "k10") == 0)) {
			board[9][9] = player_color ;
			if (send_msg(sock_fd, draw, strlen(draw)) != 0)
				return 0x0 ;
		} else if (player_color == WHITE && strcmp(draw, "") == 0) {
			bufptr = recv_msg(sock_fd) ;
			if (bufptr == 0x0)
				return 0x0 ;
			if (strcmp(bufptr, "K10") != 0 && strcmp(bufptr, "k10") != 0)
				return 0x0 ;
			board[9][9] = opponent_color ;

			return bufptr ;
		} else {
			send_err(sock_fd, draw, err_str[BADINPUT]) ;
		}
	} else {
		errcode_t err = update_board(draw, player_color) ;

		if (err != GOOD) {
			send_err(sock_fd, draw, err_str[err]) ;
		} else {
			strict_format(draw) ;
			if (send_msg(sock_fd, draw, strlen(draw)) != 0)
				return 0x0 ;
		}
	}

	bufptr = recv_msg(sock_fd) ;
	if (bufptr == 0x0)
		return 0x0 ;

	if (strcmp(bufptr, "WIN") != 0 && strcmp(bufptr, "LOSE") != 0 && strcmp(bufptr, "TIE") != 0) {
		update_board(bufptr, opponent_color) ; 
		strict_format(bufptr) ;
	}

	return bufptr ;
}

char
get_stone_at (char * position) {
	int hor = -1 ;
	int ver = -1 ;
	errcode_t err = parse(position, &hor, &ver) ;
	if (err != GOOD)
		return 'N' ;

	switch (board[ver][hor]) {
		case EMPTY : return 'E' ;
		case BLACK : return 'B' ;
		case WHITE : return 'W' ;
		case RED : return 'R' ; 
		default : return 0x0 ;
	}
}
/* API functions */
