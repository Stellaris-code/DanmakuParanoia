#ifndef EASING_H
#define EASING_H

typedef enum easing_function_enum
{
    EaseConstantUp = 0,
    EaseLinear,
    EaseInSine,
    EaseOutSine,
    EaseInOutSine,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseInQuart,
    EaseOutQuart,
    EaseInOutQuart,
    EaseInQuint,
    EaseOutQuint,
    EaseInOutQuint,
    EaseInExpo,
    EaseOutExpo,
    EaseInOutExpo,
    EaseInCirc,
    EaseOutCirc,
    EaseInOutCirc,
    EaseInBack,
    EaseOutBack,
    EaseInOutBack,
    EaseInElastic,
    EaseOutElastic,
    EaseInOutElastic,
    EaseInBounce,
    EaseOutBounce,
    EaseInOutBounce
} easing_function_enum;

typedef float(*easing_function_t)(float);

float easeInSine( float t );
float easeOutSine( float t );
float easeInOutSine( float t );

float easeInQuad( float t );

float easeOutQuad( float t );

float easeInOutQuad( float t );

float easeInCubic( float t );

float easeOutCubic( float t );

float easeInOutCubic( float t );

float easeInQuart( float t );

float easeOutQuart( float t );

float easeInOutQuart( float t );

float easeInQuint( float t );

float easeOutQuint( float t );

float easeInOutQuint( float t );

float easeInExpo( float t );

float easeOutExpo( float t );

float easeInOutExpo( float t );

float easeInCirc( float t );

float easeOutCirc( float t );
float easeInOutCirc( float t );

float easeInBack( float t );

float easeOutBack( float t );
float easeInOutBack( float t );

float easeInElastic( float t );
float easeOutElastic( float t );

float easeInOutElastic( float t );

float easeInBounce( float t );

float easeOutBounce( float t );

float easeInOutBounce( float t );

easing_function_t getEasingFunction( easing_function_enum function );
easing_function_enum strToEasing(const char* str);

#endif // EASING_H
