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

	//does not work me dumb
	bool IsInside(d2Math::Vector2 pos)
	{
		array<float> intersections;

		d2Math::LineFunc l1 = d2Math::LineFunc(p1, p2);
		l1.SetBounds(p1, p2);
		if (l1.IsWithinBounds(d2Math::Vector2(l1.GetRevValue(pos.y), pos.y))) { intersections.insertLast(l1.GetRevValue(pos.y)); }
		d2Math::LineFunc l2 = d2Math::LineFunc(p2, p3);
		l2.SetBounds(p2, p3);
		if (l2.IsWithinBounds(d2Math::Vector2(l2.GetRevValue(pos.y), pos.y))) { intersections.insertLast(l2.GetRevValue(pos.y)); }
		d2Math::LineFunc l3 = d2Math::LineFunc(p3, p4);
		l3.SetBounds(p3, p4);
		if (l3.IsWithinBounds(d2Math::Vector2(l3.GetRevValue(pos.y), pos.y))) { intersections.insertLast(l3.GetRevValue(pos.y)); }
		d2Math::LineFunc l4 = d2Math::LineFunc(p4, p1);
		l4.SetBounds(p4, p1);
		if (l4.IsWithinBounds(d2Math::Vector2(l4.GetRevValue(pos.y), pos.y))) { intersections.insertLast(l4.GetRevValue(pos.y)); }
		if (intersections.length() < 2) { return false; }
		return (intersections[0] - pos.x) * (intersections[1] - pos.x) < 0;
	}
}

class d2CQuad
{
	d2Quad@ base;
	CollisionManager@ manager;
	array<array<int>> oldCollision;

	array<int> activeLines = {1,2,3,4};

	//to be implemented
	array<int> spikeLines;
	array<int> dustLines;

	d2CQuad() { @base = @d2Quad(); @manager = null; }
	d2CQuad(d2Math::Vector2 p1, d2Math::Vector2 p2, d2Math::Vector2 p3, d2Math::Vector2 p4, uint colour, CollisionManager@ manager)
	{
		@base = @d2Quad(p1, p2, p3, p4, colour);
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
		
		ClearOldCache();

		array<d2Math::Vector2> arr;
		for (uint i = 0; i < activeLines.length(); i++)
		{
			array<d2Math::Vector2> arr1 = d2Math::LineFunc(PointByNumber(i), PointByNumber(i+1))
				.IterateOverLine(manager.collisionOrder, PointByNumber(i), PointByNumber(i+1));
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

	void ClearOldCache() 
	{
		for (uint i = 0; i < oldCollision.length(); i++)
		{
			int x = oldCollision[i][0];
			int y = oldCollision[i][1];
			manager.RemoveFromCG(x, y, this);
		}

		oldCollision = array<array<int>>(0);
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
	//divide map into x*x tiles, this is a grid of every seperate 
	//collider inside every tile. 
	//starting from topleft of selected play-area.
	array<array<array<d2CQuad@>>> collisionGrid;

	d2Math::IntRect playArea;
	//changes the size of collision cache squares.
	//smaller = better performance up untill like 3-5 with very diminishing returns but bigger lagspike at load
	//bigger = worse performance but smaller lagspike at load
	int collisionOrder = 4;
	
	script@ s;

	CollisionManager()
	{
		//idk why but using collisionOrder here doesn't work, so I have to do this shit instead
		playArea = d2Math::IntRect();
	}

	void Init(d2Math::IntRect playArea)
	{
		this.playArea = d2Math::IntRect(
			d2Math::RoundToOrder(playArea.x1, collisionOrder), 
			d2Math::RoundToOrder(playArea.y1, collisionOrder), 
			d2Math::RoundToOrder(playArea.x2, collisionOrder), 
			d2Math::RoundToOrder(playArea.y2, collisionOrder)
		);
		uint w = this.playArea.width >> collisionOrder;
		uint h = this.playArea.height >> collisionOrder;
		collisionGrid.resize(w);
		for (uint i = 0; i < w; i++) {
			collisionGrid[i].resize(h);
		}
	}

	void PlayInit(script@ s, d2Math::IntRect playArea)
	{
		Init(playArea);
		@this.s = @s;
		CollisionOverride@ c = CollisionOverride(this);
		controllable@ player = controller_controllable(uint(get_active_player()));
		player.set_collision_handler(c, "CollisionCallback", 0);
	}

	//takes in real world coords
	void AddToCG(int x, int y, d2CQuad@ quad)
	{
		uint nx = (x - playArea.x1) >> collisionOrder;
		uint ny = (y - playArea.y1) >> collisionOrder;
		if (nx < 0 || ny < 0 || nx >= playArea.width >> collisionOrder || ny >= playArea.height >> collisionOrder)
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
		uint nx = (x - playArea.x1) >> collisionOrder;
		uint ny = (y - playArea.y1) >> collisionOrder;
		if (nx < 0 || ny < 0 || nx >= playArea.width >> collisionOrder || ny >= playArea.height >> collisionOrder)
		{
			return;
		}
		int i = collisionGrid[nx][ny].findByRef(quad);
		if (i >= 0)
		{
			collisionGrid[nx][ny].removeAt(i);
		}
	}

	array<d2CQuad@> GetCollidersInArea(d2Math::IntRect rect, bool debug = false)
	{
		array<d2CQuad@> result;

		uint x1 = (rect.x1 - playArea.x1) >> collisionOrder;
		uint y1 = (rect.y1 - playArea.y1) >> collisionOrder;
		uint x2 = (rect.x2 - playArea.x1) >> collisionOrder;
		uint y2 = (rect.y2 - playArea.y1) >> collisionOrder;

		for (uint x = x1; x <= x2; x++)
		{
			for (uint y = y1; y <= y2; y++)
			{
				if (debug) 
				{
					int w = 1 << collisionOrder;
					s.debugDraw.insertLast(d2Math::Rect((x << collisionOrder) + playArea.x1, (y << collisionOrder) + playArea.y1, (x << collisionOrder) + playArea.x1 + w, (y << collisionOrder) + playArea.y1 + w));
				}
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

	//debug
	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		for (uint x = 0; x < collisionGrid.length(); x++)
		{
			for (uint y = 0; y < collisionGrid.length(); y++)
			{
				if (collisionGrid[x][y].length() == 0) { continue; }
				int dx = (x << collisionOrder) + playArea.x1;
				// puts(playArea.x1 + "");
				int dy = (y << collisionOrder) + playArea.y1;
				s.draw_rectangle_world(
					layer,
					sub_layer,
					dx,dy,
					dx+(1 << collisionOrder),dy+(1 << collisionOrder),
					0,
					0x8000FF00
				);
			}
		}
	}
}

}
