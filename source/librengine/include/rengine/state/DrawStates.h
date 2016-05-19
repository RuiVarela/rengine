// __!!rengine_copyright!!__ //

#ifndef __RENGINE_DRAW_STATES_H__
#define __RENGINE_DRAW_STATES_H__

#include <rengine/lang/Lang.h>

#include <map>
#include <vector>
#include <string>

#define META_STATE_FUNCTIONS(meta_type) \
	virtual meta_type* create() const { return new meta_type(); } \
	virtual meta_type* clone() const { return new meta_type(*this); } \
	virtual void apply(RenderEngine& render_engine) { render_engine.apply(*this); } \
	virtual std::string name() const { return #meta_type; }

#define META_STATE_HAS_OSTREAM_DECLARATION() \
	virtual std::ostream& serializeTo(std::ostream& out) const;

#define META_STATE_HAS_OSTREAM_IMPLEMENTATION(meta_type) \
	std::ostream& meta_type::serializeTo(std::ostream& out) const { out << *this; return out; }

#define META_STATE_COMPARE(meta_type, rhs_variable, casted_var) \
	if (this == &rhs_variable) { return 0; } \
	if (type() != rhs_variable.type()) { return -1; } \
	meta_type const& casted_var = static_cast<meta_type const&>(rhs_variable);

#define META_STATE_COMPARE_PROPERTY(casted_rhs, property) \
	if (property < casted_rhs.property) { return -1; } \
	if (casted_rhs.property < property) { return 1; }

namespace rengine
{
	class Texture2D;
	class RenderEngine;
	class Program;

	class DrawStates
	{
	public:

		enum Capability
		{
			Undefined		= 0x0000,
			Blend			= 0x0BE2,
			CullFace		= 0x0B44,
			DepthTest		= 0x0B71,
			StencilTest  	= 0x0B90
		};

		enum Value
		{
			Off				= 0x0000,
			On				= 0x0001
		};

		//
		// Some capabilities are also state types
		//
		// DepthTest <-> Depth
		// CullFace <-> CullFace
		// AlphaTest <-> AlphaFunction
		// Blend <-> BlendFunction
		// StencilTest <-> Stencil
		//
		// Additional State Types
		//
		enum StateType
		{
			BlendEquation		= 0x0F00,
			BlendColor			= 0x0F01,
			ColorChannelMask	= 0x0F02,
			OperationBuffer		= 0x0F03,
			PolygonMode			= 0x0F04,
			Program				= 0x0F05,
			Texture2D			= 0x0F06
		};

		//
		// An aggregation state allow multiple definition of the same state
		//
		// Texture2D is an aggregation state (multitexturing)
		//

		typedef std::map<Capability, Value> CapabilityValueMap;
		typedef std::vector<Capability> CapabilityVector;

		//
		// Base class for state representation
		//
		class State
		{
		public:
			typedef Uint Type;

			State();
			State(Type const& type);
			virtual ~State();

			virtual Int compare(State const& rhs) const;

			virtual State* clone() const = 0;
			virtual State* create() const = 0;

			virtual void apply(RenderEngine& render_engine) = 0;
			virtual std::string name() const;

			//return true if uses capabilities
			virtual Bool capabilityUsage(CapabilityVector& capabilities) const;

			Type const& type() const;

			virtual std::ostream& serializeTo(std::ostream& out) const;
		protected:
			Type state_type;
		};

		typedef std::pair< SharedPointer<State>, Value> StateValuePair;
		typedef std::vector<StateValuePair> StateVector;
		typedef std::map<State::Type, StateValuePair> StateMap;
		typedef std::map<State::Type, StateVector> StateVectorMap;

		DrawStates();
		DrawStates(DrawStates const& rhs);
		~DrawStates();

		DrawStates const& operator = (DrawStates const& rhs);
		void merge(DrawStates const& rhs);

		void clear();
		void setDefaults();

		//
		// Capabilities management
		//
		CapabilityValueMap const& getCapabilities() const;

		void setCapability(Capability const& capability, Value const& value);
		void clearCapability(Capability const& capability);
		Bool hasCapability(Capability const& capability) const;
		Value getCapability(Capability const& capability) const;

		//
		// setState checks for capability dependencies
		// If needed enables dependencies
		//
		// if the the input state is an aggregation type,
		// adds the state to the corresponding aggregation vector
		//
		void setState(SharedPointer<State> const& state, Value const& value = On);
		void setStateWithoutDependencyCheck(SharedPointer<State> const& state, Value const& value = On);
		Bool hasState(State::Type const type) const;
		void clearState(State::Type const type);

		Uint getNumberOfStates(State::Type const type) const;
		StateValuePair& getState(State::Type const type, Uint index = 0);
		StateValuePair const& getState(State::Type const type, Uint index = 0) const;
		StateMap const& getStates() const;

		//for aggregation states
		StateVectorMap const& getAggregationStates() const;
		StateVector const& getAggregationStates(State::Type const type) const;
		StateVector& getAggregationStates(State::Type const type);
		// creates the aggregation state, if the state already exists, sets the number of elements to 0
		void createOrZeroAggregationState(State::Type const type);

		//
		// Texture Units can also be added with setState
		//

		// adds a new texture unit to the texture aggregation, this method does not replace existing texture units
		void setTexture(Uint unit, SharedPointer<rengine::Texture2D> const& texture, Value const& value = On);
		// adds a new texture unit to the texture aggregation, the unit number is the current size of the aggregation
		void setTexture(SharedPointer<rengine::Texture2D> const& texture, Value const& value = On);

		//Sets the program
		void setProgram(SharedPointer<rengine::Program> const& program, Value const& value = On);

		// Shortcuts to the state program
		Bool hasProgram() const;
		SharedPointer<rengine::Program> const& getProgram() const;
		SharedPointer<rengine::Program>& getProgram();


		//
		// Comparison operation
		//
		Int compare(DrawStates const& rhs) const;
		Bool operator == (DrawStates const& rhs) const;
		Bool operator != (DrawStates const& rhs) const;
		Bool operator > (DrawStates const& rhs) const;
		Bool operator < (DrawStates const& rhs) const;

		static Bool isStateAggregation(State::Type const type);
	private:
		CapabilityVector state_dependencies;
		CapabilityValueMap capabilities;
		StateMap states;
		StateVectorMap aggregation_states;
	};

	//
	// Implementation
	//

	RENGINE_INLINE DrawStates::State::State() :
		state_type(Undefined)
	{
	}

	RENGINE_INLINE DrawStates::State::State(Type const& type) :
		state_type(type)
	{
	}

	RENGINE_INLINE DrawStates::State::~State()
	{

	}

	RENGINE_INLINE Bool DrawStates::State::capabilityUsage(CapabilityVector& capabilities) const
	{
		return false;
	}

	RENGINE_INLINE DrawStates::State::Type const& DrawStates::State::type() const
	{
		return state_type;
	}

	RENGINE_INLINE std::string DrawStates::State::name() const
	{
		return "State";
	}

	RENGINE_INLINE Int DrawStates::State::compare(State const& rhs) const
	{
		return ((rhs.type() == type()) ? 0 : -1);
	}

	//
	// DrawStates
	//
	RENGINE_INLINE DrawStates::DrawStates()
	{
		setDefaults();
	}

	RENGINE_INLINE DrawStates::~DrawStates()
	{
	}

	RENGINE_INLINE DrawStates::DrawStates(DrawStates const& rhs)
	{
		merge(rhs);
	}

	RENGINE_INLINE DrawStates const& DrawStates::operator = (DrawStates const& rhs)
	{
		state_dependencies.clear();
		capabilities.clear();
		states.clear();
		aggregation_states.clear();

		merge(rhs);

		return *this;
	}

	RENGINE_INLINE Bool DrawStates::isStateAggregation(State::Type const type)
	{
		return (type == DrawStates::Texture2D);
	}

	RENGINE_INLINE DrawStates::CapabilityValueMap const& DrawStates::getCapabilities() const
	{
		return capabilities;
	}

	RENGINE_INLINE Bool DrawStates::hasProgram() const
	{
		return hasState(DrawStates::Program);
	}

	RENGINE_INLINE Bool DrawStates::operator == (DrawStates const& rhs) const
	{
		return (compare(rhs) == 0);
	}

	RENGINE_INLINE Bool DrawStates::operator != (DrawStates const& rhs) const
	{
		return (compare(rhs) != 0);
	}

	RENGINE_INLINE Bool DrawStates::operator > (DrawStates const& rhs) const
	{
		return (compare(rhs) > 0);
	}

	RENGINE_INLINE Bool DrawStates::operator < (DrawStates const& rhs) const
	{
		return (compare(rhs) < 0);
	}

}

#endif //__RENGINE_DRAW_STATES_H__
