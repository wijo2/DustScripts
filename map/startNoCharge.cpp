//DustScripts\map\startNoCharge.cpp
class script
{
	script()
	{
		puts("airchargeRemoval working c:");
	}

	void on_level_start()
	{
		dustman@ p = @controller_controllable(get_active_player()).as_dustman();
		p.dash(0);
	}
	void checkpoint_load() { on_level_start(); }
}
