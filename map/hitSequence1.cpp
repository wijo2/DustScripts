//#include "lib/std.cpp"
class sequenceEnemy : enemy_base, callback_base
{
	[position,mode:world,y:y_pos,snap:28|label:"pos helper"] int x;
	[hidden] int y;

	[text] array<PosUI> positions;
	[hidden] uint currentIndex = 0;

	[colour,alpha] uint32 colour;

	scriptenemy@ object;
	sprites@ sprite;


	void init(script@ s, scriptenemy@ self)
	{
		@object = @self;
		
		object.as_hittable().on_hurt_callback(this, "hurt", 0);

		@sprite = @create_sprites();
		sprite.add_sprite_set("apple");

		bool found = false;
		for (uint i = 0; i < s.enemies.length(); i++)
		{
			if (@s.enemies[i] == null)
			{
				@s.enemies[i] = object;
	   			found = true;
	   			break;
			}

			if (s.enemies[i].id() == object.id())
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			int l = s.enemies.length();
			s.enemies.resize(l*2);
			@s.enemies[l] = object;
			//puts("!found " + s.enemies.length());
		}

		if (is_playing())
		{
			s.init();
		}
	}

	void InitPos()
	{
		//puts("enemy init " + object.id() + ", " + currentIndex);
		PosUI c = positions[0];
		//puts(c.x + ", " + c.y);
		object.x(c.x);
		object.y(c.y);
	}

	void draw(float useless_garbage)
	{
		sprite.draw_world(20, 1, "idle", 0, 1, object.x(), object.y(), 0, 1, 1, colour);
	}

	void editor_draw(float useless_garbage)
	{
		sprite.draw_world(20, 1, "idle", 0, 1, object.x(), object.y(), 0, 1, 1, colour);

		for (uint i = 0; i < positions.length(); i++)
		{
			sprite.draw_world(18, 1, "idle", 0, 1, positions[i].x, positions[i].y, 0, 0.5, 0.5, 
					 0xFF000000 + 0x00FFFFFF*i/(positions.length()-1));
		}
	}

	void hurt(controllable@ attacked, controllable@ attacker, hitbox@ attack_hitbox, int arg)
	{
		currentIndex++;
		if (currentIndex == positions.length())
		{
			get_scene().remove_entity(object.as_entity());
		}
		//puts("enemy hurt " + object.id() + ", " + currentIndex);
		PosUI c = positions[currentIndex];
		object.x(c.x);
		object.y(c.y);
	}
}

class PosUI {
  [position,mode:world,y:y,snap:28] int x;
  [hidden] int y;
}


class script
{
	array<scriptenemy@> enemies(1);

	script() 
	{ 
		puts("hitSequence1 working c:"); 
	}

	void init()
	{
		//puts("script init " + enemies.length());
		for (uint i = 0; i < enemies.length(); i++)
		{
			if (@enemies[i] == null) { break; }
			sequenceEnemy@ e = cast<sequenceEnemy@>(enemies[i].get_object());
			e.InitPos();
		}
	}

	void on_level_start() { init(); }
	void checkpoint_load() { init(); }
}
