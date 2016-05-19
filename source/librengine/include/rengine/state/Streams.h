// __!!rengine_copyright!!__ //

#ifndef __RENGINE_DRAW_STATES_STREAMS_H__
#define __RENGINE_DRAW_STATES_STREAMS_H__

#include <rengine/state/BaseStates.h>
#include <rengine/state/Texture.h>
#include <rengine/state/Program.h>

namespace rengine
{
	std::ostream& operator << (std::ostream& out, DrawStates::Capability const& capability);
	std::ostream& operator << (std::ostream& out, DrawStates::Value const& value);
	std::ostream& operator << (std::ostream& out, DrawStates const& states);

	std::ostream& operator << (std::ostream& out, DrawStates::State const& state);
	std::ostream& operator << (std::ostream& out, BlendFunction const& state);
	std::ostream& operator << (std::ostream& out, BlendEquation const& state);
	std::ostream& operator << (std::ostream& out, BlendColor const& state);
	std::ostream& operator << (std::ostream& out, CullFace const& state);
	std::ostream& operator << (std::ostream& out, Depth const& state);
	std::ostream& operator << (std::ostream& out, AlphaFunction const& state);
	std::ostream& operator << (std::ostream& out, ColorChannelMask const& state);
	std::ostream& operator << (std::ostream& out, PolygonMode const& state);
	std::ostream& operator << (std::ostream& out, Stencil const& state);
	std::ostream& operator << (std::ostream& out, OperationBuffer const& state);

	std::ostream& operator << (std::ostream& out, Texture2DUnit const& state);
	std::ostream& operator << (std::ostream& out, ProgramUnit const& state);
}

#endif //__RENGINE_DRAW_STATES_STREAMS_H__
