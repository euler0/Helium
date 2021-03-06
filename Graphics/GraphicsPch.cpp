#include "GraphicsPch.h"

#include "Platform/Memory.h"

// Define the memory heap for the current module and include the "new"/"delete" operator implementations.
HELIUM_DEFINE_DEFAULT_MODULE_HEAP( Graphics );

#if HELIUM_DEBUG
#include "Platform/NewDelete.h"
#endif
