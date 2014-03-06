function draw_board(ctx, size)
{
	var colors = ["rgb(255, 255, 255)", "rgb(20, 20, 20)"];
	
	for (var i = 0; i < 8; i++)
	{
		for (var j = 0; j < 8; j++)
		{
			ctx.fillStyle = colors[(i+j)%2];
			var x = i*size/8;
			var y = j*size/8;
			ctx.fillRect(x, y, size/8, size/8);
		}
	}
}

function draw_spot(ctx, cx, cy, r)
{
	ctx.fillStyle = "rgb(0, 0, 150)";
	ctx.beginPath();
	ctx.arc(cx, cy, r, 0, 2*Math.PI, false);
	ctx.closePath();
	ctx.fill();
}

function draw_checkers_piece(ctx, color, king, cx, cy, r, dz, lx, ly)
{
	if (dz > 0)
	{
		ctx.fillStyle = "rgba(0, 0, 0, 0.8)";
		var sx = (cx - lx/2)/(1-dz) + lx/2;
		var sy = (cy - ly/2)/(1-dz) + ly/2;
		
		ctx.beginPath();
		ctx.arc(sx, sy, r, 0, 2*Math.PI, false);
		ctx.closePath();
		ctx.fill();
	}
	
	var c1, c2;
	if (color == '0')
	{
		c1 = "rgb(200, 0, 0)";
		c2 = "rgb(255, 0, 0)";
	}
	else
	{
		c1 = "rgb(100, 100, 100)";
		c2 = "rgb(255, 255, 255)";
	}
	
	ctx.fillStyle = c1;
	ctx.beginPath();
	ctx.arc(cx, cy, r, 0, 2*Math.PI, false);
	ctx.closePath();
	ctx.fill();
	
	ctx.fillStyle = c2;
	ctx.beginPath();
	ctx.arc(cx, cy, 0.8*r, 0, 2*Math.PI, false);
	ctx.closePath();
	ctx.fill();
	
	if (king == '1')
	{
		ctx.strokeStyle = c1;
		var sr = 0.6*r;
		ctx.fillStyle = c1;
		ctx.beginPath();
		ctx.moveTo(cx, cy - sr);
		for (var i = 1; i <= 5; i++)
		{
			ctx.lineTo(cx + Math.cos(Math.PI*(1/2 + (2*i - 1)/5))*sr/2, cy - Math.sin(Math.PI*(1/2 + (2*i - 1)/5))*sr/2);
			ctx.lineTo(cx + Math.cos(Math.PI*(1/2 + 2*i/5))*sr, cy - Math.sin(Math.PI*(1/2 + 2*i/5))*sr);
		}
		ctx.closePath();
		ctx.fill()
	}
	else
	{
		ctx.strokeStyle = c1;
		for (var i = 0.2; i < 0.8; i = i + 0.2)
		{
			ctx.beginPath();
			ctx.arc(cx, cy, i*r, 0, 2*Math.PI, false);
			ctx.closePath();
			ctx.stroke();
		}
	}
}

function refresh_background(ctx, occupied, color, king, holding)
{
	size = Math.min(canvas.width, canvas.height);
	draw_board(ctx, size);
	for (var i = 0; i < 32; i++)
	{
		if (i == holding)
		{
			continue;
		}
		if (occupied[i]==1)
		{
			var x = (2*(i>>>3) - (i%2) + 1.5)*size/8;
			var y = ((i%8) + 0.5)*size/8;
			var c = color[i];
			var k = king[i];
			draw_checkers_piece(ctx, c, k, x, y, size/20, 0);
		}
	}
	background = ctx.getImageData(0, 0, canvas.width, canvas.height);
}
