// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SYSTEM_VARIABLE_H__
#define __RENGINE_SYSTEM_VARIABLE_H__

#include <string>
#include <vector>
#include <rengine/lang/Lang.h>

#include <iostream>
namespace rengine
{
	//
	// Variable
	//

	class Variable
	{
	public:
		enum Type
		{
			IntType,
			FloatType,
			BoolType,
			StringType
		};
		virtual ~Variable();

		Variable(Type const type = IntType);
		Variable(Char const* value);
		Variable(std::string const& value);
		Variable(Real const value);
		Variable(Int const value);
		Variable(Bool const value);

		virtual void set(std::string const& value);
		virtual void set(Char const* value);
		virtual void set(Real const value);
		virtual void set(Int const value);
		virtual void set(Bool const value);
		virtual void set(Variable const& value);

		Variable& operator =(Real const value);
		Variable& operator =(Bool const value);
		Variable& operator =(Int const value);
		Variable& operator =(std::string const& value);
		Variable& operator =(Char const* value);
		Variable& operator =(Variable const& value);

		Type type() const;

		operator Real() const;
		operator Int() const;
		operator Bool() const;
		operator std::string() const;

		std::string const& asString() const;
		Real const& asFloat() const;
		Int const& asInt() const;
		Bool const& asBool() const;

		std::string toString() const;
		//does not change the current type, converts to current type
		void fromString(std::string const& string);

		static Variable* instance(std::string const& string);

	private:
		Type variable_type;
		std::string as_string;
		union
		{
			Real as_float;
			Int as_int;
			Bool as_bool;
		};
	};

	typedef std::vector< SharedPointer<Variable> > VariableVector;

	//
	// System Variable
	//

	class SystemVariable : public Variable
	{
	public:
		typedef VariableVector Arguments;
		struct Handler
		{
			virtual ~Handler() {}
			// to allow the system to change the variable return true
			virtual Bool operator()(SystemVariable& variable, Arguments const& arguments) = 0;
		};

		enum Flag
		{
			NoneFlag 	= 0,
			ArchiveFlag = 1,
			ConstFlag 	= 2,
			LatchFlag	= 4
		};

		virtual ~SystemVariable();

		SystemVariable(std::string const& name, Char const* value, Uint const flags = 0);
		SystemVariable(std::string const& name, std::string const& vadue, Uint const flags = 0);
		SystemVariable(std::string const& name, Int const value, Uint const flags = 0);
		SystemVariable(std::string const& name, Real const value, Uint const flags = 0);
		SystemVariable(std::string const& name, Bool const value, Uint const flags = 0);

		virtual void set(std::string const& value);
		virtual void set(Char const* value);
		virtual void set(Real const value);
		virtual void set(Int const value);
		virtual void set(Bool const value);
		virtual void set(Variable const& value);

		SystemVariable& operator =(Real const value);
		SystemVariable& operator =(Bool const value);
		SystemVariable& operator =(Int const value);
		SystemVariable& operator =(std::string const& value);
		SystemVariable& operator =(Char const* value);
		SystemVariable& operator =(Variable const& value);

		void unlatch();
		void reset();

		Uint flags() const;

		void setHandler(Handler* handler);
		void setDescription(std::string const& short_description);

		std::string const& description() const;
		std::string const& name() const;
		Handler* handler();
	private:
		Variable variable_default;
		std::string name_;
		std::string variable_description;
		Uint variable_flags;
		SystemVariable::Handler* variable_handler;
	};


	//
	// Implementation
	//

	//
	// Variable
	//

	RENGINE_INLINE Variable::~Variable() {}
	RENGINE_INLINE Variable::Variable(Type const type) :variable_type(type) { set(0); }
	RENGINE_INLINE Variable::Variable(Char const* value) :variable_type(StringType) { set(std::string(value)); }
	RENGINE_INLINE Variable::Variable(std::string const& value) :variable_type(StringType) { set(value); }
	RENGINE_INLINE Variable::Variable(Real const value) :variable_type(FloatType) { set(value); }
	RENGINE_INLINE Variable::Variable(Int const value) :variable_type(IntType) { set(value); }
	RENGINE_INLINE Variable::Variable(Bool value) :variable_type(BoolType) { set(value); }

	RENGINE_INLINE void Variable::set(std::string const& value) { as_string = value; }
	RENGINE_INLINE void Variable::set(Char const* value) { set(std::string(value)); }
	RENGINE_INLINE void Variable::set(Real const value) { as_float = value; }
	RENGINE_INLINE void Variable::set(Int const value) { as_int = value; }
	RENGINE_INLINE void Variable::set(Bool const value) { as_bool = value; }
	RENGINE_INLINE Variable& Variable::operator =(Real const value) { set(value); return *this; }
	RENGINE_INLINE Variable& Variable::operator =(Bool const value) { set(value); return *this; }
	RENGINE_INLINE Variable& Variable::operator =(Int const value) { set(value); return *this; }
	RENGINE_INLINE Variable& Variable::operator =(std::string const& value) { set(value); return *this; }
	RENGINE_INLINE Variable& Variable::operator =(Char const* value) { set(value); return *this; }
	RENGINE_INLINE Variable& Variable::operator =(Variable const& value)  { set(value); return *this; }

	RENGINE_INLINE Variable::Type Variable::type() const { return variable_type; }

	RENGINE_INLINE Variable::operator Real() const { return as_float; }
	RENGINE_INLINE Variable::operator Int() const { return as_int; }
	RENGINE_INLINE Variable::operator Bool() const { return as_bool; }
	RENGINE_INLINE Variable::operator std::string() const { return as_string; }

	RENGINE_INLINE std::string const& Variable::asString() const { return as_string; }
	RENGINE_INLINE Real const& Variable::asFloat() const { return as_float; }
	RENGINE_INLINE Int const& Variable::asInt() const { return as_int; }
	RENGINE_INLINE Bool const& Variable::asBool() const { return as_bool; }

	//
	// System Variable
	//

	RENGINE_INLINE SystemVariable::~SystemVariable(){}

	RENGINE_INLINE SystemVariable::SystemVariable(std::string const& name, Char const* value, Uint const flags)
	:Variable(std::string(value)), variable_default(std::string(value)), name_(name), variable_flags(flags), variable_handler(0) {}

	RENGINE_INLINE SystemVariable::SystemVariable(std::string const& name, std::string const& value, Uint const flags)
	:Variable(value), variable_default(value), name_(name), variable_flags(flags), variable_handler(0) {}

	RENGINE_INLINE SystemVariable::SystemVariable(std::string const& name, Int const value, Uint const flags)
	:Variable(value), variable_default(value), name_(name), variable_flags(flags), variable_handler(0) {}

	RENGINE_INLINE SystemVariable::SystemVariable(std::string const& name, Real const value, Uint const flags)
	:Variable(value), variable_default(value), name_(name), variable_flags(flags), variable_handler(0) {}

	RENGINE_INLINE SystemVariable::SystemVariable(std::string const& name, Bool value, Uint const flags)
	:Variable(value), variable_default(value), name_(name), variable_flags(flags), variable_handler(0) {}

	RENGINE_INLINE void SystemVariable::set(std::string const& value)
	{
		if (!(variable_flags & (ConstFlag | LatchFlag)))
		{
			Variable::set(value);
		}
	}

	RENGINE_INLINE void SystemVariable::set(Char const* value)
	{
		if (!(variable_flags & (ConstFlag | LatchFlag)))
		{
			Variable::set(value);
		}
	}

	RENGINE_INLINE void SystemVariable::set(Real const value)
	{
		if (!(variable_flags & (ConstFlag | LatchFlag)))
		{
			Variable::set(value);
		}
	}

	RENGINE_INLINE void SystemVariable::set(Int const value)
	{
		if (!(variable_flags & (ConstFlag | LatchFlag)))
		{
			Variable::set(value);
		}
	}

	RENGINE_INLINE void SystemVariable::set(Bool const value)
	{
		if (!(variable_flags & (ConstFlag | LatchFlag)))
		{
			Variable::set(value);
		}
	}

	RENGINE_INLINE void SystemVariable::set(Variable const& value)
	{
		if (!(variable_flags & (ConstFlag | LatchFlag)))
		{
			Variable::set(value);
		}
	}

	RENGINE_INLINE void SystemVariable::unlatch()
	{
		if ( !(variable_flags & ConstFlag) )
		{
			variable_flags &= ~LatchFlag;
		}
	}


	RENGINE_INLINE SystemVariable& SystemVariable::operator =(Real const value) { set(value); return *this; }
	RENGINE_INLINE SystemVariable& SystemVariable::operator =(Bool const value) { set(value); return *this; }
	RENGINE_INLINE SystemVariable& SystemVariable::operator =(Int const value) { set(value); return *this; }
	RENGINE_INLINE SystemVariable& SystemVariable::operator =(std::string const& value) { set(value); return *this; }
	RENGINE_INLINE SystemVariable& SystemVariable::operator =(Char const* value) { set(value); return *this; }
	RENGINE_INLINE SystemVariable& SystemVariable::operator =(Variable const& value) { set(value); return *this; }

	RENGINE_INLINE Uint SystemVariable::flags() const { return variable_flags; }
	RENGINE_INLINE void SystemVariable::setDescription(std::string const& short_description) { variable_description = short_description; }
	RENGINE_INLINE std::string const& SystemVariable::description() const { return variable_description; }
	RENGINE_INLINE std::string const& SystemVariable::name() const { return name_; }
	RENGINE_INLINE void SystemVariable::setHandler(SystemVariable::Handler* handler) { variable_handler = handler; }
	RENGINE_INLINE SystemVariable::Handler* SystemVariable::handler() { return variable_handler; }

} // namespace rengine

#endif // __RENGINE_SYSTEM_VARIABLE_H__
