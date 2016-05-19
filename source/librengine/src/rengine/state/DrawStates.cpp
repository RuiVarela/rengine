// __!!rengine_copyright!!__ //

#include <rengine/state/DrawStates.h>
#include <rengine/state/Texture.h>
#include <rengine/state/Streams.h>
#include <rengine/lang/debug/Debug.h>

namespace rengine
{

	void DrawStates::merge(DrawStates const& rhs)
	{
		for (StateMap::const_iterator state = rhs.states.begin();
			 state != rhs.states.end();
			 ++state)
		{
			states[state->first] = StateValuePair(state->second.first->clone(), state->second.second);
		}

		for (StateVectorMap::const_iterator state_vector = rhs.aggregation_states.begin();
			 state_vector != rhs.aggregation_states.end();
			 ++state_vector)
		{
			for (StateVector::const_iterator state = state_vector->second.begin();
				 state != state_vector->second.end();
				++state)
			{
				if (aggregation_states.find(state_vector->first) == aggregation_states.end())
				{
					aggregation_states[state_vector->first] = StateVector();
				}

				aggregation_states[state_vector->first].push_back(StateValuePair(state->first->clone(), state->second));

			}
		}

		capabilities = rhs.capabilities;
	}

	void DrawStates::clear()
	{
		state_dependencies.clear();
		capabilities.clear();
		states.clear();
		aggregation_states.clear();
	}

	void DrawStates::setDefaults()
	{
		clear();
	}

	void DrawStates::setCapability(Capability const& capability, Value const& value)
	{
		capabilities[capability] = value;
	}

	void DrawStates::clearCapability(Capability const& capability)
	{
		capabilities.erase(capability);
	}

	Bool DrawStates::hasCapability(Capability const& capability) const
	{
		return (capabilities.find(capability) != capabilities.end() );
	}

	DrawStates::Value DrawStates::getCapability(Capability const& capability) const
	{
		RENGINE_ASSERT(hasCapability(capability));

		CapabilityValueMap::const_iterator found = capabilities.find(capability);
		return found->second;
	}

	void DrawStates::setState(SharedPointer<State> const& state, Value const& value)
	{
		setStateWithoutDependencyCheck(state, value);

		if (value && state->capabilityUsage(state_dependencies))
		{
			for (CapabilityVector::const_iterator i = state_dependencies.begin();
				 i != state_dependencies.end();
				 ++i)
			{
				setCapability(*i, On);
			}
		}
		state_dependencies.clear();
	}

	void DrawStates::setStateWithoutDependencyCheck(SharedPointer<State> const& state, Value const& value)
	{
		if (isStateAggregation(state->type()))
		{
			if (aggregation_states.find(state->type()) == aggregation_states.end())
			{
				aggregation_states[state->type()] = StateVector();
			}

			aggregation_states[state->type()].push_back(StateValuePair(state, value));
		}
		else
		{
			states[state->type()] = StateValuePair(state, value);
		}
	}

	Bool DrawStates::hasState(State::Type const type) const
	{
		if (isStateAggregation(type))
		{
			return (aggregation_states.find(type) != aggregation_states.end());
		}
		else
		{
			return (states.find(type) != states.end());
		}
	}

	void DrawStates::clearState(State::Type const type)
	{
		if (isStateAggregation(type))
		{
			aggregation_states.erase(type);
		}
		else
		{
			states.erase(type);
		}
	}

	DrawStates::StateMap const& DrawStates::getStates() const
	{
		return states;
	}

	DrawStates::StateVectorMap const& DrawStates::getAggregationStates() const
	{
		return aggregation_states;
	}

	DrawStates::StateVector const& DrawStates::getAggregationStates(State::Type const type) const
	{
		RENGINE_ASSERT(aggregation_states.find(type) != aggregation_states.end());
		RENGINE_ASSERT(isStateAggregation(type));

		StateVectorMap::const_iterator found = aggregation_states.find(type);
		return found->second;
	}

	DrawStates::StateVector& DrawStates::getAggregationStates(State::Type const type)
	{
		RENGINE_ASSERT(aggregation_states.find(type) != aggregation_states.end());
		RENGINE_ASSERT(isStateAggregation(type));

		StateVectorMap::iterator found = aggregation_states.find(type);
		return found->second;
	}

	void DrawStates::createOrZeroAggregationState(State::Type const type)
	{
		if (aggregation_states.find(type) == aggregation_states.end())
		{
			aggregation_states[type] = StateVector();
		}
		else
		{
			aggregation_states[type].clear();
		}
	}

	DrawStates::StateValuePair const& DrawStates::getState(State::Type const type, Uint index) const
	{
		RENGINE_ASSERT(hasState(type));
		if (isStateAggregation(type))
		{
			StateVectorMap::const_iterator found = aggregation_states.find(type);
			RENGINE_ASSERT(index < found->second.size());

			return found->second[index];
		}
		else
		{
			RENGINE_ASSERT(index == 0);
			StateMap::const_iterator found = states.find(type);
			return found->second;
		}
	}

	DrawStates::StateValuePair& DrawStates::getState(State::Type const type, Uint index)
	{
		RENGINE_ASSERT(hasState(type));
		if (isStateAggregation(type))
		{
			StateVectorMap::iterator found = aggregation_states.find(type);
			RENGINE_ASSERT(index < found->second.size());

			return found->second[index];
		}
		else
		{
			RENGINE_ASSERT(index == 0);
			StateMap::iterator found = states.find(type);
			return found->second;
		}
	}

	Uint DrawStates::getNumberOfStates(State::Type const type) const
	{
		Uint count = 0;

		if (hasState(type))
		{
			count = 1;
			if (isStateAggregation(type))
			{
				count = Uint(getAggregationStates(type).size());
			}
		}

		return count;
	}

	void DrawStates::setTexture(Uint unit, SharedPointer<rengine::Texture2D> const& texture, Value const& value)
	{
		if (texture)
		{
			SharedPointer<Texture2DUnit> texture_unit = new Texture2DUnit(texture, Texture2DUnit::Unit(unit));
			setState(texture_unit, value);
		}
	}

	void DrawStates::setTexture(SharedPointer<rengine::Texture2D> const& texture, Value const& value)
	{
		setTexture(getNumberOfStates(DrawStates::Texture2D), texture, value);
	}

	void DrawStates::setProgram(SharedPointer<rengine::Program> const& program, Value const& value)
	{
		if (program)
		{
			setState( new ProgramUnit(program), value );
		}
	}

	SharedPointer<rengine::Program> const& DrawStates::getProgram() const
	{
		RENGINE_ASSERT( hasProgram() );
		DrawStates::StateValuePair const& state_value = getState(DrawStates::Program);
		SharedPointer<rengine::Program> const& program = dynamic_cast<ProgramUnit*>( state_value.first.get() )->get();
		return program;
	}

	SharedPointer<rengine::Program>& DrawStates::getProgram()
	{
		RENGINE_ASSERT( hasProgram() );
		typedef ProgramUnit const* ProgramUnitType;

		DrawStates::StateValuePair& state_value = getState(DrawStates::Program);
		SharedPointer<rengine::Program>& program = dynamic_cast<ProgramUnit*>( state_value.first.get() )->get();
		return program;
	}

	Int DrawStates::compare(DrawStates const& rhs) const
	{
		//TODO: implement draw states compariosn
		return 1;
	}

	std::ostream& DrawStates::State::serializeTo(std::ostream& out) const
	{
		return out;
	}

} //namespace
