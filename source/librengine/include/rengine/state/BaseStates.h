// __!!rengine_copyright!!__ //

#ifndef __RENGINE_BASE_STATES_H__
#define __RENGINE_BASE_STATES_H__

#include <rengine/state/DrawStates.h>
#include <rengine/RenderEngine.h>

namespace rengine
{
	//
	// BlendFunction
	//
	class BlendFunction : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(BlendFunction);
		META_STATE_HAS_OSTREAM_DECLARATION()

		enum Operand
		{
			Zero						= 0x0000,
			One							= 0x0001,
			SourceColor					= 0x0300,
			OneMinusSourceColor			= 0x0301,
			DestinationColor			= 0x0306,
			OneMinusDestinationColor	= 0x0307,
			SourceAlpha					= 0x0302,
			OneMinusSourceAlpha			= 0x0303,
			DestinationAlpha			= 0x0304,
			OneMinusDestinationAlpha	= 0x0305,
			SourceAlphaSaturate			= 0x0308,
			ConstantColor				= 0x8001,
			OneMinusConstantColor		= 0x8002,
			ConstantAlpha				= 0x8003,
			OneMinusConstanteAlpha		= 0x8004
		};

		BlendFunction();
		BlendFunction(Operand const& source_operand, Operand const& destination_operand);
		BlendFunction(BlendFunction const& rhs);
		~BlendFunction();

		void setSource(Operand const& operand);
		Operand const& getSource() const;

		void setDestination(Operand const& operand);
		Operand const& getDestination() const;

		virtual Bool capabilityUsage(DrawStates::CapabilityVector& capabilities) const;
		virtual Int compare(DrawStates::State const& rhs) const;
	private:
		Operand source;
		Operand destination;
	};

	//
	// BlendEquation
	//
	class BlendEquation : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(BlendEquation);
		META_STATE_HAS_OSTREAM_DECLARATION()

		enum Equation
		{
			Add					= 0x8006,
			Subtract			= 0x800A,
			ReverseSubtract		= 0x800B,
			Min					= 0x8007,
			Max					= 0x8008
		};

		BlendEquation();
		BlendEquation(Equation const& equation);
		BlendEquation(BlendEquation const& rhs);
		~BlendEquation();

		void set(Equation const& equation);
		Equation const& get() const;

		virtual Bool capabilityUsage(DrawStates::CapabilityVector& capabilities) const;
		virtual Int compare(DrawStates::State const& rhs) const;
	private:
		Equation state_equation;
	};

	//
	// BlendColor
	//
	class BlendColor : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(BlendColor);
		META_STATE_HAS_OSTREAM_DECLARATION()

		BlendColor();
		BlendColor(Vector4D const& color);
		BlendColor(BlendColor const& rhs);
		~BlendColor();

		void set(Vector4D const& color);
		Vector4D const& get() const;

		virtual Bool capabilityUsage(DrawStates::CapabilityVector& capabilities) const;
		virtual Int compare(DrawStates::State const& rhs) const;
	private:
		Vector4D color_;
	};

	//
	// CullFace
	//
	class CullFace : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(CullFace);
		META_STATE_HAS_OSTREAM_DECLARATION()

		enum Operand
		{
			Front			= 0x0404,
			Back			= 0x0405,
			FrontAndBack	= 0x0408
		};

		CullFace();
		CullFace(Operand const& operand);
		CullFace(CullFace const& rhs);
		~CullFace();

		void set(Operand const& operand);
		Operand const& get() const;

		virtual Bool capabilityUsage(DrawStates::CapabilityVector& capabilities) const;
		virtual Int compare(DrawStates::State const& rhs) const;
	private:
		Operand cull_face;
	};

	//
	// Depth
	//
	class Depth : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(Depth);
		META_STATE_HAS_OSTREAM_DECLARATION()

		enum Function
		{
			Never				= 0x0200,
			Less				= 0x0201,
			Equal				= 0x0202,
			LessOrEqual			= 0x0203,
			Greater				= 0x0204,
			NotEqual			= 0x0205,
			GreaterOrEqual		= 0x0206,
			Always				= 0x0207
		};

		Depth();
		Depth(Function const& depth_function, Real const near = 0.0, Real const far = 1.0, Bool const write = true);
		Depth(Depth const& rhs);
		~Depth();

		void setFunction(Function const& depth_function);
		Function const& getFunction() const;

		void setNear(Real const value);
		Real getNear() const;

		void setFar(Real const value);
		Real getFar() const;

		void setWriteFlag(Bool const value);
		Bool getWriteFlag() const;

		virtual Bool capabilityUsage(DrawStates::CapabilityVector& capabilities) const;
		virtual Int compare(DrawStates::State const& rhs) const;
	private:
		Function function;
		Real z_near;
		Real z_far;
		Bool mask;
	};

	//
	// ColorChannelMask
	//
	class ColorChannelMask : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(ColorChannelMask);
		META_STATE_HAS_OSTREAM_DECLARATION()

		ColorChannelMask();
		ColorChannelMask(Bool const red, Bool const green, Bool const blue, Bool const alpha);
		ColorChannelMask(ColorChannelMask const& rhs);
		~ColorChannelMask();

		void setRed(Bool const value);
		Bool getRed() const;

		void setGreen(Bool const value);
		Bool getGreen() const;

		void setBlue(Bool const value);
		Bool getBlue() const;

		void setAlpha(Bool const value);
		Bool getAlpha() const;

		virtual Int compare(DrawStates::State const& rhs) const;
	private:
		Bool red_;
		Bool green_;
		Bool blue_;
		Bool alpha_;
	};

	//
	// PolygonMode
	//
	class PolygonMode : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(PolygonMode);
		META_STATE_HAS_OSTREAM_DECLARATION()

		enum Face
		{
			Front			= 0x0404,
			Back			= 0x0405,
			FrontAndBack	= 0x0408
		};

		enum Mode
		{
			Point			= 0x1B00,
			Line			= 0x1B01,
			Fill			= 0x1B02
		};

		PolygonMode();
		PolygonMode(Face const face, Mode const mode);
		PolygonMode(PolygonMode const& rhs);
		~PolygonMode();

		void setMode(Face const face, Mode const mode);
		Mode const& getMode(Face const face) const;

		Mode const& getFront() const;
		Mode const& getBack() const;

		virtual Int compare(DrawStates::State const& rhs) const;
	private:
		Mode front_face;
		Mode back_face;
	};


	//
	// Stencil
	//
	class Stencil : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(Stencil);
		META_STATE_HAS_OSTREAM_DECLARATION()

		enum Function
		{
			Never				= 0x0200,
			Less				= 0x0201,
			Equal				= 0x0202,
			LessOrEqual			= 0x0203,
			Greater				= 0x0204,
			NotEqual			= 0x0205,
			GreaterOrEqual		= 0x0206,
			Always				= 0x0207
		};

        enum Operation
        {
            Zero 				= 0,
			Keep 				= 0x1E00,
            Replace 			= 0x1E01,
            Increment 			= 0x1E02,
            Decrement 			= 0x1E03,
            Invert 				= 0x150A,
            Increment_Wrap 		= 0x8507,
            Decrement_Wrap 		= 0x8508
        };


		Stencil();
		Stencil(Function const& function, Int const& reference, Uint const& mask);
		Stencil(Stencil const& rhs);
		~Stencil();

		void setFunction(Function const& function, Int const& reference, Uint const& mask);
		Function const& getFunction() const;
		Int getReference() const;
		Uint getMask() const;

		void setStencilMask(Uint const& stencil_mask);
		Uint getStencilMask() const;

		void setOperation(Operation const& stencil_fail, Operation const& stencil_pass_depth_fail, Operation const& stencil_pass_depth_pass);
		Operation getStencilFail() const;
		Operation getStencilPassDepthFail() const;
		Operation getStencilPassDepthPass() const;


		virtual Bool capabilityUsage(DrawStates::CapabilityVector& capabilities) const;

		virtual Int compare(DrawStates::State const& rhs) const;

	private:
		Function function;
		Int reference;
		Uint mask;
		Operation stencil_fail;
		Operation stencil_pass_depth_fail;
		Operation stencil_pass_depth_pass;
		Uint stencil_mask;
	};

	//
	// OperationBuffer
	//
	class OperationBuffer : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(OperationBuffer);
		META_STATE_HAS_OSTREAM_DECLARATION()

		enum Buffer
		{
			None			= 0,
			Front			= 0x0404,
			Back			= 0x0405,
			FrontAndBack	= 0x0408,
			Left			= 0x0406,
			Right			= 0x0407,
			FrontLeft		= 0x0400,
			FrontRight		= 0x0401,
			BackLeft		= 0x0402,
			BackRight		= 0x0403
		};

		OperationBuffer();
		OperationBuffer(Buffer const& draw_buffer, Buffer const& read_buffer);
		OperationBuffer(OperationBuffer const& rhs);

		~OperationBuffer();

		virtual Int compare(DrawStates::State const& rhs) const;

		void setReadBuffer(Buffer const read);
		Buffer getReadBuffer() const;

		void setDrawBuffer(Buffer const draw);
		Buffer getDrawBuffer() const;
	private:
		Buffer draw_;
		Buffer read_;
	};



	//
	// Implementation
	//


	//
	// BlendFunction
	//
	RENGINE_INLINE BlendFunction::BlendFunction() :
		State(DrawStates::Blend), source(SourceAlpha), destination(OneMinusSourceAlpha)
	{
	}

	RENGINE_INLINE BlendFunction::BlendFunction(Operand const& source_operand, Operand const& destination_operand) :
		State(DrawStates::Blend), source(source_operand), destination(destination_operand)
	{
	}

	RENGINE_INLINE BlendFunction::BlendFunction(BlendFunction const& rhs) :
		State(DrawStates::Blend), source(rhs.source), destination(rhs.destination)
	{
	}

	RENGINE_INLINE BlendFunction::~BlendFunction()
	{
	}

	RENGINE_INLINE void BlendFunction::setSource(Operand const& operand)
	{
		source = operand;
	}

	RENGINE_INLINE BlendFunction::Operand const& BlendFunction::getSource() const
	{
		return source;
	}

	RENGINE_INLINE void BlendFunction::setDestination(Operand const& operand)
	{
		destination = operand;
	}

	RENGINE_INLINE BlendFunction::Operand const& BlendFunction::getDestination() const
	{
		return destination;
	}

	RENGINE_INLINE Bool BlendFunction::capabilityUsage(DrawStates::CapabilityVector& capabilities) const
	{
		capabilities.push_back(DrawStates::Blend);
		return true;
	}

	RENGINE_INLINE Int BlendFunction::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(BlendFunction, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, source);
		META_STATE_COMPARE_PROPERTY(casted_rhs, destination);
		return 0;
	}

	//
	// BlendEquation
	//
	RENGINE_INLINE BlendEquation::BlendEquation() :
		State(DrawStates::BlendEquation), state_equation(BlendEquation::Add)
	{
	}

	RENGINE_INLINE BlendEquation::BlendEquation(Equation const& equation) :
		State(DrawStates::BlendEquation), state_equation(equation)
	{
	}

	RENGINE_INLINE BlendEquation::BlendEquation(BlendEquation const& rhs) :
		State(DrawStates::BlendEquation), state_equation(rhs.state_equation)
	{
	}

	RENGINE_INLINE BlendEquation::~BlendEquation()
	{
	}

	RENGINE_INLINE void BlendEquation::set(Equation const& equation)
	{
		state_equation = equation;
	}

	RENGINE_INLINE BlendEquation::Equation const& BlendEquation::get() const
	{
		return state_equation;
	}

	RENGINE_INLINE Bool BlendEquation::capabilityUsage(DrawStates::CapabilityVector& capabilities) const
	{
		capabilities.push_back(DrawStates::Blend);
		return true;
	}

	RENGINE_INLINE Int BlendEquation::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(BlendEquation, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, state_equation);
		return 0;
	}

	//
	// BlendColor
	//
	RENGINE_INLINE BlendColor::BlendColor() :
		State(DrawStates::BlendColor)
	{
	}

	RENGINE_INLINE BlendColor::BlendColor(Vector4D const& color) :
		State(DrawStates::BlendColor), color_(color)
	{
	}

	RENGINE_INLINE BlendColor::BlendColor(BlendColor const& rhs) :
		State(DrawStates::BlendColor), color_(rhs.color_)
	{
	}

	RENGINE_INLINE BlendColor::~BlendColor()
	{
	}

	RENGINE_INLINE void BlendColor::set(Vector4D const& color)
	{
		color_ = color;
	}

	RENGINE_INLINE Vector4D const& BlendColor::get() const
	{
		return color_;
	}

	RENGINE_INLINE Bool BlendColor::capabilityUsage(DrawStates::CapabilityVector& capabilities) const
	{
		capabilities.push_back(DrawStates::Blend);
		return true;
	}

	RENGINE_INLINE Int BlendColor::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(BlendColor, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, color_);
		return 0;
	}

	//
	// CullFace
	//
	RENGINE_INLINE CullFace::CullFace() :
		State(DrawStates::CullFace), cull_face(CullFace::Back)
	{
	}

	RENGINE_INLINE CullFace::CullFace(Operand const& operand) :
		State(DrawStates::CullFace), cull_face(operand)
	{
	}

	RENGINE_INLINE CullFace::CullFace(CullFace const& rhs) :
		State(DrawStates::CullFace), cull_face(rhs.cull_face)
	{
	}

	RENGINE_INLINE CullFace::~CullFace()
	{
	}

	RENGINE_INLINE void CullFace::set(Operand const& operand)
	{
		cull_face = operand;
	}

	RENGINE_INLINE CullFace::Operand const& CullFace::get() const
	{
		return cull_face;
	}

	RENGINE_INLINE Bool CullFace::capabilityUsage(DrawStates::CapabilityVector& capabilities) const
	{
		capabilities.push_back(DrawStates::CullFace);
		return true;
	}

	RENGINE_INLINE Int CullFace::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(CullFace, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, cull_face);
		return 0;
	}

	//
	// Depth
	//
	RENGINE_INLINE Depth::Depth() :
		State(DrawStates::DepthTest), function(Less), z_near(0.0), z_far(1.0), mask(true)
	{
	}

	RENGINE_INLINE Depth::Depth(Function const& depth_function, Real const near, Real const far, Bool const write)
	:State(DrawStates::DepthTest), function(depth_function), z_near(near), z_far(far), mask(write)
	{
	}

	RENGINE_INLINE Depth::Depth(Depth const& rhs)
	:State(DrawStates::DepthTest), function(rhs.function), z_near(rhs.z_near), z_far(rhs.z_far), mask(rhs.mask)
	{
	}

	RENGINE_INLINE Depth::~Depth()
	{
	}

	RENGINE_INLINE void Depth::setFunction(Function const& depth_function)
	{
		function = depth_function;
	}

	RENGINE_INLINE Depth::Function const& Depth::getFunction() const
	{
		return function;
	}

	RENGINE_INLINE void Depth::setNear(Real const value)
	{
		z_near = value;
	}

	RENGINE_INLINE Real Depth::getNear() const
	{
		return z_near;
	}

	RENGINE_INLINE void Depth::setFar(Real const value)
	{
		z_far = value;
	}

	RENGINE_INLINE Real Depth::getFar() const
	{
		return z_far;
	}

	RENGINE_INLINE void Depth::setWriteFlag(Bool const value)
	{
		mask = value;
	}

	RENGINE_INLINE Bool Depth::getWriteFlag() const
	{
		return mask;
	}

	RENGINE_INLINE Bool Depth::capabilityUsage(DrawStates::CapabilityVector& capabilities) const
	{
		capabilities.push_back(DrawStates::DepthTest);
		return true;
	}

	RENGINE_INLINE Int Depth::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(Depth, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, function);
		META_STATE_COMPARE_PROPERTY(casted_rhs, z_far);
		META_STATE_COMPARE_PROPERTY(casted_rhs, z_near);
		META_STATE_COMPARE_PROPERTY(casted_rhs, mask);
		return 0;
	}

	//
	// ColorChannelMask
	//
	RENGINE_INLINE ColorChannelMask::ColorChannelMask() :
		State(DrawStates::ColorChannelMask), red_(true), green_(true), blue_(true), alpha_(true)
	{
	}

	RENGINE_INLINE ColorChannelMask::ColorChannelMask(Bool const red, Bool const green, Bool const blue, Bool const alpha) :
		State(DrawStates::ColorChannelMask), red_(red), green_(green), blue_(blue), alpha_(alpha)
	{
	}

	RENGINE_INLINE ColorChannelMask::ColorChannelMask(ColorChannelMask const& rhs) :
		State(DrawStates::ColorChannelMask), red_(rhs.red_), green_(rhs.green_), blue_(rhs.blue_), alpha_(rhs.alpha_)
	{
	}

	RENGINE_INLINE ColorChannelMask::~ColorChannelMask()
	{
	}

	RENGINE_INLINE void ColorChannelMask::setRed(Bool const value)
	{
		red_ = value;
	}

	RENGINE_INLINE Bool ColorChannelMask::getRed() const
	{
		return red_;
	}

	RENGINE_INLINE void ColorChannelMask::setGreen(Bool const value)
	{
		green_ = value;
	}

	RENGINE_INLINE Bool ColorChannelMask::getGreen() const
	{
		return green_;
	}

	RENGINE_INLINE void ColorChannelMask::setBlue(Bool const value)
	{
		blue_ = value;
	}

	RENGINE_INLINE Bool ColorChannelMask::getBlue() const
	{
		return blue_;
	}

	RENGINE_INLINE void ColorChannelMask::setAlpha(Bool const value)
	{
		alpha_ = value;
	}

	RENGINE_INLINE Bool ColorChannelMask::getAlpha() const
	{
		return alpha_;
	}

	RENGINE_INLINE Int ColorChannelMask::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(ColorChannelMask, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, red_);
		META_STATE_COMPARE_PROPERTY(casted_rhs, green_);
		META_STATE_COMPARE_PROPERTY(casted_rhs, blue_);
		META_STATE_COMPARE_PROPERTY(casted_rhs, alpha_)
		return 0;
	}

	//
	// PolygonMode
	//
	RENGINE_INLINE PolygonMode::PolygonMode() :
		State(DrawStates::PolygonMode), front_face(Fill), back_face(Fill)
	{
	}

	RENGINE_INLINE PolygonMode::PolygonMode(Face const face, Mode const mode) :
		State(DrawStates::PolygonMode), front_face(Fill), back_face(Fill)
	{
		setMode(face, mode);
	}

	RENGINE_INLINE PolygonMode::PolygonMode(PolygonMode const& rhs) :
		State(DrawStates::PolygonMode), front_face(rhs.front_face), back_face(rhs.back_face)
	{
	}

	RENGINE_INLINE PolygonMode::~PolygonMode()
	{
	}

	RENGINE_INLINE void PolygonMode::setMode(Face const face, Mode const mode)
	{
		if (face == Front)
		{
			front_face = mode;
		}
		else if (face == Back)
		{
			back_face = mode;
		}
		else
		{
			front_face = mode;
			back_face = mode;
		}
	}

	RENGINE_INLINE PolygonMode::Mode const& PolygonMode::getMode(Face const face) const
	{
		if ((face == Front) || (face == FrontAndBack))
		{
			return front_face;
		}
		else
		{
			return back_face;
		}
	}

	RENGINE_INLINE PolygonMode::Mode const& PolygonMode::getFront() const
	{
		return front_face;
	}

	RENGINE_INLINE PolygonMode::Mode const& PolygonMode::getBack() const
	{
		return back_face;
	}

	RENGINE_INLINE Int PolygonMode::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(PolygonMode, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, front_face);
		META_STATE_COMPARE_PROPERTY(casted_rhs, back_face);
		return 0;
	}

	//
	// Stencil
	//
	RENGINE_INLINE Stencil::Stencil()
		:State(DrawStates::StencilTest)
	{
		setFunction(Always, 0, 0x0000FF);
		setStencilMask(0x0000FF);
		setOperation(Keep, Keep, Keep);
	}

	RENGINE_INLINE Stencil::Stencil(Function const& function, Int const& reference, Uint const& mask)
		:State(DrawStates::StencilTest)
	{
		setFunction(function, reference, mask);
		setStencilMask(0x0000FF);
		setOperation(Keep, Keep, Keep);
	}

	RENGINE_INLINE Stencil::Stencil(Stencil const& rhs) :
		State(DrawStates::StencilTest)
	{
		setFunction(rhs.function, rhs.reference, rhs.mask);
		setStencilMask(rhs.stencil_mask);
		setOperation(rhs.stencil_fail, rhs.stencil_pass_depth_fail, rhs.stencil_pass_depth_pass);
	}

	RENGINE_INLINE Stencil::~Stencil()
	{
	}

	RENGINE_INLINE void Stencil::setFunction(Function const& function, Int const& reference, Uint const& mask)
	{
		this->function = function;
		this->reference = reference;
		this->mask = mask;
	}

	RENGINE_INLINE Stencil::Function const& Stencil::getFunction() const
	{
		return function;
	}

	RENGINE_INLINE Int Stencil::getReference() const
	{
		return reference;
	}

	RENGINE_INLINE Uint Stencil::getMask() const
	{
		return mask;
	}

	RENGINE_INLINE void Stencil::setStencilMask(Uint const& stencil_mask)
	{
		this->stencil_mask = stencil_mask;
	}

	RENGINE_INLINE Uint Stencil::getStencilMask() const
	{
		return stencil_mask;
	}

	RENGINE_INLINE void Stencil::setOperation(Operation const& stencil_fail, Operation const& stencil_pass_depth_fail, Operation const& stencil_pass_depth_pass)
	{
		this->stencil_fail = stencil_fail;
		this->stencil_pass_depth_fail = stencil_pass_depth_fail;
		this->stencil_pass_depth_pass = stencil_pass_depth_pass;
	}

	RENGINE_INLINE Stencil::Operation Stencil::getStencilFail() const
	{
		return stencil_fail;
	}

	RENGINE_INLINE Stencil::Operation Stencil::getStencilPassDepthFail() const
	{
		return stencil_pass_depth_fail;
	}

	RENGINE_INLINE Stencil::Operation Stencil::getStencilPassDepthPass() const
	{
		return stencil_pass_depth_pass;
	}

	RENGINE_INLINE Bool Stencil::capabilityUsage(DrawStates::CapabilityVector& capabilities) const
	{
		capabilities.push_back(DrawStates::StencilTest);
		return true;
	}

	RENGINE_INLINE Int Stencil::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(Stencil, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, function);
		META_STATE_COMPARE_PROPERTY(casted_rhs, reference);
		META_STATE_COMPARE_PROPERTY(casted_rhs, mask);
		META_STATE_COMPARE_PROPERTY(casted_rhs, stencil_fail);
		META_STATE_COMPARE_PROPERTY(casted_rhs, stencil_pass_depth_fail);
		META_STATE_COMPARE_PROPERTY(casted_rhs, stencil_pass_depth_pass);
		META_STATE_COMPARE_PROPERTY(casted_rhs, stencil_mask);
		return 0;
	}

	//
	// OperationBuffer
	//
	RENGINE_INLINE OperationBuffer::OperationBuffer()
		:State(DrawStates::OperationBuffer), draw_(Back), read_(Back)
	{
	}

	RENGINE_INLINE OperationBuffer::OperationBuffer(Buffer const& draw_buffer, Buffer const& read_buffer)
		:State(DrawStates::OperationBuffer), draw_(draw_buffer), read_(read_buffer)
	{
	}

	RENGINE_INLINE OperationBuffer::OperationBuffer(OperationBuffer const& rhs)
		:State(DrawStates::OperationBuffer), draw_(rhs.draw_), read_(rhs.read_)
	{
	}

	RENGINE_INLINE OperationBuffer::~OperationBuffer()
	{
	}

	RENGINE_INLINE void OperationBuffer::setReadBuffer(Buffer const read)
	{
		read_ = read;
	}

	RENGINE_INLINE void OperationBuffer::setDrawBuffer(Buffer const draw)
	{
		draw_ = draw;
	}

	RENGINE_INLINE OperationBuffer::Buffer OperationBuffer::getReadBuffer() const
	{
		return read_;
	}

	RENGINE_INLINE OperationBuffer::Buffer OperationBuffer::getDrawBuffer() const
	{
		return draw_;
	}

	RENGINE_INLINE Int OperationBuffer::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(OperationBuffer, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, read_);
		META_STATE_COMPARE_PROPERTY(casted_rhs, draw_);
		return 0;
	}
}

#endif //__RENGINE_BASE_STATES_H__
