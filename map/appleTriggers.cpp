#include"../../lib/std.cpp";
class script
{
	[position,mode:world,y:jailY] float jailX;
	[hidden] float jailY;

	script()
	{
		puts("appleTriggers working! c:");
	}
}

class AppleTrigger : trigger_base, callback_base
{
	script@ script;
	scripttrigger@ self;

	[position,mode:world,y:spawnY] float spawnX;
	[hidden] float spawnY;

	[position,mode:world,y:velY] float velX;
	[hidden] float velY;

	[option,"apple","killable"] int type;
	hittable@ apple;
	hittable@ gradeEnemy;

	[text] bool capFallSpeed;
	[text] float maxFallSpeed;

	[text|label:"keep spawn visible"] bool keepVisible;

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
		if (type == 1) { colour = 0xA0556655; }
		s.draw_rectangle_world(21, 1, self.x() - hw, self.y() - hh, self.x() + hw, self.y() + hh, 0, colour);
	}

	void editor_draw(float lolxd) { 
		draw(lolxd);
		if (self.editor_selected() || keepVisible)
		{
			float w = 30;
			scene@ s = get_scene();
			s.draw_rectangle_world(21, 1, spawnX - w, spawnY - w, spawnX + w, spawnY + w, 0, 0x500000FF);
			s.draw_rectangle_world(21, 1, -w/2 + velX, -w/2 + velY, w/2 + velX, w/2 + velY, 0, 0x500040FF);
		}
	}

	void step()
	{
		if (@apple != null && apple.destroyed())
		{
			@apple = null;
			get_scene().remove_entity(gradeEnemy.as_entity());
		}
		if (capFallSpeed && @apple != null && apple.y_speed() > maxFallSpeed)
		{
			apple.set_speed_xy(apple.x_speed(), maxFallSpeed);
		}
	}

	void activate(controllable@ c)
	{
		if (@apple != null || @c.as_dustman() == null) { return; }
		@apple = @create_entity("hittable_apple").as_hittable();
		get_scene().add_entity(apple.as_entity(), true);
		apple.x(spawnX);
		apple.y(spawnY);
		apple.set_speed_xy((velX - spawnX) * 5, (velY - spawnY) * 5);
		if (type == 1)
		{
			apple.on_hurt_callback(this, "KillApple", 0);
		}

		@gradeEnemy = @create_entity("enemy_flag").as_hittable();
		gradeEnemy.x(script.jailX);
		gradeEnemy.y(script.jailY);
		get_scene().add_entity(gradeEnemy.as_entity(), true);
	}

	void KillApple(controllable@ attacked, controllable@ attacker, hitbox@ attack_hitbox, int arg)
	{
		RemoveSelf();
	}

	void RemoveSelf()
	{
		puts("removeSelf!");
		scene@ s = get_scene();
		s.remove_entity(apple.as_entity());
		s.remove_entity(gradeEnemy.as_entity());
	}
}

class AppleKillTrigger : trigger_base
{
	script@ script;
	scripttrigger@ self;

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
		s.draw_rectangle_world(21, 1, self.x() - hw, self.y() - hh, self.x() + hw, self.y() + hh, 0, 0x50FF0000);
	}

	void editor_draw(float lolxd) { draw(lolxd); }

	void activate(controllable@ c)
	{
		if (c.type_name() == "hittable_apple")
		{
			get_scene().remove_entity(c.as_entity());
		}
	}
}
