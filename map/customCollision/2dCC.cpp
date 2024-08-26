//WIP!
#include "mathHelper.cpp";

namespace d2
{

class d2Quad
{
	d2Math::Vector2 p1;
	d2Math::Vector2 p2;
	d2Math::Vector2 p3;
	d2Math::Vector2 p4;
	uint colour;

	d2Quad()
	{
		p1 = d2Math::Vector2();
		p2 = d2Math::Vector2();
		p3 = d2Math::Vector2();
		p4 = d2Math::Vector2();
	}

	d2Quad(d2Math::Vector2 p1, d2Math::Vector2 p2, d2Math::Vector2 p3, d2Math::Vector2 p4, uint colour)
	{
		this.p1 = p1;
		this.p2 = p2;
		this.p3 = p3;
		this.p4 = p4;
		this.colour = colour;
	}

	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		s.draw_quad_world(layer, sub_layer, false, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, colour, colour, colour, colour);
	}
}

class d2CQuad
{
	d2Quad base;
	CollisionManager@ manager;

	d2CQuad() { base = d2Quad(); @manager = null; }
	d2CQuad(d2Math::Vector2 p1, d2Math::Vector2 p2, d2Math::Vector2 p3, d2Math::Vector2 p4, uint colour, CollisionManager@ manager)
	{
		base = d2Quad(p1, p2, p3, p4, colour);
		@this.manager = @manager;
	}

	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		base.Draw(s, layer, sub_layer);
		array<d2Math::Vector2> arr = d2Math::LineFunc(base.p1, base.p2).IterateOverLine(
		4,
		base.p1,
		base.p2
		);
		for (uint i = 0; i < arr.length(); i++)
		{
			int x = int(arr[i].x);
			int y = int(arr[i].y);
			s.draw_rectangle_world(
			layer,
			sub_layer,
			x,y,
			x+16,y+16,
			0,
			0x80FF0000);
		}
	}
}

class CollisionManager
{
	array<d2CQuad> collisions;
	//divide map into 16x16 tiles, this is a grid of every seperate 
	//collider inside every tile. 
	//starting from topleft of selected play-area.
	array<array<array<d2Quad>>> collisionGrid;
}

}
