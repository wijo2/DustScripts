#include "3dNodeCluster.cpp";
//DustScripts/map/customCollision/3dCollisionEditor.cpp

class script : script_base
{
	d3::d3Manager@ manager;
	input_api@ input;
	editor_api@ editor;

	[text] int collisionOrder = 8;
	[text] bool showPlayArea;
	[text] bool showCacheDebug;

	[position,mode:world,layer:19,y:playAreaCornerY] int playAreaCornerX;
	[hidden] int playAreaCornerY;
	[text] int playAreaWidth = 10000;
	[text] int playAreaHeight = 10000;

	[colour,alpha] uint spikeColour = 0xFFFF0000;
	[colour,alpha] uint dustColour = 0xFF00FF00;
	[colour,alpha] uint spikeColour3d = 0xCCFF0000;
	[colour,alpha] uint dustColour3d = 0xCC00FF00;
	[colour,alpha] uint edgeColour = 0x11000000;
	[colour,alpha] uint fogColour = 0x66000000;
	//more = fog is further
	[text] float fogDist = 500;
	[colour,alpha] uint default3dCol = 0xFFFFFFFF;
	[colour,alpha] uint default2dCol = 0xBB888888;

	[position,mode:world,layer:19,y:dustPosY] int dustPosX;
	[hidden] int dustPosY;

	Vector2 oldCamPos;
	[hidden] float rotation;
	float oldRotation;

	array<d3NodeCluster@> nodeClusters;
	array<FakeTrigger@> fakeTriggers;

	d3StartPos@ startPos;
	[hidden] Vector3 startCoords;

	Vector2 middleDragStart;
	float middleDragStartRot;
	[label:"middle mouse hotkeys"] bool middleDragEnable = true;

	bool wasRotating = false;
	//bools for up/down keys cause reasons
	bool right90 = false;
	bool left90 = false;
	bool wasGrounded;

	bool firstFrame = true;

	//warning light system
	float yellowTreshold = 8000;
	//interestingly there seems to be more leniency to the right, 13k is fine for right but
	//this is about the limit for left
	float redTreshold = 11500;
	//0 = green, 1 = yellow, 2 = red
	int currentDistCol = 0;

	[text] bool showCompass = true;
	[text] bool showCompassGame = true;
	[position,mode:world,layer:5,y:compassPosY|label:"compass pos"] float compassPosX;
	[hidden] float compassPosY;
	[slider,min:0,max:1000|label:"compass size"] float compassSize;

	[text|label:"join distance"] float joinDist = 40;

	//debug
	[text] bool quadDebug = false;
	[text] bool extraQuadDebug = false;
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
		manager.manager.Init(d2Math::IntRect(Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));
		@manager.script = @this;
		rotation = 0;
		oldRotation = 0;
		camera@ c = get_active_camera();
		manager.cam.igCoords = Vector2(c.x(), c.y());
		manager.cam.centre = Vector3(c.x(), c.y(), 0);
		manager.cam.centre = startCoords;
	}

	void PlayInit()
	{
		manager.manager.collisionOrder = collisionOrder;
		manager.manager.PlayInit(this, d2Math::IntRect(Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));	
		@manager.script = @this;
		rotation = 0;
		oldRotation = 0;
		controllable@ c = controller_controllable(uint(get_active_player()));
		manager.cam.igCoords = Vector2(c.x(), c.y());
		manager.cam.centre = Vector3(c.x(), c.y(), 0);
		get_active_camera().controller_mode(4);
		manager.cam.centre = startCoords;
	}

	void on_level_start() { PlayInit(); }
	void checkpoint_load() { PlayInit(); }

	void editor_step()
	{
		if (firstFrame)
		{
			firstFrame = false;
			UpdateRotation(true);
		}
		for(uint i = 0; i < fakeTriggers.length(); i++)
		{
			fakeTriggers[i].EditorStep();
		}

		camera@ cam = get_active_camera();
		if (abs(cam.x()) > yellowTreshold || abs(cam.y()) > yellowTreshold)
		{
			if (abs(cam.x()) > redTreshold || abs(cam.y()) > redTreshold)
			{
				currentDistCol = 2;
			}
			else
			{
				currentDistCol = 1;
			}
		}
		else
		{
			currentDistCol = 0;
		}

		for(uint i = 0; i < fakeTriggers.length(); i++)
		{
			fakeTriggers[i].EditorStep();
		}

		//fuck this game I fucking hate this why are you this shit fuck you
		if (input.key_check_pressed_vk(0x58) && input.key_check_gvb(11) && currentDistCol != 2)
		{
			oldCamPos = Vector2(0,0);
			cam.x(0);
			cam.y(0);
			manager.cam.igCoords = Vector2(0,0);
			UpdateRotation(true);
		}

		HandleMiddleCommands();
		UpdateCamPos();
		UpdateRotation();
		CentraliseTriggers();
	}

	uint ApplyFog(uint col, float dist)
	{
		if (dist == 0)
		{
			return col;
		}
		float ed = fogDist/dist;
		if (ed > 1  || ed < 0) { return col; }
		uint a = uint(((col & 0xFF000000) >> 24) * ed + ((fogColour & 0xFF000000) >> 24) * (1-ed));
		uint r = uint(((col & 0xFF0000) >> 16) * ed + ((fogColour & 0xFF0000) >> 16) * (1-ed));
		uint g = uint(((col & 0xFF00) >> 8) * ed + ((fogColour & 0xFF00) >> 8) * (1-ed));
		uint b = uint((col & 0xFF) * ed + (fogColour & 0xFF) * (1-ed));
		// puts("cols " + a + ", " + r + ", " + g + ", " + b);
		return ((a & 0xFF) << 24) + ((r & 0xFF) << 16) + ((g & 0xFF) << 8) + (b & 0xFF);
	}

	//everything with middle mouse
	void HandleMiddleCommands()
	{
		if (!middleDragEnable) { return; }
		if (input.mouse_state() & 0x80 != 0)
		{
			middleDragStartRot = rotation;
			middleDragStart = Vector2(input.mouse_x_hud(true), input.mouse_y_hud(true));
		}
		if (input.mouse_state() & 0x10 != 0)
		{
			rotation = middleDragStartRot + (input.mouse_x_hud(true) - middleDragStart.x) / 200;
		}
		if (input.key_check_gvb(10))
		{
			if (input.mouse_state() & 0x1 != 0)
			{
				manager.cam.centre = manager.cam.CamToWorldPos(
					manager.cam.WorldToCamPos(manager.cam.centre) + Vector3(0,0,24));
				UpdateRotation(true);
				manager.UpdateCollision();
			}
			if (input.mouse_state() & 0x2 != 0)
			{
				manager.cam.centre = manager.cam.CamToWorldPos(
					manager.cam.WorldToCamPos(manager.cam.centre) - Vector3(0,0,24));
				UpdateRotation(true);
				manager.UpdateCollision();
			}
		}
	}

	void step(int idc) 
	{
		if (firstFrame)
		{
			firstFrame = false;
			UpdateRotation(true);
			manager.UpdateCollision();
		}
		debugDraw = array<d2Math::Rect>(0);
		HandleGameplayRotation();
		manager.Step();
		UpdateCamPos();
		UpdateRotation();
		UpdatePlayArea();
		CentraliseTriggers();
	}

	void CentraliseTriggers()
	{
		camera@ cam = get_active_camera();
		Vector2 camPos = Vector2(cam.x(), cam.y());
		for(uint i = 0; i < fakeTriggers.length(); i++)
		{
			entity@ t = fakeTriggers[i].trigger;
			if (@t != null)
			{
				t.x(camPos.x);
				t.y(camPos.y);
			}
		}
	}

	void UpdatePlayArea()
	{
		controllable@ p = controller_controllable(uint(get_active_player()));
		if (@p == null) { return; }
		bool update = false;
		if (p.x() - playAreaCornerX < playAreaWidth/6)
		{
			playAreaCornerX -= playAreaWidth/2;
			update = true;
		}
		if (playAreaCornerX + playAreaWidth - p.x() < playAreaWidth/6)
		{
			playAreaCornerX += playAreaWidth/2;
			update = true;
		}
		if (p.y() - playAreaCornerY < playAreaHeight/6)
		{
			playAreaCornerY -= playAreaHeight/2;
			update = true;
		}
		if (playAreaCornerY + playAreaHeight - p.y() < playAreaWidth/6)
		{
			playAreaCornerY += playAreaHeight/2;
			update = true;
		}
		if (update)
		{
			manager.manager.Init(d2Math::IntRect(Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));
			manager.UpdateCollision();
		}
	}

	void HandleGameplayRotation()
	{
		float rotPerSec = 3;
		camera@ ca = get_active_camera();
		controllable@ co = controller_controllable(uint(get_active_player()));
		if (ca.input_taunt() != 0)
		{
			co.as_entity().time_warp(0);
			if (!wasRotating)
			{
				wasGrounded = co.ground();
			}
			wasRotating = true;
			//right
			if (ca.input_x() & 0x2 != 0)
			{
				rotation += rotPerSec/60;
			}
			//left
			if (ca.input_x() & 0x1 != 0)
			{
				rotation -= rotPerSec/60;
			}
			//down = right 90
			float hpi = asin(1);
			if (ca.input_y() & 0x2 != 0)
			{
				if (!right90)
				{
					right90 = true;
					rotation += hpi;
				}
			}
			else { right90 = false; }
			//up = left 90
			if (ca.input_y() & 0x1 != 0)
			{
				if (!left90)
				{
					left90 = true;
					rotation -= hpi;
				}
			}
			else { left90 = false; }

		}
		else
		{
			if (wasRotating)
			{
				co.as_entity().time_warp(1);
				wasRotating = false;
				manager.UpdateCollision();
				co.ground(wasGrounded);
			}
		}
	}

	//sorry it's also used for scroll up/down fuck you, whoever got confused by this later (probably me)
	void UpdateRotation(bool force = false)
	{
		if (oldRotation != rotation || force)
		{
			oldRotation = rotation;
			manager.cam.rotation = rotation;
			for (uint i = 0; i < nodeClusters.length(); i++)
			{
				nodeClusters[i].UpdateRotation();
			}
			if (@startPos != null)
			{
				startPos.UpdateRotation();
			}
			manager.UpdateLooks();
		}
	}

	void UpdateCamPos()
	{
		if (!is_playing())
		{
			camera@ rcam = get_active_camera();
			auto rcamPos = Vector2(rcam.x(), rcam.y());
			if (oldCamPos == Vector2(0,0))
			{
				oldCamPos = rcamPos;
				return;
			}
			Vector2 dif = oldCamPos - rcamPos;
			if (dif.Magnitude() > 0.1)
			{
				Vector3 dif2 = manager.cam.CamToWorldDir(Vector3(dif.x, dif.y, 0));
				manager.cam.centre -= dif2;
				manager.cam.igCoords = rcamPos;
				oldCamPos = rcamPos;
				// puts("cam pos updated! " + manager.cam.centre.x + ", " + manager.cam.centre.y + ", " + manager.cam.centre.z);
			}
		}
		else
		{
			controllable@ player = controller_controllable(uint(get_active_player()));
			auto rcamPos = Vector2(player.x(), player.y());
			if (oldCamPos == Vector2(0,0))
			{
				oldCamPos = rcamPos;
				return;
			}
			Vector2 dif = oldCamPos - rcamPos;
			if (dif.Magnitude() > 0.1)
			{
				Vector3 dif2 = manager.cam.CamToWorldDir(Vector3(dif.x, dif.y, 0));
				manager.cam.centre -= dif2;
				manager.cam.igCoords = rcamPos;
				oldCamPos = rcamPos;
			}
		}
	}

	void editor_draw(float lolxd) 
	{
		scene@ s = get_scene();
		if (showPlayArea) 
		{
			manager.manager.playArea.Draw(s, 22, 1);
		}
		manager.Draw();
		if (showCacheDebug) 
		{
			manager.manager.Draw(s, 22, 1);
		}
		for(uint i = 0; i < fakeTriggers.length(); i++)
		{
			fakeTriggers[i].Draw(s);
		}
		if (showCompass)
		{
			DrawCompass();
		}
		
		float squareSize = 0;
		uint squareCol = 0;
		switch (currentDistCol)
		{
			case 0:
				squareSize = 20;
				squareCol = 0xFF00FF00;
			break;
			case 1:
				squareSize = 30;
				squareCol = 0xFFFFFF00;
			break;
			case 2:
				squareSize = 40;
				squareCol = 0xFFFF0000;
			break;
		}
		Vector2 pos = Vector2(750, 400);
		s.draw_rectangle_hud(21, 1, pos.x-squareSize, pos.y-squareSize, pos.x+squareSize, pos.y+squareSize, 0, squareCol);
	}

	void draw(float idkAnymore) 
	{
		scene@ sc = get_scene();
		if (showPlayArea) 
		{
			manager.manager.playArea.Draw(sc, 22, 1);
		}
		manager.Draw();
		if (showCacheDebug) 
		{
			manager.manager.Draw(sc, 22, 1);
		}
		for (uint i = 0; i < debugDraw.length(); i++) 
		{
			debugDraw[i].Draw(sc, 22, 1);
		}
		if (showCompassGame)
		{
			DrawCompass();
		}
	}
	void DrawCompass()
	{
		scene@ s = get_scene();
		Vector3 dx = manager.cam.WorldToCamDir(Vector3(1,0,0)) * compassSize;
		Vector3 dz = manager.cam.WorldToCamDir(Vector3(0,0,1)) * compassSize;
		uint col1 = 0;
		uint col2 = 0;
		if (dx.z > 0)
		{
			col1 = 0xFF550000;
		}
		else
		{
			col1 = 0xFFFF0000;
		}
		if (dz.z > 0)
		{
			col2 = 0xFF000055;
		}
		else
		{
			col2 = 0xFF0000FF;
		}
		s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX,compassPosY-compassSize, 5, 0xFF00FF00);
		if (d2Math::sign(dx.x) != d2Math::sign(dz.x))
		{
			s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dx.x,compassPosY+dx.y, 5, col1);
			s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dz.x,compassPosY+dz.y, 5, col2);
		}
		else if (dx.z < dz.z)
		{
			s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dz.x,compassPosY+dz.y, 5, col2);
			s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dx.x,compassPosY+dx.y, 5, col1);
		}
		else
		{
			s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dx.x,compassPosY+dx.y, 5, col1);
			s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dz.x,compassPosY+dz.y, 5, col2);
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
	// 	Vector2 pos = Vector2(dustPosX, dustPosY);
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

class d3StartPos : trigger_base
{
	scripttrigger@ self;
	script@ script;
	d3::d3Manager@ manager;

	[hidden] d3FakeTrigger fakeTrigger;
	[hidden] bool hasInit = false;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@manager = @s.manager;
		@this.self = @self;

		if (@s.startPos != null && !(s.startPos is this)&& !self.destroyed())
		{
			get_scene().remove_entity(s.startPos.self.as_entity());
		}
		if (!self.destroyed())
		{
			@s.startPos = @this;
		}
		if (!hasInit)
		{
			fakeTrigger = d3FakeTrigger();
			fakeTrigger.colour = 0xFFFF0000;
			hasInit = true;
		}
		fakeTrigger.Init(self.as_entity(), s, manager);
		self.editor_handle_size(0);
		UpdateRotation();
		script.startCoords = fakeTrigger.pos;
		s.firstFrame = true;
	}

	void editor_step()
	{
		fakeTrigger.EditorStep();
		script.startCoords = fakeTrigger.pos;
	}

	void UpdateRotation()
	{
		fakeTrigger.UpdateRotation();
	}

	void on_remove()
	{
		fakeTrigger.DeleteSelf();
	}
}
