var initialize = function()
{
	select_players();
};

function select_players()
{
	var page = document.getElementById('p0');
	page.hidden = false;
	var pl1 = document.getElementById('pl1');
	var pl2 = document.getElementById('pl2');
	var next = document.getElementById('next');
	next.onclick = function()
	{
		if (pl1.value == 'h')
		{
			player0 = new human_player(0);
		}
		else
		{
			player0 = new computer_player(0, 0.1, 22, 10, 40);
		}
		if (pl2.value == 'h')
		{
			player1 = new human_player(1);
		}
		else
		{
			player1 = new computer_player(1, 0.1, 22, 10, 40);
		}
		page.hidden = true;
		start_pos();
	}
}

function start_pos()
{
	var page = document.getElementById('p1');
	page.hidden = false;
	var play = document.getElementById('play');
	var set = document.getElementById('set');
	play.onclick = function()
	{
		g_occupied = ['1', '1', '1', '0', '0', '1', '1', '1',
					'1', '1', '1', '0', '0', '1', '1', '1',
					'1', '1', '1', '0', '0', '1', '1', '1',
					'1', '1', '1', '0', '0', '1', '1', '1'];
		g_player = ['1', '1', '1', '0', '0', '0', '0', '0',
					'1', '1', '1', '0', '0', '0', '0', '0',
					'1', '1', '1', '0', '0', '0', '0', '0',
					'1', '1', '1', '0', '0', '0', '0', '0'];
		g_king = ['0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0'];
		page.hidden = true;
		start_game(0);
	}
	set.onclick = function()
	{
		page.hidden = true;
		set_pos();
	}
}

function get_pos(e)
{
	var x = e.pageX - canvas.offsetLeft;
	var y = e.pageY - canvas.offsetTop;
	var ly = Math.floor(8*y/canvas.height);
	var lx = Math.floor(8*x/canvas.width);
	if ((ly>7)|(lx>7)|(ly<0)|(lx<0))
		return -1;
	if (0==((lx + ly)%2))
		return -1;
	lx = Math.floor(lx/2);
	var pos = (lx<<3) + ly;
	return pos;
}

function set_left_cl(e, ctx)
{
	var pos = get_pos(e);
	if (pos == -1)
		return;
	if (g_occupied[pos] == '1')
	{
		if (g_king[pos] == '1')
		{
			g_occupied[pos] = '0';
			g_king[pos] = '0';
		}
		else
		{
			g_king[pos] = '1';
		}
	}
	else
	{
		g_occupied[pos] = '1';
	}
	refresh_background(ctx, g_occupied, g_player, g_king, -1);
}

function set_right_cl(e, ctx)
{
	var pos = get_pos(e);
	if (pos == -1)
		return false;
	if (g_occupied[pos] == '1')
	{
		if (g_king[pos] == '1')
		{
			g_occupied[pos] = '0';
			g_king[pos] = '0';
			g_player[pos] = '0';
		}
		else
		{
			g_king[pos] = '1';
		}
	}
	else
	{
		g_occupied[pos] = '1';
		g_player[pos] = '1';
	}
	refresh_background(ctx, g_occupied, g_player, g_king, -1);
	return false;
}

function set_pos()
{
	var page = document.getElementById('p2');
	page.hidden = false;
	g_occupied = ['0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0'];
	g_player = ['0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0'];
	g_king = ['0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0',
					'0', '0', '0', '0', '0', '0', '0', '0'];
	canvas = document.getElementById('set_board');
    var ctx = canvas.getContext('2d');
    refresh_background(ctx, g_occupied, g_player, g_king, -1);
    canvas.onclick = function(e){set_left_cl(e, ctx);};
    canvas.oncontextmenu = function(e){return set_right_cl(e, ctx);};
    var fplayer = document.getElementById('set_turn');
    var begin = document.getElementById('finish_set');
    begin.onclick = function()
    {
    	canvas.onclick = function(){;};
    	canvas.oncontextmenu = function(){return false;};
    	page.hidden = true;
    	start_game(fplayer.value);
    };
}

function start_game(first_player)
{
	var page = document.getElementById('p3');
	page.hidden = false;
	canvas = document.getElementById('board');
    ctx = canvas.getContext('2d');
    refresh_background(ctx, g_occupied, g_player, g_king, -1);
    if (first_player == 0)
    	player0.beginMove(player0);
    else
    	player1.beginMove(player1);
}

function switch_turns(side)
{
	if (side == 0)
	{
		player1.beginMove(player1);
	}
	else
	{
		player0.beginMove(player0);
	}
}

function lose(player)
{
	var winner = 2 - player;
	var string = "PLAYER " + winner + " WINS!";
	ctx.fillStyle = "rgba(0, 20, 200, 0.7)";
	ctx.font = 36 + "px sans-serif";
	ctx.textAlign = "center";
	ctx.fillText(string, canvas.width/2, canvas.height/2);
	canvas.onclick = function()
	{
		;
	};
}
