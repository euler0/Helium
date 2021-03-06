#include "FoundationPch.h"
#include "Foundation/Inspect/Controls/ButtonControl.h"
#include "Foundation/Inspect/Container.h"
#include "Foundation/Inspect/Canvas.h"

REFLECT_DEFINE_OBJECT( Helium::Inspect::Button );

using namespace Helium;
using namespace Helium::Inspect;

Button::Button()
{
}

bool Button::Process( const tstring& key, const tstring& value )
{
    if ( Base::Process(key, value) )
    {
        return true;
    }

    if ( key == BUTTON_ATTR_TEXT )
    {
        a_Label.Set( value );
        return true;
    }

    if ( key == BUTTON_ATTR_ICON )
    {
        a_Icon.Set( value );
        return true;
    }

    return false;
}
