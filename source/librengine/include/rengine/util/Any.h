// __!!rengine_copyright!!__ //

#ifndef __RENGINE_ANY__
#define __RENGINE_ANY__

#include <rengine/lang/Lang.h>

#include <typeinfo>
#include <string>

/*
 * This class is based on
 * "Valued Conversions" by Kevlin Henney, C++ Report 12(7), July/August 2000
 * http://www.two-sdg.demon.co.uk/curbralan/papers/ValuedConversions.pdf
 */

namespace rengine
{
	class Any
	{
	public:
		Any() :
			variable(0)
		{
		}

		Any(Any const& copy) :
			variable(0)
		{
			if (copy.variable)
			{
				variable = copy.variable->clone();
			}
		}

		/*
		 * C-strings are converted to std::string
		 */
		template <Int N>
		Any(Char const (&value)[N]) :
			variable( new Holder<std::string>( std::string(value) ) )
		{
		}

		template<typename ValueType>
		Any(ValueType const& value) :
			variable( new Holder<ValueType>(value) )
		{
		}

		Any &operator=(Any const& rhs)
		{
			Any swap_any(rhs);
		    return swap(swap_any);
		}

		template<typename ValueType>
		Any &operator=(ValueType const& value)
		{
			Any swap_any(value);
			return swap(swap_any);
		}

		Any& swap(Any& rhs)
		{
			Placeholder *swap_variable = rhs.variable;
			rhs.variable = variable;
			variable = swap_variable;
			return *this;
		}

		template<typename ValueType>
		ValueType const* to_ptr() const
		{
		//	return (sameType<ValueType>() ? &static_cast<Holder<ValueType>*> (variable)->value : 0);

			if (sameType<ValueType>())
			{
				Holder<ValueType>* holder = dynamic_cast< Holder<ValueType> *>(variable);
				if (holder)
				{
					return &(holder->value);
				}
			}

			return 0;
		}

		virtual std::type_info const& type_info() const
		{
			return (variable ? variable->type_info() : typeid(void));
		}

		template<typename ValueType>
		Bool sameType() const
		{
			return (type_info() == typeid(ValueType));
		}

		Bool empty() const
		{
			return !variable;
		}

		~Any()
		{
			delete(variable);
		}
	private:

		class Placeholder
		{
		public:
			virtual ~Placeholder()
			{
			}
			virtual std::type_info const& type_info() const = 0;
			virtual Placeholder *clone() const = 0;
		};

		template <typename ValueType>
		class Holder : public Placeholder
		{
		public:
			Holder(ValueType const& hold_value) :
				value(hold_value)
			{
			}

			virtual std::type_info const& type_info() const
			{
				return typeid(value);
			}

			virtual Holder *clone() const
			{
				return new Holder(value);
			}

			ValueType const value;
		};

		Placeholder *variable;
	};

	template<typename ValueType>
	ValueType any_cast(Any const& operand)
	{
		ValueType const* value_ptr = operand.to_ptr<ValueType>();
		if (!value_ptr)
		{
			throw std::bad_cast();
		}
		ValueType copy(*value_ptr);
		return copy;
	}
}

#endif //__RENGINE_ANY__
