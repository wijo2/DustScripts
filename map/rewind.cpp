//DustScripts\map\rewind.cpp

class script
{
	array<PlayerState> history;
	dustman@ player;
	scene@ scene;
	input_api@ input;

	sprites@ playerSprites;
	int playerPalette;

	int jumplinger = -1;
	int dashlinger = -1;

	bool jumpHeld = false;
	bool dashHeld = false;

	int flashtimer = -1;
	int flashtime = 10;

	float displacementIntensity = 0;
	float displacementSize = 1;
	float displacementSpeed = 0;

	//this was originally all in the trigger but oh right they despawn well it's messy as fuck now deal with it c:
	int activationSide = 0;
	int pauseTime = 20;
	int pauseTimer1 = -1;
	int pauseTimer2 = -1;
	float cutoff = 0;

	[text] int drawFrequency = 5;
	[text] int leniency = 10;
	[text] int punishmentFactor = 6;

	bool rewinding = false;
	bool paused = false;
	//actual speed = rewindSpeed/punishmentFactor
	int rewindSpeed = punishmentFactor;
	int rewindTimer = 0;

	script()
	{
		puts("rewind working c:");
		@input = @get_input_api();
	}

	void on_level_start()
	{
		entity@ e = controller_entity(0);
		if (@e == null) { return; }
		@player = @e.as_dustman();
		@scene = get_scene();
		@playerSprites = player.get_sprites();
		playerPalette = player.palette();
	}

	void step(int no)
	{
		if (player is null) { return; }
		camera@ cam = get_active_camera();

		//timers and stuff
		if (jumplinger > -1) { jumplinger -= 1; }
		if (jumplinger == 0)
		{
			Penalise();
			jumplinger = -1;
		}
		if (dashlinger > -1) { dashlinger -= 1; }
		if (dashlinger == 0)
		{
			Penalise();
			dashlinger = -1;
		}
		if (flashtimer > -1) { flashtimer -= 1; }

		//state tracking
		if (!rewinding)
		{
			bool ji = player.jump_intent() > 0;
			if (!ji && jumpHeld) { jumpHeld = false; }
			if (ji && jumpHeld) { ji = false; }
			if (ji && !jumpHeld) { jumpHeld = true; }
			bool di = player.dash_intent() > 0;
			if (!di && dashHeld) { dashHeld = false; }
			if (di && dashHeld) { di = false; }
			if (di && !dashHeld) { dashHeld = true; }

			if (!ji && !di)
			{
				PlayerState newState = PlayerState(player.x(), player.y(), player.x_speed(), player.y_speed(), player.sprite_index(), playerPalette, player.face(), player.dash(), cam.x(), cam.y());
				history.insertLast(newState);
			}
			else
			{
				string input = "";
				if (ji) { input += "j"; }
				if (di) { input += "d"; }
				PlayerState newState = PlayerState(player.x(), player.y(), player.x_speed(), player.y_speed(), player.sprite_index(), playerPalette, player.face(), player.dash(), cam.x(), cam.y(), input);
				history.insertLast(newState);
			}
		}
		else
		{
			//input checking
			if (!paused && player.jump_intent() > 0 && !jumpHeld)
			{
				jumpHeld = true;
				if (jumplinger != -1)
				{
					jumplinger = -1;
					return;
				}
				bool found = false;
				for (uint i = history.length()-1; i >= history.length()-leniency; i--)
				{
					int f = history[i].input.findFirst("j");
					if (f >= 0)
					{
						found = true;
						history[i].input.erase(f, 1);
						break;
					}
					if (i == 0) { break; }
				}
				if (!found) { Penalise(); }
			}
			if (player.jump_intent() == 0)
			{
				jumpHeld = false;
			}
			if (!paused && player.dash_intent() > 0 && !dashHeld)
			{
				dashHeld = true;
				if (dashlinger != -1)
				{
					dashlinger = -1;
					return;
				}
				bool found = false;
				for (uint i = history.length()-1; i >= history.length()-leniency; i--)
				{
					int f = history[i].input.findFirst("d");
					if (f >= 0)
					{
						found = true;
						history[i].input.erase(f, 1);
						break;
					}
					if (i == 0) { break; }
				}
				if (!found) { Penalise(); }
			}
			if (player.dash_intent() == 0)
			{
				dashHeld = false;
			}

			//progression in rewind
			rewindTimer++;
			if (!paused && rewindSpeed > rewindTimer % punishmentFactor) { history.removeLast(); }
			if (!paused && history.length() == 1)
			{
				pauseTimer2 = pauseTime;
				paused = true;
			}

			//immediate state setting
			PlayerState@ lh = @history[history.length()-1];
			if (lh.input.findFirst("j") >= 0) 
			{ 
				if (jumplinger != -1) { Penalise(); }
				jumplinger = leniency; 
			}
			if (lh.input.findFirst("d") >= 0) 
			{ 
				if (dashlinger != -1) { Penalise(); }
				dashlinger = leniency; 
			}
			lh.input = "";
			player.set_speed_xy(-lh.xv,-lh.yv);
			player.x(lh.x);
			player.y(lh.y);
			// cam.x(lh.camX);
			// cam.y(lh.camY);

			//pause logic
			if (rewinding)
			{
				if (pauseTimer1 > 0) { pauseTimer1 -= 1; }
				if (pauseTimer1 == 0) { paused = false; pauseTimer1 = -1;}
				if (pauseTimer2 == -1 && ((activationSide == 0 && player.x() > cutoff) || (activationSide == 1 && player.x() < cutoff)))
				{
					pauseTimer2 = pauseTime;
					paused = true;
				}

				if (pauseTimer2 > 0) { pauseTimer2 -= 1; }
				if (pauseTimer2 == 0)
				{
					rewinding = false;
					paused = false;
					player.set_sprites(playerSprites);

					player.set_speed_xy(lh.xv, lh.yv);
					player.dash(lh.aircharges);
					player.face(lh.direction);
					rewindSpeed = punishmentFactor;
					pauseTimer2 = -1;
				}
			}
		}
	}

	void Penalise()
	{
		flashtimer = flashtime;
		if (rewindSpeed > 1) { rewindSpeed -= 1; }
	}

	void draw(float sure) 
	{
		if (rewinding)
		{
			for (uint i = 0; i < history.length(); i++)
			{
				float x = i/displacementSize;
				float t = scene.time_in_level()*displacementSpeed;
				float d = displacementIntensity * (sin(x - t) + cos(5*x + t));
				if (history[i].input != "")
				{
					history[i].drawAsInput(scene, d);
					continue;
				}
				if (i % drawFrequency != 0) { continue; }
				history[i].DrawAsPoint(scene, d);
			}
			history[history.length()-1].DrawAsSprite(playerSprites, flashtimer);
		}
	}
}

class PlayerState
{
	float x = 0;
	float y = 0;
	float xv = 0;
	float yv = 0;
	int aircharges = 0;
	string input = "";
	string sprite = "";
	int palette = 0;
	int direction = 1;
	float camX = 0;
	float camY = 0;

	sprites@ arrowSprites;

	PlayerState() {}
	PlayerState(float x, float y, float xv, float yv, string sprite, int palette, int direction, int aircharges, float camX, float camY, string input = "")
	{
		this.x = x;
		this.y = y;
		this.xv = xv;
		this.yv = yv;
		this.sprite = sprite;
		this.input = input;
		this.palette = palette;
		if (direction != -1) { direction = 1; }
		this.direction = direction;
		this.aircharges = aircharges;
		this.camX = camX;
		this.camY = camY;
		@arrowSprites = create_sprites();
		arrowSprites.add_sprite_set("props5");
	}

	void DrawAsPoint(scene@ scene, float displacement)
	{
		const float verticalOffset = -32 + displacement;
		const float scalar = 0.28867513459481288225; //sqrt3/6
		const float trigSize = 20;
		const float p1ox = 0;
		const float p1oy = scalar*2*trigSize;
		const float p2ox = -0.5*trigSize;
		const float p2oy = -scalar*trigSize;
		const float p3ox = 0.5*trigSize;
		const float p3oy = -scalar*trigSize;
		const uint c = 0xFFFF0000;
		scene.draw_quad_world(20, 0, false, x+p1ox, y+p1oy + verticalOffset, x+p2ox, y+p2oy + verticalOffset, x+p3ox, y+p3oy + verticalOffset, x+p3ox, y+p3oy + verticalOffset, c,c,c,c);
		scene.draw_quad_world(20, 0, false, x+p1ox, y-p1oy + verticalOffset, x+p2ox, y-p2oy + verticalOffset, x+p3ox, y-p3oy + verticalOffset, x+p3ox, y-p3oy + verticalOffset, c,c,c,c);
	}

	void DrawAsSprite(sprites@ sprites, int flashtimer)
	{
		if (flashtimer == -1)
		{
			sprites.draw_world(20, 0, sprite, 0, palette, x, y, 0, direction, 1, 0xFFFFFFFF);
		}
		else
		{
			sprites.draw_world(20, 0, sprite, 0, palette, x, y, 0, direction, 1, 0xFFFF0000);
		}
	}

	void drawAsInput(scene@ scene, float displacement)
	{
		if (input.findFirst("j") >= 0)
		{
			arrowSprites.draw_world(20, 1, "symbol_1", 1, 1, x+80, y-128 + displacement, -90, 1, 1, 0xFF00FFFF);
		}
		if (input.findFirst("d") >= 0)
		{
			arrowSprites.draw_world(20, 1, "symbol_1", 1, 1, x+100, y+80 + displacement, 0, 1, 1, 0xFF00FFFF);
		}
	}
}

class RewindTrigger : trigger_base
{
	script@ script;
	scripttrigger@ self;

	//x coordinate at which we stop rewinding
	[text] float cutoff = 0;
	float cutoffSize = 16;

	// 0 = mouse not pressed, 1 = mouse pressed but not dragged, 2 = dragged
	int dragState = 0;

	int activationState = 0; // 0 = not active, 1 = used
	[text] float displacementIntensity = 0;
	[text] float displacementSize = 1;
	[text] float displacementSpeed = 0;

	void init(script@ s, scripttrigger@ self) 
	{ 
		@script = @s; 
		@this.self = @self; 
		self.editor_show_radius(true);
	}

	void editor_init(script@ s, scripttrigger@ self)
	{
		self.square(true);
		self.radius(100);
		self.height(100);
		cutoff = self.x();
	}

	void activate(controllable@ c)
	{
		if (activationState != 0 || c.as_dustman() is null) { return; }
		script.rewinding = true;
		script.player.set_sprites(create_sprites());
		activationState = 1;
		int activationSide = 0;
		if (script.player.x() > cutoff) { activationSide = 1; }
		script.paused = true;
		script.pauseTimer1 = script.pauseTime;
		script.displacementIntensity = displacementIntensity;
		script.displacementSize = displacementSize;
		script.displacementSpeed = displacementSpeed;
		script.cutoff = cutoff;
		script.activationSide = activationSide;
	}

	void editor_draw(float f)
	{
		scene@ s = get_scene();
		float hh = self.height(); //halfheight
		float hw = self.radius();
		uint colour = 0x5000FF00;
		s.draw_rectangle_world(21, 1, self.x() - hw, self.y() - hh, self.x() + hw, self.y() + hh, 0, colour);
		if (self.editor_selected())
		{
			uint cutoffColour = 0xA0FFFFFF;
			s.draw_rectangle_world(20, 1, cutoff - cutoffSize, self.y()-100000, cutoff + cutoffSize, self.y()+100000, 0, cutoffColour);
		}
	}

	void editor_step()
	{
		if (!self.editor_selected()) { return; }

		float mousePos = get_scene().mouse_x_world(0,20);
		bool mousePressed = script.input.mouse_state() & 0x4 != 0;

		if (mousePressed && dragState == 0)
		{
			if (mousePos < cutoff + cutoffSize && mousePos > cutoff - cutoffSize)
			{
				dragState = 2;
			}
			else
			{
				dragState = 1;
			}
		}
		if (mousePressed && dragState == 2)
		{
			cutoff = mousePos;
		}
		if (!mousePressed) { dragState = 0; }
	}
}
