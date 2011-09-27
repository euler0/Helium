#pragma once

#include <vector>

#include "Foundation/API.h"
#include "Foundation/Memory/SmartPtr.h"

namespace Helium
{
    class HELIUM_FOUNDATION_API UndoCommand : public Helium::RefCountBase<UndoCommand>
    {
    public:
        UndoCommand();
        virtual ~UndoCommand();

        virtual void Undo() = 0;
        virtual void Redo() = 0;

        //
        // A significant command is one that changes data in such a way that a save is required.
        //  By default, all commands are significant.  Override this in a derived class if you do not 
        //  want your command to cause the application to think that a save is required.  It is up
        //  to the application code to check this value and behave appropriately.
        //

        virtual bool IsSignificant() const
        {
            return true;
        }
    };

    typedef Helium::SmartPtr<UndoCommand> UndoCommandPtr;
    typedef std::vector<UndoCommandPtr> V_UndoCommandSmartPtr;
}