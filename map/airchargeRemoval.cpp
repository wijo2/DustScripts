//DustScripts\map\airchargeRemoval.cpp

class script
{
	script()
	{
		puts("airchargeRemoval working c:");
	}

	void on_level_start()
	{
		dustman@ p = @controller_controllable(get_active_player()).as_dustman();
		p.dash_max(0);
	}
}
