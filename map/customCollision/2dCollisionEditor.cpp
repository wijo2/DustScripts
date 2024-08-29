#include "2dCC.cpp";

class script : script_base
{
	QuadManager@ quadManager;
	input_api@ input;
	editor_api@ editor;
	[text] int collisionOrder;
	[text] bool showPlayArea;
	[text] bool showCacheDebug;
	
	array<d2Math::Rect> debugDraw;

	script() 
	{
		puts("2dCollisionEditor working c:");
		if (@quadManager == null) 
		{
			@quadManager = @QuadManager(this, d2Math::IntRect(-2000, -2000, uint(2000), uint(2000)));
		}
		@input = @get_input_api();
		@editor = @get_editor_api();
	}
	
	void on_editor_start() 
	{
		quadManager.manager.collisionOrder = collisionOrder;
	}

	void on_level_start()
	{
		quadManager.manager.collisionOrder = collisionOrder;
		quadManager.manager.PlayInit(this);
	}

	void editor_step()
	{
		if (input.mouse_state() & 0x20 != 0 
			&& editor.editor_tab() == "Triggers"
			&& @editor.get_selected_trigger() == null)
		{
			d2Math::Vector2 mousePos = d2Math::Vector2();
			mousePos.x = input.mouse_x_world(21);
			mousePos.y = input.mouse_y_world(21);
			for (uint i = 0; i < quadManager.quads.length(); i++)
			{
				if (quadManager.quads[i].quad.base.IsInside(mousePos))
				{
					editor.set_selected_trigger(
						quadManager.quads[i].self.as_entity()
					);
					break;
				}
			}
		}
	}
	
	void step(int idc) 
	{
		debugDraw = array<d2Math::Rect>(0);
	}
	
	void editor_draw(float lolxd) 
	{
		if (showPlayArea) 
		{
			quadManager.manager.playArea.Draw(get_scene(), 22, 1);
		}
		if (showCacheDebug) 
		{
			quadManager.manager.Draw(get_scene(), 22, 1);
		}
	}
	
	void draw(float idkAnymore) 
	{
		for (uint i = 0; i < debugDraw.length(); i++) 
		{
			debugDraw[i].Draw(get_scene(), 22, 1);
		}
		if (showPlayArea) 
		{
			quadManager.manager.playArea.Draw(get_scene(), 22, 1);
		}
		if (showCacheDebug) 
		{
			quadManager.manager.Draw(get_scene(), 22, 1);
		}
	}
}

class QuadManager 
{
	d2::CollisionManager@ manager;
	array<QuadEntity> quads;
	script@ s;

	QuadManager() { @manager = @d2::CollisionManager(d2Math::IntRect()); }
	QuadManager(script@ s, d2Math::IntRect field) 
	{
		@manager = @d2::CollisionManager(field);
		@this.s = @s;
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
	[hidden] d2::d2CQuad@ quad;

	[colour,alpha] uint colour;

	scripttrigger@ self;

	void init(script@ s, scripttrigger@ self)
	{
		if (colour == 0x00000000)
		{
			colour = 0xFFFFFFFF;
		}
		if (p1x == 0 && p1y == 0 &&
			p2x == 0 && p2y == 0&&
			p3x == 0 && p3y == 0&&
			p4x == 0 && p4y == 0) 
		{
			p1x = self.x() - 48;
			p1y = self.y() - 48;
			p2x = self.x() + 48;
			p2y = self.y() - 48;
			p3x = self.x() + 48;
			p3y = self.y() + 48;
			p4x = self.x() - 48;
			p4y = self.y() + 48;
		}
		@this.self = @self;
		@this.quadManager = @s.quadManager; 
		@quad = @d2::d2CQuad(
		d2Math::Vector2(p1x, p1y),
		d2Math::Vector2(p2x, p2y),
		d2Math::Vector2(p3x, p3y),
		d2Math::Vector2(p4x, p4y),
		colour,
		quadManager.manager);
		if (quadManager.quads.findByRef(this) < 0)
		{
			quadManager.quads.insertLast(this);
		}
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
		if (@quad == null) { return; }
		quad.Draw(get_scene(), layer, sub_layer);
	}

	void draw(float doublefuck)
	{
		if (@quad == null) { return; }
		quad.Draw(get_scene(), layer, sub_layer);
	}

	void UpdateSelf()
	{
		quad.base.colour = colour; 
		quad.base.p1 = d2Math::Vector2(p1x, p1y);
		quad.base.p2 = d2Math::Vector2(p2x, p2y);
		quad.base.p3 = d2Math::Vector2(p3x, p3y);
		quad.base.p4 = d2Math::Vector2(p4x, p4y);
		quad.UpdateCollision();
		d2Math::Vector2 centre = quad.FindCentre();
		self.set_centre(centre.x, centre.y);
	}
}
