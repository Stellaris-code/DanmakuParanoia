#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// Custom variables
#define PI 3.14159265358979323846
uniform float uTime = 0.0;
uniform float uAspectRatio = 1.0;

vec2 rotate(vec2 vec, float angle)
{
    vec2 rotated;
    rotated.x = vec.x*cos(angle) - vec.y*sin(angle);
    rotated.y = vec.x*sin(angle) + vec.y*cos(angle);
    
    return rotated;
}

float astroid(vec2 coords, float radius)
{
    float dist = pow(coords.x*coords.x+coords.y*coords.y-radius*radius, 3.f) 
        + 27.f*radius*radius*coords.x*coords.x*coords.y*coords.y; // astroid
    return step(dist, 0.0f);
}

/* circle 
void main()
{ 
	float aspect_ratio = 800.f/450.f;
    vec2 uv2 = fragTexCoord * vec2(uAspectRatio, 1.0);
    
	const float duration = 0.65f;
    float radius = (clamp(uTime, 0.0f, duration)/duration)*0.707f; // sqrt(0.5²+0.5²)
    
    vec2 mod_coords = mod(uv2, 0.1f)/0.1f; // normalized mod coordinates
    float dist = distance(mod_coords, vec2(0.5f, 0.5f));

    vec3 pink = vec3(1, 0.427, 0.760);
	
    // Output to screen
    finalColor = vec4(pink,step(dist, radius));
}
*/

/* astroid */
// Uglied shader code I've ever made
void main()
{
	const float invert_time = 1.5f*0.75f;
    float total_progress = uTime/0.75f;
    float progress = min(total_progress, 1.0f);
	if (uTime > invert_time)
        progress = 1.0f - (total_progress-1.5f)/0.7f;
    if (uTime > invert_time+0.5f)
        progress = 0.0f;
    
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv2 = fragTexCoord* vec2(uAspectRatio, 1.0);
    vec2 uv2_cpy = uv2;
    
    uv2 += vec2(0.0f, sin(progress*4.f + uv2.y*2.f)/40.f);
    
	float y_coord = uTime > invert_time ? uv2_cpy.y : (1.0f-uv2_cpy.y);
    float radius = min(progress*3.f, max(0.0f, 2.f*3.0f*(progress-(y_coord/1.5f)))); // sqrt(0.5²+0.5²)
    //radius = mod(iTime, 0.707f);
    
    vec2 mod_coords = 2.f*(mod(uv2, 0.1f)/0.1f) - vec2(1, 1); // normalized mod coordinates
    float angle = progress * 1.5f * 3.1415f/2.f;
    vec2 coords = rotate(mod_coords, angle);

    vec3 pink = vec3(1, 0.427, 0.760);
    //pink *= (progress+2.f+2.f*uv2_cpy.y)/2.9f;
    pink = mix(pink, pink*1.5f, (min(0.5f*progress+2.f*uv2_cpy.y, 1.f)));
	
    // Output to screen
    finalColor = vec4(pink,astroid(coords, radius));
}
