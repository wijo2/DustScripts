//DustScripts\map\gravitySwitcher.cpp
class script
{
	bool switched = false;
	bool jumpUsed = false;
	bool dashUsed = false;
	bool higherPrev = false; //used for goddamn hitraise god I hate hitrise
	int lastDash = 0;
	dustman@ player;
	float lastSpeed = 0;

	float faDef = 0;
	float haDef = 0;
	float jaDef = 0;
	float fmDef = 0;
	float wsDef = 0;
	float hrDef = 0;

	int firstFrames = 4;

	[entity,"trigger"] int normalFogId = 0;
	[entity,"trigger"] int switchedFogId = 0;

	fog_setting@ normalSetting;
	fog_setting@ switchedSetting;
	bool hasFogs = false;

	float changeTime = 0.3;

	script()
	{
		puts("gravitySwithcer working! c:");
	}

	void PlayInit()
	{
		@player = @controller_controllable(get_active_player()).as_dustman();
		// puts("huh " + player.fall_accel()); //3456
		// puts("huh2 " + player.hover_accel()); //3456
		// puts("huh3 " + player.jump_a()); //-864
		// SwitchGravity();
		
		puts(player.fall_max() + "");
		if (faDef == 0)
		{
			faDef = player.fall_accel();
			haDef = player.hover_accel();
			jaDef = player.jump_a();
			fmDef = player.fall_max();
			wsDef = player.wall_slide_speed();
			hrDef = player.hitrise_speed();
		}

		entity@ normalFog = entity_by_id(normalFogId);
		entity@ switchedFog = entity_by_id(switchedFogId);

		hasFogs = !(normalFog is null) && !(switchedFog is null);

		if (hasFogs)
		{
			@normalSetting = @create_fog_setting(normalFog);
			@switchedSetting = @create_fog_setting(switchedFog);
		}
		get_active_camera().change_fog(normalSetting, changeTime);
		firstFrames = 4;
	}

	void SwitchGravity()
	{
		if (firstFrames > 0) { return; }
		if (switched)
		{
			SetGravityPositive();
			switched = false;
		}
		else
		{
			SetGravityNegative();
			switched = true;
		}
	}

	void SetGravityNegative()
	{
		get_active_camera().change_fog(switchedSetting, changeTime);
		player.fall_accel(-faDef);
		player.hover_accel(-haDef);
		player.jump_a(-jaDef); //not actually negative caps to 0
		player.wall_slide_speed(-wsDef); //not actually negative caps to 0, too lazy to fix not important for my lvl
		player.hitrise_speed(-fmDef); //can't be set to negative, I don't have much choise other than setting a signal with this
	}

	void SetGravityPositive()
	{
		get_active_camera().change_fog(normalSetting, changeTime);
		player.fall_accel(faDef);
		player.hover_accel(haDef);
		player.jump_a(jaDef);
		player.wall_slide_speed(wsDef);
		player.hitrise_speed(hrDef);
	}

	void step(int dontcare)
	{
		if (firstFrames > 0)
		{
			SetGravityPositive();
			switched = false;
			firstFrames--;
		}
		if (switched && player.state() == 9 && player.ground()) { player.state_timer(10); } //if illegal dashing stop, can still give speed that's cool I think
		if (switched && player.ground()) 
		{ 
			player.set_speed_xy(player.x_speed(), -5); 
			player.dash(lastDash); 
		}
		lastDash = player.dash();

		if (switched && player.y_speed() < -player.fall_max()) { player.set_speed_xy(player.x_speed(),-player.fall_max()); }

		if (player.jump_intent() == 2 && !switched && !jumpUsed)
		{
			jumpUsed = true;
		}
		if ((player.jump_intent() == 1 || player.jump_intent() == 2) && switched && !jumpUsed && player.dash() > 0)
		{
			player.set_speed_xy(player.x_speed(),864);
			jumpUsed = true;
		}
		if (player.jump_intent() == 0) {jumpUsed = false;}

		if (switched && (player.fall_intent() == 1 || player.fall_intent() == 2) && (player.y_speed() <= 0 || player.state() == 7) && !dashUsed)
		{
			player.set_speed_xy(player.x_speed(), -fmDef/1.2);	
			dashUsed = true;
			get_scene().play_sound("sfx_dm_fast_fall", player.x(), player.y(), 1, false, false);
		}
		if (switched && (player.fall_intent() == 1 || player.fall_intent() == 2) && (player.y_speed() > 0 && player.state() != 7) && !dashUsed)
		{
			if (player.fall_intent() == 2)
			{
				player.y(player.y() - player.y_speed()/50);
				player.set_speed_xy(player.x_speed(), lastSpeed - faDef/50);
			}
			player.fall_intent(2);
			dashUsed = true;
		}
		lastSpeed = player.y_speed();
		if ((player.fall_intent() == 1 || player.fall_intent() == 2) && !dashUsed)
		{
			dashUsed = true;
		}
		if (player.fall_intent() == 0) { dashUsed = false; }

		if (switched && player.y_speed() < -fmDef+20 && !higherPrev)
		{
			player.set_speed_xy(player.x_speed(), 200);
			player.y(player.y() + fmDef/50);
		}
		higherPrev = player.y_speed() < -fmDef*0.95;
	}

	void on_level_start() { PlayInit(); }
	void checkpoint_load() { PlayInit(); }
}

class SwitchTrigger : trigger_base
{
	script@ script;
	scripttrigger@ self;

	[text] int length = 240;

	float lastX = 0;
	float lastY = 0;
	float hOffset = 24;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@this.self = @self;
	}

	void PlayInit()
	{
		lastX = script.player.x();
		lastY = script.player.y() - hOffset;
	}

	void step()
	{
		float newX = script.player.x();
		float newY = script.player.y() - hOffset;

		if (abs(newX - self.x()) < length/2 && ((lastY - self.y() < 0 && newY - self.y() > 0) || (lastY - self.y() > 0 && newY - self.y() < 0)))
		{
			script.SwitchGravity();
			puts("switch");
		}

		lastX = script.player.x();
		lastY = script.player.y() - hOffset;
	}

	void D()
	{
		scene@ s = get_scene();
		s.draw_rectangle_world(18,1, self.x() - length/2, self.y() - 12, self.x() + length/2, self.y() + 12, 0, 0xA0000000);
	}

	void draw(float somethingidk)
	{
		D();
	}
	void editor_draw(float somethingelseidk)
	{
		D();
	}

	void on_level_start() { PlayInit(); }
	void checkpoint_load() { PlayInit(); }
}
