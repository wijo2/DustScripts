class script
{
	array<scriptenemy@> enemies(1);

	script() 
	{ 
		puts("noHitrise working c:"); 
	}

	void init()
	{
		for (int i = 0; i < 4; i++)
		{
			entity@ e = controller_entity(i);
			if (@e == null) { continue; }
			dustman@ d = e.as_dustman();
			if (@d == null) { continue; }
			d.hitrise_speed(99999);
		}
	}

	void on_level_start() { init(); }
	void checkpoint_load() { init(); }
}
