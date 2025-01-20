#include "2dCC.cpp";
#include "mathHelper.cpp";

namespace d3
{

class d3Cam
{
	d3Math::Vector3 centre;
	float rotation; //0 = normal xy, z away from cam, upwards = angle in radians topdown clockwise
}

class d3Quad
{
	d3Math::Vector3 p1;
	d3Math::Vector3 p2;
	d3Math::Vector3 p3;
	d3Math::Vector3 p4;
	uint colour;

	//""projected"" positions
	d2Math::Vector2 pp1;
	d2Math::Vector2 pp2;
	d2Math::Vector2 pp3;
	d2Math::Vector2 pp4;

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
		if (colour == 0x00000000) { return; }
		s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp3.x, pp3.y, pp3.x, pp3.y, colour, colour, colour, colour);
		s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
		s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
		s.draw_quad_world(layer, sub_layer, false, pp2.x, pp2.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
	}

	void ApplyProjection(d3Cam@ cam)
	{
		float s = sin(cam.rotation);
		float c = cos(cam.rotation);

		pp1 = d2Math
	}


	}


}

}
