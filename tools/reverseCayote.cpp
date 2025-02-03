class script
{
	int crouchTimer = 0;
	void step(int idk)
	{
		controllable@ c = controller_entity(uint(get_active_player())).as_controllable();
		int state = c.state();
		dustman@ d = c.as_dustman();
		if (state == 8 && crouchTimer < 7 && crouchTimer > 0 && d.dash() != 0) { d.dash(d.dash()-1); }
		else if (state == 10) { crouchTimer++; }
		else if (state == 11 || state == 12 || state == 13) { crouchTimer = 7; }
		else if (state != 11 && state != 12 && state != 13) { crouchTimer = 0; }
	}
}
