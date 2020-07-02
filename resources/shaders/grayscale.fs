#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

#define PI 3.14159265358979323846
uniform float uTime = 0.0;
uniform float uAspectRatio = 1.0;

// NOTE: Add here your custom variables

vec4 invertColor(vec4 col)
{
    col.rgb = 1.0 - col.rgb;
    return col;
}

float ellipsis(vec2 uv, vec2 M1, vec2 M2, float len)
{
    return step(len, distance(uv, M1) + distance(uv, M2));
}

vec2 rotate(vec2 origin, vec2 vec, float angle)
{
    vec -= origin;
    
    vec2 rotated;
    rotated.x = vec.x*cos(angle) - vec.y*sin(angle);
    rotated.y = vec.x*sin(angle) + vec.y*cos(angle);
    
    rotated += origin;
    
    return rotated;
}

void main()
{
/*
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord)*colDiffuse*fragColor;
    
    // Convert texel color to grayscale using NTSC conversion weights
    float gray = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));
    
    // Calculate final fragment color
    finalColor = vec4(gray, gray, gray, texelColor.a);
	
	// inverted colors :
	finalColor.rgb = 1.0 - texelColor.rgb;
	finalColor.a = texelColor.a;
	*/
	
	   float total_progress = uTime/1.0f;
    float progress = min(total_progress, 1.0f);
    float ring1_prog = smoothstep(0.0, 0.3, progress);
    float ring2_prog = smoothstep(0.6, 1.0, progress);
    
	vec2 uv = fragTexCoord* vec2(uAspectRatio, -1.0);
    vec4 col = texture(texture0, fragTexCoord)*colDiffuse*fragColor;
    vec4 inv = invertColor(col*1.5)/1.5;
    
	vec2 pos = vec2(0.5, 0.5);
	//pos = vec2(uPos.x, uPos.y);
    vec2 M1 = (pos - vec2(0.05, 0.0))*vec2(uAspectRatio, 1.0);
    vec2 M2 = (pos + vec2(0.05, 0.0))*vec2(uAspectRatio, 1.0);
    
    //M1 = rotate(vec2(0.5, 0.5), M1, 3.1415/2.0);
    //M2 = rotate(vec2(0.5, 0.5), M2, 3.1415/2.0);
    
    vec2 uv2 = rotate(vec2(0.5, 0.5)*vec2(uAspectRatio, 1.0), uv, 2.0*progress*3.1415/1.0);
    
    float lol = ellipsis(uv2, M1, M2, 2.5*smoothstep(0.0, 1.0, ring1_prog));
    float lol2 = ellipsis(uv2, M1, M2, 2.5*smoothstep(0.0, 1.0, ring2_prog));
    finalColor = mix(inv, col, lol);
    //finalColor = mix(col, finalColor, lol2);
}