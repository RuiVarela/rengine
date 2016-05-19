// __!!rengine_copyright!!__ //

#include <rengine/state/Streams.h>
#include <rengine/math/Streams.h>
#include <rengine/lang/debug/Debug.h>

#include <ostream>
#include <iomanip>
#include <string>

namespace rengine
{

	std::ostream& operator << (std::ostream& out, DrawStates::Capability const& capability)
	{
		RENGINE_QUOTE_ENUM_BEGIN(as_string, capability);
			RENGINE_QUOTE_ENUM_NS(as_string, DrawStates, Undefined);
			RENGINE_QUOTE_ENUM_NS(as_string, DrawStates, Blend);
			RENGINE_QUOTE_ENUM_NS(as_string, DrawStates, CullFace);
			RENGINE_QUOTE_ENUM_NS(as_string, DrawStates, DepthTest);
			RENGINE_QUOTE_ENUM_NS(as_string, DrawStates, StencilTest);
		RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

		out << as_string;
		return out;
	}

	std::ostream& operator <<(std::ostream& out, DrawStates::Value const& value)
	{
		RENGINE_QUOTE_ENUM_BEGIN(as_string, value);
			RENGINE_QUOTE_ENUM_NS(as_string, DrawStates, Off);
			RENGINE_QUOTE_ENUM_NS(as_string, DrawStates, On);
		RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

		out << as_string;
		return out;
	}

	std::ostream& operator <<(std::ostream& out, DrawStates const& states)
	{
		if (!states.getCapabilities().empty())
		{
			out << "Capabilities:" << std::endl;
			for(DrawStates::CapabilityValueMap::const_iterator capability_iterator = states.getCapabilities().begin();
				capability_iterator != states.getCapabilities().end();
				++capability_iterator)
			{
				out << std::setfill(' ') << std::left << std::setw(15) << capability_iterator->first << "[" << capability_iterator->second << "]"<< std::endl;
			}
		}

		if (!states.getStates().empty())
		{
			out << "States:" << std::endl;
			for(DrawStates::StateMap::const_iterator state_iterator = states.getStates().begin();
				state_iterator != states.getStates().end();
				++state_iterator)
			{
				out << std::setfill(' ') << std::left << std::setw(17) << state_iterator->second.first->name() << " : "
					<< *(state_iterator->second.first.get()) << " [" << state_iterator->second.second << "]" << std::endl;
			}
		}

		if (!states.getAggregationStates().empty())
		{
			out << "AggregationStates:" << std::endl;
			for(DrawStates::StateVectorMap::const_iterator aggregation_iterator = states.getAggregationStates().begin();
				aggregation_iterator != states.getAggregationStates().end();
				++aggregation_iterator)
			{
				for(DrawStates::StateVector::const_iterator iterator = aggregation_iterator->second.begin();
					iterator != aggregation_iterator->second.end();
					++iterator)
				{
					out << std::setfill(' ') << std::left << std::setw(17)
					<< iterator->first->name() << " : "
					<< *(iterator->first.get())
					<< " [" << iterator->second << "]" << std::endl;
				}
			}
		}

		return out;
	}

	//
	// serialization hook, will double dispatch calling to the concrete type
	//
	std::ostream& operator <<(std::ostream& out, DrawStates::State const& state)
	{
		return state.serializeTo(out);
	}

	std::ostream& operator <<(std::ostream& out, BlendFunction const& state)
	{
		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getSource());
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, Zero);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, One);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, SourceColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusSourceColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, DestinationColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusDestinationColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, SourceAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusSourceAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, DestinationAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusDestinationAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, SourceAlphaSaturate);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, ConstantColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusConstantColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, ConstantAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusConstanteAlpha);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << as_string;
		}

		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getDestination());
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, Zero);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, One);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, SourceColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusSourceColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, DestinationColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusDestinationColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, SourceAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusSourceAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, DestinationAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusDestinationAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, SourceAlphaSaturate);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, ConstantColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusConstantColor);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, ConstantAlpha);
				RENGINE_QUOTE_ENUM_NS(as_string, BlendFunction, OneMinusConstanteAlpha);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << "->" << as_string;
		}

		return out;
	}

	std::ostream& operator << (std::ostream& out, BlendEquation const& state)
	{
		RENGINE_QUOTE_ENUM_BEGIN(as_string, state.get());
			RENGINE_QUOTE_ENUM_NS(as_string, BlendEquation, Add);
			RENGINE_QUOTE_ENUM_NS(as_string, BlendEquation, Subtract);
			RENGINE_QUOTE_ENUM_NS(as_string, BlendEquation, ReverseSubtract);
			RENGINE_QUOTE_ENUM_NS(as_string, BlendEquation, Min);
			RENGINE_QUOTE_ENUM_NS(as_string, BlendEquation, Max);
		RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

		out << as_string;
		return out;
	}

	std::ostream& operator << (std::ostream& out, BlendColor const& state)
	{
		out << state.get();
		return out;
	}

	std::ostream& operator << (std::ostream& out, CullFace const& state)
	{
		RENGINE_QUOTE_ENUM_BEGIN(as_string, state.get());
			RENGINE_QUOTE_ENUM_NS(as_string, CullFace, Front);
			RENGINE_QUOTE_ENUM_NS(as_string, CullFace, Back);
			RENGINE_QUOTE_ENUM_NS(as_string, CullFace, FrontAndBack);
		RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

		out << as_string;
		return out;
	}

	std::ostream& operator << (std::ostream& out, Depth const& state)
	{
		RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getFunction());
			RENGINE_QUOTE_ENUM_NS(as_string, Depth, Never);
			RENGINE_QUOTE_ENUM_NS(as_string, Depth, Less);
			RENGINE_QUOTE_ENUM_NS(as_string, Depth, Equal);
			RENGINE_QUOTE_ENUM_NS(as_string, Depth, LessOrEqual);
			RENGINE_QUOTE_ENUM_NS(as_string, Depth, Greater);
			RENGINE_QUOTE_ENUM_NS(as_string, Depth, NotEqual);
			RENGINE_QUOTE_ENUM_NS(as_string, Depth, GreaterOrEqual);
			RENGINE_QUOTE_ENUM_NS(as_string, Depth, Always);
		RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

		out << as_string << " [" << state.getNear() <<  ", " << state.getFar() << "] " << (state.getWriteFlag() ? "Write" : "Read") ;

		return out;
	}

	std::ostream& operator << (std::ostream& out, ColorChannelMask const& state)
	{

		out << "[ " << (state.getRed() ? "Write" : "Read")
		    << " , " << (state.getGreen() ? "Write" : "Read")
		    << " , " << (state.getBlue() ? "Write" : "Read")
		    << " , " << (state.getAlpha() ? "Write" : "Read")
		    << " ]";
		return out;
	}

	std::ostream& operator << (std::ostream& out, PolygonMode const& state)
	{
		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getFront());
				RENGINE_QUOTE_ENUM_NS(as_string, PolygonMode, Point);
				RENGINE_QUOTE_ENUM_NS(as_string, PolygonMode, Line);
				RENGINE_QUOTE_ENUM_NS(as_string, PolygonMode, Fill);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << "[ " << as_string << " , ";
		}

		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getBack());
				RENGINE_QUOTE_ENUM_NS(as_string, PolygonMode, Point);
				RENGINE_QUOTE_ENUM_NS(as_string, PolygonMode, Line);
				RENGINE_QUOTE_ENUM_NS(as_string, PolygonMode, Fill);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << as_string << " ]";
			return out;
		}
	}

	std::ostream& operator << (std::ostream& out, Stencil const& state)
	{
		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getFunction());
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Never);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Less);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Equal);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, LessOrEqual);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Greater);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, NotEqual);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, GreaterOrEqual);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Always);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << as_string << " [" << state.getReference() << ", " << state.getMask() << "] Stencil Mask : " << state.getStencilMask() << " Operation [";
		}

		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getStencilFail());
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Zero);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Keep);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Replace);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Increment);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Decrement);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Invert);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Increment_Wrap);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Decrement_Wrap);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << as_string << ", ";
		}

		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getStencilPassDepthFail());
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Zero);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Keep);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Replace);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Increment);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Decrement);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Invert);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Increment_Wrap);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Decrement_Wrap);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << as_string << ", ";
		}

		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getStencilPassDepthPass());
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Zero);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Keep);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Replace);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Increment);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Decrement);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Invert);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Increment_Wrap);
				RENGINE_QUOTE_ENUM_NS(as_string, Stencil, Decrement_Wrap);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << as_string << "]";
		}
		return out;
	}

	std::ostream& operator << (std::ostream& out, OperationBuffer const& state)
	{
		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getDrawBuffer());
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, None);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, Front);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, Back);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, FrontAndBack);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, Left);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, Right);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, FrontLeft);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, FrontRight);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, BackLeft);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, BackRight);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << "[" << as_string << ", ";
		}

		{
			RENGINE_QUOTE_ENUM_BEGIN(as_string, state.getReadBuffer());
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, None);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, Front);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, Back);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, FrontAndBack);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, Left);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, Right);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, FrontLeft);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, FrontRight);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, BackLeft);
				RENGINE_QUOTE_ENUM_NS(as_string, OperationBuffer, BackRight);
			RENGINE_QUOTE_ENUM_END(as_string, "invalid_name");

			out << as_string << "]";
		}

		return out;
	}

	std::ostream& operator << (std::ostream& out, Texture2DUnit const& state)
	{
		std::string as_string;

		out << "Texture " << state.getTexture().get()  << " [ " << state.getUnit() << " ] ";
		out << (state.getTexture().get() ? "Valid" : "Empty");

		return out;
	}

	std::ostream& operator << (std::ostream& out, ProgramUnit const& state)
	{
		out << "Program " << state.get().get() << " ";

		if (state.get())
		{
			if (state.get()->getShader(Shader::Vertex))
			{
				out << "Vs " << state.get()->getShader(Shader::Vertex).get() << " ";
			}

			if (state.get()->getShader(Shader::Fragment))
			{
				out << "Fs " << state.get()->getShader(Shader::Fragment).get() << " ";
			}

			out << "ChangeFlags " << state.get()->changeFlags() << " ";
		}


		return out;
	}


} // namespace
