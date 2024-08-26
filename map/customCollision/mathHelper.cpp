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
}

class LineFunc
{
	float k = 0;
	float b = 0;
	bool upright = false;
	float upx = 0;

	LineFunc() { k = 1; b = 0; }
	LineFunc(float k, float b) { this.k = k; this.b = b; }
	LineFunc(Vector2 p1, Vector2 p2) 
	{ 
		if (p1.x - p2.x != 0)
		{
			k = (p1.y - p2.y)/(p1.x - p2.x); b = p1.y - k*p1.x; 
		}
		else
		{
			upright = true;
			upx = p1.x;
		}
	}

	float GetValue(float x) 
	{ 
		if (!upright) 
		{
			return this.k * x + b; 
		}
		return upx;
	}

	float GetRevValue(float y)
	{
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
		uint increment = 2 ** precision;

		//flat case
		if (abs(k) < 1)
		{
			int dir = start.x < end.x ? 1 : -1;
			for (float x = start.x; dir * x - 2 ** precision <= dir * end.x; x += dir * 2 ** precision)
			{
				float y = GetValue(x);
				int rx = int(x);
				int ry = int(y);
				//rounding magic
				rx = (rx >> precision) << precision;
				ry = (ry >> precision) << precision;
				result.insertLast(Vector2(rx, ry));
			}
		}

		//vertical case
		else
		{
			int dir = start.y < end.y ? 1 : -1;
			for (float y = start.y; dir * y - 2 ** precision <= dir * end.y; y += dir * 2 ** precision)
			{
				float x = GetRevValue(y);
				int rx = int(x);
				int ry = int(y);
				//rounding magic
				rx = (rx >> precision) << precision;
				ry = (ry >> precision) << precision;
				result.insertLast(Vector2(rx, ry));
			}
		}

		return result;
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

}
