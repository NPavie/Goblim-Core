/**
*	@author Nicolas Pavie, Nicolas.pavie@xlim.fr
*	@date December 13, 2012
*	@version 1.0
*/

// Required variable
mat4 _splatting_MVP;		// Projection * View * Model matrix
vec2 _splatting_fragment;	// Fragment courant
vec4 _splatting_screenInfo; // screen viewPort : width, height, near, far

/**
*	@brief	Initialisation du splatting
*	@param	s_MVP			sauvegarde de la MVP pour les calculs
*	@param	s_fragmentCoord	coordonnees du pixel courant dans la fenetre de rendu ([0,1],[0,1])
*	@param	s_screenInfo	Informations liés à la zone de rendu (viewport) : widht, height, near, far
*/
void initSplat(mat4 s_MVP, vec2 s_fragmentCoord, vec4 s_screenInfo)
{
	_splatting_MVP = s_MVP;
	_splatting_fragment = s_fragmentCoord;
	_splatting_screenInfo = s_screenInfo;
}

vec2 getFragment()
{
	return _splatting_fragment;
}



/**
*	@brief	projection dans l'espace écran normalisé d'un point
*	@param	point Point 3D à splatter sur l'écran
*	@return position du point à l'écran
*/
vec3 splat(vec3 point)
{

	vec4 point_MVP = _splatting_MVP * vec4(point,1.0);
	vec3 point_NDC = point_MVP.xyz / point_MVP.w;
	

	// calcul des coordonnées dans un pseudo espace écran ( passage de [-1;1] a [0;1])
	vec3 pointScreen;
	//pointScreen = 0.5 + point_NDC * 0.5; 
	pointScreen.x = fma(point_NDC.x,0.5,0.5);// 0.5 + point_NDC * 0.5;
	pointScreen.y = fma(point_NDC.y,0.5,0.5);// 0.5 + point_NDC * 0.5;
	pointScreen.z = fma(point_NDC.z,0.5,0.5);// 0.5 + point_NDC * 0.5;

	return pointScreen;
}


/**
*	@brief	Reprojection dans l'espace objet d'un point de l'écran
*	@param	point Point reprojeter
*	@return position du point dans l'espace objet
*/
vec3 unsplat(vec4 point){
	
	vec3 point_NDC = point.xyz / 0.5 - 1.0;

	vec4 point_MVP = vec4(point_NDC.xyz * point.w , point.w);
	vec3 objectPoint = (inverse(_splatting_MVP) * point_MVP).xyz;
	
	return objectPoint;

}

/**
*	@brief	projection du pixel dans une coupe d'un espace
*	@param	point	centre de la coupe
*	@param	slice_x	Axe X de la coupe
*	@pram	slice_y	Axe Y de la coupe
*	@return fragment exprimé
*/
vec3 splat(vec3 point, vec3 slice_x, vec3 slice_y)
{
	vec2 PScreen = splat(point).xy;
	vec2 XScreen = splat(point + slice_x).xy;
	vec2 YScreen = splat(point + slice_y).xy;
	
	// nouveaux axes
	vec2 axeX = XScreen - PScreen;
	vec2 axeY = YScreen - PScreen;

	mat3 XYScreenSpace = mat3(	vec3(axeX,0),
								vec3(axeY,0),
								vec3(PScreen,1));
	// test
	//vec3 temp = XYScreenSpace * vec3(0.0,0.0,1.0);
	//temp.xy =  temp.xy - _splatting_fragment.xy;
	//temp.z = 0;
	//return temp;
	return (inverse(XYScreenSpace) * vec3(_splatting_fragment,1));
}


// retourne la matrice de transformation d'un point du noyau vers l'espace écran
mat3 splat_v2(vec3 point, vec3 slice_x, vec3 slice_y)
{
	vec4 point_MVP = _splatting_MVP * vec4(point,1.0);
	vec3 point_NDC = point_MVP.xyz / point_MVP.w;
	// calcul des coordonnées dans un pseudo espace écran ( passage de [-1;1] a [0;1])
	vec3 pointScreen;
	//pointScreen = 0.5 + point_NDC * 0.5; 


	vec2 PScreen = splat(point).xy;
	vec2 XScreen = splat(point + slice_x).xy;
	vec2 YScreen = splat(point + slice_y).xy;
	
	// nouveaux axes
	vec2 axeX = XScreen - PScreen;
	vec2 axeY = YScreen - PScreen;

	// test
	mat3 XYScreenSpace = mat3(	vec3(axeX,0),
								vec3(axeY,0),
								vec3(PScreen,1));
	

	return XYScreenSpace;
}

vec2 projectAxe(vec3 axe_origine, vec3 axe_end)
{
	vec2 OrigineOnScreen = splat(axe_origine).xy;
	vec2 AxeEndOnScreen = splat(axe_origine + axe_end).xy;

	return (AxeEndOnScreen - OrigineOnScreen);
}

/**
*	@brief	Fonction spéciale d'affichage d'un gizmo pour un espace d'évaluation
*	@param	space	Espace d'évaluation à projeter
*	@return Couleur RGB du fragment selon sa position sur le gizmo
*/
vec3 gizmoOf(mat4 space)
{
	vec3 retour = vec3(0);

	// projection du noyau dans l'espace ecran
	vec2 spacePScreen = splat(space[3].xyz).xy;

	vec2 spaceXScreen = splat(space[3].xyz + space[0].xyz).xy;
	vec2 spaceYScreen = splat(space[3].xyz + space[1].xyz).xy;
	vec2 spaceZScreen = splat(space[3].xyz + space[2].xyz).xy;

	// Affichage d'un gizmo
	vec2 posK = abs(_splatting_fragment.xy - spacePScreen.xy);
	vec2 posX = abs(_splatting_fragment.xy - spaceXScreen.xy);
	vec2 posY = abs(_splatting_fragment.xy - spaceYScreen.xy);
	vec2 posZ = abs(_splatting_fragment.xy - spaceZScreen.xy);

	
	if(length(posX) < 0.005) retour = vec3(1,0,0);
	if(length(posY) < 0.005) retour = vec3(0,1,0);
	if(length(posZ) < 0.005) retour = vec3(0,0,1);
	if(length(posK) < 0.005) retour = vec3(1,1,1);
	// */

	return retour;
}

// TODO : outil vec3 gizmoLine(mat4 space)
// Intersection fragment/ligne de chaque axe


/**
*	@brief	Affichage d'un gizmo sous forme de ligne
*	@param	space	Espace afficher
*	@return Couleur RGB du fragment selon sa position sur le gizmo
*/
vec4 gizmoLine(mat4 space){
	vec4 retour = vec4(0);

	// projection du noyau dans l'espace ecran
	vec2 spacePScreen = splat(space[3].xyz).xy;

	vec2 spaceXScreen = splat(space[3].xyz + space[0].xyz).xy;
	vec2 spaceYScreen = splat(space[3].xyz + space[1].xyz).xy;
	vec2 spaceZScreen = splat(space[3].xyz + space[2].xyz).xy;

	// Affichage d'un gizmo
	// distance des points au fragment
	float f_p = distance(spacePScreen,_splatting_fragment.xy);
	float f_x = distance(spaceXScreen,_splatting_fragment.xy);
	float f_y = distance(spaceYScreen,_splatting_fragment.xy);
	float f_z = distance(spaceZScreen,_splatting_fragment.xy);

	// Distance entre les extremites des axes et le centre du repere
	float p_x = distance(spacePScreen,spaceXScreen);
	float p_y = distance(spacePScreen,spaceYScreen);
	float p_z = distance(spacePScreen,spaceZScreen);

	// Intersection du fragment avec (pK,pX), (pK,pY) et (pK,pZ) 
	// TODO : cet truc ne marche pas
	if ( abs( f_p + f_x - p_x ) < 0.0001 ) retour = vec4(1,0,0,1);
	if ( abs( f_p + f_y - p_y ) < 0.0001 ) retour = vec4(0,1,0,1);
	if ( abs( f_p + f_z - p_z ) < 0.0001 ) retour = vec4(0,0,1,1);
	if( f_p < 0.005) retour = vec4(1,1,1,1);	// Point central

	return retour;
}

