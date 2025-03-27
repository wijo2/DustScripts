//DustScripts/map/maxSpecial.cpp

class script
{
	script(){}

	void step(int no)
	{
		entity@ e = controller_entity(0);
		if (@e == null) { return; }
		dustman@ d = e.as_dustman();
		if (@d == null) { return; }
		d.skill_combo_max(0);
	}
}
