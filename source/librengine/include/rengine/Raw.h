#ifndef __RENGINE_RAW_H__
#define __RENGINE_RAW_H__

#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>
#include <rengine/outputstream/OutputStream.h>
#include <rengine/resource/ResourceManager.h>

namespace raw
{
	RENGINE_INLINE rengine::CoreEngine* core() { return rengine::CoreEngine::instance(); }
	RENGINE_INLINE rengine::RenderEngine* render() { return &core()->renderEngine(); }
	RENGINE_INLINE rengine::OutputStream& log() { return core()->log(); }
	RENGINE_INLINE rengine::ResourceManager& resource() { return core()->resourceManager(); }
}

#endif //__RENGINE_RAW_H__
