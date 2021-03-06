#include "SceneGraphPch.h"
#include "Color.h"

using namespace Helium;
using namespace Helium::SceneGraph;

void SceneGraph::Color::UnpackColor( uint32_t packed, uint32_t& a, uint32_t& r, uint32_t& g, uint32_t& b )
{
    a = ( packed >> 24 ) & 0xff;
    r = ( packed >> 16) & 0xff;
    g = ( packed >> 8 ) & 0xff;
    b = ( packed & 0xff );
}

Helium::Color SceneGraph::Color::BlendColor( Helium::Color color1, Helium::Color color2, float32_t weight )
{
    uint32_t a1, a2, r1, r2, g1, g2, b1, b2;

    SceneGraph::Color::UnpackColor( color1.GetArgb(), a1, r1, g1, b1 );
    SceneGraph::Color::UnpackColor( color2.GetArgb(), a2, r2, g2, b2 );

    return Helium::Color(
        static_cast< uint8_t >( static_cast< float32_t >( r1 ) + static_cast< float32_t >( r2 - r1 ) * weight + 0.5f ),
        static_cast< uint8_t >( static_cast< float32_t >( g1 ) + static_cast< float32_t >( g2 - g1 ) * weight + 0.5f ),
        static_cast< uint8_t >( static_cast< float32_t >( b1 ) + static_cast< float32_t >( b2 - b1 ) * weight + 0.5f ),
        static_cast< uint8_t >( static_cast< float32_t >( a1 ) + static_cast< float32_t >( a2 - a1 ) * weight + 0.5f ) );
}

const Helium::Color SceneGraph::Color::SNOW( 255, 250, 250, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::GHOSTWHITE( 248, 248, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::WHITESMOKE( 245, 245, 245, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::GAINSBORO( 220, 220, 220, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::FLORALWHITE( 255, 250, 240, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::OLDLACE( 253, 245, 230, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LINEN( 250, 240, 230, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::ANTIQUEWHITE( 250, 235, 215, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PAPAYAWHIP( 255, 239, 213, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::BLANCHEDALMOND( 255, 235, 205, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::BISQUE( 255, 228, 196, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PEACHPUFF( 255, 218, 185, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::NAVAJOWHITE( 255, 222, 173, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MOCCASIN( 255, 228, 181, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::CORNSILK( 255, 248, 220, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::IVORY( 255, 255, 240, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LEMONCHIFFON( 255, 250, 205, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SEASHELL( 255, 245, 238, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::HONEYDEW( 240, 255, 240, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MINTCREAM( 245, 255, 250, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::AZURE( 240, 255, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::ALICEBLUE( 240, 248, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LAVENDER( 230, 230, 250, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LAVENDERBLUSH( 255, 240, 245, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MISTYROSE( 255, 228, 225, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::WHITE( 255, 255, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::BLACK( 0, 0, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SILVER( 192, 192, 192, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKSLATEGRAY( 47, 79, 79, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKSLATEGREY( 47, 79, 79, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DIMGRAY( 105, 105, 105, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DIMGREY( 105, 105, 105, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SLATEGRAY( 112, 128, 144, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SLATEGREY( 112, 128, 144, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTSLATEGRAY( 119, 136, 153, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTSLATEGREY( 119, 136, 153, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::GRAY( 190, 190, 190, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::GREY( 190, 190, 190, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTGREY( 211, 211, 211, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTGRAY( 211, 211, 211, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MIDNIGHTBLUE( 25, 25, 112, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::NAVY( 0, 0, 128, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::NAVYBLUE( 0, 0, 128, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::CORNFLOWERBLUE( 100, 149, 237, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKSLATEBLUE( 72, 61, 139, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SLATEBLUE( 106, 90, 205, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MEDIUMSLATEBLUE( 123, 104, 238, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTSLATEBLUE( 132, 112, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MEDIUMBLUE( 0, 0, 205, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::ROYALBLUE( 65, 105, 225, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::BLUE( 0, 0, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DODGERBLUE( 30, 144, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DEEPSKYBLUE( 0, 191, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SKYBLUE( 135, 206, 235, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTSKYBLUE( 135, 206, 250, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::STEELBLUE( 70, 130, 180, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTSTEELBLUE( 176, 196, 222, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTBLUE( 173, 216, 230, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::POWDERBLUE( 176, 224, 230, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PALETURQUOISE( 175, 238, 238, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKTURQUOISE( 0, 206, 209, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MEDIUMTURQUOISE( 72, 209, 204, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::TURQUOISE( 64, 224, 208, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::CYAN( 0, 255, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTCYAN( 224, 255, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::CADETBLUE( 95, 158, 160, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MEDIUMAQUAMARINE( 102, 205, 170, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::AQUAMARINE( 127, 255, 212, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKGREEN( 0, 100, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKOLIVEGREEN( 85, 107, 47, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKSEAGREEN( 143, 188, 143, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SEAGREEN( 46, 139, 87, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MEDIUMSEAGREEN( 60, 179, 113, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTSEAGREEN( 32, 178, 170, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PALEGREEN( 152, 251, 152, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SPRINGGREEN( 0, 235, 127, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LAWNGREEN( 124, 252, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::GREEN( 0, 255, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::CHARTREUSE( 127, 255, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MEDIUMSPRINGGREEN( 0, 250, 154, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::GREENYELLOW( 173, 255, 47, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIMEGREEN( 50, 205, 50, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::YELLOWGREEN( 154, 205, 50, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::FORESTGREEN( 44, 149, 44, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::OLIVEDRAB( 107, 142, 35, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKKHAKI( 189, 183, 107, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::KHAKI( 240, 230, 140, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PALEGOLDENROD( 238, 232, 170, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTGOLDENRODYELLOW( 250, 250, 210, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTYELLOW( 255, 255, 224, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::YELLOW( 255, 255, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::GOLD( 255, 215, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTGOLDENROD( 238, 221, 130, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::GOLDENROD( 218, 165, 32, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKGOLDENROD( 184, 134, 11, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::ROSYBROWN( 188, 143, 143, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::INDIAN( 205, 92, 92, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::INDIANRED( 205, 92, 92, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SADDLEBROWN( 139, 69, 19, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SIENNA( 160, 82, 45, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PERU( 205, 133, 63, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::BURLYWOOD( 222, 184, 135, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::BEIGE( 245, 245, 220, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::WHEAT( 245, 222, 179, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SANDYBROWN( 244, 164, 96, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::TAN( 210, 180, 140, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::CHOCOLATE( 210, 105, 30, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::FIREBRICK( 178, 34, 34, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::BROWN( 165, 42, 42, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKSALMON( 233, 150, 122, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::SALMON( 250, 128, 114, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTSALMON( 255, 160, 122, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::ORANGE( 255, 165, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKORANGE( 255, 140, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::CORAL( 255, 127, 80, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTCORAL( 240, 128, 128, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::TOMATO( 255, 99, 71, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::ORANGERED( 255, 69, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::RED( 255, 0, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTRED( 255, 127, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::HOTPINK( 255, 105, 180, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DEEPPINK( 255, 20, 147, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PINK( 255, 192, 203, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTPINK( 255, 182, 193, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PALEVIOLETRED( 219, 112, 147, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MAROON( 176, 48, 96, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MEDIUMVIOLETRED( 199, 21, 133, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::VIOLETRED( 208, 32, 144, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MAGENTA( 255, 0, 255, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::VIOLET( 238, 130, 238, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PLUM( 221, 160, 221, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::ORCHID( 218, 112, 214, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MEDIUMORCHID( 186, 85, 211, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKORCHID( 153, 50, 204, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKVIOLET( 148, 0, 211, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::BLUEVIOLET( 138, 43, 226, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::PURPLE( 160, 32, 240, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::MEDIUMPURPLE( 147, 112, 219, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::THISTLE( 216, 191, 216, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKGREY( 169, 169, 169, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKGRAY( 169, 169, 169, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKBLUE( 0, 0, 139, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKCYAN( 0, 139, 139, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKMAGENTA( 139, 0, 139, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::DARKRED( 139, 0, 0, static_cast< uint8_t >( 255 ) );
const Helium::Color SceneGraph::Color::LIGHTGREEN( 144, 238, 144, static_cast< uint8_t >( 255 ) );
