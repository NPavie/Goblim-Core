#version 420

/**
*	@author Nicolas Pavie, Nicolas.pavie@xlim.fr
*	@date	October 25, 2012
*	@version 1.0
*/

layout(std140) uniform CPU
{
	mat4 objectToScreen;
};

layout(triangles) in;
layout (triangle_strip, max_vertices=21) out;

// --- From Vertex shader
in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
}gl_in[3];

// --- To next stages
out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

struct Vertex{
	vec3 Position;
	vec3 PositionBeforeExtrusion;
	vec3 Normal;
	vec3 Texture_Coordinates;
	vec3 BorderData;
};


struct Edge{
	int s1;
	int s2;
	bool isBorder;
};

in Vertex currentVertex[3];
out Vertex currentSample;

// Construction of shell 
Vertex v_shell[3];	// -> shell vertices
Vertex v_subShell[3];

Edge edges[3];

float min_weight = 1.0;

void extrude(Edge currentEdge)
{
	Vertex v_face1[3];	// -> border face 1 vertices
	Vertex v_face2[3];	// -> border face 2 vertices

	// face 1 : s1 - s2 - sh2
	v_face1[0] = v_subShell[currentEdge.s1];
	v_face1[1] = v_subShell[currentEdge.s2];
	v_face1[2] = v_shell[currentEdge.s2];
	
	// face 2 : sh2 - sh1 - s1
	v_face2[0] = v_shell[currentEdge.s2];
	v_face2[1] = v_shell[currentEdge.s1];
	v_face2[2] = v_subShell[currentEdge.s1];
	

	// shell face side 1
	for(int i = 0; i < 3; i++)
	{
		currentSample = v_face1[i];
		gl_Position = objectToScreen * (vec4(currentSample.Position,1.0));
		EmitVertex();
	}
	EndPrimitive();



	// shell face side 2
	for(int i = 0; i < 3; i++)
	{
		currentSample = v_face2[i];
		gl_Position = objectToScreen * (vec4(currentSample.Position,1.0));
		EmitVertex();
	}
	EndPrimitive();
	// */
}





void main()
{
	// Creating basic overshell
	for(int i = 0; i < 3; i++)
	{
		// next vertex
		int i_n = int(mod((i+1),3)); 

		// Creating edges - default not on the border
		edges[i].s1 = i;
		edges[i].s2 = i_n;
		edges[i].isBorder = false;
		
		// if both vertex are border vertex and are direct neighbours
		// if v[i].isBorder.x != -1 && v[i_n].isBorder.x != -1 --> both are border vertex
		// if v[i].isBorder.y == v[i_n].isBorder.x ||  v[i].isBorder.z == v[i_n].isBorder.x // 
		if( (currentVertex[i].BorderData.x > -0.5 && currentVertex[i_n].BorderData.x > -0.5) ) // if both vertex are on border
		{
			if( ( currentVertex[i].BorderData.y == currentVertex[i_n].BorderData.x || currentVertex[i].BorderData.z == currentVertex[i_n].BorderData.x ) )
			edges[i].isBorder = true;	
		}

		v_subShell[i] = v_shell[i] = currentSample = currentVertex[i];
		v_subShell[i].Position = currentSample.PositionBeforeExtrusion;
		v_subShell[i].Texture_Coordinates.z = 0.0;

		gl_Position = objectToScreen * (vec4(currentSample.Position,1.0));
		EmitVertex();
	}
	EndPrimitive();

	
	// Create the border faces	
	for(int i = 0; i < 3; i++)
	{
		if( edges[i].isBorder == true ) extrude(edges[i]);
	}

}
