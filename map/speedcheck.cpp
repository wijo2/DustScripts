//DustScripts/map/speedcheck.cpp
class script
{
	float lastValue = 0;

	script()
	{
		puts("speedcheck working! c:");
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

		scene@ s = get_scene();
		if (found)
		{
			lastValue = abs(d.x_speed());
			textfield@ tf = create_textfield();
			tf.text(formatFloat(abs(d.x_speed()), "", 0, 0));
			tf.set_font("Caracteres", 40);
			tf.colour(0xFF000000);
			tf.draw_hud(21, 1, 700, -398, 1, 1, 0);
			tf.colour(0xFFFFFFFF);
			tf.draw_hud(21, 1, 699, -400, 1, 1, 0);
		}
		else
		{
			textfield@ tf = create_textfield();
			tf.text(formatFloat(lastValue, "", 0, 0));
			tf.set_font("Caracteres", 40);
			tf.colour(0xFF000000);
			tf.draw_hud(21, 1, 700, -398, 1, 1, 0);
			tf.colour(0xFFFFFFFF);
			tf.draw_hud(21, 1, 699, -400, 1, 1, 0);
		}
	}
}

class SpeedcheckTrigger : trigger_base, callback_base
{
	script@ script;
	scripttrigger@ self;

	[text] float requiredSpeed;

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
		float hh = self.height(); //halfheight
		float hw = self.radius();
		uint colour = 0x5000FF00;
		s.draw_rectangle_world(21, 1, self.x() - hw, self.y() - hh, self.x() + hw, self.y() + hh, 0, colour);
		textfield@ tf = create_textfield();
		tf.text(formatFloat(requiredSpeed, "", 0, 0) + "+");
		tf.set_font("Caracteres", 40);
		tf.draw_world(21, 1, self.x(), self.y(), 1, 1, 0);
	}

	void editor_draw(float lolxd) { 
		draw(lolxd);
	}

	void activate(controllable@ c)
	{
		if (@c.as_dustman() == null) { return; }
		if (abs(c.x_speed()) < requiredSpeed) { c.as_dustman().kill(false); }
	}
}
