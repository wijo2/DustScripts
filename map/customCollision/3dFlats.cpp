#include "3dCC.cpp";
#include "3dExtras.cpp";

//prop and specific enemy implementations

class d3ESmallPrism : d3EnemyBase, trigger_base
{
	void init(script@ s, scripttrigger@ self)
	{
		// get_scene().remove_entity(self.as_entity());
		sprite = "airidle";
		thickness = 80;
		if (entityId == -1) 
		{
			@entity = create_entity("enemy_tutorial_square");
			get_scene().add_entity(entity);
			entityId = entity.id();
			// puts("new " + entityId);
		}
		else
		{
			// puts("id: " + entityId);
			@entity = @entity_by_id(uint(entityId));
			if (@entity == null)
			{
				// puts("couldn't find");
				@entity = create_entity("enemy_tutorial_square");
				get_scene().add_entity(entity);
				entityId = entity.id();
				// puts("new " + entityId);
			}
		}
		d3EnemyBase::init(s, self);
	}
}

class d3EBigPrism : d3EnemyBase, trigger_base
{
	void init(script@ s, scripttrigger@ self)
	{
		// get_scene().remove_entity(self.as_entity());
		sprite = "airidle";
		thickness = 120;
		if (entityId == -1) 
		{
			@entity = create_entity("enemy_tutorial_hexagon");
			get_scene().add_entity(entity);
			entityId = entity.id();
			// puts("new " + entityId);
		}
		else
		{
			// puts("id: " + entityId);
			@entity = @entity_by_id(uint(entityId));
			if (@entity == null)
			{
				// puts("couldn't find");
				@entity = create_entity("enemy_tutorial_square");
				get_scene().add_entity(entity);
				entityId = entity.id();
				// puts("new " + entityId);
			}
		}
		d3EnemyBase::init(s, self);
	}
}
