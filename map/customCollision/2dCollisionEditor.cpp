#include "2dCC.cpp";

class script : script_base
{
	QuadManager@ quadManager;
	input_api@ input;
	editor_api@ editor;

	[text] int collisionOrder = 7;
	[text] bool showPlayArea;
	[text] bool showCacheDebug;

	[position,mode:world,layer:19,y:playAreaCornerY] int playAreaCornerX;
	[hidden] int playAreaCornerY;
	[text] int playAreaWidth;
	[text] int playAreaHeight;

	[colour,alpha] uint spikeColour;
	[colour,alpha] uint dustColour;

	[text] array<ControllableHolder> extraGuys;

	bool dontGrabCorner = false;

	//debug
	array<d2Math::Rect> debugDraw;
	[position,mode:world,layer:19,y:debugY] int debugX;
	[hidden] int debugY;

	script() 
	{
		puts("2dCollisionEditor working c:");
		if (@quadManager == null) 
		{
			@quadManager = @QuadManager(this);
		}
		@input = @get_input_api();
		@editor = @get_editor_api();
	}

	void on_editor_start() 
	{
		quadManager.manager.collisionOrder = collisionOrder;
		quadManager.manager.Init(d2Math::IntRect(d2Math::Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));
	}

	void PlayInit()
	{
		quadManager.manager.collisionOrder = collisionOrder;
		for (uint i = 0; i < extraGuys.length(); i++)
		{
			entity@ e = entity_by_id(extraGuys[i].entity);
			if (@e == null) { continue; }
			controllable@ c = e.as_controllable();
			if (@c == null) { continue; }
			quadManager.manager.additionalControllables.insertLast(c);
		}
		quadManager.manager.PlayInit(this, d2Math::IntRect(d2Math::Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));	
	}

	void on_level_start() { PlayInit(); }
	void checkpoint_load() { PlayInit(); }

	void editor_step()
	{
		dontGrabCorner = false;
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
					dontGrabCorner = true;
					break;
				}
			}
		}
	}

	void step(int idc) 
	{
		debugDraw = array<d2Math::Rect>(0);
		quadManager.step();
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
		scene@ sc = get_scene();
		for (uint i = 0; i < debugDraw.length(); i++) 
		{
			debugDraw[i].Draw(sc, 22, 1);
		}
		if (showPlayArea) 
		{
			quadManager.manager.playArea.Draw(sc, 22, 1);
		}
		if (showCacheDebug) 
		{
			quadManager.manager.Draw(sc, 22, 1);
		}
	}
};

class ControllableHolder
{
	[entity] int entity;
};

class QuadManager 
{
	d2::CollisionManager@ manager;
	array<QuadEntity@> quads;
	script@ s;

	QuadManager() { @manager = @d2::CollisionManager(); }
	QuadManager(script@ s) 
	{
		@manager = @d2::CollisionManager();
		@this.s = @s;
	}

	void step()
	{
		manager.step();
	}
};

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
	[colour,alpha] uint colour;

	[text] bool side1active = true;
	[text] bool side2active = true;
	[text] bool side3active = true;
	[text] bool side4active = true;
	[text] bool side1dust;
	[text] bool side2dust;
	[text] bool side3dust;
	[text] bool side4dust;
	[text] bool side1spikes;
	[text] bool side2spikes;
	[text] bool side3spikes;
	[text] bool side4spikes;
	
	int selectedCorner = 0;

	d2Math::Vector2 oldCentre;

	QuadManager@ quadManager;
	d2::d2CQuad@ quad;

	scripttrigger@ self;
	script@ script;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
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
		oldCentre = d2Math::Vector2(self.x(), self.y());
		@this.self = @self;
		@this.quadManager = @s.quadManager; 
		@quad = @d2::d2CQuad(
			d2Math::Vector2(p1x, p1y),
			d2Math::Vector2(p2x, p2y),
			d2Math::Vector2(p3x, p3y),
			d2Math::Vector2(p4x, p4y),
			colour,
			quadManager.manager);
		@quad.script = @quadManager.s;
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
		UpdateSelf();
		UpdateSides();
		quad.UpdateCollision();
	}

	void editor_var_changed(var_info@ info)
	{
		UpdateSelf();
		UpdateSides();
		quad.UpdateCollision();
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
		d2Math::Vector2 centre = quad.base.FindCentre();
		self.set_centre(centre.x, centre.y);
	}

	//seperating since this should only happen at start of level
	void UpdateSides()
	{
		quad.activeLines[0] = side1active;
		quad.activeLines[1] = side2active;
		quad.activeLines[2] = side3active;
		quad.activeLines[3] = side4active;
		quad.dustLines[0] = side1dust;
		quad.dustLines[1] = side2dust;
		quad.dustLines[2] = side3dust;
		quad.dustLines[3] = side4dust;
		quad.spikeLines[0] = side1spikes;
		quad.spikeLines[1] = side2spikes;
		quad.spikeLines[2] = side3spikes;
		quad.spikeLines[3] = side4spikes;
	}

	//corner drag
	void editor_step()
	{
		scene@ s = get_scene();

		d2Math::Vector2 mousePosWorld = d2Math::Vector2(s.mouse_x_world(0,20), s.mouse_y_world(0,20));
		d2Math::Vector2 mousePosHud = d2Math::Vector2(s.mouse_x_hud(0), s.mouse_y_hud(0));

		if (script.input.mouse_state() & 0x20 != 0 
			&& script.editor.editor_tab() == "Triggers"
			&& @script.editor.get_selected_trigger() != null
			&& script.editor.get_selected_trigger().is_same(self.as_entity())
			&& !script.dontGrabCorner) 
		{

			if (selectedCorner == 0) 
			{
				array<d2Math::Vector2> corners = { quad.base.p1, quad.base.p2, quad.base.p3, quad.base.p4 };
				for (uint i = 0; i < corners.length(); i++)
				{
					d2Math::Vector2 pos = d2Math::WorldToScreenPos(d2Math::Vector2(corners[i].x, corners[i].y));
					if (pos.Distance(mousePosHud) < 50) 
					{
						selectedCorner = i + 1;
						return;
					}
				}
			}
			else 
			{
				switch(selectedCorner) 
				{
					case 1:
						p1x = mousePosWorld.x;
						p1y = mousePosWorld.y;
					break;
					case 2:
						p2x = mousePosWorld.x;
						p2y = mousePosWorld.y;
					break;
					case 3:
						p3x = mousePosWorld.x;
						p3y = mousePosWorld.y;
					break;
					case 4:
						p4x = mousePosWorld.x;
						p4y = mousePosWorld.y;
					break;
				}
				selectedCorner = 0;
				UpdateSelf();
			}
			return;
		}

		if (script.editor.editor_tab() == "Triggers"
			&& @script.editor.get_selected_trigger() != null
			&& script.editor.get_selected_trigger().is_same(self.as_entity())
			&& selectedCorner != 0)
		{
			switch(selectedCorner) 
			{
				case 1:
					p1x = mousePosWorld.x;
					p1y = mousePosWorld.y;
				break;
				case 2:
					p2x = mousePosWorld.x;
					p2y = mousePosWorld.y;
				break;
				case 3:
					p3x = mousePosWorld.x;
					p3y = mousePosWorld.y;
				break;
				case 4:
					p4x = mousePosWorld.x;
					p4y = mousePosWorld.y;
				break;
			}
			UpdateSelf();
		}

		d2Math::Vector2 curCen = d2Math::Vector2(self.x(), self.y());
		d2Math::Vector2 dif = oldCentre - curCen;
		if (dif.Magnitude() > 0.1)
		{
			p1x -= dif.x;
			p1y -= dif.y;
			p2x -= dif.x;
			p2y -= dif.y;
			p3x -= dif.x;
			p3y -= dif.y;
			p4x -= dif.x;
			p4y -= dif.y;
			UpdateSelf();
			oldCentre = quad.base.FindCentre();
		}
	}

	void on_remove()
	{
		quad.ClearOldCache();
		int i = quadManager.quads.findByRef(this);
		if (i >= 0)
		{
			quadManager.quads.removeAt(i);
		}
	}
};
