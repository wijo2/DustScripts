//DustScripts/map/reverseCollision.cpp

class script : callback_base
{
	script()
	{
		puts("whatever this shit is, working c:");
	}

	bool active = false;

	void CollisionCallback(controllable@ ec, tilecollision@ tc, int side, bool moving, float snap_offset, int arg)
	{
		if (!active)
		{
			ec.check_collision(tc, side, moving, snap_offset);
			return;
		}

		switch (side)
		{
			case 0:
				if (!ec.check_collision(tc, 1, moving, snap_offset)) { return; }
				tc.type(tc.type() + 180);
				tc.hit_x(tc.hit_x() - 30);
				return;
			case 1:
				if (!ec.check_collision(tc, 0, moving, snap_offset)) { return; }
				tc.type(tc.type() - 180);
				tc.hit_x(tc.hit_x() + 30);
				return;
			case 2:
				if (!ec.check_collision(tc, 3, moving, snap_offset)) { return; }
				if (tc.type() > 0) { tc.type(tc.type() - 180); }
				else if (tc.type() <= 0) { tc.type(tc.type() + 180); }
				tc.hit_y(tc.hit_y() - 48);
				return;
			case 3:
				if (!ec.check_collision(tc, 2, moving, snap_offset)) { return; }
				if (tc.type() > 0) { tc.type(tc.type() - 180); }
				else if (tc.type() <= 0) { tc.type(tc.type() + 180); }
				tc.hit_y(tc.hit_y() + 48);
				return;
		}
	}

	void entity_on_remove(entity@ e)
	{
		if (e.as_controllable() is null) { return; }
		active = !active;
	}

	void PlayInit()
	{
		controllable@ player = controller_controllable(uint(get_active_player()));
		player.set_collision_handler(this, "CollisionCallback", 0);
	}

	void on_level_start() { PlayInit(); }
	void checkpoint_load() { PlayInit(); }
}




