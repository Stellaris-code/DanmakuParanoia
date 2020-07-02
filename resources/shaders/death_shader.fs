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
uniform vec2 uPos = vec2(0.5, 0.5);

float astroid(vec2 coords, float radius, float thinness)
{
    float dist = pow(coords.x*coords.x+coords.y*coords.y-radius*radius, 3.f) 
        + thinness*radius*radius*coords.x*coords.x*coords.y*coords.y; // astroid
    return step(dist, 0.0f);
}

vec2 rotate(vec2 vec, float angle)
{
    vec2 rotated;
    rotated.x = vec.x*cos(angle) - vec.y*sin(angle);
    rotated.y = vec.x*sin(angle) + vec.y*cos(angle);
    
    return rotated;
}

float circle(vec2 pos, vec2 center, float radius)
{
    const float smooth_factor = 0.005;
    return smoothstep(radius*(1.0-smooth_factor), radius*(1.0+smooth_factor),
                      distance(pos, center));
}

float exponentialOut(float t) {
  return t == 1.0 ? t : 1.0 - pow(2.0, -10.0 * t);
}

#if 1
#define VAL1A 0.4
#define VAL1B 1.0
#define VAL2A 0.4
#define VAL2B 1.2
#define VAL3A 1.0 
#define VAL3B 1.4
#else // NP's version
#define VAL1A 0.9
#define VAL1B 1.4
#define VAL2A 0.45
#define VAL2B 0.9
#define VAL3A 0.65 
#define VAL3B 1.05
#endif

#define SINGULARITY
void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragTexCoord*vec2(uAspectRatio, 1.0);
    
    const float anim_duration = 0.6;
    
    //float progress = mod(uTime, anim_duration)/anim_duration;
    float progress = min(uTime/anim_duration, 1.0);
#ifndef SINGULARITY
    float progress_1 = min(progress/0.5, 1.0);
#else
	float progress_1 = min(progress/0.3, 1.0);
    if (progress > 0.3)
        progress_1 = clamp(1.0 - (progress-0.3)/0.3, 0.0, 1.0);
#endif
    float progress_2 = max((progress-0.5)/0.5, 0.0);
    float progress_3 = smoothstep(VAL1A, VAL1B, progress);
    float progress_astro = smoothstep(VAL2A, VAL2B, uTime/1.0);
    float progress_fadeout = smoothstep(VAL3A, VAL3B, uTime/1.0);
    
    vec2 circle_pos = uPos*vec2(uAspectRatio, 1.0);

	vec4 col = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 alpha = vec4(0.0, 0.0, 0.0, 0.0);
    
    
    const float circle_radius = 0.4;
    // outer ring
    col = mix(col, alpha, circle(uv, circle_pos, circle_radius*exponentialOut(progress_1)));
#ifndef SINGULARITY
    // inner ring
    col = mix(alpha, col, circle(uv, circle_pos, circle_radius*smoothstep(0.0, 1.0, progress_2)));
    
    col = mix(col, alpha, 0.4*circle(uv, circle_pos + vec2(0.3, 0.0), 1.0-(1.0-circle_radius)*progress_3));
    col = mix(col, alpha, 0.4*circle(uv, circle_pos - vec2(0.3, 0.0), 1.0-(1.0-circle_radius)*progress_3));
#endif
    
    col = mix(alpha, col, step(uTime, anim_duration)); // force black
    
    float astro_t = exp(progress_astro*3.0)-1.0;
    
    vec2 uv2 = rotate(circle_pos - uv, astro_t/4.0f);
    vec4 astro_col = mix(alpha, vec4(1.0, 1.0, 1.0, 1.0), astroid(uv2, 1.5*astro_t, 1600.f));
    
    col += astro_col;
    
    col = mix(alpha, col, circle(uv, circle_pos, 2.5*(progress_fadeout)));
	
    // Output to screen
    finalColor = col;
}