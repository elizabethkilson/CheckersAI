var cp_begin = function(c)
{
	var tmp_o = g_occupied.join('');
	tmp_o = tmp_o.split('').reverse().join('');
	var tmp_p = g_player.join('');
	tmp_p = tmp_p.split('').reverse().join('');
	var tmp_k = g_king.join('');
	tmp_k = tmp_k.split('').reverse().join('');
	jQuery.ajax({
		type: "POST",
		data: {
			"type": "compMove",
			"occupied": tmp_o,
			"player": tmp_p,
			"king": tmp_k,
			"turn": c.side,
			"time": c.time,
			"k_val": c.k_val,
			"p_val": c.p_val,
			"prop_val": c.prop_val
		},
		success: function (data) {
			if (data == "NO MOVES")
			{
				lose(c.side);
				return;
			}
			var move = data.split(';');
			g_occupied = move[1].split('').reverse();
			g_player = move[2].split('').reverse();
			g_king = move[3].split('').reverse();
			refresh_background(ctx, g_occupied, g_player, g_king, -1);
			cp_end(c);
		},
		error: function (xhr, ajaxOptions, thrownError) 
			{;}
	});
};

var cp_end = function(c)
{
	switch_turns(c.side);
};

function computer_player(side, time, k_val, p_val, prop_val)
{
	this.side = side;
	this.time = time;
	this.beginMove = cp_begin;
	this.k_val = k_val;
	this.p_val = p_val;
	this.prop_val = prop_val;
}
