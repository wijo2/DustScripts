#include "2dCC.cpp";

class script
{
	script()
	{
		puts("test works c:");
		puts(int(1.99999) + "");

	}
}

class TestQuad : enemy_base
{
	[position,mode:world,layer:19,y:p1y] float p1x;
	[hidden] float p1y;
	[position,mode:world,layer:19,y:p2y] float p2x;
	[hidden] float p2y;
	[position,mode:world,layer:19,y:p3y] float p3x;
	[hidden] float p3y;
	[position,mode:world,layer:19,y:p4y] float p4x;
	[hidden] float p4y;

	void editor_draw(float fuck)
	{
		d2::d2CQuad(
		d2Math::Vector2(p1x, p1y),
		d2Math::Vector2(p2x, p2y),
		d2Math::Vector2(p3x, p3y),
		d2Math::Vector2(p4x, p4y),
		0xFF3333FF,
		null).Draw(get_scene(), 22, 1);
	}
}
