// ==============================================================
//	This file is part of Glest (www.glest.org)
//
//	Copyright (C) 2001-2010 Mark Vejvoda
//
//	You can redistribute this code and/or modify it under
//	the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the
//	License, or (at your option) any later version
// ==============================================================

#ifndef _BASE_RENDERER_H_
#define _BASE_RENDERER_H_

#include "graphics_interface.h"
#include "leak_dumper.h"

namespace Shared { namespace Graphics {

// ===============================================
//	class BaseRenderer
// ===============================================

class BaseRenderer : public RendererMapInterface {
public:
	BaseRenderer() { }
	virtual ~BaseRenderer() { }

	virtual void initMapSurface(int clientW, int clientH);
    virtual void renderMap(MapPreview *map, int x, int y, int clientW, int clientH, int cellSize, bool grid=false, bool heightMap=false, bool hideWater=false, pair<int, int>* mouse_pos=NULL, int* radius=NULL);
};

}} // end namespace

#endif
