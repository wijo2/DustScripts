#include "3dCC.cpp";
#include "3dExtras.cpp";

//prop and specific enemy implementations

class d3Prop : d3FlatObjectBase, trigger_base
{
	scripttrigger@ self;
	sprites@ sprites;
	[text] string spriteSet;
	[text] string sprite;
	[text] uint palette = 1;
	[text] float rotation = 0;
	[text] float scaleX = 1;
	[text] float scaleY = 1;
	[colour,alpha] uint colour = 0xFFFFFFFF;

	void init(script@ s, scripttrigger@ self)
	{
		@this.self = self;
		self.editor_handle_size(0);
		@sprites = create_sprites();
		sprites.add_sprite_set(spriteSet);
		d3FlatObjectBase::init(s, self.as_entity());
		fakeTrigger.colour = 0xFFAA00FF;
	}

	void editor_step() override
	{
		d3FlatObjectBase::editor_step();
	}

	void editor_var_changed(var_info@ info)
	{
		@sprites = create_sprites();
		sprites.add_sprite_set(spriteSet);
	}

	void UpdateRotation() override
	{
		d3FlatObjectBase::UpdateRotation();
		rectangle@ sr = sprites.get_sprite_rect(sprite, 1);
		Vector2 pos = Vector2(fakeTrigger.ssp.x, fakeTrigger.ssp.y);
		drawRect = d2Math::Rect(pos.x + sr.left()*scaleX, pos.y + sr.top()*scaleY, pos.x + sr.right()*scaleX, pos.y + sr.bottom()*scaleY);
	}

	void Draw(scene@ s) override
	{
		if (depth < 0) { return; }
		// return;
		uint ncolour = script.ApplyFog(colour, depth);
		sprites.draw_world(18, 1, sprite, 1, palette, fakeTrigger.ssp.x, 
					 fakeTrigger.ssp.y, rotation, scaleX, scaleY, ncolour);
	}

	void on_remove() override
	{
		d3FlatObjectBase::on_remove();
	}
}

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
