// __!!rengine_copyright!!__ //

#include <rengine/state/DrawResource.h>

namespace rengine
{
	void DrawResource::setId(ResourceId const& id, RenderEngine* engine)
	{
		resource_id_ = id;
	}

	ResourceId const& DrawResource::getId(RenderEngine* engine) const
	{
		return resource_id_;
	}

	Bool DrawResource::drawResourceLoaded(RenderEngine* engine) const
	{
		return resource_id_ != 0;
	}

} //namespace
