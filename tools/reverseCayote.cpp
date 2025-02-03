class script
{
	int crouchTimer = 0;
	void step(int idk)
	{
		controllable@ c = controller_entity(uint(get_active_player())).as_controllable();
		int state = c.state();
		dustman@ d = c.as_dustman();
		if (state == 8 && crouchTimer < 4 && crouchTimer > 0 && d.dash() != 0) { d.dash(d.dash()-1); }
		if (state == 10) { crouchTimer++; }
		else { crouchTimer = 0; }
	}
}
