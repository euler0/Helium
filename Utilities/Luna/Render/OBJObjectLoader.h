////////////////////////////////////////////////////////////////////////////////////////////////
//
//  OBJObjectLoader.h
//
//  Loads obj files
//
////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Platform.h"
#include <d3dx9.h>

#include "ObjectLoader.h"

namespace Render
{
  class OBJObjectLoader : public ObjectLoader
  {
  public:
    u32 ParseFile(const char* fname, bool winding = false);
  };
}