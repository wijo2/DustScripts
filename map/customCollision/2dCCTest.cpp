#include "2dCC.cpp";

class script
{
	script()
	{
		puts("test works c:");
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

	[text] bool update;

	d2::CollisionManager@ manager;
	d2::d2CQuad@ quad;

	void init(script@ s, scriptenemy@ self)
	{
		@manager = @d2::CollisionManager(d2Math::IntRect(-1000, -1000, uint(2000), uint(2000)));
		@quad = @d2::d2CQuad(
		d2Math::Vector2(p1x, p1y),
		d2Math::Vector2(p2x, p2y),
		d2Math::Vector2(p3x, p3y),
		d2Math::Vector2(p4x, p4y),
		0xFF3333FF,
		manager);
	}

	void editor_draw(float fuck)
	{
		quad.Draw(get_scene(), 22, 1);
		//manager.Draw(get_scene(), 22, 1);
	}

	void draw(float doublefuck)
	{
		quad.Draw(get_scene(), 22, 1);
	}

	void editor_var_changed(var_info@ info)
	{
		quad.base.p1 = d2Math::Vector2(p1x, p1y);
		quad.base.p2 = d2Math::Vector2(p2x, p2y);
		quad.base.p3 = d2Math::Vector2(p3x, p3y);
		quad.base.p4 = d2Math::Vector2(p4x, p4y);
		quad.UpdateCollision();
	}
}
