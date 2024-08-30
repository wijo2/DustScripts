//WIP!
namespace d2Math
{

class Vector2
{
	float x;
	float y;

	Vector2()
	{
		x = 0;
		y = 0;
	}

	Vector2(float x, float y)
	{
		this.x = x;
		this.y = y;
	}


	float Distance(Vector2 o)
	{
		return sqrt((x-o.x)**2 + (y-o.y)**2);
	}

	Vector2 opNeg()
	{
		return Vector2(-x, -y);
	}
	Vector2 opAdd(Vector2 o)
	{
		return Vector2(x + o.x, y + o.y);
	}
	Vector2 opSub(Vector2 o)
	{
		return Vector2(x - o.x, y - o.y);
	}
	Vector2 opMul(float o)
	{
		return Vector2(o*x, o*y);
	}
	Vector2 opMul_r(float o) { return opMul(o); }
	Vector2 opDiv(float o)
	{
		return Vector2(x/o, y/o);
	}
	string opImplConv()
	{
		return "(" + formatFloat(x, "", 0, 3) + ", " + formatFloat(y, "", 0, 3) + ")";

	}
	bool opEquals(Vector2 o)
	{
		return x == o.x && y == o.y;
	}
}

class LineFunc
{
	float k = 0;
	float b = 0;
	bool upright = false;
	float upx = 0;
	bool nullLine = false;

	Vector2 bound1 = Vector2();
	Vector2 bound2 = Vector2();

	LineFunc() { k = 1; b = 0; }
	LineFunc(float k, float b) { this.k = k; this.b = b; }
	LineFunc(Vector2 p1, Vector2 p2) 
	{ 
		if (p1 == p2)
		{
			nullLine = true;
		}
		else if (p1.x - p2.x == 0)
		{
			upright = true;
			upx = p1.x;
			k = 99999;
		}
		else
		{
			k = (p1.y - p2.y)/(p1.x - p2.x); b = p1.y - k*p1.x; 
		}
	}

	float GetValue(float x) 
	{ 
		if (nullLine) { return 0; }
		if (!upright) 
		{
			return this.k * x + b; 
		}
		return upx;
	}

	float GetRevValue(float y)
	{
		if (nullLine) { return 0; }
		if (upright) { return upx; }
		if (k == 0)
		{
			return b;
		}
		return (y - b) / k;
	}

	//make list of points on line in increments of 2 ^ precision.
	//if the line is more horisontal it will go 2 ^ precision units x - wise each time and vice versa. 
	array<Vector2> IterateOverLine(uint precision, Vector2 start, Vector2 end)
	{
		array<Vector2> result;
		if (nullLine) { return result; }
		uint increment = 1 << precision;

		bool firstLoop = true;
		int lrx = 0;
		int lry = 0;
		//flat case
		if (abs(k) < 1)
		{
			int dir = start.x < end.x ? 1 : -1;
			for (float x = start.x - dir*increment; dir * x - increment <= dir * end.x; x += dir * increment)
			{
				float y = GetValue(x);
				int rx = int(x);
				int ry = int(y);
				//this is a bunch of rounding bs compensation, if you're reading this and this works
				//don't bother trying to make it better it's way spaghettier than you think
				rx = RoundToOrder(rx, precision) - increment;
				ry = RoundToOrder(ry, precision) - increment;

				if (firstLoop) 
				{
					firstLoop = false;
					lry = ry;
				}
				if (ry != lry) 
				{
					result.insertLast(Vector2(rx, lry));
					result.insertLast(Vector2(lrx, ry));
					lry = ry;
				}
				lrx = rx;
				result.insertLast(Vector2(rx, ry));
			}
		}

		//vertical case
		else
		{
			int dir = start.y < end.y ? 1 : -1;
			for (float y = start.y - dir*increment; dir * y - increment <= dir * end.y; y += dir * increment)
			{
				float x = GetRevValue(y);
				int rx = int(x);
				int ry = int(y);
				//this is a bunch of rounding bs compensation, if you're reading this and this works
				//don't bother trying to make it better it's way spaghettier than you think
				rx = RoundToOrder(rx, precision) - increment;
				ry = RoundToOrder(ry, precision) - increment;

				if (firstLoop) 
				{
					firstLoop = false;
					lrx = rx;
				}
				if (rx != lrx) 
				{
					result.insertLast(Vector2(rx, lry));
					result.insertLast(Vector2(lrx, ry));
					lrx = rx;
				}
				lry = ry;
				result.insertLast(Vector2(rx, ry));
			}
		}

		return result;
	}

	void SetBounds(Vector2 b1, Vector2 b2)
	{
		if (abs(k) > 1)
		{
			if (b2.y > b1.y)
			{
				bound1 = b1;
				bound2 = b2;
			}
			else
			{
				bound1 = b2;
				bound2 = b1;
			}
		}
		else
		{
			if (b2.x > b1.x)
			{
				bound1 = b1;
				bound2 = b2;
			}
			else
			{
				bound1 = b2;
				bound2 = b1;
			}
		}
	}

	bool IsWithinBounds(Vector2 pos)
	{
		if (abs(k) > 1)
		{
			return pos.y > bound1.y && pos.y < bound2.y;
		}
		else
		{
			return pos.x > bound1.x && pos.x < bound2.x;
		}
	}
}

class Rect
{
	float x1;
	float y1;
	float x2;
	float y2;
	Vector2 p1;
	Vector2 p2;
	float width;
	float height;

	Rect()
	{
		x1 = 0;
		x2 = 0;
		y1 = 0;
		y2 = 0;
		p1 = Vector2();
		p2 = Vector2();
		width = 0;
		height = 0;
	}
	Rect(float x1, float y1, float x2, float y2)
	{
		this.x1 = x1;
		this.y1 = y1;
		this.x2 = x2;
		this.y2 = y2;
		this.p1 = Vector2(x1,y1);
		this.p2 = Vector2(x2,y2);
		this.width = abs(x1-x2);
		this.height = abs(y1-y2);
	}

	Rect(Vector2 pos, float width, float height)
	{
		this.x1 = pos.x;
		this.y1 = pos.y;
		this.x2 = x1 + width;
		this.y2 = y1 + height;
		this.p1 = Vector2(x1,y1);
		this.p2 = Vector2(x2,y2);
		this.width = width;
		this.height = height;
	}

	Rect(Vector2 p1, Vector2 p2)
	{
		this.x1 = p1.x;
		this.y1 = p1.y;
		this.x2 = p2.x;
		this.y2 = p2.y;
		this.p1 = p1;
		this.p2 = p2;
		this.width = abs(x1-x2);
		this.height = abs(y1-y2);
	}

	bool CheckLineIntersection(LineFunc line)
	{
		float lx1 = line.GetValue(x1);	
		float lx2 = line.GetValue(x2);	
		float ly1 = line.GetRevValue(y1);	
		float ly2 = line.GetRevValue(y2);	

		return
			y1 < lx1 && lx1 < y2 ||
			y1 < lx2 && lx2 < y2 ||
			x1 < ly1 && ly1 < x2 ||
			x1 < ly2 && ly2 < x2;
	}

	Vector2 LineIntersectionPosition(LineFunc line)
	{
		float lx1 = line.GetValue(x1);	
		if (!line.upright && y1 < lx1 && lx1 < y2) { return Vector2(x1, lx1); }
		float lx2 = line.GetValue(x2);	
		if (!line.upright && y1 < lx2 && lx2 < y2) { return Vector2(x2, lx2); }
		float ly1 = line.GetRevValue(y1);	
		if (x1 < ly1 && ly1 < x2) { return Vector2(ly1, y1); }
		float ly2 = line.GetRevValue(y2);	
		if (x1 < ly2 && ly2 < x2) { return Vector2(ly2, y2); }
		return Vector2();
	}

	bool BoundedIntersectionPosition(LineFunc line, Vector2 &out pos)
	{
		array<Vector2> touches;

		float lx1 = line.GetValue(x1);	
		float lx2 = line.GetValue(x2);	
		float ly1 = line.GetRevValue(y1);	
		float ly2 = line.GetRevValue(y2);	
		if (!line.upright && y1 < lx1 && lx1 < y2) { touches.insertLast(Vector2(x1, lx1)); }
		if (!line.upright && y1 < lx2 && lx2 < y2) { touches.insertLast(Vector2(x2, lx2)); }
		if (!(line.k == 0) && x1 < ly1 && ly1 < x2) { touches.insertLast(Vector2(ly1, y1)); }
		if (!(line.k == 0) && x1 < ly2 && ly2 < x2) { touches.insertLast(Vector2(ly2, y2)); }

		for (uint i = 0; i < touches.length(); i++)
		{
			if (line.IsWithinBounds(touches[i])) 
			{ 
				pos = touches[i]; 
				return true;
			}
		}
		return false;
	}

	void Draw(scene@ s, int layer, int sub_layer)
	{
		s.draw_rectangle_world(layer, sub_layer, 
						 x1,y1,x2,y2,0,0xFF3333FF);
	}
}

class IntRect
{
	int x1;
	int y1;
	int x2;
	int y2;
	Vector2 p1;
	Vector2 p2;
	uint width;
	uint height;

	IntRect()
	{
		x1 = 0;
		x2 = 0;
		y1 = 0;
		y2 = 0;
		p1 = Vector2();
		p2 = Vector2();
		width = 0;
		height = 0;
	}
	IntRect(int x1, int y1, int x2, int y2)
	{
		this.x1 = x1;
		this.y1 = y1;
		this.x2 = x2;
		this.y2 = y2;
		this.p1 = Vector2(x1,y1);
		this.p2 = Vector2(x2,y2);
		this.width = abs(x1-x2);
		this.height = abs(y1-y2);
	}

	IntRect(Vector2 pos, uint width, uint height)
	{
		this.x1 = int(pos.x);
		this.y1 = int(pos.y);
		this.x2 = x1 + width;
		this.y2 = y1 + height;
		this.p1 = Vector2(x1,y1);
		this.p2 = Vector2(x2,y2);
		this.width = width;
		this.height = height;
	}

	IntRect(Vector2 p1, Vector2 p2)
	{
		this.x1 = int(p1.x);
		this.y1 = int(p1.y);
		this.x2 = int(p2.x);
		this.y2 = int(p2.y);
		this.p1 = p1;
		this.p2 = p2;
		this.width = abs(x1-x2);
		this.height = abs(y1-y2);
	}

	bool CheckLineIntersection(LineFunc line)
	{
		float lx1 = line.GetValue(x1);	
		float lx2 = line.GetValue(x2);	
		float ly1 = line.GetRevValue(y1);	
		float ly2 = line.GetRevValue(y2);	

		return
			y1 < lx1 && lx1 < y2 ||
			y1 < lx2 && lx2 < y2 ||
			x1 < ly1 && ly1 < x2 ||
			x1 < ly2 && ly2 < x2;
	}

	Vector2 LineIntersectionPosition(LineFunc line)
	{
		float lx1 = line.GetValue(x1);	
		if (!line.upright && y1 < lx1 && lx1 < y2) { return Vector2(x1, lx1); }
		float lx2 = line.GetValue(x2);	
		if (!line.upright && y1 < lx2 && lx2 < y2) { return Vector2(x2, lx2); }
		float ly1 = line.GetRevValue(y1);	
		if (x1 < ly1 && ly1 < x2) { return Vector2(ly1, y1); }
		float ly2 = line.GetRevValue(y2);	
		if (x1 < ly2 && ly2 < x2) { return Vector2(ly2, y2); }
		return Vector2();
	}

	bool BoundedIntersectionPosition(LineFunc line, Vector2 &out pos)
	{
		array<Vector2> touches;

		float lx1 = line.GetValue(x1);	
		float lx2 = line.GetValue(x2);	
		float ly1 = line.GetRevValue(y1);	
		float ly2 = line.GetRevValue(y2);	
		if (!line.upright && y1 < lx1 && lx1 < y2) { touches.insertLast(Vector2(x1, lx1)); }
		if (!line.upright && y1 < lx2 && lx2 < y2) { touches.insertLast(Vector2(x2, lx2)); }
		if (!(line.k == 0) && x1 < ly1 && ly1 < x2) { touches.insertLast(Vector2(ly1, y1)); }
		if (!(line.k == 0) && x1 < ly2 && ly2 < x2) { touches.insertLast(Vector2(ly2, y2)); }

		for (uint i = 0; i < touches.length(); i++)
		{
			if (line.IsWithinBounds(touches[i])) 
			{ 
				pos = touches[i]; 
				return true;
			}
		}
		return false;
	}

	void Draw(scene@ s, int layer, int sub_layer)
	{
		s.draw_rectangle_world(layer, sub_layer, 
						 x1,y1,x2,y2,0,0xFF3333FF);
	}
}

int abs(int a)
{
	return a < 0 ? -a : a;
}
uint abs(uint a)
{
	return a < 0 ? -a : a;
}
float abs(float a)
{
	return a < 0 ? -a : a;
}

int sign(uint a)
{
	return a < 0 ? -1 : 1;
}
int sign(int a)
{
	return a < 0 ? -1 : 1;
}
int sign(float a)
{
	return a < 0 ? -1 : 1;
}

//ughhhhh
// int RoundToOrder(int a, uint o)
// {
// 	return sign(a) * ((abs(a) >> o) << o);
// }
// int RoundToOrder(int a, uint o)
// {
// 	return -sign(a) * ((-abs(a) >> o) << o);
// }
// int RoundToOrder(int a, uint o)
// {
// 	return (a >> o) << o;
// }
int RoundToOrder(int a, uint o)
{
	return -((-a >> o) << o);
}

}
