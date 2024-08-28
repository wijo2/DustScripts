#include "2dCC.cpp";

class script 
{
	[text] QuadManager@ manager;

	script() 
	{
		puts("2dCollisionEditor working c:");
		if (@manager == null) 
		{
			@manager = @QuadManager(d2Math::IntRect(-2000, -2000, uint(2000), uint(2000)));
		}
	}
}

class QuadManager 
{
	[hidden] d2::CollisionManager@ manager;
	[text] array<QuadEntity> quads;

	QuadManager() { @manager = @d2::CollisionManager(d2Math::IntRect()); }
	QuadManager(d2Math::IntRect field) 
	{
		@manager = @d2::CollisionManager(field);
	}
}

class QuadEntity : trigger_base 
{
	[position,mode:world,layer:19,y:p1y] float p1x;
	[hidden] float p1y;
	[position,mode:world,layer:19,y:p2y] float p2x;
	[hidden] float p2y;
	[position,mode:world,layer:19,y:p3y] float p3x;
	[hidden] float p3y;
	[position,mode:world,layer:19,y:p4y] float p4x;
	[hidden] float p4y;

	QuadManager@ quadManager;
	d2::d2CQuad@ quad;

	QuadEntity() {}

	QuadEntity(QuadManager@ manager, uint colour) 
	{ 
		@this.quadManager = @manager; 
		@quad = @d2::d2CQuad(
		d2Math::Vector2(p1x, p1y),
		d2Math::Vector2(p2x, p2y),
		d2Math::Vector2(p3x, p3y),
		d2Math::Vector2(p4x, p4y),
		colour,
		manager.manager);
	}
}
