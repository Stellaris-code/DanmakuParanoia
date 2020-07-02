/*******************************************************************************************
*
*   rPBR [shader] - Background skybox vertex shader
*
*   Copyright (c) 2017 Victor Fisac
*
**********************************************************************************************/

#version 330

// Input vertex attributes
in vec3 vertexPosition;

// Input uniform values
uniform mat4 projection;
uniform mat4 view;
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;

void main()
{
    // Calculate fragment position based on model transformations
    fragPosition = vertexPosition;

    // Remove translation from the view matrix
    mat4 rotView = mat4(mat3(view));
    vec4 clipPos = projection*rotView*vec4(vertexPosition, 1.0);

	vec4 cool = mvp*vec4(vertexPosition, 1.0);
    // Calculate final vertex position
    gl_Position = clipPos.xyzw;
	//gl_Position = mvp*vec4(vertexPosition, 1.0);
}
