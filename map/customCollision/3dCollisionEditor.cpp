#include "3dCC.cpp";

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

	[position,mode:world,layer:19,y:dustPosY] int dustPosX;
	[hidden] int dustPosY;

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

	void on_level_end() 
	{
		int count = 0;
		for (uint i = 0; i < quadManager.quads.length(); i++) 
		{
			count += quadManager.quads[i].quad.GetDustCount();
		}
		MakeDust(count);
	}

	void MakeDust(int n) 
	{
		d2Math::Vector2 pos = d2Math::Vector2(dustPosX, dustPosY);
		int width = int(sqrt(n));
		if (sqrt(n) - width > 0.01) { width += 1; } //round up while not fucking up square cases
		int height = n/width;
		int lastRow = n - width*height;
		puts(width+ ", " + height + ", " + lastRow);

		tileinfo@ t = create_tileinfo();
		t.solid(true);
		t.sprite_set(2);
		t.sprite_tile(13);
		t.sprite_palette(1);
		t.set_dustblock(2);

		scene@ s = get_scene();

		for (int x = floor(-width/2.0); x < width/2; x += 1)
		{
			for (int y = floor(-height/2.0); y < height/2; y += 1) //stops 1 short 
			{
				s.set_tile(floor(pos.x/48+x), floor(pos.y/48+y), 19, t, false);
			}
		}
		for (int x = floor(-width/2.0); x < floor(-width/2.0) + lastRow; x += 1) 
		{
			s.set_tile(floor(pos.x/48)+x, floor(pos.y/48)+int(height/2), 19, t, false);
		}
	}
};
