//DustScripts/map/speedcheck.cpp
class script
{
	float lastVelocity = 0;
	[position,mode:hud,layer:21,y:ui_y|tooltip:"initial pos of speed/point ui"] float ui_x;
	[hidden] float ui_y;
	float ui_hh = 50;
	float ui_hw = 130;
	float ui_bh = 20;
	bool beingDragged = false;
	float ui_lastx = ui_x;
	float ui_lasty = ui_y;
	float ui_lastMx = ui_x;
	float ui_lastMy = ui_y;
	bool disableUi = false;

	float respawnX = 0;
	float respawnY = 0;

	bool fadeHappening = false;
	bool unfadeHappening = false;
	float fadeOpacity = 0;

	int maxPoints = 0;
	int collectedPoints = 0;
	int resets = 0;

	[text|tooltip:"Add like 10-20 dust entities (like flags)\nto this list to allow completion rank to work"] array<CompletionEntity> compEnts;
	[text|tooltip:"Multiplier for reset counter before passing it to finesse score"] float finesseMulti = 1;

	RankedTrigger@ beatReset = null;

	script()
	{
		puts("speedcheck working! c:");
		get_scene().time_warp(1);
	}

	void on_level_end()
	{
		scene@ s = get_scene();
		s.combo_break_count(uint(float(resets) * finesseMulti));
		int ents = compEnts.length();
		float compP = 0;
		if (maxPoints > 0) { compP = float(collectedPoints)/float(maxPoints); }
		else { compP = 1; }
		puts("compP: " + compP);
		uint entsToKill = uint(float(ents) * compP);
		puts("entsToKill: " + entsToKill);
		for (uint n = 0; n < entsToKill; n++)
		{
			int e = compEnts[compEnts.length()-1].ent;
			compEnts.removeLast();
			entity@ ent = entity_by_id(e);
	   		if (@ent == null) { continue; }
			s.remove_entity(ent);
		}
	}

	void RecordPoints(int points, int max)
	{
		maxPoints += max;
		collectedPoints += points;
	}

	void step(int idk) { HandleDrag(); HandleFade(); }

	void on_level_start() { PlayInit(); }
	void checkpoint_load() { PlayInit(); }
	void PlayInit()
	{
		entity@ e = controller_entity(0);
		if (@e == null) { return; }
		respawnX = e.x();
		respawnY = e.y();
	}

	void Reset(float x, float y)
	{
		if (fadeHappening) { return; };
		resets++;
		respawnX = x;
		respawnY = y;
		fadeHappening = true;
		get_scene().time_warp(0.3);
		entity@ e = controller_entity(0);
		if (@e == null) { return; }
		dustman@ d;
		@d = @e.as_dustman();
		if (@d == null) { return; }
	}

	void HandleFade()
	{
		if (fadeHappening)
		{
			fadeOpacity += 1.0/60.0 * 2.5;
			if (fadeOpacity > 1.1)
			{
				get_scene().time_warp(1);
				fadeHappening = false;
				unfadeHappening = true;
				fadeOpacity = 1;
				entity@ e = controller_entity(0);
				if (@e == null) { return; }
				dustman@ d;
				@d = @e.as_dustman();
				if (@d == null) { return; }
				d.set_speed_xy(0,0);
				d.set_xy(respawnX, respawnY);
				d.state(0);
				if (@beatReset != null)
				{
					beatReset.beat = false;
				}
			}
		}
		if (unfadeHappening)
		{
			fadeOpacity -= 1.0/60.0 * 4;
			if (fadeOpacity <= 0)
			{
				unfadeHappening = false;
				fadeOpacity = 0;
			}
		}
	}

	void HandleDrag()
	{
		scene@ s = get_scene();
		const bool mp = s.mouse_state(get_active_player()) & 4 != 0;
		const float mx = s.mouse_x_hud(get_active_player());
		const float my = s.mouse_y_hud(get_active_player());
		if (!beingDragged && mp && mx > ui_x-ui_hw && mx < ui_x+ui_hw && my > ui_y-ui_hh-ui_bh && my < ui_y-ui_hh)
		{
			beingDragged = true;
			ui_lastMx = mx;
			ui_lastMy = my;
			ui_lastx = ui_x;
			ui_lasty = ui_y;
		}
		if (beingDragged && !mp)
		{
			beingDragged = false;
		}
		if (beingDragged && mp)
		{
			ui_x = ui_lastx + (mx-ui_lastMx);
			ui_y = ui_lasty + (my-ui_lastMy);
		}
	}

	void draw(float fuckYouRandomPerson)
	{
		bool found = true;
		entity@ e = controller_entity(0);
		if (@e == null) { found = false; }
		dustman@ d;
		if (found) 
		{
			@d = @e.as_dustman();
			if (@d == null) { found = false; }
		}
		if (found)
		{
			lastVelocity = abs(d.x_speed());
		}
		DrawUi();
	}
	void editor_draw(float idk)
	{
		DrawUi();
	}

	void DrawUi()
	{
		if (disableUi) { return; }
		scene@ s = get_scene();
		const uint colour = 0xff444444;
		const uint colour2 = 0xff111111;
		s.draw_rectangle_hud(21, 1, ui_x - ui_hw, ui_y - ui_hh, ui_x + ui_hw, ui_y + ui_hh, 0, colour);
		s.draw_rectangle_hud(21, 1, ui_x - ui_hw, ui_y - ui_hh, ui_x + ui_hw, ui_y - ui_hh - ui_bh, 0, colour2);
		textfield@ tf = create_textfield();
		tf.text("vel: " + formatFloat(lastVelocity, "", 0, 0) + "\n" + 
			"points: " + collectedPoints + "/" + maxPoints
		);
		tf.set_font("Caracteres", 40);
		tf.colour(0xFF000000);
		tf.draw_hud(21, 1, ui_x, ui_y, 1, 1, 0);
		tf.colour(0xFFFFFFFF);
		tf.draw_hud(21, 1, ui_x-1, ui_y-2, 1, 1, 0);

		if (fadeHappening || unfadeHappening)
		{
			float op = fadeOpacity;
			if (op > 1) { op = 1; }
			int opacity = int(op * 255.0);
			int col = opacity * 0x01000000;
			s.draw_rectangle_hud(21, 1, 5000, 5000, -5000, -5000, 0, col);
		}
	}
}

class CompletionEntity
{
	[entity] int ent;
}

class SpeedcheckTrigger : trigger_base, callback_base
{
	script@ script;
	scripttrigger@ self;

	[position,mode:world,layer:21,y:spawnY] float spawnX;
	[hidden] float spawnY;
	[text|label:"required speed"] float requiredSpeed;
	
	float spHSize = 20;

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
	}

	void draw(float fuckYouRandomPerson)
	{
		scene@ s = get_scene();
		const float hh = self.height(); //halfheight
		const float hw = self.radius();
		const uint colour = 0x5000ff00;
		s.draw_rectangle_world(21, 1, self.x() - hw, self.y() - hh, self.x() + hw, self.y() + hh, 0, colour);
		textfield@ tf = create_textfield();
		tf.text(formatFloat(requiredSpeed, "", 0, 0) + "+");
		tf.set_font("Caracteres", 40);
		tf.draw_world(21, 1, self.x(), self.y(), 1, 1, 0);
	}

	void editor_draw(float lolxd) { 
		draw(lolxd);
		get_scene().draw_rectangle_world(21, 1, spawnX-spHSize, spawnY-spHSize, spawnX+spHSize, spawnY+spHSize, 45, 0xffff0000);
	}

	void activate(controllable@ c)
	{
		if (@c.as_dustman() == null) { return; }
		if (abs(c.x_speed()) < requiredSpeed) 
		{
			script.Reset(spawnX, spawnY);
		}
	}
}

class RankedTrigger : SpeedcheckTrigger
{
	[text|label:"silver speed"] float silverSpeed;
	[text|label:"gold speed"] float goldSpeed;

	bool beat = false;
	bool uiVisible = false;
	float uiInteractionTimer = 0;
	int currentMedal = 0;

	void draw(float fuckYouRandomPerson)
	{
		scene@ s = get_scene();
		const float hh = self.height(); //halfheight
		const float hw = self.radius();
		const uint colour = 0x5000ff00;
		s.draw_rectangle_world(21, 1, self.x() - hw, self.y() - hh, self.x() + hw, self.y() + hh, 0, colour);
		textfield@ tf = create_textfield();
		tf.text("B: " + formatFloat(requiredSpeed, "", 0, 0) + "+\n" +
		  "S: " + formatFloat(silverSpeed, "", 0, 0) + "+\n" +
		  "G: " + formatFloat(goldSpeed, "", 0, 0) + "+");
		tf.set_font("Caracteres", 40);
		tf.draw_world(21, 1, self.x(), self.y(), 1, 1, 0);

		if (uiVisible)
		{
			string medalText = "";
			if (currentMedal == 1) { medalText = "Bronze"; }
			if (currentMedal == 2) { medalText = "Silver"; }
			s.draw_rectangle_hud(21, 1, 5000, 5000, -5000, -5000, 0, 0xCC000000);
			textfield@ tf2 = create_textfield();
			tf2.text("You got a " + medalText + " Medal!\nWould you like to try again\n\nyes (taunt)           no (dash)");
			tf2.set_font("Caracteres", 40);
			tf2.draw_hud(21, 1, 0, 0, 1, 1, 0);
		}
	}

	void step()
	{
		if (!uiVisible) { return; }
		if (uiInteractionTimer > 0)
		{
			uiInteractionTimer -= 1.0/60.0;
			return;
		}
		entity@ e = controller_entity(0);
		if (@e == null) { return; }
		dustman@ d;
		@d = @e.as_dustman();
		if (@d == null) { return; }
		bool ti = d.taunt_intent() > 0;
		bool di = d.dash_intent() > 0;
		if (ti || di) { get_scene().time_warp(1); uiVisible = false; script.disableUi = false; }
		if (ti)
		{
			currentMedal = 0;
			script.Reset(spawnX, spawnY);
			@script.beatReset = this;
			return;
		}
		if (di)
		{
			script.RecordPoints(currentMedal, 3);
		}
	}

	void activate(controllable@ c)
	{
		if (beat) { return; }
		dustman@ d;
		@d = @c.as_dustman();
		if (@d == null) { return; }
		const float speed = abs(c.x_speed());
		if (speed < requiredSpeed) 
		{
			script.Reset(spawnX, spawnY);
			return;
		}
		beat = true;
		if (speed >= goldSpeed)
		{
			script.RecordPoints(3, 3);
			return;
		}
		uiVisible = true;
		uiInteractionTimer = 0.3;
		// script.disableUi = true;
		get_scene().time_warp(0.02);
		currentMedal = 1;
		if (speed >= silverSpeed) { currentMedal = 2; }
	}
}

class CustomDeathzone : trigger_base, callback_base
{
	script@ script;
	scripttrigger@ self;

	[position,mode:world,layer:21,y:spawnY] float spawnX;
	[hidden] float spawnY;
	
	float spHSize = 20;

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
	}

	void draw(float fuckYouRandomPerson)
	{
		scene@ s = get_scene();
		const float hh = self.height(); //halfheight
		const float hw = self.radius();
		const uint colour = 0x50ff0000;
		s.draw_rectangle_world(21, 1, self.x() - hw, self.y() - hh, self.x() + hw, self.y() + hh, 0, colour);
	}

	void editor_draw(float lolxd) { 
		draw(lolxd);
		get_scene().draw_rectangle_world(21, 1, spawnX-spHSize, spawnY-spHSize, spawnX+spHSize, spawnY+spHSize, 45, 0xffff0000);
	}

	void activate(controllable@ c)
	{
		if (@c.as_dustman() == null) { return; }
		script.Reset(spawnX, spawnY);
	}
}
