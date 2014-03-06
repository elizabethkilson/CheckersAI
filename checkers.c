#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define MAXCJ 10
#define MAXJP 15
#define MAXANSWERSIZE 2048


int find_jumps_piece(uint32_t, uint32_t, uint32_t, uint8_t, int8_t[][MAXCJ], \
	uint32_t[][3], int, int);

char *int2bin(uint32_t n, char *buf)
{
    static char static_buf[33];
    int i;

    if (buf == NULL)
        buf = static_buf;

    for (i = 32; i > 0; --i) {
        buf[i-1] = (n & 1) ? '1' : '0';
        n >>= 1;
    }

    buf[32] = '\0';
    return buf;
}

int abs(int value) {
  if (value < 0) {
    return -1*value;
  }
  else {
    return value;  
  }
}

void move(uint32_t occupied, uint32_t player, uint32_t king, uint8_t piece, \
	int8_t moves[][MAXCJ], uint32_t result[][3], int i, int e, int k, int kr,\
	int turn)
{
	moves[i][0] = piece;
	moves[i][1] = e;
	moves[i][2] = -1;
	uint32_t t_occupied = occupied - (1<<piece) + (1<<e);
	uint32_t t_player = player - (turn<<piece) + (turn<<e);
	uint32_t t_king = king - (((king>>piece)%2)<<piece) + \
		(((king>>piece)%2)<<e);
	if ((!k)&&((e%8)==kr))
	{
		t_king = t_king + (1<<e);
	}
	result[i][0] = t_occupied;
	result[i][1] = t_player;
	result[i][2] = t_king;
}

int find_moves_piece(uint32_t occupied, uint32_t player, uint32_t king,\
	uint8_t piece, int8_t moves[][MAXCJ], uint32_t result[][3], int i)
{
	if (!((occupied>>piece)%2))
		return 0;
	int ret = 0;
	int turn = (player>>piece)%2;
	int sign = 1 - 2*turn;
	int k = (king>>piece)%2;
	int kr = turn*7;
	if ((turn^(piece%2))||(k==1))
	{
		if (k==1)
		{
			sign = 2*(piece%2) -1;
		}
		int e = piece - sign*9;
		int vd = abs((piece%8) - (e%8));
		if ((e>=0)&&(e<32)&&(vd<2))
		{
			if (!((occupied>>e)%2))
			{
				move(occupied, player, king, piece, moves, result, i, e, \
					k, kr, turn);
				i++;
				ret++;
			}
		}
		e = piece - sign;
		vd = abs((piece%8) - (e%8));
		if ((e>=0)&&(e<32)&&(vd<2))
		{
			if (!((occupied>>e)%2))
			{
				move(occupied, player, king, piece, moves, result, i, e, \
					k, kr, turn);
				i++;
				ret++;
			}
		}
	}
	if ((!(turn^(piece%2)))||(k==1))
	{
		if (k==1)
		{
			sign = 1 - 2*(piece%2);
		}
		int e = piece - sign;
		int vd = abs((piece%8) - (e%8));
		if ((e>=0)&&(e<32)&&(vd<2))
		{
			if (!((occupied>>e)%2))
			{
				move(occupied, player, king, piece, moves, result, i, e, \
					k, kr, turn);
				i++;
				ret++;
			}
		}
		e = piece + sign*7;
		vd = abs((piece%8) - (e%8));
		if ((e>=0)&&(e<32)&&(vd<2))
		{
			if (!((occupied>>e)%2))
			{
				move(occupied, player, king, piece, moves, result, i, e, \
					k, kr, turn);
				i++;
				ret++;
			}
		}
	}
	moves[i][0] = -1;
	return ret;
}

int jump(uint32_t occupied, uint32_t player, uint32_t king, uint8_t piece, \
	int e, int o, int8_t jumps[][MAXCJ], uint32_t result[][3],\
	int i, int ti, int j, int turn, int k, int kr)
{
	if (((!((occupied>>e)%2)))&&\
		((occupied>>o)%2)&&\
		(turn != ((player>>o)%2)))
	{
		jumps[ti][j] = piece;
		if (ti > i)
		{
			int l;
			for (l = 0; l < j; l++)
			{
				jumps[ti][l] = jumps[i][l];
			}
		}
		uint32_t t_occupied = occupied - (1<<piece) - (1<<o) + (1<<e);
		uint32_t t_player = player - (turn<<piece) - ((1 - turn)<<o)\
			+ (turn<<e);
		uint32_t t_king = king - (k<<piece) - (((king>>o)%2)<<o) + (k<<e);
		if ((!k)&&((e%8)==kr))
		{
			t_king = t_king + (1<<e);
			jumps[ti][j+1] = e;
			jumps[ti][j+2] = -1;
			jumps[ti+1][0] = -1;
			result[ti][0] = t_occupied;
			result[ti][1] = t_player;
			result[ti][2] = t_king;
			return 1;
		}
		else
		{
			int incr = find_jumps_piece(t_occupied, t_player, t_king,\
				e, jumps, result, ti, j+1);
			return incr;
		}
	}
	return 0;
}

int find_jumps_piece(uint32_t occupied, uint32_t player, uint32_t king,\
	uint8_t piece, int8_t jumps[][MAXCJ], uint32_t result[][3], int i, int j)
{
	if (!((occupied>>piece)%2))
		return 0;
	int ret = 0;
	int ti = i;
	int turn = (player>>piece)%2;
	int sign = 1 - 2*turn;
	int k = (king>>piece)%2;
	int kr = turn*7;
	if ((turn^(piece%2))||(k==1))
	{
		if (k==1)
		{
			sign = 2*(piece%2) -1;
		}
		int e = piece - sign*10;
		int o = piece - sign*9;
		int vd = abs((piece%8) - (e%8));
		if ((e>=0)&&(e<32)&&(vd<3))
		{
			int incr = jump(occupied,player, king, piece, e, o, \
				jumps, result, i, ti, j, turn, k, kr);
			ti += incr;
			ret += incr;
		}
		e = piece + sign*6;
		o = piece - sign;
		vd = abs((piece%8) - (e%8));
		if ((e>=0)&&(e<32)&&(o>=0)&&(o<32)&&(vd<3))
		{
			int incr = jump(occupied,player, king, piece, e, o, \
				jumps, result, i, ti, j, turn, k, kr);
			ti += incr;
			ret += incr;
		}
	}
	if ((!(turn^(piece%2)))||(k==1))
	{
		if (k==1)
		{
			sign = 1 - 2*(piece%2);
		}
		int e = piece - sign*10;
		int o = piece - sign;
		int vd = abs((piece%8) - (e%8));
		if ((e>=0)&&(e<32)&&(vd<3))
		{
			int incr = jump(occupied,player, king, piece, e, o, \
				jumps, result, i, ti, j, turn, k, kr);
			ti += incr;
			ret += incr;
		}
		e = piece + sign*6;
		o = piece + sign*7;
		vd = abs((piece%8) - (e%8));
		if ((e>=0)&&(e<32)&&(o>=0)&&(o<32)&&(vd<3))
		{
			int incr = jump(occupied,player, king, piece, e, o, \
				jumps, result, i, ti, j, turn, k, kr);
			ti += incr;
			ret += incr;
		}
	}
	if (ti==i)
	{
		jumps[i][j] = piece;
		jumps[i][j+1] = -1;
		jumps[i+1][0] = -1;
		result[i][0] = occupied;
		result[i][1] = player;
		result[i][2] = king;
		if (j > 0)
			ret++;
	}
	return ret;
}

bool find_moves(uint32_t occupied, uint32_t player, uint32_t king,\
	uint8_t turn, int8_t moves[][MAXCJ], uint32_t result[][3])
{
	int ret = false;
	int i = 0;
	uint8_t k;
	for (k = 0; k < 32; k++)
	{
		if (((occupied>>k)%2)&&(turn==((player>>k)%2)))
		{
			int incr = find_moves_piece(occupied, player, king, k, moves,\
				result, i);
			if (incr > 0)
			{
				ret = true;
				i = i + incr;
			}
		}
	}
	return ret;
}

bool find_jumps(uint32_t occupied, uint32_t player, uint32_t king,\
	uint8_t turn, int8_t jumps[][MAXCJ], uint32_t result[][3], int init_val)
{
	int ret = false;
	int i = 0;
	uint8_t k;
	for (k = 0; k < 32; k++)
	{
		if (((occupied>>k)%2)&&(turn==((player>>k)%2)))
		{
			int incr = find_jumps_piece(occupied, player, king, k, jumps,\
				result, i, 0);
			if ((incr==0)&&(jumps[i][1]==init_val))
			{
				jumps[i][0] = init_val;
			}
			else
			{
				ret = true;
				i = i + incr;
			}
		}
	}
	return ret;
}

bool fill_legal_moves(uint32_t occupied, uint32_t player, uint32_t king,\
	uint8_t turn, int8_t moves[][MAXCJ], uint32_t result[][3], int init_val)
{
	bool f = find_jumps(occupied, player, king, turn, moves, result, init_val);
	if (f)
		return f;
	f = find_moves(occupied, player, king, turn, moves, result);
	return f;
}

char *legal_moves_string(uint32_t occupied, uint32_t player, uint32_t king,\
	uint8_t turn)
{
	int8_t moves[12*MAXJP][MAXCJ];
	uint32_t result[12*MAXJP][3];
	char* answer;
	answer = malloc(MAXANSWERSIZE);
	sprintf(answer, "");
	bool m = fill_legal_moves(occupied, player, king, turn, moves, result, -1);
	if (!m)
	{
		snprintf(answer, MAXANSWERSIZE, "NO MOVES");
		return answer;
	}
	int num_moves = 0;
	int i;
	int j;
	char tmp[20];
	for (i = 0; i < 12*MAXJP; i++)
	{
		if (moves[i][0]==-1)
		{
			break;
		}
		if (i > 0)
		{
			strcat(answer, ":");
		}
		num_moves++;
		for(j = 0; j < MAXCJ; j++)
		{
			if (moves[i][j]==-1)
			{
				break;
			}
			if (j>0)
			{
				strcat(answer, ",");
			}
			sprintf(tmp, "%i", moves[i][j]);
			strcat(answer, tmp);
		}
	}
	for (i = 0; i < 3; i++)
	{
		strcat(answer, ";");
		for (j = 0; j < num_moves; j++)
		{
			if (j>0)
			{
				strcat(answer, ",");
			}
			//sprintf(tmp, "%s", int2bin(result[j][i], NULL));
			strcat(answer, int2bin(result[j][i], NULL));
		}
	}
	return answer;
}
