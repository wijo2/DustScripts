#include "3dCC.cpp";
#include "3dExtras.cpp";

class fuckThis{}
class d3NodeCluster : trigger_base 
{
	[hidden] bool hasInit = false;
	[text|tooltip:"what layer is drawn on, don't change in normal circumstances."] int layer;
	[text|tooltip:"what sub layer is drawn on, don't change in normal circumstances."] int sub_layer;
	[colour,alpha|tooltip:"colour of 2d slices"] uint d2colour;
	[colour,alpha|tooltip:"colour of 3d shape"] uint d3colour;

	[text|tooltip:"disables sorting between cluster members.\nVery efficient but will cause layering problems\nif cluster isn't horisontally convex."] bool isConvex = true;
	[text|tooltip:"enables an important optimisation that shouldn't\ncause problems but added this setting just in case\ncause I'm not 100% sure"] bool simplerLayering = true;
	[text|tooltip:"enables line comparisons, this can fix a specific\nissue but is EXTREMELY SLOW.\nOnly use if you absolutely have to."] bool lineComp = false;

	//when nodes are deleted they're just set to 0
	//to preserve quad nodes so work around that c:
	[hidden] array<Vector3> nodes;
	array<uint> selectedNodes;

	//"tile entities"
	//node triplets stored
	[hidden] array<array<uint>> spikes;
	[hidden] array<array<uint>> dust;
	[hidden] array<array<uint>> deactivated;

	//for dragging nodes
	Vector2 oldMousePos;
	bool heldLastFrame = false;

	d3::d3Manager@ manager;
	//stores node indecies of each quad
	//each int array should be 4 indecies long
	[hidden] array<array<uint>> quadNodes;
	array<d3::d3CQuad@> quads;

	bool rotating = false;
	Vector2 rotStart;
	//0 = idk, 1 = x, 2 = y, 3 = z
	int rotatingDir;
	array<Vector3> rotStartNodes;
	Vector3 rotCentre;

	bool scaling = false;
	Vector2 scaleStart;
	int scalingDir;
	//0 = idk, 1 = x, 2 = y, 3 = z
	array<Vector3> scaleStartNodes;
	Vector3 scaleCentre;

	//can cycle presets
	bool canPreset = false;

	uint deactiveFrameCounter;

	scripttrigger@ self;
	script@ script;
	input_api@ input;

	[hidden] d3FakeTrigger fakeTrigger;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@this.self = @self;
		@input = @get_input_api();
		@this.manager = @s.manager; 
		if (s.nodeClusters.findByRef(this) < 0)
		{
			s.nodeClusters.insertLast(this);
		}
		if (!hasInit)
		{
			if (d2colour == 0x00000000)
			{
				d2colour = s.default2dCol;
			}
			if (d3colour == 0x00000000)
			{
				d3colour = s.default3dCol;
			}
			if (layer == 0)
			{
				layer = 18;
			}
			if (sub_layer == 0)
			{
				sub_layer = 1;
			}
			fakeTrigger = d3FakeTrigger();
			hasInit = true;
		}
		fakeTrigger.Init(self.as_entity(), s, manager);
		self.editor_handle_size(0);
		InitQuads();
		UpdateRotation(true);
		s.firstFrame = true;
	}

	void checkpoint_load() { InitQuads(); }
	void on_level_start() { InitQuads(); }

	void editor_step()
	{
		if(script.editor.editor_tab() == "Triggers" 
			&& @script.editor.get_selected_trigger() != null
	 		&& script.editor.get_selected_trigger().is_same(self.as_entity()))
		{
			//rotate
			if (!scaling && (input.key_check_pressed_vk(0x52) || (rotating && input.key_check_pressed_gvb(2))))
			{ 
				rotating = !rotating; 
				if (rotating)
				{
					rotStart = Vector2(input.mouse_x_hud(true), input.mouse_y_hud(true));
					rotStartNodes = nodes;
					Vector3 sum;
					uint count = 0;
					for(uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] != Vector3()) { sum += nodes[i]; count++; }
					}
					rotCentre = sum/count;
				}
				else { rotatingDir = 0; }
			}
			if (rotating)
			{
				if (input.key_check_pressed_vk(0x58)) { rotatingDir = 1; } 
				if (input.key_check_pressed_vk(0x59)) { rotatingDir = 2; } 
				if (input.key_check_pressed_vk(0x5A)) { rotatingDir = 3; } 
				if (rotatingDir == 0) { return; }
				auto curPos = Vector2(input.mouse_x_hud(true), input.mouse_y_hud(true));			
				auto dif = curPos - rotStart;
				float a = dif.x/100;
				float s = sin(a);
				float c = cos(a);
				if (rotatingDir == 1)
				{
					for(uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] == Vector3()) { continue; }
						Vector3 on = rotStartNodes[i];
		 				Vector3 dir = on - rotCentre;
						nodes[i] = Vector3(on.x, rotCentre.y+dir.y*c+dir.z*s, rotCentre.z+dir.z*c-dir.y*s);
					}
				}
				if (rotatingDir == 2)
				{
					for(uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] == Vector3()) { continue; }
						Vector3 on = rotStartNodes[i];
		 				Vector3 dir = on - rotCentre;
						nodes[i] = Vector3(rotCentre.x+dir.x*c-dir.z*s,on.y,rotCentre.z+dir.z*c+dir.x*s);
					}
				}
				if (rotatingDir == 3)
				{
					for(uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] == Vector3()) { continue; }
						Vector3 on = rotStartNodes[i];
		 				Vector3 dir = on - rotCentre;
						nodes[i] = Vector3(rotCentre.x+dir.x*c-dir.y*s,rotCentre.y+dir.y*c+dir.x*s,on.z);
					}
				}
				UpdatePositions();
				manager.UpdateLooks(true);
				return;
			}

			//scaling
			if ((input.key_check_pressed_vk(0x53) && !input.key_check_gvb(11)) || (scaling && input.key_check_pressed_gvb(2))) 
			{ 
				scaling = !scaling;
				if (scaling)
				{
					scaleStart = Vector2(input.mouse_x_hud(true), input.mouse_y_hud(true));
					scaleStartNodes = nodes;
					Vector3 sum;
					uint count = 0;
					for(uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] != Vector3()) { sum += nodes[i]; count++; }
					}
					scaleCentre = sum/count;
				}
				else { scalingDir = 0; }
			}
			if (scaling)
			{
				if (input.key_check_pressed_vk(0x57)) { scalingDir = 0; } 
				if (input.key_check_pressed_vk(0x58)) { scalingDir = 1; } 
				if (input.key_check_pressed_vk(0x59)) { scalingDir = 2; } 
				if (input.key_check_pressed_vk(0x5A)) { scalingDir = 3; } 
				auto curPos = Vector2(input.mouse_x_hud(true), input.mouse_y_hud(true));			
				auto dif = curPos - scaleStart;
				float m = 1+dif.x/300;
				if (scalingDir == 0)
				{
					for(uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] == Vector3()) { continue; }
						Vector3 on = scaleStartNodes[i];
						Vector3 dir = on - scaleCentre;
						nodes[i] = scaleCentre + dir*m;
					}
				}
				if (scalingDir == 1)
				{
					for(uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] == Vector3()) { continue; }
						Vector3 on = scaleStartNodes[i];
						Vector3 dir = on - scaleCentre;
						nodes[i] = scaleCentre + Vector3(dir.x*m,dir.y,dir.z);
					}
				}
				if (scalingDir == 2)
				{
					for(uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] == Vector3()) { continue; }
						Vector3 on = scaleStartNodes[i];
						Vector3 dir = on - scaleCentre;
						nodes[i] = scaleCentre + Vector3(dir.x,dir.y*m,dir.z);
					}
				}
				if (scalingDir == 3)
				{
					for(uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] == Vector3()) { continue; }
						Vector3 on = scaleStartNodes[i];
						Vector3 dir = on - scaleCentre;
						nodes[i] = scaleCentre + Vector3(dir.x,dir.y,dir.z*m);
					}
				}
				UpdatePositions();
				manager.UpdateLooks(true);
				return;
			}

			//add node
			if (input.key_check_pressed_gvb(18) ||
				(input.key_check_pressed_vk(0x41) && input.key_check_gvb(10) && selectedNodes.length() == 0))
			{
				Vector3 mpos = Vector3(input.mouse_x_world(21), input.mouse_y_world(21), 0);
				Vector3 centre = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
				AddNode(manager.cam.CamToWorldPos(mpos-centre));
				canPreset = false;
			}
			//select node
			if (input.key_check_pressed_gvb(2) && input.key_check_gvb(10))
			{
				Vector2 mpos = Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
				int cb = -1;
				float cbz = 0;
				for (uint i = 0; i < nodes.length(); i++)
				{
					Vector3 centre = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
					Vector3 pos = manager.cam.WorldToCamPos(nodes[i]) + centre;
					float w = GetNodeRadius(i);
					if (abs(mpos.x-pos.x) < w && abs(mpos.y-pos.y) < w &&
						selectedNodes.find(i) == -1 &&
						(cb == -1 || cbz > pos.z))
					{
						cb = i;
						cbz = pos.z;
					}
				}
				if (cb != -1)
				{
					selectedNodes.insertLast(cb);
					puts("selected: " + cb);
				}
			}
			//deselect node
			if (input.key_check_pressed_gvb(3) && input.key_check_gvb(10))
			{
				Vector2 mpos = Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
				int cb = -1;
				float cbz = 0;
				for (uint i = 0; i < nodes.length(); i++)
				{
					Vector3 centre = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
					Vector3 pos = manager.cam.WorldToCamPos(nodes[i]) + centre;
					float w = GetNodeRadius(i);
					if (abs(mpos.x-pos.x) < w && abs(mpos.y-pos.y) < w &&
						selectedNodes.find(i) != -1 &&
						(cb == -1 || cbz > pos.z))
					{
						cb = i;
						cbz = pos.z;
					}
				}
				if (cb != -1)
				{
					selectedNodes.removeAt(selectedNodes.find(cb));
				}
			}
			//deselect all
			if (input.key_check_pressed_gvb(5))
			{
				selectedNodes.resize(0);
			}
			//delete nodes
			if (input.key_check_pressed_gvb(22))
			{
				for (uint n = 0; n < selectedNodes.length(); n++)
				{
					//puts("node being removed " + selectedNodes[n]);
					//this is looped backwards to ensure removal doesn't
					//mess with indexes
					for (int q = quadNodes.length()-1; q >= 0; q--)
					{
						// puts("quad at test " + quadNodes[q][0] + ", " +
						// 	quadNodes[q][1] + ", " + 
						// 	quadNodes[q][2] + ", " + 
						// 	quadNodes[q][3]);
						if (quadNodes[uint(q)].find(selectedNodes[n]) != -1)
						{
							puts("removing!");
							manager.RemoveQuad(quads[uint(q)]);
							manager.RemoveRenderable(quads[uint(q)].base.renderable);
							quads.removeAt(q);
							quadNodes.removeAt(q);
						}
					}
					nodes[selectedNodes[n]] = Vector3();
				}
				selectedNodes.resize(0);
				SetActiveSidesAll();
			}
			//add quad
			if (input.key_check_pressed_vk(0x41) && input.key_check_gvb(10)
				&& selectedNodes.length() == 4)
			{
				selectedNodes.sortAsc();
				if (quadNodes.find(selectedNodes) != -1) { return; }
				quadNodes.insertLast(selectedNodes);
				d3::d3CQuad nq;
				nq.layer = layer;
				nq.sub_layer = sub_layer;
				nq.base.colour = d3colour;
				nq.collisionBase.base.colour = d2colour;
				@nq.collisionBase.manager = @manager.manager;
				@nq.manager = @manager;
				quads.insertLast(@nq);
				manager.allQuads.insertLast(@nq);
				d3::Renderable r = d3::Renderable(0);
				@r.quad = @nq;
				@nq.base.renderable = @r;
				manager.renderables.insertLast(r);
				UpdatePositions();
				SetActiveSidesAll();
				manager.UpdateLooks(true);
				canPreset = false;
				//look I have no idea why but I just really don't care anymore
				script.UpdateRotation(true);
			}

			//delete quad
			if (input.key_check_pressed_vk(0x44) && input.key_check_gvb(10))
			{
				Vector3 mpos = Vector3(input.mouse_x_world(21), input.mouse_y_world(21), 0);
				int bq = -1;
				for (uint q = 0; q < quadNodes.length(); q++)
				{
					if (quads[q].base.PointRelation(mpos) != 0 &&
						(bq == -1 || quads[q] > quads[bq]))
					{
						bq = q;
					}
				}
				if (bq != -1)
				{
					manager.RemoveQuad(quads[bq]);
					manager.RemoveRenderable(quads[bq].base.renderable);
					quads.removeAt(bq);
					quadNodes.removeAt(bq);
					SetActiveSidesAll();
					//look I have no idea why but I just really don't care anymore
					script.UpdateRotation(true);
				}
			}

			//drag
			if (!input.key_check_gvb(2) || input.key_check_gvb(10)) { heldLastFrame = false; }
			if (input.key_check_gvb(2) && !heldLastFrame && !input.key_check_gvb(10))
			{
				heldLastFrame = true;
				oldMousePos = Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
			}
			if (input.key_check_gvb(2) && heldLastFrame && !input.key_check_gvb(10))
			{
				Vector2 curMouse = Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
				Vector2 diff = curMouse - oldMousePos;
				Vector3 diff3 = manager.cam.CamToWorldDir(Vector3(diff.x, diff.y, 0));
				if (selectedNodes.length() == 0)
				{
					for (uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] == Vector3()) { continue; }
						nodes[i] += diff3;
					}
				}
				else
				{
					for (uint i = 0; i < selectedNodes.length(); i++)
					{
						if (nodes[selectedNodes[i]] == Vector3()) { continue; }
						nodes[selectedNodes[i]] += diff3;
					}
				}
				oldMousePos = curMouse;
				UpdatePositions();
				manager.UpdateLooks(true);
			}

			//join nodes
			if (input.key_check_pressed_vk(0x4A) && input.key_check_gvb(10))
			{
				for(uint cluster = 0; cluster < script.nodeClusters.length(); cluster++)
				{
					array<Vector3>@ otherNodes = script.nodeClusters[cluster].nodes;
					if (otherNodes == nodes) { continue; }
					for(uint otherNode = 0; otherNode < otherNodes.length(); otherNode++)
					{
						if (otherNodes[otherNode] == Vector3()) { continue; }
						for(uint ownNode = 0; ownNode < nodes.length(); ownNode++)
						{
							if (nodes[ownNode] == Vector3()) { continue; }
							Vector3 dir = nodes[ownNode] - otherNodes[otherNode];
							if (dir.Magnitude() < script.joinDist)
							{
								nodes[ownNode] = otherNodes[otherNode];
							}
						}
					}
				}
				UpdateSelf();
			}

			//spikes, dust and deactive
			bool qKey = input.key_check_pressed_vk(0x51);
			bool wKey = input.key_check_pressed_vk(0x57);
			bool eKey = input.key_check_pressed_vk(0x45);

			if (qKey || wKey || eKey)
			{
				EnableAllChecks();
				Vector3 mousePos = Vector3(input.mouse_x_world(21), input.mouse_y_world(21),0);
				Vector2 mousePos2d = Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
				int best = -1;
				for(uint i = 0; i < quads.length(); i++)
				{
					if (!(quads[i].base.drawn || eKey) || quads[i].base.behind) { continue; }
					if (quads[i].base.PointOverlaps(mousePos) != 0)
					{
						puts("found one");
						if (best == -1 || quads[i].opCmpForce(quads[best]) == 1)
						{
							if (eKey)
							{
								int side = quads[i].base.SideFromPoint(mousePos2d, eKey);
								if (side == -1) { continue; }
							}
							puts("chose one");
							best = i;
						}
					}
				}
				if (best != -1)
				{
					puts("doing something maybe, e: " + eKey);
					int side = quads[best].base.SideFromPoint(mousePos2d, eKey);
					if (side != -1)
					{
						puts("def doing something");
						array<uint> n = NodesFromSide(uint(best),uint(side));
						if (qKey)
						{
							int i = FindArray(spikes, n);
							if (i == -1)
							{
								spikes.insertLast(n);
							}
							else
							{
								spikes.removeAt(i);
							}
						}
						if (wKey)
						{
							int i = FindArray(dust, n);
							if (i == -1)
							{
								dust.insertLast(n);
							}
							else
							{
								dust.removeAt(i);
							}
						}
						if (eKey)
						{
							int i = FindArray(deactivated, n);
							if (i == -1)
							{
								deactivated.insertLast(n);
							}
							else
							{
								deactivated.removeAt(i);
							}
						}
						ResetAllSides();
						SetActiveSidesAll();
						ApplyTileEnts();
						UpdateRotation(true);
						manager.UpdateLooks(true);
					}
				}
				ResetChecks();
			}

			//presets
			
			//cube
			if (input.key_check_pressed_vk(0x31) && (GetActiveNodeCount() == 0 || canPreset))
			{
				nodes.resize(0);
				AddNode(fakeTrigger.pos + Vector3(96,96.01,96));
				AddNode(fakeTrigger.pos + Vector3(96,96.02,-96));
				AddNode(fakeTrigger.pos + Vector3(96,-96.03,96));
				AddNode(fakeTrigger.pos + Vector3(96,-96.04,-96));
				AddNode(fakeTrigger.pos + Vector3(-96,96.05,96));
				AddNode(fakeTrigger.pos + Vector3(-96,96.06,-96));
				AddNode(fakeTrigger.pos + Vector3(-96,-96.07,96));
				AddNode(fakeTrigger.pos + Vector3(-96,-96.08,-96));
				canPreset = true;
			}
			//slope
			if (input.key_check_pressed_vk(0x32) && (GetActiveNodeCount() == 0 || canPreset))
			{
				nodes.resize(0);
				AddNode(fakeTrigger.pos + Vector3(96,96.01,96));
				AddNode(fakeTrigger.pos + Vector3(96,96.02,-96));
				AddNode(fakeTrigger.pos + Vector3(-96,96.03,96));
				AddNode(fakeTrigger.pos + Vector3(-96,96.04,-96));
				AddNode(fakeTrigger.pos + Vector3(-96,-96.05,96));
				AddNode(fakeTrigger.pos + Vector3(-96,-96.06,-96));
				canPreset = true;
			}
			//pyramid
			if (input.key_check_pressed_vk(0x33) && (GetActiveNodeCount() == 0 || canPreset))
			{
				nodes.resize(0);
				AddNode(fakeTrigger.pos + Vector3(96,96.01,96));
				AddNode(fakeTrigger.pos + Vector3(96,96.02,-96));
				AddNode(fakeTrigger.pos + Vector3(-96,96.03,96));
				AddNode(fakeTrigger.pos + Vector3(-96,96.04,-96));
				AddNode(fakeTrigger.pos + Vector3(0,-96.05,0));
				canPreset = true;
			}
		}
		else { rotating = false; }
		fakeTrigger.EditorStep();
	}

	void EnableAllChecks()
	{
		for(uint i = 0; i < quads.length(); i++)
		{
			quads[i].base.clusterId = -1;
			quads[i].base.lineComp = true;
			quads[i].base.simplerPointRelation = false;
		}
	}

	void ResetChecks()
	{
		for(uint i = 0; i < quads.length(); i++)
		{
			quads[i].base.clusterId = self.id();
			quads[i].base.lineComp = lineComp;
			quads[i].base.simplerPointRelation = simplerLayering;
		}
	}

	uint GetActiveNodeCount()
	{
		uint r = 0;
		for(uint i = 0; i < nodes.length(); i++)
		{
			if (nodes[i] != Vector3()) { r++; }
		}
		return r;
	}

	void editor_draw(float ok)
	{
		scene@ s = get_scene();
		if(script.editor.editor_tab() == "Triggers" 
			&& @script.editor.get_selected_trigger() != null
	 		&& script.editor.get_selected_trigger().is_same(self.as_entity()))
		{
			for (uint i = 0; i < nodes.length(); i++)
			{
				if (nodes[i] == Vector3()) { continue; }
				Vector3 centre = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
				Vector3 pos = manager.cam.WorldToCamPos(nodes[i]) + centre;
				float w = GetNodeRadius(i);
				uint c;
				if (selectedNodes.find(i) == -1)
				{
					c = 0xBBFF0055;
					if (pos.z < -10) { c = 0x22FF0055; }
				}
				else
				{
					c = 0xBB0055FF;
					if (pos.z < -10) { c = 0x220055FF; }
				}
				s.draw_rectangle_world(21,1, pos.x-w, pos.y-w, pos.x+w, pos.y+w, 0, c);
			}
		}
	}

	void UpdateRotation(bool force = false)
	{
		UpdateSelf();
		fakeTrigger.UpdateRotation();
		if (!force && deactiveFrameCounter < script.layerFrames) { deactiveFrameCounter++; return; }
		deactiveFrameCounter = 0;
		SetActiveSidesAll();
	}

	void editor_var_changed(var_info@ info)
	{
		SetActiveSidesAll();
		UpdateSelf();
		manager.UpdateLooks(true);
	}

	void UpdateSelf()
	{
		float closest = -1;
		for(uint i = 0; i < quadNodes.length(); i++)
		{
			for(uint a = 0; a < quadNodes[i].length(); a++)
			{
				float nz = manager.cam.WorldToCamPos(nodes[a]).z;
				if (nz < closest || closest == -1) { closest = nz; }
			}
		}
		uint d2col = script.ApplyFog(d2colour, closest);
		uint d3col = script.ApplyFog(d3colour, closest);
		for(uint i = 0; i < quads.length(); i++)
		{
			quads[i].collisionBase.base.colour = d2col;
			quads[i].base.colour = d3col;
			quads[i].base.closest = closest;
			quads[i].base.simplerPointRelation = simplerLayering;
			quads[i].base.lineComp = lineComp;
			if (isConvex)
			{
				quads[i].base.clusterId = self.id();
			}
			else
			{
				quads[i].base.clusterId = -1;
			}
		}
		UpdatePositions();
	}

	//finds first available spot and adds the node, returns index of node
	uint AddNode(Vector3 n)
	{
		for (uint i = 0; i < nodes.length(); i++)
		{
			if (nodes[i] == Vector3())
			{
				nodes[i] = n;
				return i;
			}
		}
		nodes.insertLast(n);
		return nodes.length()-1;
	}

	//it's a square but radius as in width/2 :p
	float GetNodeRadius(uint n)
	{
		Vector3 centre = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
		Vector3 pos = manager.cam.WorldToCamPos(nodes[n]) + centre;
		float w = 10 + 0.5*pow(2.72, 2.6-pos.z/100);
		if (w > 25) { w = 25; }
		return w;
	}

	//initialises the quad list. only call in init!!!
	void InitQuads()
	{
		quads.resize(0);
	   	// puts("id: " + self.id());
		for (uint i = 0; i < quadNodes.length(); i++)
		{
			d3::d3CQuad nq;
			nq.layer = layer;
			nq.sub_layer = sub_layer;
			nq.base.colour = d3colour;
			nq.collisionBase.base.colour = d2colour;
			@nq.collisionBase.manager = @manager.manager;
			@nq.collisionBase.script = @script;
			@nq.manager = @manager;
			d3::Renderable r = d3::Renderable(0);
			@r.quad = @nq;
			@nq.base.renderable = @r;
			nq.base.simplerPointRelation = simplerLayering;
			if (isConvex)
			{
				nq.base.clusterId = self.id();
			}
	   		nq.base.lineComp = lineComp;
			manager.renderables.insertLast(r);
			quads.insertLast(@nq);
			manager.allQuads.insertLast(@nq);
		}
		UpdatePositions();
		SetActiveSidesAll();
		ApplyTileEnts();
		// puts("quads " + quads.length());
	}

	void UpdatePositions()
	{
		for (uint i = 0; i < quadNodes.length(); i++)
		{
			d3::d3CQuad@ q = quads[i];
			array<uint>@ p = @quadNodes[i];
			q.base.p1 = nodes[p[0]];
			q.base.p2 = nodes[p[1]];
			q.base.p3 = nodes[p[2]];
			q.base.p4 = nodes[p[3]];

			//shrinking the quads slightly so that layering can work
			Vector3 c = q.base.Find3dCentre();
			int shrinkAmount = 100000;
			if (script.quadDebug) { shrinkAmount = 10; }
			q.base.p1 += (c-q.base.p1)/shrinkAmount;
			q.base.p2 += (c-q.base.p2)/shrinkAmount;
			q.base.p3 += (c-q.base.p3)/shrinkAmount;
			q.base.p4 += (c-q.base.p4)/shrinkAmount;
			q.base.UpdateMaxDist();
		}
	}

	void SetActiveSidesAll()
	{
		ResetDeactivatedOnly();
		// puts("quad 0: " + quadNodes[0][0]+","+quadNodes[0][1]+","+quadNodes[0][2]+","+quadNodes[0][3]);
		// puts("quad 1: " + quadNodes[1][0]+","+quadNodes[1][1]+","+quadNodes[1][2]+","+quadNodes[1][3]);
		if (script.extraQuadDebug) { return; }
		for (uint i = 0; i < quadNodes.length(); i++)
		{
			// puts("");
			// puts("");
			// puts("doing quad " + i + "!!!!");
			DealWithSharedTrigs(i);
		}
		ApplyDeactivatedOnly();
		for(uint i = 0; i < quads.length(); i++)
		{
			quads[i].base.UpdateDrawn();
		}
	}

	//resets dust, only do at init
	void ApplyTileEnts()
	{
		ResetAllSides();
		//don't feel like copypasting rn so just gonna do this
		array<array<array<uint>>> things;
		things.insertLast(spikes);
		things.insertLast(dust);
		things.insertLast(deactivated);
		for(uint thing = 0; thing < things.length(); thing++)
		{
			array<array<uint>>@ objects = things[thing];
			for(uint object = 0; object < objects.length(); object++)
			{
				array<uint>@ triplet = objects[object];
				array<uint> qs = FindSharedTrig(triplet[0], triplet[1], triplet[2]);
				for(uint q = 0; q < qs.length(); q++)
				{
					int side = SideFromNodes(qs[q], triplet[0], triplet[1], triplet[2]);
					if (side < 0) { continue; }
					switch (thing)
					{
						case 0:
							quads[qs[q]].spikeSides[side-1] = true;
							break;
						case 1:
							quads[qs[q]].dustSides[side-1] = true;
							break;
						case 2:
							quads[qs[q]].activeSides[side-1] = false;
							quads[qs[q]].base.drawnSides[side-1] = false;
							break;
					}
				}
			}
		}
	}

	void ApplyDeactivatedOnly()
	{
		for(uint object = 0; object < deactivated.length(); object++)
		{
	  		array<uint>@ triplet = deactivated[object];
			array<uint> qs = FindSharedTrig(triplet[0], triplet[1], triplet[2]);
			for(uint q = 0; q < qs.length(); q++)
			{
				int side = SideFromNodes(qs[q], triplet[0], triplet[1], triplet[2]);
				if (side < 0) { continue; }
				quads[qs[q]].activeSides[side-1] = false;
				quads[qs[q]].base.drawnSides[side-1] = false;
			}
		}
	}

	void ResetDeactivatedOnly()
	{
		array<bool> a = {true, true, true, true};
		for (uint i = 0; i < quads.length(); i++)
		{
	   		d3::d3CQuad@ q = quads[i];
			q.activeSides = a;
			q.base.drawnSides = a;
		}
	}

	//finds all other quads that share a trig with
	//this one and disable both of the sides
	void DealWithSharedTrigs(uint quad)
	{
		UpdatePositions();
		array<uint>@ q = quadNodes[quad];

		array<uint> m1 = FindSharedTrig(q[0], q[1], q[2]);
		array<uint> m2 = FindSharedTrig(q[0], q[1], q[3]);
		array<uint> m3 = FindSharedTrig(q[0], q[2], q[3]);
		array<uint> m4 = FindSharedTrig(q[1], q[2], q[3]);
		// puts("m1: " + m1.length());
		// puts("m2: " + m2.length());
		// puts("m3: " + m3.length());
		// puts("m4: " + m4.length());

		if (m1.length() > 1)
		{
			bool draw = false;
			for (uint i = 0; i < m1.length(); i++)
			{
				int side = SideFromNodes(m1[i], q[0], q[1], q[2]);
				if (side < 0) { continue; }
				quads[m1[i]].activeSides[side-1] = false;
				if (quads[m1[i]].base.behind) { draw = true; }
			}
			if (!draw)
			{
				for (uint i = 0; i < m1.length(); i++)
				{
					int side = SideFromNodes(m1[i], q[0], q[1], q[2]);
					quads[m1[i]].base.drawnSides[side-1] = false;
				}
			}
		}
		if (m2.length() > 1)
		{
			bool draw = false;
			for (uint i = 0; i < m2.length(); i++)
			{
				int side = SideFromNodes(m2[i], q[0], q[1], q[3]);
				if (side < 0) { continue; }
				quads[m2[i]].activeSides[side-1] = false;
				if (quads[m2[i]].base.behind) { draw = true; }
			}
			if (!draw)
			{
				for (uint i = 0; i < m2.length(); i++)
				{
					int side = SideFromNodes(m2[i], q[0], q[1], q[3]);
					quads[m2[i]].base.drawnSides[side-1] = false;
				}
			}
		}
		if (m3.length() > 1)
		{
			bool draw = false;
			for (uint i = 0; i < m3.length(); i++)
			{
				int side = SideFromNodes(m3[i], q[0], q[2], q[3]);
				if (side < 0) { continue; }
				quads[m3[i]].activeSides[side-1] = false;
				if (quads[m3[i]].base.behind) { draw = true; }
			}
			if (!draw)
			{
				for (uint i = 0; i < m3.length(); i++)
				{
					int side = SideFromNodes(m3[i], q[0], q[2], q[3]);
					quads[m3[i]].base.drawnSides[side-1] = false;
				}
			}
		}
		if (m4.length() > 1)
		{
			bool draw = false;
			for (uint i = 0; i < m4.length(); i++)
			{
				int side = SideFromNodes(m4[i], q[1], q[2], q[3]);
				if (side < 0) { continue; }
				quads[m4[i]].activeSides[side-1] = false;
				if (quads[m4[i]].base.behind) { draw = true; }
			}
			if (!draw)
			{
				for (uint i = 0; i < m4.length(); i++)
				{
					int side = SideFromNodes(m4[i], q[1], q[2], q[3]);
					quads[m4[i]].base.drawnSides[side-1] = false;
				}
			}
		}
	}

	void ResetAllSides()
	{
		array<bool> a = {true, true, true, true};
		array<bool> b = {false, false, false, false};
		for (uint i = 0; i < quads.length(); i++)
		{
	   		d3::d3CQuad@ q = quads[i];
			q.activeSides = a;
			q.base.drawnSides = a;
			q.spikeSides = b;
			q.dustSides = b;
		}
	}

	int SideFromNodes(uint quad, uint n1, uint n2, uint n3)
	{
		array<uint>@ q = quadNodes[quad];
		array<uint> na = {n1, n2, n3};
		array<uint> s1 = {q[0], q[1], q[2]};
		array<uint> s2 = {q[0], q[1], q[3]};
		array<uint> s3 = {q[0], q[2], q[3]};
		array<uint> s4 = {q[1], q[2], q[3]};
		if (IsSameArr(na, s1)) { return 1; }
		if (IsSameArr(na, s2)) { return 2; }
		if (IsSameArr(na, s3)) { return 3; }
		if (IsSameArr(na, s4)) { return 4; }
		return -1;
	}

	array<uint> NodesFromSide(uint quad, uint side)
	{
		array<uint>@ qs = quadNodes[quad];
		array<uint> s1 = {qs[0], qs[1], qs[2]};
		array<uint> s2 = {qs[0], qs[1], qs[3]};
		array<uint> s3 = {qs[0], qs[2], qs[3]};
		array<uint> s4 = {qs[1], qs[2], qs[3]};
		switch(side)
		{
			case 1:
				return s1;
			case 2:
				return s2;
			case 3:
				return s3;
			case 4:
				return s4;
		}
		array<uint> ret;
		return ret;
	}

	bool IsSameArr(array<uint> a1, array<uint> a2)
	{
		// puts("is same");
		// puts("a1: " + a1[0] + ", " + a1[1] + ", " + a1[2]);
		// puts("a2: " + a2[0] + ", " + a2[1] + ", " + a2[2]);
		a1.sortAsc();
		a2.sortAsc();
		// puts("same? " + (a1 == a2));
		return a1 == a2;
	}

	int FindArray(array<array<uint>> arr, array<uint> item)
	{
		for(uint i = 0; i < arr.length(); i++)
		{
			if (IsSameArr(arr[i], item))
			{
				return i;
			}
		}
		return -1;
	}

	array<uint> FindSharedTrig(uint n1, uint n2, uint n3)
	{
		array<uint> ret;
		for (uint i = 0; i < quadNodes.length(); i++)
		{
			array<uint>@ l = quadNodes[i];
			if (l.find(n1) != -1 && l.find(n2) != -1 && l.find(n3) != -1)
			{
				// puts("found shared!");
				// puts("ns: " + n1+", "+n2+", "+n3);
				// puts("quad: " + l[0]+", "+l[1]+", "+l[2]+", "+l[3]);
				ret.insertLast(i);
			}
		}
		return ret;
	}

	void on_remove()
	{
		for (uint i = 0; i < quads.length(); i++)
		{
			manager.RemoveQuad(quads[i]);
			manager.RemoveRenderable(quads[i].base.renderable);
		}
		fakeTrigger.DeleteSelf();
	}
}
