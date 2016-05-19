#include <rengine/system/System.h>
#include <rengine/string/String.h>

namespace rengine
{
	std::string Variable::toString() const
	{
		switch (type())
		{
			case IntType:
			return lexical_cast<std::string> (as_int);
				break;
			case FloatType:
			return lexical_cast<std::string> (as_float);
				break;
			case BoolType:
			return as_bool ? "true" : "false";
				break;
			default:
				break;
		}

		return as_string;
	}

	void Variable::fromString(std::string const& string)
	{
		switch (type())
		{
			case IntType:
			set(lexical_cast<Int> (string));
				break;
			case FloatType:
			set(lexical_cast<Real> (string));
				break;
			case BoolType:
			{
				if (string == "true")
				{
					set(true);
				}
				else if (string == "false")
				{
					set(false);
				}
				else
				{
					set(lexical_cast<Bool>(string, false));
				}
			}
				break;
			default:
			set(string);
				break;
		}
	}

	Variable* Variable::instance(std::string const& string)
	{
		Variable* variable = 0;
		if (isInteger(string))
		{
			variable = new Variable(Variable::IntType);
		}
		else if(isReal(string))
		{
			variable = new Variable(Variable::FloatType);
		}
		else if(isBoolean(string))
		{
			variable = new Variable(Variable::BoolType);
		}
		else
		{
			variable = new Variable(Variable::StringType);
		}

		variable->fromString(string);
		return variable;
	}

	void Variable::set(Variable const& value)
	{
		switch (type())
		{
			case IntType:
			{
				switch (value.type())
				{
					case IntType:
					set(value.asInt());
						break;
					case FloatType:
					set(Int(value.asFloat()));
						break;
					case BoolType:
					set(Int(value.asBool()));
						break;
					default:
					set(lexical_cast<Int>(value.asString(), 0));
						break;
				}
			}
				break;

			case FloatType:
			{
				switch (value.type())
				{
					case IntType:
					set(Real(value.asInt()));
						break;
					case FloatType:
					set(value.asFloat());
						break;
					case BoolType:
					set(Real(value.asBool()));
						break;
					default:
					set(lexical_cast<Real> (value.asString(), 0.0f));
						break;
				}

			}
				break;

			case BoolType:
			{
				switch (value.type())
				{
					case IntType:
					set(value.asInt() > 0);
						break;
					case FloatType:
					set(value.asFloat() > 0.0f);
						break;
					case BoolType:
					set(value.asBool());
						break;
					default:
					{
						if (value.asString() == "true")
						{
							set(true);
						}
						else if (value.asString() == "false")
						{
							set(false);
						}
						else
						{
							set(lexical_cast<Bool> (value.asString(), false));
						}
					}
					break;
				}

			}
				break;

			default:
			{
				set(value.asString());

			}
				break;
		}
	}

	void SystemVariable::reset()
	{
		switch (type())
		{
			case IntType:
			set(variable_default.asInt());
				break;
			case FloatType:
			set(variable_default.asFloat());
				break;
			case BoolType:
			set(variable_default.asBool());
				break;
			case StringType:
			set(variable_default.asString());
				break;
			default:
				break;
		}
	}

} //namespace rengine
