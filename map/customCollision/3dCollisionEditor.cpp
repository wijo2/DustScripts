#include "3dCC.cpp";
//DustScripts/map/customCollision/3dCollisionEditor.cpp

class script : script_base
{
	d3::d3Manager@ manager;
	input_api@ input;
	editor_api@ editor;

	[text] int collisionOrder = 4;
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

	[slider,min:0,max:6.2] float rotation;

	bool dontGrabCorner = false;

	//debug
	array<d2Math::Rect> debugDraw;
	[position,mode:world,layer:19,y:debugY] int debugX;
	[hidden] int debugY;

	script() 
	{
		puts("3dCollisionEditor working c:");
		if (@manager == null) 
		{
			@manager = @d3::d3Manager();
		}
		@input = @get_input_api();
		@editor = @get_editor_api();
	}

	void on_editor_start() 
	{
		manager.manager.collisionOrder = collisionOrder;
		manager.manager.Init(d2Math::IntRect(d2Math::Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));
	}

	void PlayInit()
	{
		manager.manager.collisionOrder = collisionOrder;
		manager.manager.PlayInit(this, d2Math::IntRect(d2Math::Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));	
	}

	void on_level_start() { PlayInit(); }
	void checkpoint_load() { PlayInit(); }

	void editor_step()
	{
		// dontGrabCorner = false;
		// if (input.mouse_state() & 0x20 != 0 
		// 	&& editor.editor_tab() == "Triggers"
		// 	&& @editor.get_selected_trigger() == null)
		// {
		// 	d2Math::Vector2 mousePos = d2Math::Vector2();
		// 	mousePos.x = input.mouse_x_world(21);
		// 	mousePos.y = input.mouse_y_world(21);
		// 	for (uint i = 0; i < quadManager.quads.length(); i++)
		// 	{
		// 		if (quadManager.quads[i].quad.base.IsInside(mousePos))
		// 		{
		// 			editor.set_selected_trigger(
		// 				quadManager.quads[i].self.as_entity()
		// 			);
		// 			dontGrabCorner = true;
		// 			break;
		// 		}
		// 	}
		// }
		manager.cam.rotation = rotation;
		manager.UpdateLooks();
	}

	void step(int idc) 
	{
		debugDraw = array<d2Math::Rect>(0);
		manager.manager.step();
	}

	void editor_draw(float lolxd) 
	{
		if (showPlayArea) 
		{
			manager.manager.playArea.Draw(get_scene(), 22, 1);
		}
		if (showCacheDebug) 
		{
			manager.manager.Draw(get_scene(), 22, 1);
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
			manager.manager.playArea.Draw(sc, 22, 1);
		}
		if (showCacheDebug) 
		{
			manager.manager.Draw(sc, 22, 1);
		}
	}

	// void on_level_end() 
	// {
	// 	int count = 0;
	// 	for (uint i = 0; i < manager.allQuads.length(); i++) 
	// 	{
	// 		count += quadManager.quads[i].quad.GetDustCount();
	// 	}
	// 	MakeDust(count);
	// }

	// void MakeDust(int n) 
	// {
	// 	d2Math::Vector2 pos = d2Math::Vector2(dustPosX, dustPosY);
	// 	int width = int(sqrt(n));
	// 	if (sqrt(n) - width > 0.01) { width += 1; } //round up while not fucking up square cases
	// 	int height = n/width;
	// 	int lastRow = n - width*height;
	// 	puts(width+ ", " + height + ", " + lastRow);
	//
	// 	tileinfo@ t = create_tileinfo();
	// 	t.solid(true);
	// 	t.sprite_set(2);
	// 	t.sprite_tile(13);
	// 	t.sprite_palette(1);
	// 	t.set_dustblock(2);
	//
	// 	scene@ s = get_scene();
	//
	// 	for (int x = floor(-width/2.0); x < width/2; x += 1)
	// 	{
	// 		for (int y = floor(-height/2.0); y < height/2; y += 1) //stops 1 short 
	// 		{
	// 			s.set_tile(floor(pos.x/48+x), floor(pos.y/48+y), 19, t, false);
	// 		}
	// 	}
	// 	for (int x = floor(-width/2.0); x < floor(-width/2.0) + lastRow; x += 1) 
	// 	{
	// 		s.set_tile(floor(pos.x/48)+x, floor(pos.y/48)+int(height/2), 19, t, false);
	// 	}
	// }
}

class d3QuadEntity : trigger_base 
{
	[text] int layer;
	[text] int sub_layer;
	[colour,alpha] uint d2colour;
	[colour,alpha] uint d3colour;

	d3::d3Manager@ manager;
	d3::d3CQuad@ quad;

	scripttrigger@ self;
	script@ script;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@this.self = @self;
		if (d2colour == 0x00000000)
		{
			d2colour = 0xFFFFFFFF;
		}
		if (d3colour == 0x00000000)
		{
			d3colour = 0xFFFFFFFF;
		}
		// oldCentre = d2Math::Vector2(self.x(), self.y());
		@this.manager = @s.manager; 
		@quad = @d3::d3CQuad();
		@quad.base = @d3::d3Quad(
			d3Math::Vector3(self.x()-48, self.y()-48, 0),
			d3Math::Vector3(self.x()+48, self.y()-48, 0),
			d3Math::Vector3(self.x(), self.y()+48, 48),
			d3Math::Vector3(self.x(), self.y(), 48),
			d3colour);
		quad.collisionBase.base.colour = d2colour;
		quad.activeSides[0] = true;
		quad.activeSides[1] = true;
		quad.activeSides[2] = true;
		quad.activeSides[3] = true;
		if (s.manager.allQuads.findByRef(quad) < 0)
		{
			s.manager.allQuads.insertLast(quad);
		}
		if (layer == 0)
		{
			layer = 21;
		}
		if (sub_layer == 0)
		{
			sub_layer = 1;
		}
		quad.base.ApplyProjection(manager.cam);
		// quad.UpdateIntersectQuad(script.manager.cam);
		UpdateSelf();
		// UpdateSides();
	}
	//
	void editor_var_changed(var_info@ info)
	{
		UpdateSelf();
		// UpdateSides();
		// quad.UpdateCollision();
	}

	void UpdateRotation()
	{
		quad.base.ApplyProjection(manager.cam);
		quad.UpdateIntersectQuad(script.manager.cam);
	}

	void editor_draw(float fuck)
	{
		if (@quad == null) { return; }
		quad.Draw(get_scene(), layer, sub_layer);
	}
	//
	// void draw(float doublefuck)
	// {
	// 	if (@quad == null) { return; }
	// 	quad.Draw(get_scene(), layer, sub_layer);
	// }
	//
	void UpdateSelf()
	{
		// quad.base.colour = colour; 
		// quad.base.p1 = d2Math::Vector2(p1x, p1y);
		// quad.base.p2 = d2Math::Vector2(p2x, p2y);
		// quad.base.p3 = d2Math::Vector2(p3x, p3y);
		// quad.base.p4 = d2Math::Vector2(p4x, p4y);
		// quad.UpdateCollision();
		// d2Math::Vector2 centre = quad.base.FindCentre();
		// oldCentre = centre;
		// self.set_centre(centre.x, centre.y);
		quad.base.colour = d3colour;
		quad.collisionBase.base.colour = d2colour;
	}
	//
	// //seperating since this should only happen at start of level
	// void UpdateSides()
	// {
	// 	quad.activeLines[0] = side1active;
	// 	quad.activeLines[1] = side2active;
	// 	quad.activeLines[2] = side3active;
	// 	quad.activeLines[3] = side4active;
	// 	quad.dustLines[0] = side1dust;
	// 	quad.dustLines[1] = side2dust;
	// 	quad.dustLines[2] = side3dust;
	// 	quad.dustLines[3] = side4dust;
	// 	quad.spikeLines[0] = side1spikes;
	// 	quad.spikeLines[1] = side2spikes;
	// 	quad.spikeLines[2] = side3spikes;
	// 	quad.spikeLines[3] = side4spikes;
	// }
	//
	// //corner drag
	// void editor_step()
	// {
	// 	scene@ s = get_scene();
	//
	// 	d2Math::Vector2 mousePosWorld = d2Math::Vector2(s.mouse_x_world(0,20), s.mouse_y_world(0,20));
	// 	d2Math::Vector2 mousePosHud = d2Math::Vector2(s.mouse_x_hud(0), s.mouse_y_hud(0));
	//
	// 	if (script.input.mouse_state() & 0x20 != 0 
	// 		&& script.editor.editor_tab() == "Triggers"
	// 		&& @script.editor.get_selected_trigger() != null
 //   && script.editor.get_selected_trigger().is_same(self.as_entity())
	// 		&& !script.dontGrabCorner) 
	// 	{
	//
	// 		if (selectedCorner == 0) 
	// 		{
	// 			array<d2Math::Vector2> corners = { quad.base.p1, quad.base.p2, quad.base.p3, quad.base.p4 };
	// 			for (uint i = 0; i < corners.length(); i++)
	// 			{
	// 				d2Math::Vector2 pos = d2Math::WorldToScreenPos(d2Math::Vector2(corners[i].x, corners[i].y));
	// 				if (pos.Distance(mousePosHud) < 50) 
	// 				{
	// 					selectedCorner = i + 1;
	// 					return;
	// 				}
	// 			}
	// 		}
	// 		else 
	// 		{
	// 			switch(selectedCorner) 
	// 			{
	// 				case 1:
	// 					p1x = mousePosWorld.x;
	// 					p1y = mousePosWorld.y;
	// 				break;
	// 				case 2:
	// 					p2x = mousePosWorld.x;
	// 					p2y = mousePosWorld.y;
	// 				break;
	// 				case 3:
	// 					p3x = mousePosWorld.x;
	// 					p3y = mousePosWorld.y;
	// 				break;
	// 				case 4:
	// 					p4x = mousePosWorld.x;
	// 					p4y = mousePosWorld.y;
	// 				break;
	// 			}
	// 			selectedCorner = 0;
	// 			UpdateSelf();
	// 		}
	// 		return;
	// 	}
	//
	// 	if (script.editor.editor_tab() == "Triggers"
	// 		&& @script.editor.get_selected_trigger() != null
 //   && script.editor.get_selected_trigger().is_same(self.as_entity())
	// 		&& selectedCorner != 0)
	// 	{
	// 		switch(selectedCorner) 
	// 		{
	// 			case 1:
	// 				p1x = mousePosWorld.x;
	// 				p1y = mousePosWorld.y;
	// 			break;
	// 			case 2:
	// 				p2x = mousePosWorld.x;
	// 				p2y = mousePosWorld.y;
	// 			break;
	// 			case 3:
	// 				p3x = mousePosWorld.x;
	// 				p3y = mousePosWorld.y;
	// 			break;
	// 			case 4:
	// 				p4x = mousePosWorld.x;
	// 				p4y = mousePosWorld.y;
	// 			break;
	// 		}
	// 		UpdateSelf();
	// 	}
	//
	// 	d2Math::Vector2 curCen = d2Math::Vector2(self.x(), self.y());
	// 	d2Math::Vector2 dif = oldCentre - curCen;
	// 	if (dif.Magnitude() > 0.1 && selectedCorner == 0)
	// 	{
	// 		p1x -= dif.x;
	// 		p1y -= dif.y;
	// 		p2x -= dif.x;
	// 		p2y -= dif.y;
	// 		p3x -= dif.x;
	// 		p3y -= dif.y;
	// 		p4x -= dif.x;
	// 		p4y -= dif.y;
	// 		UpdateSelf();
	// 		oldCentre = quad.base.FindCentre();
	// 	}
	// }
	//
	// void on_remove()
	// {
	// 	quad.ClearOldCache();
	// 	int i = quadManager.quads.findByRef(this);
	// 	if (i >= 0)
	// 	{
	// 		quadManager.quads.removeAt(i);
	// 	}
	// }
};
