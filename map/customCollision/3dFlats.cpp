#include "3dCC.cpp";
#include "3dExtras.cpp";

//prop and specific enemy implementations

class d3ESmallPrism : d3EnemyBase, trigger_base
{
	[hidden] int entityId = -1;
	void init(script@ s, scripttrigger@ self)
	{
		// get_scene().remove_entity(self.as_entity());
		sprite = "airidle";
		if (entityId == -1) 
		{
			puts("neg");
			@entity = create_entity("enemy_tutorial_square");
			entityId = entity.id();
			puts("new " + entityId);
		}
		else
		{
			puts("id: " + entityId);
			@entity = @entity_by_id(uint(entityId));
			if (@entity == null)
			{
				entityId = -1;
				puts("couldn't find");
			}
		}
		d3EnemyBase::init(s, self);
	}
}
