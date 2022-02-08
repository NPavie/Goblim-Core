
/*			-- headers Subdivision --
	float initGrid(vec2 sampleIn, vec2 sampleOut, int sizeOfGrid)

	void setGridSize(int size)
	void setGridSize(int length, int width, int depth )

	vec3 cellOf(vec3 point)
	vec2 cellOf(vec2 point)

	vec2 realPositionOf(vec2 cellPoint)
	vec3 realPositionOf(vec3 cellPoint)
*/

ivec3 gridSize = ivec3(1);

float eps = 0.01;

void setGridSize(int size){
	gridSize = ivec3(max(size,1));
}

void setGridSize(int length, int width, int depth ){
	gridSize = ivec3(max(length,1),max(width,1),max(depth,1));
}

vec3 gridPositionOf(vec3 point)
{
	return point * gridSize;
}

vec2 gridPositionOf(vec2 point)
{
	return point * gridSize.xy;
}

ivec3 cellOf(vec3 point){
	return ivec3(floor(point * gridSize));
}

ivec2 cellOf(vec2 point){
	return ivec2(floor(point * gridSize.xy));
}

vec2 cellFractOf(vec2 point)
{
	return fract(point * gridSize.xy);
}


vec2 realPositionOf(vec2 cellPoint){
	return cellPoint / gridSize.xy;
}

vec3 realPositionOf(vec3 cellPoint){
	return cellPoint / gridSize;
}



// Variables liées au parcours du rayon par l'algorithme de bresenham
ivec2 d;
vec2 dd;
ivec2 s;

bool p;
float err;
float err_p;
float err_d;

/*
*	@brief initialisation du parcours du rayon 2D et calcul de l'itération de sortie
*	@return end of path
*/
float initGrid(vec2 sampleIn, vec2 sampleOut, int sizeOfGrid)
{
	gridSize = ivec3(max(sizeOfGrid,1));
	
	// Cellules d'entrée et de sortie du rayon
	d = (cellOf(sampleOut) - cellOf(sampleIn));
	s = sign(d);
	d = abs(d);

	p = false;

	// Tri des différentiels pour permutation
	if(d.y > d.x){
		p = true;
		d = d.yx;
	}
	dd = 2.0f *d;
	
	// Initialisation du calcul de l'erreur par pas sur la grille
	err = err_p = d.x;
	err_d = (d.y / d.x);

	// repère de fin de parcours
	return d.x;
}
// */

float getGridEndIndex()
{
	return d.x;
}

/*
*	@brief initialisation du parcours d'un rayon infini
*/
void initGridPath(vec2 gridRayDir, int sizeOfGrid) {
	gridSize = ivec3(max(sizeOfGrid,1));

	d = ivec2(gridRayDir);
	s = sign(d);
	d = abs(d);

	// Tri des différentiels pour permutation
	if(d.y > d.x){
		p = true;
		d = d.yx;
	}

	// Initialisation du calcul de l'erreur par pas sur la grille
	err = -0.5;
	err_d = float(d.y) / float(d.x);

}

vec2 previousCellOf(vec2 currentCell)
{
	vec2 previous = currentCell;
	// Parcours principal sur le plus grand des différentiels
	if(p){
		previous.y -= s.y;
	}else{
		previous.x -= s.x;
	}

	float keepErr = err + dd.y;
	
	if(keepErr > dd.x ){	// Si la cellule est la diagonale
		if(p){
			previous.x -= s.x;
		}else{
			previous.y -= s.y;
		}
	}
	return previous;
}


/**
*	@brief Parcours du rayon 
*	@return Prochaine cellule parcourue
*/
ivec2 nextCell( ivec2 currentCell )
{
	ivec2 next = currentCell;
	err_p = err; // keep track of previous error for previous cell computation
	// Parcours principal sur le plus grand des différentiels
	if(p){
		next.y += s.y;
	}else{
		next.x += s.x;
	}
	
	// Test de changement de cellule sur le 2em axe
	err += dd.y;
	if(err >= dd.x){
			
		if(p){
			next.x += s.x;
		}else{
			next.y += s.y;
		}
		err -= dd.x;
		
	}
	
	return next;
}
// */

vec2 nextCellOf( vec2 currentCell)
{
	vec2 next = currentCell;
	
	// Parcours principal sur le plus grand des différentiels
	if(p){
		next.y += s.y;
	}else{
		next.x += s.x;
	}

	float keepErr = err + dd.y;
	// Test de changement de cellule sur le 2em axe
	if(keepErr > dd.x){
			
		if(p){
			next.x += s.x;
		}else{
			next.y += s.y;
		}
		
	}
	
	
	return next;
}

// Le probleme est la : les fonctions d'acces sont pas bonne pour gauche et droite !
// next et previous sont correctes car elles utilisent le taux d'erreur,
// La meme chose doit s'appliquer a left et right
// Une solution serait de faire toutes les fonctions d'acces : 
// Les coins nico ! les coins ! les coincoins !
// nextLeft,next, nextRight
// Left, current, Right
// PreviousLeft, Previous, PreviousRight

vec2 rightCellOf(vec2 currentCell) {
	vec2 right = currentCell;
	if(p)
	{
		right.x += s.y;
	}
	else
	{
		right.y -= s.x;
	}

	float keepErr = err + dd.y;
	// Test de changement de cellule sur le 2em axe

	if(keepErr > dd.x)
	{ 
		if(p)
		{ // Si direction favorisant Y
			right.y -= s.x; // en diagonal, la cellule est en dessous de la courante
		}
		else
		{		// Si direction favorisant X, la cellule de droite 
			right.x += s.y;
		}
		
	}

	return right;
	



}

vec2 leftCellOf(vec2 currentCell) {
	vec2 left = currentCell;
	if(p)
	{
		left.x -= s.y;
	}
	else
	{
		left.y += s.x;
	}

	float keepErr = err + dd.y;
	// Test de changement de cellule sur le 2em axe
	if(keepErr > dd.x)
	{ 
		if(p){ // Si direction favorisant Y
			left.y += s.x; 
		}
		else
		{		
			left.x -= s.y;
		}
		
	}

	return left;

}

vec2 nextRightCellOf(vec2 currentCell) {
	vec2 right = currentCell;
	
	float keepErr = err + dd.y;
	// Test de changement de cellule sur le 2em axe
	if(keepErr > dd.x)
	{ 
		if(s.x == s.y)
		{
			right.x += s.x;
		}
		else
		{
			right.y += s.y;
		}
	}
	else
	{
		// Parcours principal sur le plus grand des différentiels
		if(p){
			right.y += s.y;
			right.x += s.y;
		}else{
			right.x += s.x;
			right.y -= s.x;
		}
	}
	
	return right;

}

vec2 nextLeftCellOf(vec2 currentCell) {
	vec2 left = currentCell;
	
	float keepErr = err + dd.y;
	// Test de changement de cellule sur le 2em axe
	if(keepErr > dd.x)
	{ 
		if(s.x == s.y)
		{
			left.y += s.y;
		}
		else
		{
			left.x += s.x;
		}
	}
	else
	{
		// Parcours principal sur le plus grand des différentiels
		if(p){
			left.y += s.y;
			left.x -= s.y;
		}else{
			left.x += s.x;
			left.y += s.x;
		}
	}
	
	return left;

}

vec2 previousRightCellOf(vec2 currentCell) {
	
	vec2 previous = currentCell;
	
	
	float keepErr = err + dd.y; // Utilisation de l'erreur courante (evaluation local a la cellule)
	
	if(keepErr > dd.x ){	// Si la cellule fait la diagonale vers la suivante
		if(s.x == s.y)
		{
			previous.y -= s.y;
		}
		else
		{
			previous.x -= s.x;
		}
	}
	else
	{
		// Parcours principal sur le plus grand des différentiels
		if(p){
			previous.y -= s.y;
			previous.x += s.y;
		}else{
			previous.x -= s.x;
			previous.y -= s.x;
		}
	}
	return previous;
	
}

vec2 previousLeftCellOf(vec2 currentCell) {
	vec2 previous = currentCell;
	
	
	float keepErr = err + dd.y; // Utilisation de l'erreur courante (evaluation local a la cellule)
	
	if(keepErr > dd.x ){	// Si la cellule fait la diagonale vers la suivante
		if(s.x == s.y)
		{
			previous.x -= s.x;
		}
		else
		{
			previous.y -= s.y;
		}
	}
	else
	{
		// Parcours principal sur le plus grand des différentiels
		if(p){
			previous.y -= s.y;
			previous.x -= s.y;
		}else{
			previous.x -= s.x;
			previous.y += s.x;
		}
	}
	return previous;

}




