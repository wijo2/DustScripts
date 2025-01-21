#include "2dCC.cpp";
#include "mathHelper.cpp";

namespace d3
{
class nothing{} //this tricks my lsp to obey

class d3Cam
{
	d3Math::Vector3 centre;
	float rotation; //0 = normal xy, z away from cam, upwards = angle in radians topdown clockwise

	d3Math::Vector3 WorldToCamPos(d3Math::Vector3 vec)
	{
		vec -= centre;
		float c = cos(rotation);
		float s = sin(rotation);
		return d3Math::Vector3(
			vec.x * c - vec.z * s,
			vec.y,
			vec.x * s + vec.z * c
		);
	}
}

class nothing2{} //this tricks my lsp to obey
class d3Quad
{
	d3Math::Vector3 p1;
	d3Math::Vector3 p2;
	d3Math::Vector3 p3;
	d3Math::Vector3 p4;
	uint colour;

	//cam space coords
	d3Math::Vector3 csp1;
	d3Math::Vector3 csp2;
	d3Math::Vector3 csp3;
	d3Math::Vector3 csp4;

	//""projected"" positions
	d2Math::Vector2 pp1;
	d2Math::Vector2 pp2;
	d2Math::Vector2 pp3;
	d2Math::Vector2 pp4;

	//behind cam, don't draw
	bool behind;
	//intersecting with cam plane
	bool intersecting;
	//false for 1-3, true for 2-2
	bool intersectionType;

	//for every point, which side of cam is it on?
	//1 = front
	array<bool> sides(4);

	d3Quad()
	{
		p1 = d3Math::Vector3();
		p2 = d3Math::Vector3();
		p3 = d3Math::Vector3();
		p4 = d3Math::Vector3();
	}

	d3Quad(d3Math::Vector3 p1, d3Math::Vector3 p2, d3Math::Vector3 p3, d3Math::Vector3 p4, uint colour)
	{
		this.p1 = p1;
		this.p2 = p2;
		this.p3 = p3;
		this.p4 = p4;
		this.colour = colour;
	}

	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		if (colour == 0x00000000 || behind) { return; }
		s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp3.x, pp3.y, pp3.x, pp3.y, colour, colour, colour, colour);
		s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
		s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
		s.draw_quad_world(layer, sub_layer, false, pp2.x, pp2.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
	}

	void ApplyProjection(d3Cam@ cam)
	{
		csp1 = cam.WorldToCamPos(p1);
		csp2 = cam.WorldToCamPos(p2);
		csp3 = cam.WorldToCamPos(p3);
		csp4 = cam.WorldToCamPos(p4);

		pp1 = d2Math::Vector2(csp1.x, cps1.y);
		pp2 = d2Math::Vector2(csp2.x, cps2.y);
		pp3 = d2Math::Vector2(csp3.x, cps3.y);
		pp4 = d2Math::Vector2(csp4.x, cps4.y);

		int behind = 0;
		if (csp1.z <= 0) { behind += 1; sides[0] = 0; } else { sides[0] = 1; }
		if (csp2.z <= 0) { behind += 1; sides[1] = 0; } else { sides[1] = 1; }
		if (csp3.z <= 0) { behind += 1; sides[2] = 0; } else { sides[2] = 1; }
		if (csp4.z <= 0) { behind += 1; sides[3] = 0; } else { sides[3] = 1; }
		this.behind = behind == 4;
		this.intersecting = !this.behind && behind > 0;
		intersectionType = behind == 2;
	}

	//sides:
	//1: 1, 2, 3
	//2: 1, 2, 4
	//3: 1, 3, 4
	//4: 2, 3, 4
	bool DoesSideIntersect(int s)
	{
		return (s == 1 && (sides[0] != sides[1] || sides[1] != sides[2])) ||
		(s == 2 && (sides[0] != sides[1] || sides[1] != sides[3])) ||
		(s == 3 && (sides[0] != sides[2] || sides[2] != sides[3])) ||
		(s == 4 && (sides[1] != sides[2] || sides[2] != sides[3]));
	}

	bool GetSide(int point)
	{
		switch (point)
		{
			case 1: return csp1.z > 0;
			case 2: return csp2.z > 0;
			case 3: return csp3.z > 0;
			case 4: return csp4.z > 0;
		}
	}

	bool IsIntersected(int side)
	{
		switch (side)
		{
			case 1: return GetSide(1) == GetSide(2) && GetSide(2) == GetSide(3);
			case 2: return GetSide(1) == GetSide(2) && GetSide(2) == GetSide(4);
			case 3: return GetSide(1) == GetSide(3) && GetSide(3) == GetSide(4);
			case 4: return GetSide(2) == GetSide(3) && GetSide(3) == GetSide(4);
		}
	}

}

class nothing3{} //this tricks my lsp to obey

class d3CQuad
{
	d3Quad@ base;
	d2::d2CQuad collisionBase;

	//sides:
	//1: 1, 2, 3
	//2: 1, 2, 4
	//3: 1, 3, 4
	//4: 2, 3, 4
	array<bool> activeSides(4);

	array<bool> spikeSides(4);
	array<bool> dustSides(4);

	//given 2 sides, what are the 2 points that they share (points 1 indexed)
	//for x,x pairs it's empty array
	array<array<array<int>>> sideLookup = {
		{{}, {1,2}, {1,3}, {2,3}},
		{{1,2}, {}, {1,4}, {2,4}},
		{{1,3}, {1,4}, {}, {3,4}},
		{{2,3}, {2,4}, {3,4}, {}}
	};

	//doesn't update collision
	void UpdateIntersectQuad(d3Cam@ cam)
	{
		if (!base.intersecting) { return; }
		int currentSide = 1;
		if (!base.intersectionType && !base.IsIntersected(1)) { currentSide = 2; }
		int lastSide = 0;
		//3-1 = triangle
		if (base.intersectionType)
		{
			lastSide = GetNextSide(currentSide, lastSide);
		}
	}

	//next side for loop around intersection that is not side2
	int GetNextSide(int side, int side2)
	{
		auto shared1 = sideLookup[side-1][0];
		auto shared2 = sideLookup[side-1][1];
		auto shared3 = sideLookup[side-1][2];
		auto shared4 = sideLookup[side-1][3];
		if (side != 1 && side2 != 1 &&
			((base.intersectionType && base.GetSide(shared1[0]) != base.GetSide(shared1[1])) || 
			(!base.intersectionType && base.IsIntersected(1)))) { return 1; }
		if (side != 2 && side2 != 2 &&
			((base.intersectionType && base.GetSide(shared2[0]) != base.GetSide(shared2[1])) || 
			(!base.intersectionType && base.IsIntersected(2)))) { return 2; }
		if (side != 3 && side2 != 3 &&
			((base.intersectionType && base.GetSide(shared3[0]) != base.GetSide(shared3[1])) || 
			(!base.intersectionType && base.IsIntersected(3)))) { return 3; }
		if (side != 4 && side2 != 4 &&
			((base.intersectionType && base.GetSide(shared4[0]) != base.GetSide(shared4[1])) || 
			(!base.intersectionType && base.IsIntersected(4)))) { return 4; }
		return 0;
	}
}	

class d3Manager
{

}

}
