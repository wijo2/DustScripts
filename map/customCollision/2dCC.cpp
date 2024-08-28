//WIP!
#include "mathHelper.cpp";
#include "2dCollisionOverride.cpp";

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

	array<int> activeLines = {1,2,3,4};
	array<int> spikeLines;
	array<int> dustLines;

	d2CQuad() { base = d2Quad(); @manager = null; }
	d2CQuad(d2Math::Vector2 p1, d2Math::Vector2 p2, d2Math::Vector2 p3, d2Math::Vector2 p4, uint colour, CollisionManager@ manager)
	{
		base = d2Quad(p1, p2, p3, p4, colour);
		@this.manager = @manager;
		UpdateCollision();
	}

	d2Math::Vector2@ PointByNumber(int n)
	{
		int n2 = n;
		if (n2 > 4) { n2 -= 4; }
		if (n2 < 1) { n2 += 4; }
		switch (n2)
		{
			case 1:
				return @base.p1;
			case 2:
				return @base.p2;
			case 3:
				return @base.p3;
			case 4:
				return @base.p4;
		}
		return d2Math::Vector2();
	}

	void UpdateCollision()
	{
		if (@manager == null) { return; }
		for (uint i = 0; i < oldCollision.length(); i++)
		{
			int x = oldCollision[i][0];
			int y = oldCollision[i][1];
			manager.RemoveFromCG(x, y, this);
		}

		oldCollision = array<array<int>>(0);

		array<d2Math::Vector2> arr;
		for (uint i = 0; i < activeLines.length(); i++)
		{
			array<d2Math::Vector2> arr1 = d2Math::LineFunc(PointByNumber(i), PointByNumber(i+1))
				.IterateOverLine(4, PointByNumber(i), PointByNumber(i+1));
			arr = AddArrays(arr, arr1);
		}

		for (uint i = 0; i < arr.length(); i++)
		{
			int x = int(arr[i].x);
			int y = int(arr[i].y);
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

	d2Math::Vector2 FindCentre()
	{
		return d2Math::Vector2(
			(base.p1.x + base.p2.x + base.p3.x + base.p4.x) / 4,
			(base.p1.y + base.p2.y + base.p3.y + base.p4.y) / 4);

	}

	d2Math::LineFunc GetLineFunc(int l)
	{
		d2Math::LineFunc r = d2Math::LineFunc(PointByNumber(l), PointByNumber(l+1));
		r.SetBounds(PointByNumber(l), PointByNumber(l+1));
		return r;
	}

	array<d2Math::LineFunc> GetSideEdges(int side)
	{
		array<d2Math::LineFunc> result;
		d2Math::Vector2 centre = FindCentre();
		for (int l = 1; l <= 4; l++)
		{
			d2Math::LineFunc f = GetLineFunc(l);
			if (f.nullLine || activeLines.find(l) < 0) { continue; }
			switch (side)
			{
				case 0:
					if (centre.x < f.GetRevValue(centre.y)) { result.insertLast(f); }
				break;
				case 1:
					if (centre.x > f.GetRevValue(centre.y)) { result.insertLast(f); }
				break;
				case 2:
					if (centre.y < f.GetValue(centre.x)) { result.insertLast(f); }
				break;
				case 3:
					if (centre.y > f.GetValue(centre.x)) { result.insertLast(f); }
				break;
			}
		}
		return result;
	}

	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		base.Draw(s, layer, sub_layer);
		// DrawDebug(s, layer, sub_layer);
	}

	void DrawDebug(scene@ s, uint layer, uint sub_layer)
	{
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
				0x80FF0000
			);
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

	d2Math::IntRect playArea;

	CollisionManager()
	{
		playArea = d2Math::IntRect();
	}
	CollisionManager(d2Math::IntRect playArea)
	{
		this.playArea = d2Math::IntRect(playArea.x1 - playArea.x1 % 16, playArea.y1 - playArea.y1 % 16, playArea.width, playArea.height);
		uint w = this.playArea.width >> 4;
		uint h = this.playArea.height >> 4;
		collisionGrid.resize(w);
		for (uint i = 0; i < w; i++) {
			collisionGrid[i].resize(h);
		}
		if (is_playing())
		{
			controller_controllable(uint(get_active_player())).
				set_collision_handler(CollisionOverride(this), "CollisionCallback", 0);
		}
	}

	//takes in real world coords
	void AddToCG(int x, int y, d2CQuad@ quad)
	{
		uint nx = (x - playArea.x1) >> 4;
		uint ny = (y - playArea.y1) >> 4;
		if (nx < 0 || ny < 0 || nx >= playArea.width >> 4 || ny >= playArea.height >> 4)
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
		uint nx = (x - playArea.x1) >> 4;
		uint ny = (y - playArea.y1) >> 4;
		if (nx < 0 || ny < 0 || nx >= playArea.width >> 4 || ny >= playArea.height >> 4)
		{
			return;
		}
		int i = collisionGrid[nx][ny].findByRef(quad);
		if (i >= 0)
		{
			collisionGrid[nx][ny].removeAt(i);
		}
	}

	array<d2CQuad@> GetCollidersInArea(d2Math::IntRect rect)
	{
		array<d2CQuad@> result;

		uint x1 = (rect.x1 - playArea.x1) >> 4;
		uint y1 = (rect.y1 - playArea.y1) >> 4;
		uint x2 = (rect.x2 - playArea.x1) >> 4;
		uint y2 = (rect.y2 - playArea.y1) >> 4;

		for (uint x = x1; x <= x2; x++)
		{
			for (uint y = y1; y <= y2; y++)
			{
				for (uint i = 0; i < collisionGrid[x][y].length(); i++)
				{
					if (result.findByRef(collisionGrid[x][y][i]) < 0)
					{
						result.insertLast(collisionGrid[x][y][i]);
					}
				}
			}
		}

		return result;
	}

	//I'll leave this here for debugging purposes. warning: extremely laggy with big play area, recommend 4k*4k max
	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		for (uint x = 0; x < collisionGrid.length(); x++)
		{
			for (uint y = 0; y < collisionGrid.length(); y++)
			{
				if (collisionGrid[x][y].length() == 0) { continue; }
				int dx = (x << 4) + playArea.x1;
				int dy = (y << 4) + playArea.y1;
				s.draw_rectangle_world(
					layer,
					sub_layer,
					dx,dy,
					dx+16,dy+16,
					0,
					0x8000FF00
				);
			}
		}
	}
}

}
