#include <time.h>
#include <math.h>
#define WIN 8192
#define INF 16384
#define TIMEOUT 32768
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
     
     
int heuristic(uint32_t occupied, uint32_t player, uint32_t king, int side,\
	int k_val, int p_val, int prop_val)
{
	int p = 0;
	int pp = 0;
	int pk = 0;
	int ppos = 0;
	uint32_t ppl = 0;
	uint32_t pkl = 0;
	int o = 0;
	int op = 0;
	int ok = 0;
	int opos = 0;
	uint32_t opl = 0;
	uint32_t okl = 0;
	
	int i;
	int j;
	uint64_t pweights;
	uint64_t oweights;
	uint64_t kweights = 0xF00000000000000F;
	if (side==0)
	{
		pweights = 0xFBBBE517E447EEEF;
		//pweights = 18139343814941732591;
		oweights = 0xFBBBD11BD45BEEEF;
		//oweights = 18139321841621921519;
	}
	else
	{
		pweights = 0xFBBBD11BD45BEEEF;
		//pweights = 18139321841621921519;
		oweights = 0xFBBBE517E447EEEF;
		//oweights = 18139343814941732591;
	}
	for (i = 0; i < 32; i++)
	{
		if ((occupied>>i)%2)
		{
			if (((player>>i)%2)==side)
			{
				if ((king>>i)%2)
				{
					p+=k_val;
					pk++;
					pkl += (1<<i);
					ppos += (kweights>>(2*i))%4;
				}
				else
				{
					p+=p_val;
					ppos += (pweights>>(2*i))%4;
				}
				pp++;
				ppl += (1<<i);
			}
			else
			{
				if ((king>>i)%2)
				{
					o+=k_val;
					ok++;
					okl += (1<<i);
					opos += (kweights>>(2*i))%4;
				}
				else
				{
					o+=p_val;
					opos += (oweights>>(2*i))%4;
				}
				op++;
				opl += (1<<i);
			}
		}
	}
	if (op == 0)
	{
		return WIN;
	}
	if (pp == 0)
	{
		return -1*WIN;
	}
	double pkdist = 0;
	int vd;
	int hd;
	if (pkl > 0)
	{
		for (i = 0; i < 32; i++)
		{
			if ((pkl>>i)%2)
			{
				for (j = 0; j < 32; j++)
				{
					if ((opl>>j)%2)
					{
						vd = abs((i%8) - (j%8));
						hd = 2*abs((i>>3) - (j>>3));
						pkdist += max(vd, hd)/op;
					}
				}
			}
		}
	}
	double okdist = 0;
	if (okl > 0)
	{
		for (i = 0; i < 32; i++)
		{
			if ((okl>>i)%2)
			{
				for (j = 0; j < 32; j++)
				{
					if ((ppl>>j)%2)
					{
						vd = abs((i%8) - (j%8));
						hd = 2*abs((i>>3) - (j>>3));
						okdist += max(vd, hd)/op;
					}
				}
			}
		}
	}
	double prop;
	if (side==1)
	{
		prop = ((double)pp)/op;
	}
	else
	{
		prop = ((double)op)/pp;
	}
	if (pp < op)
	{
		prop = -1*prop;
	}
	else if (pp==op)
	{
		prop = 0;
	}
	int propi = (int) round(prop_val*prop);
	int ret = p - o + propi;
	ret += ppos - opos;
	//printf("%f %f %i\n", pk*pkdist, ok*okdist, (int)round(ok*okdist - pk*pkdist));
	ret += (int) round(ok*okdist - pk*pkdist);
	return ret;
}

int negemax_value(uint32_t occupied, uint32_t player, uint32_t king, int side,\
	int a, int B, int depth, int max_depth, \
	struct timespec t_offset, float max_t,\
	int k_val, int p_val, int prop_val)
{
	//printf("depth = %i side = %i sign = %i\n", depth, side, sign);
	struct timespec tim;
	clock_gettime(CLOCK_REALTIME, &tim);
	float t1 = ((float)(tim.tv_nsec - t_offset.tv_nsec))/1000000000;
	t1 += (tim.tv_sec - t_offset.tv_sec);
	if (t1 > max_t)
	{
		return TIMEOUT;
	}
	if (depth >= max_depth)
	{
		//printf("d = %i h = %i s = %i\t", depth, heuristic(occupied, player, king, side, k_val, p_val, prop_val), side);
		int ret = heuristic(occupied, player, king, side, k_val, p_val, prop_val);
		if ((ret < -1*INF)||(ret > INF))
		{
			printf("ERROR h %i d %i o %u p %u k %u\n\n\n\n", ret, depth, occupied, player, king);
		}
		return -1*ret;
	}
	int8_t moves[12*MAXJP][MAXCJ];
	uint32_t result[12*MAXJP][3];
	bool m = fill_legal_moves(occupied, player, king, side, moves, result, -1);
	if (!m)
	{
		return WIN;
	}
	int v = -1*INF;
	int s = 1 - side;
	int i;
	for (i = 0; i < 12*MAXJP; i++)
	{
		if (moves[i][0]==-1)
		{
			break;
		}
		int tmp = negemax_value(result[i][0], result[i][1], result[i][2], s, \
			-1*B, -1*a, depth+1, max_depth, t_offset, max_t, \
			k_val, p_val, prop_val);
		if (tmp == TIMEOUT)
		{
			return TIMEOUT;
		}
		v = max(v, tmp);
		if (v > B)
		{
			//printf("\nd = %i B r = %i\n", depth, sign*v);
			return -1*v;
		}
		a = max(a, v);
	}
	//printf("d %i o %u p %u k %u s %i sc %i si %i\n", depth, occupied, player, king, side, v, sign);
	//printf("\nd = %i r = %i\n", depth, sign*v);
	return -1*v;
}

char* aBsearch(uint32_t occupied, uint32_t player, uint32_t king, int side,\
	float time, int k_val, int p_val, int prop_val)
{
	struct timespec t_offset;
	clock_gettime(CLOCK_REALTIME, &t_offset);
	int8_t moves[12*MAXJP][MAXCJ];
	uint32_t result[12*MAXJP][3];
	bool m = fill_legal_moves(occupied, player, king, side, moves, result, -1);
	char* answer;
	answer = malloc(MAXANSWERSIZE);
	bool finished = false;
	int completed = -1;
	if (!m)
	{
		snprintf(answer, MAXANSWERSIZE, "NO MOVES");
		return answer;
	}
	if (moves[1][0]==-1)
	{
		completed = 0;
		finished = true;
	}
	int current_score = -1*INF;
	int score = 0;
	int current_ind = -1;
	int depth = 1;
	int s = 1 - side;
	int a;
	int B;
	float max_t = 0.9*time;
	int i;
	int ties = 1;
	while (!finished)
	{
		completed = current_ind;
		score = current_score;
		if (current_score == WIN)
		{
			finished = true;
			break;
		}
		a = -1*INF;
		B = INF;
		current_score = -1*INF;
		current_ind = -1;
		for (i = 0; i < 12*MAXJP; i++)
		{
			//printf("i = %i \n", i);
			if (moves[i][0]==-1)
			{
				break;
			}
			int tmp = negemax_value(result[i][0], result[i][1], result[i][2],\
				s, -1*B, -1*a, 1, depth, t_offset, max_t, \
				k_val, p_val, prop_val);
			//printf("score = %i\n", tmp);
			if (tmp == TIMEOUT)
			{
				finished = true;
				break;
			}
			if (tmp == current_score)
			{
				ties++;
				if (((rand())%ties)==0)
				{
					current_ind = i;
				}
			}
			if (tmp > current_score)
			{
				current_score = tmp;
				current_ind = i;
				ties = 1;
			}
			a = max(a, tmp);
		}
		//printf("depth = %i\n", depth);
		//printf("score = %i ind = %i\n\n\n\n", current_score, current_ind);
		if (current_score == -1*WIN)
		{
			finished = true;
		}
		depth++;
	}
	printf("depth = %i\n", depth - 1);
	printf("score = %i\n", score);
	struct timespec tim;
	clock_gettime(CLOCK_REALTIME, &tim);
	float t1 = ((float)(tim.tv_nsec - t_offset.tv_nsec))/1000000000;
	t1 += (tim.tv_sec - t_offset.tv_sec);
	printf("time = %f\n\n", t1);
	sprintf(answer, "");
	int j;
	char tmp[20];
	for(j = 0; j < MAXCJ; j++)
	{
		if (moves[completed][j]==-1)
		{
			break;
		}
		if (j>0)
		{
			strcat(answer, ",");
		}
		sprintf(tmp, "%i", moves[completed][j]);
		strcat(answer, tmp);
	}
	for (j = 0; j < 3; j++)
	{
		strcat(answer, ";");
		strcat(answer, int2bin(result[completed][j], NULL));
	}
	return answer;
}
