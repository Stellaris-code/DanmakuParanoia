#include <math.h>
#include "easing.h"

#include <string.h>

#ifndef PI
#define PI 3.1415926545
#endif

float easeConstantUp(float t)
{
    (void)t;
    return 1;
}

float easeLinear(float t)
{
    return t;
}

float easeInSine( float t ) {
    return sin( 1.5707963 * t );
}

float easeOutSine( float t ) {
    return 1 + sin( 1.5707963 * (--t) );
}

float easeInOutSine( float t ) {
    return 0.5 * (1 + sin( 3.1415926 * (t - 0.5) ) );
}

float easeInQuad( float t ) {
    return t * t;
}

float easeOutQuad( float t ) {
    return t * (2 - t);
}

float easeInOutQuad( float t ) {
    return t < 0.5 ? 2 * t * t : t * (4 - 2 * t) - 1;
}

float easeInCubic( float t ) {
    return t * t * t;
}

float easeOutCubic( float t ) {
    --t;
    return 1 + (t) * t * t;
}

float easeInOutCubic( float t ) {
    // what the fuck is this code
    return ((t *= 2) <= 1 ? t * t * t : (t -= 2) * t * t + 2) / 2;
}

float easeInQuart( float t ) {
    t *= t;
    return t * t;
}

float easeOutQuart( float t ) {
    --t;
    t = (t) * t;
    return 1 - t * t;
}

float easeInOutQuart( float t ) {
    if( t < 0.5 ) {
        t *= t;
        return 8 * t * t;
    } else {
        --t;
        t = (t) * t;
        return 1 - 8 * t * t;
    }
}

float easeInQuint( float t ) {
    float t2 = t * t;
    return t * t2 * t2;
}

float easeOutQuint( float t ) {
    --t;
    float t2 = (t) * t;
    return 1 + t * t2 * t2;
}

float easeInOutQuint( float t ) {
    float t2;
    if( t < 0.5 ) {
        t2 = t * t;
        return 16 * t * t2 * t2;
    } else {
        --t;
        t2 = (t) * t;
        return 1 + 16 * t * t2 * t2;
    }
}

float easeInExpo( float t ) {
    return (pow( 2, 8 * t ) - 1) / 255;
}

float easeOutExpo( float t ) {
    return 1 - pow( 2, -8 * t );
}

float easeInOutExpo( float t ) {
    if( t < 0.5 ) {
        return (pow( 2, 16 * t ) - 1) / 510;
    } else {
        return 1 - 0.5 * pow( 2, -16 * (t - 0.5) );
    }
}

float easeInCirc( float t ) {
    return 1 - sqrtf( 1 - t );
}

float easeOutCirc( float t ) {
    return sqrtf( t );
}

float easeInOutCirc( float t ) {
    if( t < 0.5 ) {
        return (1 - sqrtf( 1 - 2 * t )) * 0.5;
    } else {
        return (1 + sqrtf( 2 * t - 1 )) * 0.5;
    }
}

float easeInBack( float t ) {
    return t * t * (2.70158 * t - 1.70158);
}

float easeOutBack( float t ) {
    --t;
    return 1 + (t) * t * (2.70158 * t + 1.70158);
}

float easeInOutBack( float t ) {
    if( t < 0.5 ) {
        return t * t * (7 * t - 2.5) * 2;
    } else {
        --t;
        return 1 + (t) * t * 2 * (7 * t + 2.5);
    }
}

float easeInElastic( float t ) {
    float t2 = t * t;
    return t2 * t2 * sin( t * PI * 4.5 );
}

float easeOutElastic( float t ) {
    float t2 = (t - 1) * (t - 1);
    return 1 - t2 * t2 * cos( t * PI * 4.5 );
}

float easeInOutElastic( float t ) {
    float t2;
    if( t < 0.45 ) {
        t2 = t * t;
        return 8 * t2 * t2 * sin( t * PI * 9 );
    } else if( t < 0.55 ) {
        return 0.5 + 0.75 * sin( t * PI * 4 );
    } else {
        t2 = (t - 1) * (t - 1);
        return 1 - 8 * t2 * t2 * sin( t * PI * 9 );
    }
}

float easeInBounce( float t ) {
    return pow( 2, 6 * (t - 1) ) * fabs( sin( t * PI * 3.5 ) );
}

float easeOutBounce( float t ) {
    return 1 - pow( 2, -6 * t ) * fabs( cos( t * PI * 3.5 ) );
}

float easeInOutBounce( float t ) {
    if( t < 0.5 ) {
        return 8 * pow( 2, 8 * (t - 1) ) * fabs( sin( t * PI * 7 ) );
    } else {
        return 1 - 8 * pow( 2, -8 * t ) * fabs( sin( t * PI * 7 ) );
    }
}

easing_function_t getEasingFunction( easing_function_enum function )
{
    switch (function)
    {
        case EaseConstantUp : return easeConstantUp;
        case EaseLinear : return easeLinear;
        case EaseInSine : return  	easeInSine;
        case EaseOutSine : return  	easeOutSine;
        case EaseInOutSine : return  	easeInOutSine;
        case EaseInQuad : return  	easeInQuad;
        case EaseOutQuad : return  	easeOutQuad;
        case EaseInOutQuad : return  	easeInOutQuad;
        case EaseInCubic : return  	easeInCubic;
        case EaseOutCubic : return  	easeOutCubic;
        case EaseInOutCubic : return  easeInOutCubic;
        case EaseInQuart : return  	easeInQuart;
        case EaseOutQuart : return  	easeOutQuart;
        case EaseInOutQuart : return  easeInOutQuart;
        case EaseInQuint : return  	easeInQuint;
        case EaseOutQuint : return  	easeOutQuint;
        case EaseInOutQuint : return  easeInOutQuint;
        case EaseInExpo : return  	easeInExpo;
        case EaseOutExpo : return  	easeOutExpo;
        case EaseInOutExpo : return 	easeInOutExpo;
        case EaseInCirc : return  	easeInCirc;
        case EaseOutCirc : return  	easeOutCirc;
        case EaseInOutCirc : return 	easeInOutCirc;
        case EaseInBack : return  	easeInBack;
        case EaseOutBack : return  	easeOutBack;
        case EaseInOutBack : return 	easeInOutBack;
        case EaseInElastic : return  	easeInElastic;
        case EaseOutElastic : return  easeOutElastic;
        case EaseInOutElastic : return  easeInOutElastic;
        case EaseInBounce : return  	easeInBounce;
        case EaseOutBounce : return  	easeOutBounce;
        case EaseInOutBounce : return  easeInOutBounce;
        default:
            return easeLinear; // defaut easing function
    }
}

easing_function_enum strToEasing(const char *str)
{
    if (strcmp(str, "easeConstantUp") == 0 || strcmp(str, "constantUp") == 0)
        return EaseConstantUp;
    if (strcmp(str, "none") == 0 || strcmp(str, "easeLinear") == 0 || strcmp(str, "linear") == 0)
        return EaseLinear;
    if (strcmp(str, "easeInSine") == 0)
        return EaseInSine;
    if (strcmp(str, "easeOutSine") == 0)
        return EaseOutSine;
    if (strcmp(str, "easeInOutSine") == 0 || strcmp(str, "sine") == 0)
        return EaseInOutSine;
    if (strcmp(str, "easeInQuad") == 0)
        return EaseInQuad;
    if (strcmp(str, "easeOutQuad") == 0)
        return EaseOutQuad;
    if (strcmp(str, "easeInOutQuad") == 0 || strcmp(str, "quad") == 0)
        return EaseInOutQuad;
    if (strcmp(str, "easeInCubic") == 0)
        return EaseInCubic;
    if (strcmp(str, "easeOutCubic") == 0)
        return EaseOutCubic;
    if (strcmp(str, "easeInOutCubic") == 0 || strcmp(str, "cubic") == 0)
        return EaseInOutCubic;
    if (strcmp(str, "easeInQuart") == 0)
        return EaseInQuart;
    if (strcmp(str, "easeOutQuart") == 0)
        return EaseOutQuart;
    if (strcmp(str, "easeInOutQuart") == 0 || strcmp(str, "quart") == 0)
        return EaseInOutQuart;
    if (strcmp(str, "easeInQuint") == 0)
        return EaseInQuint;
    if (strcmp(str, "easeOutQuint") == 0)
        return EaseOutQuint;
    if (strcmp(str, "easeInOutQuint") == 0 || strcmp(str, "quint") == 0)
        return EaseInOutQuint;
    if (strcmp(str, "easeInExpo") == 0)
        return EaseInExpo;
    if (strcmp(str, "easeOutExpo") == 0)
        return EaseOutExpo;
    if (strcmp(str, "easeInOutExpo") == 0 || strcmp(str, "exp") == 0)
        return EaseInOutExpo;
    if (strcmp(str, "easeInCirc") == 0)
        return EaseInCirc;
    if (strcmp(str, "easeOutCirc") == 0)
        return EaseOutCirc;
    if (strcmp(str, "easeInOutCirc") == 0 || strcmp(str, "circ") == 0)
        return EaseInOutCirc;
    if (strcmp(str, "easeInBack") == 0)
        return EaseInBack;
    if (strcmp(str, "easeOutBack") == 0)
        return EaseOutBack;
    if (strcmp(str, "easeInOutBack") == 0 || strcmp(str, "back") == 0)
        return EaseInOutBack;
    if (strcmp(str, "easeInElastic") == 0)
        return EaseInElastic;
    if (strcmp(str, "easeOutElastic") == 0)
        return EaseOutElastic;
    if (strcmp(str, "easeInOutElastic") == 0 || strcmp(str, "elastic") == 0)
        return EaseInOutElastic;
    if (strcmp(str, "easeInBounce") == 0)
        return EaseInBounce;
    if (strcmp(str, "easeOutBounce") == 0)
        return EaseOutBounce;
    if (strcmp(str, "easeInOutBounce") == 0 || strcmp(str, "bounce") == 0)
        return EaseInOutBounce;

    return EaseLinear;
}
