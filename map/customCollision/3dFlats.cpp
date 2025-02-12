#include "3dCC.cpp";
#include "3dExtras.cpp";

//prop and specific enemy implementations

class d3ESmallPrism : d3EnemyBase, trigger_base
{
	void init(script@ s, scripttrigger@ self)
	{
		sprite = "airidle";
		d3EnemyBase::init(s, self, create_entity("enemy_tutorial_square"));
	}
}
