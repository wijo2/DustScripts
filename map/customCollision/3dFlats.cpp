#include "3dCC.cpp";
#include "3dExtras.cpp";

//prop and specific enemy implementations

class d3Prop : d3FlatObjectBase, trigger_base
{
	[text|tooltip:"what layer is drawn on, don't change in normal circumstances."] int layer = 18;
	[text|tooltip:"what sub layer is drawn on, don't change in normal circumstances."] int sub_layer = 1;
	scripttrigger@ self;
	sprites@ sprites;
	[text|tooltip:"name of sprite set"] string spriteSet;
	[text|tooltip:"name of sprite"] string sprite;
	[text|tooltip:"palette number"] uint palette = 1;
	[text|tooltip:"2d rotation of sprite, layering doesn't\naccount for this so be careful."] float rotation = 0;
	[text] float scaleX = 1;
	[text] float scaleY = 1;
	[colour,alpha|tooltip:"fog trigger like colour"] uint colour = 0xFFFFFFFF;

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

//add new enemies by just making a new class following the pattern c:
//if you want new functionality override something from the chosen base class
//or add it in the base class as long as it doesn't disturb existing functionality
class d3ESmallPrism : d3EnemyBase, trigger_base
{
	void init(script@ s, scripttrigger@ self)
	{
		// get_scene().remove_entity(self.as_entity());
		sprite = "airidle";
		spritesName = "tutorial_square";
		entityName = "enemy_tutorial_square";
		thickness = 80;
		d3EnemyBase::init(s, self);
	}
}

class d3EBigPrism : d3EnemyBase, trigger_base
{
	void init(script@ s, scripttrigger@ self) override
	{
		// get_scene().remove_entity(self.as_entity());
		sprite = "airidle";
		spritesName = "tutorial_hexagon";
		entityName = "enemy_tutorial_hexagon";
		thickness = 80;
		d3EnemyBase::init(s, self);
	}
}

class d3EApple : d3MovingEnemyBase, trigger_base
{
	void init(script@ s, scripttrigger@ self) override
	{
		sprite = "fall";
		spritesName = "apple";
		entityName = "hittable_apple";
		thickness = 80;
		verticalOffset = -20;
		d3MovingEnemyBase::init(s, self);
	}
}
