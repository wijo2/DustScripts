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

	d2Math::Vector2 oldCamPos;
	[hidden] float rotation;
	float oldRotation;

	bool dontGrabCorner = false;

	array<d3QuadEntity@> quadEntities;
	array<d3NodeCluster@> nodeClusters;

	d3StartPos@ startPos;
	[hidden] Vector3 startCoords;

	d2Math::Vector2 middleDragStart;
	float middleDragStartRot;
	[label:"middle mouse hotkeys"] bool middleDragEnable = true;

	bool wasRotating = false;
	//bools for up/down keys cause reasons
	bool right90 = false;
	bool left90 = false;
	bool wasGrounded;

	bool firstFrame = true;

	//continue save mode next frame
	bool csmnf = false;
	float oldsh = 0;

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
		manager.manager.Init(d2Math::IntRect(d2Math::Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));
		@manager.script = @this;
		rotation = 0;
		oldRotation = 0;
		camera@ c = get_active_camera();
		manager.cam.igCoords = d2Math::Vector2(c.x(), c.y());
		manager.cam.centre = Vector3(c.x(), c.y(), 0);
	}

	void PlayInit()
	{
		manager.manager.collisionOrder = collisionOrder;
		manager.manager.PlayInit(this, d2Math::IntRect(d2Math::Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));	
		@manager.script = @this;
		rotation = 0;
		oldRotation = 0;
		controllable@ c = controller_controllable(uint(get_active_player()));
		manager.cam.igCoords = d2Math::Vector2(c.x(), c.y());
		manager.cam.centre = Vector3(c.x(), c.y(), 0);
		get_active_camera().controller_mode(4);
		// puts("setting start coords!");
		// puts(startCoords);
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
		dontGrabCorner = false;
		if (input.mouse_state() & 0x20 != 0 
			&& editor.editor_tab() == "Triggers"
			&& @editor.get_selected_trigger() == null)
		{
			d2Math::Vector2 mousePos = d2Math::Vector2();
			mousePos.x = input.mouse_x_world(21);
			mousePos.y = input.mouse_y_world(21);
			for (uint i = 0; i < quadEntities.length(); i++)
			{
				if (quadEntities[i].quad.collisionBase.base.IsInside(mousePos))
				{
					editor.set_selected_trigger(
						quadEntities[i].self.as_entity()
					);
					dontGrabCorner = true;
					break;
				}
			}
		}

		if (csmnf)
		{
			camera@ cam = get_active_camera();
			manager.cam.igCoords = d2Math::Vector2(0,0);
			oldCamPos = d2Math::Vector2(0,0);
			cam.x(0);
			cam.y(0);
			cam.screen_height(oldsh);
			csmnf = false;
		}

		//fuck this game I fucking hate this why are you this shit fuck you
		if (input.key_check_pressed_vk(0x58) && input.key_check_gvb(11))
		{
			camera@ cam = get_active_camera();
			oldsh = cam.screen_height();
			cam.screen_height(10000000);
			if (@startPos != null)
			{
				startPos.self.x(0);
				startPos.self.y(0);
				startPos.dontMove = true;
			}
			for(uint i = 0; i < quadEntities.length(); i++)
			{
				quadEntities[i].self.x(0);
				quadEntities[i].self.y(0);
				quadEntities[i].dontMove = true;
			}
			for(uint i = 0; i < nodeClusters.length(); i++)
			{
				nodeClusters[i].self.x(0);
				nodeClusters[i].self.y(0);
				nodeClusters[i].dontMove = true;
			}
			csmnf = true;
		}

		HandleMiddleCommands();
		UpdateCamPos();
		UpdateRotation();
	}

	uint ApplyFog(uint col, float dist)
	{
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
			middleDragStart = d2Math::Vector2(input.mouse_x_hud(true), input.mouse_y_hud(true));
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
		manager.manager.step();
		UpdateCamPos();
		UpdateRotation();
		UpdatePlayArea();
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
			manager.manager.Init(d2Math::IntRect(d2Math::Vector2(playAreaCornerX, playAreaCornerY), playAreaWidth, playAreaHeight));
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
			manager.UpdateLooks();
			for (uint i = 0; i < quadEntities.length(); i++)
			{
				quadEntities[i].UpdateRotation();
			}
			for (uint i = 0; i < nodeClusters.length(); i++)
			{
				nodeClusters[i].UpdateRotation();
			}
			if (@startPos != null)
			{
				startPos.UpdateRotation();
			}
			manager.SortQuadList();
		}
	}

	void UpdateCamPos()
	{
		if (!is_playing())
		{
			camera@ rcam = get_active_camera();
			auto rcamPos = d2Math::Vector2(rcam.x(), rcam.y());
			if (oldCamPos == d2Math::Vector2(0,0))
			{
				oldCamPos = rcamPos;
				return;
			}
			d2Math::Vector2 dif = oldCamPos - rcamPos;
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
			auto rcamPos = d2Math::Vector2(player.x(), player.y());
			if (oldCamPos == d2Math::Vector2(0,0))
			{
				oldCamPos = rcamPos;
				return;
			}
			d2Math::Vector2 dif = oldCamPos - rcamPos;
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
		if (showPlayArea) 
		{
			manager.manager.playArea.Draw(get_scene(), 22, 1);
		}
		manager.Draw();
		if (showCacheDebug) 
		{
			manager.manager.Draw(get_scene(), 22, 1);
		}
		if (showCompass)
		{
			Vector3 dx = manager.cam.WorldToCamDir(Vector3(1,0,0)) * compassSize;
			Vector3 dz = manager.cam.WorldToCamDir(Vector3(0,0,1)) * compassSize;
			scene@ s = get_scene();
			s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX,compassPosY-compassSize, 5, 0xFF00FF00);
			if (d2Math::sign(dx.x) != d2Math::sign(dz.x))
			{
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dx.x,compassPosY+dx.y, 5, 0xFFFF0000);
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dz.x,compassPosY+dz.y, 5, 0xFF0000FF);
			}
			else if (dx.z < dz.z)
			{
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dz.x,compassPosY+dz.y, 5, 0xFF0000FF);
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dx.x,compassPosY+dx.y, 5, 0xFFFF0000);
			}
			else
			{
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dx.x,compassPosY+dx.y, 5, 0xFFFF0000);
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dz.x,compassPosY+dz.y, 5, 0xFF0000FF);
			}
		}
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
			Vector3 dx = manager.cam.WorldToCamDir(Vector3(1,0,0)) * compassSize;
			Vector3 dz = manager.cam.WorldToCamDir(Vector3(0,0,1)) * compassSize;
			scene@ s = get_scene();
			s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX,compassPosY-compassSize, 5, 0xFF00FF00);
			if (d2Math::sign(dx.x) != d2Math::sign(dz.x))
			{
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dx.x,compassPosY+dx.y, 5, 0xFFFF0000);
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dz.x,compassPosY+dz.y, 5, 0xFF0000FF);
			}
			else if (dx.z < dz.z)
			{
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dz.x,compassPosY+dz.y, 5, 0xFF0000FF);
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dx.x,compassPosY+dx.y, 5, 0xFFFF0000);
			}
			else
			{
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dx.x,compassPosY+dx.y, 5, 0xFFFF0000);
				s.draw_line_hud(21,1,compassPosX,compassPosY, compassPosX+dz.x,compassPosY+dz.y, 5, 0xFF0000FF);
			}
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

class d3StartPos : trigger_base
{
	[hidden] Vector3 pos;
	d2Math::Vector2 oldCentre;
	[hidden] bool hasInit;

	scripttrigger@ self;
	script@ script;
	d3::d3Manager@ manager;

	bool dontMove = false;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@manager = @s.manager;
		@this.self = @self;
		self.editor_colour_inactive(0xFFFF0000);

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
			d2Math::Vector2 camCen = manager.cam.igCoords;
			pos = manager.cam.CamToWorldPos(Vector3(self.x()-camCen.x,self.y()-camCen.y,0));
			hasInit = true;
		}
		UpdateRotation();
		script.startCoords = pos;
		s.firstFrame = true;
	}

	void editor_step()
	{
		d2Math::Vector2 curPos = d2Math::Vector2(self.x(), self.y());
		d2Math::Vector2 dif = curPos - oldCentre;
		if (dif != d2Math::Vector2() && !dontMove)
		{
			pos += manager.cam.CamToWorldDir(Vector3(dif.x, dif.y,0));
			oldCentre = d2Math::Vector2(self.x(), self.y());
			script.startCoords = pos;
		}
	}

	void UpdateRotation()
	{
		d2Math::Vector2 camCen = manager.cam.igCoords;
		Vector3 camPos = Vector3(camCen.x, camCen.y, 0);
		Vector3 newPos = manager.cam.WorldToCamPos(pos) + camPos;
		self.x(newPos.x);
		self.y(newPos.y);
		oldCentre = d2Math::Vector2(newPos.x, newPos.y);
		if (newPos.z < 0) { self.editor_handle_size(0); }
		else { self.editor_handle_size(10); }
	}
}

//I don't want to delete this since it's already made idk maybe someone will have a use for
//it but yea it fucking sucks use nodecluster instead
class d3QuadEntity : trigger_base 
{
	[text] int layer;
	[text] int sub_layer;
	[colour,alpha] uint d2colour;
	[colour,alpha] uint d3colour;

	[text] bool side1active = true;
	[text] bool side2active = true;
	[text] bool side3active = true;
	[text] bool side4active = true;
	[text] bool side1dust = false;
	[text] bool side2dust = false;
	[text] bool side3dust = false;
	[text] bool side4dust = false;
	[text] bool side1spikes = false;
	[text] bool side2spikes = false;
	[text] bool side3spikes = false;
	[text] bool side4spikes = false;

	[hidden] Vector3 p1;
	[hidden] Vector3 p2;
	[hidden] Vector3 p3;
	[hidden] Vector3 p4;

	int selectedCorner = 0;

	d2Math::Vector2 oldCentre;

	d3::d3Manager@ manager;
	d3::d3CQuad@ quad;

	bool dontMove = false;

	scripttrigger@ self;
	script@ script;
	input_api@ input;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@this.self = @self;
		@input = @get_input_api();
		if (d2colour == 0x00000000)
		{
			d2colour = s.default2dCol;
		}
		if (d3colour == 0x00000000)
		{
			d3colour = s.default3dCol;
		}
		oldCentre = d2Math::Vector2(self.x(), self.y());
		@this.manager = @s.manager; 
		@quad = @d3::d3CQuad();
		@quad.collisionBase.script = @script;
		@quad.collisionBase.manager = @manager.manager;
		@quad.manager = @manager;
		
		if (p1 == Vector3(0,0,0) && 
			p2 == Vector3(0,0,0) && 
			p3 == Vector3(0,0,0) && 
			p4 == Vector3(0,0,0))
		{
			Vector3 cen = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
			p1 = manager.cam.CamToWorldPos(Vector3(self.x(), self.y()+100, 50) - cen);
			p2 = manager.cam.CamToWorldPos(Vector3(self.x()-100, self.y(), -50) - cen);
			p3 = manager.cam.CamToWorldPos(Vector3(self.x(), self.y()-100, 50) - cen);
			p4 = manager.cam.CamToWorldPos(Vector3(self.x()+100, self.y(), -50) - cen);
		}
		@quad.base = @d3::d3Quad(p1, p2, p3, p4, d3colour);
		quad.collisionBase.base.colour = d2colour;
		if (s.manager.allQuads.findByRef(quad) < 0)
		{
			s.manager.allQuads.insertLast(quad);
		}
		if (layer == 0)
		{
			layer = 18;
		}
		if (sub_layer == 0)
		{
			sub_layer = 1;
		}
		UpdateSides();
		UpdateSelf();
		if (quad.collisionBase.activeLines[0] || 
			quad.collisionBase.activeLines[1] || 
			quad.collisionBase.activeLines[2] || 
			quad.collisionBase.activeLines[3]) 
		{
			d2Math::Vector2 centre = quad.collisionBase.base.FindCentre();
			oldCentre = centre;
			self.x(centre.x);
			self.y(centre.y);
		}
		quad.collisionBase.UpdateCollision();
		s.quadEntities.push_back(this);
	}
	
	void editor_var_changed(var_info@ info)
	{
		UpdateSides();
		UpdateSelf();
	}

	void UpdateRotation()
	{
		UpdateSides();
		if ((quad.collisionBase.activeLines[0] || 
			quad.collisionBase.activeLines[1] || 
			quad.collisionBase.activeLines[2] || 
			quad.collisionBase.activeLines[3]) &&
			oldCentre != d2Math::Vector2(0,0)) 
		{
			oldCentre = d2Math::Vector2(0,0);
			d2Math::Vector2 centre = quad.collisionBase.base.FindCentre();
			self.x(centre.x);
			self.y(centre.y);
		}
	}

	void UpdateSelf()
	{
		quad.base.p1 = p1;
		quad.base.p2 = p2;
		quad.base.p3 = p3;
		quad.base.p4 = p4;
		quad.base.ApplyProjection(manager.cam);
		quad.UpdateIntersectQuad(manager.cam);
		quad.base.colour = d3colour;
		quad.collisionBase.base.colour = d2colour;
		quad.layer = layer;
		quad.sub_layer = sub_layer;
	}

	void UpdateSides()
	{
		quad.activeSides[0] = side1active;
		quad.activeSides[1] = side2active;
		quad.activeSides[2] = side3active;
		quad.activeSides[3] = side4active;
		quad.base.drawnSides[0] = side1active;
		quad.base.drawnSides[1] = side2active;
		quad.base.drawnSides[2] = side3active;
		quad.base.drawnSides[3] = side4active;
		quad.dustSides[0] = side1dust;
		quad.dustSides[1] = side2dust;
		quad.dustSides[2] = side3dust;
		quad.dustSides[3] = side4dust;
		quad.spikeSides[0] = side1spikes;
		quad.spikeSides[1] = side2spikes;
		quad.spikeSides[2] = side3spikes;
		quad.spikeSides[3] = side4spikes;
	}

	void editor_step()
	{
		scene@ s = get_scene();

		d2Math::Vector2 mousePosWorld = d2Math::Vector2(s.mouse_x_world(0,20), s.mouse_y_world(0,20));
		d2Math::Vector2 mousePosHud = d2Math::Vector2(s.mouse_x_hud(0), s.mouse_y_hud(0));

		//select/deselect corner
		if (script.input.mouse_state() & 0x20 != 0 
			&& script.editor.editor_tab() == "Triggers"
			&& @script.editor.get_selected_trigger() != null
   && script.editor.get_selected_trigger().is_same(self.as_entity())
			&& !script.dontGrabCorner) 
		{

			if (selectedCorner == 0) 
			{
				array<d2Math::Vector2> corners = { 
					d2Math::Vector2(quad.base.csp1.x, quad.base.csp1.y),
					d2Math::Vector2(quad.base.csp2.x, quad.base.csp2.y),
					d2Math::Vector2(quad.base.csp3.x, quad.base.csp3.y),
					d2Math::Vector2(quad.base.csp4.x, quad.base.csp4.y)
				};
				for (uint i = 0; i < corners.length(); i++)
				{
					d2Math::Vector2 pos = d2Math::WorldToScreenPos(
						d2Math::Vector2(corners[i].x, corners[i].y)
					);
					if (pos.Distance(mousePosHud) < 50) 
					{
						selectedCorner = i + 1;
						return;
					}
				}
			}
			else 
			{
				Vector3 cen = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
				switch(selectedCorner) 
				{
					case 1:
						p1 = manager.cam.CamToWorldPos(Vector3(mousePosWorld.x, mousePosWorld.y, quad.base.csp1.z) - cen);
					break;
					case 2:
						p2 = manager.cam.CamToWorldPos(Vector3(mousePosWorld.x, mousePosWorld.y, quad.base.csp2.z) - cen);
					break;
					case 3:
						p3 = manager.cam.CamToWorldPos(Vector3(mousePosWorld.x, mousePosWorld.y, quad.base.csp3.z) - cen);
					break;
					case 4:
						p4 = manager.cam.CamToWorldPos(Vector3(mousePosWorld.x, mousePosWorld.y, quad.base.csp4.z) - cen);
					break;
				}
				selectedCorner = 0;
				UpdateSelf();
				if ((quad.collisionBase.activeLines[0] || 
					quad.collisionBase.activeLines[1] || 
					quad.collisionBase.activeLines[2] || 
					quad.collisionBase.activeLines[3]) &&
					oldCentre != d2Math::Vector2(0,0)) 
				{
					oldCentre = d2Math::Vector2(0,0);
					d2Math::Vector2 centre = quad.collisionBase.base.FindCentre();
					self.x(centre.x);
					self.y(centre.y);
				}
			}
			return;
		}

		//update corner pos
		if (script.editor.editor_tab() == "Triggers"
			&& @script.editor.get_selected_trigger() != null
   && script.editor.get_selected_trigger().is_same(self.as_entity())
			&& selectedCorner != 0)
		{
			Vector3 cen = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
			switch(selectedCorner) 
			{
				case 1:
					p1 = manager.cam.CamToWorldPos(Vector3(mousePosWorld.x, mousePosWorld.y, quad.base.csp1.z) - cen);
				break;
				case 2:
					p2 = manager.cam.CamToWorldPos(Vector3(mousePosWorld.x, mousePosWorld.y, quad.base.csp2.z) - cen);
				break;
				case 3:
					p3 = manager.cam.CamToWorldPos(Vector3(mousePosWorld.x, mousePosWorld.y, quad.base.csp3.z) - cen);
				break;
				case 4:
					p4 = manager.cam.CamToWorldPos(Vector3(mousePosWorld.x, mousePosWorld.y, quad.base.csp4.z) - cen);
				break;
			}
			UpdateSelf();
			if ((quad.collisionBase.activeLines[0] || 
				quad.collisionBase.activeLines[1] || 
				quad.collisionBase.activeLines[2] || 
				quad.collisionBase.activeLines[3]) &&
				oldCentre != d2Math::Vector2(0,0)) 
			{
				oldCentre = d2Math::Vector2(0,0);
				d2Math::Vector2 centre = quad.collisionBase.base.FindCentre();
				self.x(centre.x);
				self.y(centre.y);
			}
		}

		d2Math::Vector2 curCen = d2Math::Vector2(self.x(), self.y());
		d2Math::Vector2 dif = oldCentre - curCen;
		if (dif.Magnitude() > 0.1 && !dontMove &&
			(quad.collisionBase.activeLines[0] ||
			quad.collisionBase.activeLines[1] ||
			quad.collisionBase.activeLines[2] ||
			quad.collisionBase.activeLines[3]))
		{
			// puts("updating centre! " + oldCentre.x + ", " + curCen.x);
			if (oldCentre == d2Math::Vector2(0,0))
			{
				d2Math::Vector2 centre = quad.collisionBase.base.FindCentre();
				oldCentre = centre;
			}
			else
			{
				Vector3 dif2 = manager.cam.CamToWorldDir(Vector3(dif.x, dif.y, 0));
				p1 -= dif2;
				p2 -= dif2;
				p3 -= dif2;
				p4 -= dif2;
				UpdateSelf();
				d2Math::Vector2 centre = quad.collisionBase.base.FindCentre();
				oldCentre = centre;
				self.x(centre.x);
				self.y(centre.y);
			}
		}
		
		if (self.editor_selected() && input.key_check_gvb(10))
		{
			// puts("trying to move!");
			if (input.mouse_state() & 0x1 != 0)
			{
				Vector3 dif2 = Vector3(0, 0, 24);
				p1 += dif2;
				p2 += dif2;
				p3 += dif2;
				p4 += dif2;
				UpdateSelf();
			}
			if (input.mouse_state() & 0x2 != 0)
			{
				Vector3 dif2 = Vector3(0, 0, -24);
				p1 += dif2;
				p2 += dif2;
				p3 += dif2;
				p4 += dif2;
				UpdateSelf();
			}
		}
	}

	void on_remove()
	{
		quad.collisionBase.ClearOldCache();
		int i = manager.allQuads.findByRef(quad);
		if (i >= 0)
		{
			manager.allQuads.removeAt(i);
		}
	}
};
