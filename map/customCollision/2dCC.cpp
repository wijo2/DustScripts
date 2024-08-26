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
	array<array<int>> oldCollision;

	d2CQuad() { base = d2Quad(); @manager = null; }
	d2CQuad(d2Math::Vector2 p1, d2Math::Vector2 p2, d2Math::Vector2 p3, d2Math::Vector2 p4, uint colour, CollisionManager@ manager)
	{
		base = d2Quad(p1, p2, p3, p4, colour);
		@this.manager = @manager;
		UpdateCollision();
	}

	void UpdateCollision()
	{
		if (@manager == null) { return; }
		for (uint i = 0; i < oldCollision.length(); i++)
		{
			int x = oldCollision[i][0];
			int y = oldCollision[i][1];
			if (i == 0) {puts(x + ", " + y);}
			manager.RemoveFromCG(x, y, this);
		}

		oldCollision = array<array<int>>(0);

		array<d2Math::Vector2> arr = d2Math::LineFunc(base.p1, base.p2)
			.IterateOverLine(4, base.p1, base.p2);
		array<d2Math::Vector2> arr1 = d2Math::LineFunc(base.p2, base.p3)
			.IterateOverLine(4, base.p2, base.p3);
		arr = AddArrays(arr, arr1);
		arr1 = d2Math::LineFunc(base.p3, base.p4)
			.IterateOverLine(4, base.p3, base.p4);
		arr = AddArrays(arr, arr1);
		arr1 = d2Math::LineFunc(base.p4, base.p1)
			.IterateOverLine(4, base.p4, base.p1);
		arr = AddArrays(arr, arr1);
		for (uint i = 0; i < arr.length(); i++)
		{
			int x = int(arr[i].x);
			int y = int(arr[i].y);
			if (i == 0) {puts(x + ", " + y);}
			oldCollision.insertLast(array<int> = {x, y});
			manager.AddToCG(x, y, this);
		}
	}

	array<d2Math::Vector2> AddArrays(array<d2Math::Vector2> a1, array<d2Math::Vector2> a2)
	{
		array<d2Math::Vector2> ret;
		for (uint i = 0; i < a1.length(); i++)
		{
			ret.insertLast(a1[i]);
		}
		for (uint i = 0; i < a2.length(); i++)
		{
			ret.insertLast(a2[i]);
		}
		return ret;
	}

	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		base.Draw(s, layer, sub_layer);
		array<d2Math::Vector2> arr = d2Math::LineFunc(base.p1, base.p2)
			.IterateOverLine(4, base.p1, base.p2);
		array<d2Math::Vector2> arr1 = d2Math::LineFunc(base.p2, base.p3)
			.IterateOverLine(4, base.p2, base.p3);
		arr = AddArrays(arr, arr1);
		arr1 = d2Math::LineFunc(base.p3, base.p4)
			.IterateOverLine(4, base.p3, base.p4);
		arr = AddArrays(arr, arr1);
		arr1 = d2Math::LineFunc(base.p4, base.p1)
			.IterateOverLine(4, base.p4, base.p1);
		arr = AddArrays(arr, arr1);
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
	array<d2CQuad@> collisions;
	//divide map into 16x16 tiles, this is a grid of every seperate 
	//collider inside every tile. 
	//starting from topleft of selected play-area.
	array<array<array<d2CQuad@>>> collisionGrid;

	//topleft corner based rect of play area
	int cornerX;
	int cornerY;
	int width;
	int height;

	CollisionManager()
	{
		cornerX = 0;
		cornerY = 0;
		width = 0;
		height = 0;
	}
	CollisionManager(int cornerX, int cornerY, uint width, uint height)
	{
		this.cornerX = cornerX - cornerX % 16;
		this.cornerY = cornerY - cornerY % 16;
		this.width = width;
		this.height = height;
		uint w = width >> 4;
		uint h = height >> 4;
		collisionGrid.resize(w);
		for (uint i = 0; i < w; i++)
		{
			collisionGrid[i].resize(h);
		}
	}

	//takes in real world coords
	void AddToCG(int x, int y, d2CQuad@ quad)
	{
		int nx = (x - cornerX) >> 4;
		int ny = (y - cornerY) >> 4;
		if (nx < 0 || ny < 0 || nx >= width >> 4 || ny >= height >> 4)
		{
			return;
		}
		if (collisionGrid[nx][ny].findByRef(quad) < 0)
		{
			collisionGrid[nx][ny].insertLast(quad);
		}
	}

	void RemoveFromCG(int x, int y, d2CQuad@ quad)
	{
		int nx = (x - cornerX) >> 4;
		int ny = (y - cornerY) >> 4;
		if (nx < 0 || ny < 0 || nx >= width >> 4 || ny >= height >> 4)
		{
			return;
		}
		int i = collisionGrid[nx][ny].findByRef(quad);
		if (i >= 0)
		{
			collisionGrid[nx][ny].removeAt(i);
		}
	}

	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		for (uint x = 0; x < collisionGrid.length(); x++)
		{
			for (uint y = 0; y < collisionGrid.length(); y++)
			{
				if (collisionGrid[x][y].length() == 0) { continue; }
				int dx = (x << 4) + cornerX;
				int dy = (y << 4) + cornerY;
				s.draw_rectangle_world(
				layer,
				sub_layer,
				dx,dy,
				dx+16,dy+16,
				0,
				0x8000FF00);
			}
		}
		
	}
}

}
