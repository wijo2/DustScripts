#include "2dCC.cpp";

class script 
{
	[label:"manager"] QuadManager@ quadManager;

	script() 
	{
		puts("2dCollisionEditor working c:");
		if (@quadManager == null) 
		{
			@quadManager = @QuadManager(d2Math::IntRect(-1000, -1000, uint(1000), uint(1000)));
		}
	}

	void draw(float sub_frame)
	{
		puts("why tho?");
		for (uint i = 0; i < quadManager.quads.length(); i++)
		{
			quadManager.quads[i].draw(sub_frame);
		}
	}
}

class QuadManager 
{
	[label:"manager"] d2::CollisionManager@ manager;
	[label:"quads"] array<QuadEntity> quads;

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

	[text] int layer;
	[text] int sub_layer;

	[hidden] QuadManager@ quadManager;
	[text] d2::d2CQuad@ quad;

	scripttrigger@ self;

	void init(script@ s, scripttrigger@ self)
	{
		@this.self = @self;
		@this.quadManager = @s.quadManager; 
		@quad = @d2::d2CQuad(
		d2Math::Vector2(p1x, p1y),
		d2Math::Vector2(p2x, p2y),
		d2Math::Vector2(p3x, p3y),
		d2Math::Vector2(p4x, p4y),
		0xFFFFFFFF,
		quadManager.manager);
		if (layer == 0)
		{
			layer = 21;
		}
		if (sub_layer == 0)
		{
			sub_layer = 1;
		}
	}

	void editor_var_changed(var_info@ info)
	{
		UpdateSelf();
	}

	void editor_draw(float fuck)
	{
		quad.Draw(get_scene(), layer, sub_layer);
		// manager.Draw(get_scene(), 22, 1);
	}

	void draw(float doublefuck)
	{
		puts("draw?");
		quad.Draw(get_scene(), layer, sub_layer);
	}

	void UpdateSelf()
	{
		quad.base.p1 = d2Math::Vector2(p1x, p1y);
		quad.base.p2 = d2Math::Vector2(p2x, p2y);
		quad.base.p3 = d2Math::Vector2(p3x, p3y);
		quad.base.p4 = d2Math::Vector2(p4x, p4y);
		quad.UpdateCollision();
		d2Math::Vector2 centre = quad.FindCentre();
		self.set_centre(centre.x, centre.y);
	}
}
