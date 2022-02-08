

// ---------------------- Operation Matricielle transpose(X) * V * Y
// Equivalent au dot product de X avec V * Y
// Rappel à moi meme : dot(X,Y) = X.x * Y.x + X.y * Y.y + X.z * Y.z + X.w * Y.w
float XtVY(vec2 X, mat2 V, vec2 Y)
{
	//vec2 R = V * Y;
	//return (X.x * R.x + X.y * R.y);
	return dot(X , V * Y);
}

float XtVY(vec3 X, mat3 V, vec3 Y)
{
	//vec3 R = V * Y;
	//return (X.x * R.x + X.y * R.y + X.z * R.z);
	return dot(X , V * Y);
}

float XtVY(vec4 X, mat4 V, vec4 Y)
{
	//vec4 R = V * Y;
	//return (X.x * R.x + X.y * R.y + X.z * R.z + X.w * R.w);
	return dot(X , V * Y);
}

// ---------------------- Operation Matricielle transpose(X) * V * Y XtVX END
