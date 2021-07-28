--- fragFunctionEmptyDissolve

vec4 Dissolve(vec4 finalColor, vec2 tiledUV, bool isEmissive){
	return finalColor;
}

--- fragFunctionEmptyDissolveDepth

bool MustDissolve() {
	return false;
}
