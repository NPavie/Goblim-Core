#version 420

/**
*	@author Nicolas Pavie, Nicolas.pavie@xlim.fr
*	@date	October 25, 2012
*	@version 1.0
*/

layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Texture_Coordinates;
layout (location = 4) in vec3 Tangent;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};


out Vertex{
	vec3 Position;
	vec3 Normal;
	vec3 Texture_Coordinates;
	vec3 Tangent;
}v_out;


/**
*	@brief Ce shader déplie un modèle 3D selon ses coordonnées UV
*/
void main(){
	v_out.Position = Position;

	v_out.Normal = normalize(Normal);
	v_out.Texture_Coordinates = Texture_Coordinates;
	v_out.Tangent = normalize(Tangent);
	// Dépliage du model
	gl_Position = vec4(Texture_Coordinates * 2 - vec3(1),1);
}
