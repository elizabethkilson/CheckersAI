var hp_enable_right_cl = function(e, h){
	if (h.holding == -1)
		return false;
	var pos = get_pos(e);
	if (pos == -1)
		return;
	h.move.push(pos);
	return false;
};

var hp_enable_left_cl = function(e, h){
	var pos = get_pos(e);
	if (pos == -1)
		return;
	if (h.holding == -1)
	{
		if ((g_occupied[pos])&&(g_player[pos]==h.side))
		{
			h.holding = pos;
			refresh_background(ctx, g_occupied, g_player, g_king, h.holding);
			h.move.push(pos);
		}
	}
	else
	{
		if (pos == h.holding)
		{
			h.move = [];
			h.holding = -1;
			refresh_background(ctx, g_occupied, g_player, g_king, h.holding);
			return;
		}
		h.move.push(pos);
		hp_move(h);
	}
};

var hp_move = function(h)
{
	if (h.legal == -1)
	{
		alert('data not received');
		setInterval(hp_move(h), 20);
		return;
	}
	if (h.legal.moves == -1)
	{
		alert('lose');
		return;
	}
	var attempt = h.move.join(',');
	for (var i = 0; i < h.legal.moves.length; i++)
	{
		if (attempt==h.legal.moves[i])
		{
			g_occupied = h.legal.o[i].split('').reverse();
			g_player = h.legal.p[i].split('').reverse();
			g_king = h.legal.k[i].split('').reverse();
			refresh_background(ctx, g_occupied, g_player, g_king, -1);
			hp_end(h);
			return;
		}
	}
	h.move = [];
	h.holding = -1;
	refresh_background(ctx, g_occupied, g_player, g_king, -1);
}

var hp_track_mouse = function(e, h)
{
	var x = e.pageX - canvas.offsetLeft;
	var y = e.pageY - canvas.offsetTop;
	h.mouse_loc = [x, y];
}

var hp_graphics = function(h)
{
	if (h.holding != -1)
	{
		if (!(g_occupied[h.holding]))
			return;
		var x = h.mouse_loc[0];
		var y = h.mouse_loc[1];
		
		if ((x > 0) & (x < canvas.width) & (y > 0) & (y < canvas.height))
		{
			ctx.putImageData(background, 0, 0);
			var c = g_player[h.holding];
			var k = g_king[h.holding];
			for (var i = 0; i < h.move.length; i++)
			{
				var j = h.move[i];
				var ix = (2*(j>>>3) - (j%2) + 1.5)*canvas.width/8;
				var iy = ((j%8) + 0.5)*canvas.width/8;
				draw_checkers_piece(ctx, c, k, ix, iy, canvas.width/40, 0);
			}
			if (h.show_lm)
			{
				var steps = h.legal.moves[h.show_lm - 1].split(',');
				for (var i = 0; i < steps.length; i++)
				{
					var j = parseInt(steps[i]);
					var ix = (2*(j>>>3) - (j%2) + 1.5)*canvas.width/8;
					var iy = ((j%8) + 0.5)*canvas.width/8;
					draw_spot(ctx, ix, iy, canvas.width/60);
				}
			}
			draw_checkers_piece(ctx, c, k, x, y, canvas.width/20, 0.1, 
				canvas.width, canvas.height);
		}
	}
	else if (h.show_lm)
	{
		if (h.holding == -1)
		{
			ctx.putImageData(background, 0, 0);
		}
		var steps = h.legal.moves[h.show_lm - 1].split(',');
		for (var i = 0; i < steps.length; i++)
		{
			var j = parseInt(steps[i]);
			var ix = (2*(j>>>3) - (j%2) + 1.5)*canvas.width/8;
			var iy = ((j%8) + 0.5)*canvas.width/8;
			draw_spot(ctx, ix, iy, canvas.width/60);
		}
	}
}

var hp_show_lm = function(h)
{
	if (h.legal == -1)
	{
		alert('data not received');
		setInterval(hp_show_lm(h), 20);
		return;
	}
	if (h.legal.moves == -1)
	{
		alert('lose');
		return;
	}
	var lm_button = document.getElementById('show');
	var next = document.getElementById('next_move');
	var prev = document.getElementById('prev_move');
	var label = document.getElementById('label');
	h.show_lm = 1;
	lm_button.hidden = true;
	next.hidden = false;
	prev.hidden = false;
	label.innerHTML = "Showing move " + h.show_lm + "/" + h.legal.moves.length;
	next.onclick = function()
	{
		h.show_lm = (h.show_lm)%h.legal.moves.length + 1;
		label.innerHTML = "Showing move " + h.show_lm+"/"+h.legal.moves.length;
	}
	prev.onclick = function()
	{
		h.show_lm=(h.show_lm + h.legal.moves.length -2)%h.legal.moves.length +1;
		label.innerHTML = "Showing move " + h.show_lm+"/"+h.legal.moves.length;
	}
}

var hp_begin = function(h)
{
	canvas.onclick = function(e){hp_enable_left_cl(e, h);};
	canvas.oncontextmenu = function(e){return hp_enable_right_cl(e, h);};
	document.getElementById('show').onclick = function(){hp_show_lm(h);};
	h.holding = -1;
	h.move = [];
	canvas.addEventListener('mousemove', function(e){hp_track_mouse(e, h);}, 
		false);
	h.animation = setInterval(function(){hp_graphics(h);}, 20);
	h.legal = -1;
	h.show_lm = 0;
	var tmp_o = g_occupied.join('');
	tmp_o = tmp_o.split('').reverse().join('');
	var tmp_p = g_player.join('');
	tmp_p = tmp_p.split('').reverse().join('');
	var tmp_k = g_king.join('');
	tmp_k = tmp_k.split('').reverse().join('');
	jQuery.ajax({
		type: "POST",
		data: {
			"type": "legalMoves",
			"occupied": tmp_o,
			"player": tmp_p,
			"king": tmp_k,
			"turn": h.side
		},
		success: function (data) {
			if (data == "NO MOVES")
			{
				h.legal.moves = -1;
				lose(h.side);
				return;
			}
			var legal = data.split(';');
			h.legal = {};
			h.legal.moves = legal[0].split(':');
			h.legal.o = legal[1].split(',');
			h.legal.p = legal[2].split(',');
			h.legal.k = legal[3].split(',');
		},
		error: function (xhr, ajaxOptions, thrownError) 
			{alert('error');}
	});
};

var hp_end = function(h)
{
	canvas.onclick = function(){;};
	canvas.oncontextmenu = function(){return false;};
	var lm_button = document.getElementById('show');
	var next = document.getElementById('next_move');
	var prev = document.getElementById('prev_move');
	var label = document.getElementById('label');
	label.innerHTML = "";
	lm_button.onclick = function(){;};
	next.onclick = function(){;};
	prev.onclick = function(){;};
	lm_button.hidden = false;
	next.hidden = true;
	prev.hidden = true;
	clearInterval(h.animation);
	canvas.removeEventListener('mousemove', function(e){hp_track_mouse(e, h);}, 
		false);
	switch_turns(h.side);
};

function human_player(side)
{
	this.side = side;
	this.holding = -1;
	this.beginMove = hp_begin;
}
