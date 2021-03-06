#include "MathPch.h"
#include "Math/FpuColor3.h"
#include "Math/FpuColor4.h"

using namespace Helium;

Color3& Color3::operator=( const Color4& v )
{
    r = v.r;
    g = v.g;
    b = v.b;
    return *this;
}

Color3::operator Color4()
{
    return Color4( r, g, b, 255 );
}