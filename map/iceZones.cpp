class script
{
	[label:"enable ice place"|tooltip:"place ice using mouse 3"] bool icePlace;

	input_api@ input;

	dustman@ player;
	float normIdleFric = 0;
	float normSkidFric = 0;
	float normRunStart = 0;
	float normRunAccel = 0;
	float normRunMax = 0;

	bool currentState = false; //false = normal, true = iced
	
	bool currentDrawState = false; //false = remove, true = place
	
	[hidden] array<array<int>> iceTiles;
	
	script()
	{
		puts("iceZones working c:");
		@input = @get_input_api();
	}

	void PlayInit()
	{
		@player = @controller_controllable(get_active_player()).as_dustman();
		normIdleFric = player.idle_fric();
		normSkidFric = player.skid_fric();
		normRunStart = player.run_start();
		normRunAccel = player.run_accel();
		normRunMax = player.run_max();
	}

	void GiveIcePhysics()
	{
		player.idle_fric(0);
		player.skid_fric(0);
		player.run_start(0);
		player.run_accel(0);
		player.run_max(2000);
	}

	void GiveNormalPhysics()
	{
		player.idle_fric(normIdleFric);
		player.skid_fric(normSkidFric);
		player.run_start(normRunStart);
		player.run_accel(normRunAccel);
		player.run_max(normRunMax);
		puts(normRunMax + "");
	}

	void UpdatePhysics()
	{
		if (currentState)
		{
			GiveIcePhysics();
		}
		else
		{
			GiveNormalPhysics();
		}
	}

	void step(int garbage)
	{
		array<int> playerTile = TileFromCoords(player.x(), player.y() + 24);
		bool newState = player.ground() && TileAtTile(playerTile[0], playerTile[1]);
		if (newState != currentState)
		{
			currentState = newState;
			UpdatePhysics();
		}
	}

	void editor_step()
	{
		if (!icePlace) { return; }
		array<int> mouseTile = TileFromCoords(input.mouse_x_world(19), input.mouse_y_world(19));
		if (input.mouse_state() & 0x80 != 0)
		{
			currentDrawState = !TileAtTile(mouseTile[0], mouseTile[1]);
		}
		if (input.mouse_state() & 0x10 != 0)
		{
			SetIceTile(mouseTile[0], mouseTile[1], currentDrawState);
		}
	}

	bool TileAtTile(int x, int y)
	{
		for (uint i = 0; i < iceTiles.length(); i++)
		{
			if (iceTiles[i][0] == x && iceTiles[i][1] == y) { return true; }
		}
		return false;
	}

	void RemoveAtTile(int x, int y)
	{
		for (uint i = 0; i < iceTiles.length(); i++)
		{
			if (iceTiles[i][0] == x && iceTiles[i][1] == y) { iceTiles.removeAt(i); }
		}
	}

	void SetIceTile(int x, int y, bool state)
	{
		RemoveAtTile(x, y);
		if (state)
		{
			array<int> i = {x, y};
			iceTiles.insertLast(i);
		}
	}

	array<int> TileFromCoords(float x, float y)
	{
		int x2 = int(x);
		int y2 = int(y);
		if (x2 < 0) { x2 -= 48; }
		if (y2 < 0) { y2 -= 48; }

		array<int> ret = {x2 - x2 % 48, y2 - y2 % 48};

		return ret;
	}

	void draw(float garbage)
	{
		scene@ s = get_scene();
		for (uint i = 0; i < iceTiles.length(); i++)
		{
			int x = iceTiles[i][0];
			int y = iceTiles[i][1];
			s.draw_rectangle_world(20, 4, x-1, y-1, x+49, y+49, 0, 0xFF90B0F0);	
			uint c = 0x90A0C0E0;
			s.draw_quad_world(20, 5, false, x+30, y-1, x+36, y-1, x+18, y+49, x+12, y+49, c, c, c, c);
		}
	}

	void editor_draw(float garbage) { draw(garbage); }

	void on_level_start() { PlayInit(); }
	void checkpoint_load() { PlayInit(); }
};
