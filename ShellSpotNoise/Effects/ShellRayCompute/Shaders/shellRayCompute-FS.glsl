#version 420


layout(std140) uniform FUBO {
	mat4 objectToCamera;					/// Model view matrix : (objet -> camera)
};

struct Vertex{
	vec3 Position;
	vec3 PositionBeforeExtrusion;
	vec3 Normal;
	vec3 Texture_Coordinates;
	vec3 BorderData;
};

in Vertex currentSample;

out vec4 Color;

/**
*	@brief Ce shader colorie un pixel selon les coordonnées UV 
*	@return Couleur RGBA, RGB correspondant a Texture_Coordinates et l'alpha fixé a 1.0
*/
void main(){
	
	float currentDepth = (objectToCamera * vec4(currentSample.Position,1.0)).z;
	
	Color = vec4(currentSample.Texture_Coordinates.xy,-currentDepth,1.0);
	//Color = vec4(currentSample.BorderData,1.0);
	
}