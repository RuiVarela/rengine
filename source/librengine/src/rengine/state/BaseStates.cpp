// __!!rengine_copyright!!__ //

#include <rengine/state/BaseStates.h>
#include <rengine/state/Streams.h>
#include <rengine/lang/debug/Debug.h>

namespace rengine
{
	//
	// Support for ostream
	//

	META_STATE_HAS_OSTREAM_IMPLEMENTATION(BlendFunction)
	META_STATE_HAS_OSTREAM_IMPLEMENTATION(BlendEquation)
	META_STATE_HAS_OSTREAM_IMPLEMENTATION(BlendColor)
	META_STATE_HAS_OSTREAM_IMPLEMENTATION(CullFace)
	META_STATE_HAS_OSTREAM_IMPLEMENTATION(Depth)
	META_STATE_HAS_OSTREAM_IMPLEMENTATION(ColorChannelMask)
	META_STATE_HAS_OSTREAM_IMPLEMENTATION(PolygonMode)
	META_STATE_HAS_OSTREAM_IMPLEMENTATION(Stencil)
	META_STATE_HAS_OSTREAM_IMPLEMENTATION(OperationBuffer)

} //namespace
