// __!!rengine_copyright!!__ //

#include <rengine/lang/exception/Exception.h>

#include <ostream>

namespace rengine
{

	void Exception::initialize(std::string const& type_name, CodeType const code, std::string const& message, Exception const* parent)
	{
		type_ = type_name;
		code_ = code;
		message_ = message;
		parent_ = 0;
		setParent(parent);
	}

	void Exception::serialize(std::ostream& out) const
	{
		out << type() << " : ";

		if (code() != code_not_defined)
		{
			out << "[" << code() << "] ";
		}

		if (message() != "")
		{
			out << message();
		}

		if (hasParent())
		{
			out << std::endl;
			parent_->serialize(out);
		}
	}

	void Exception::setParent(Exception const* parent)
	{
		if (parent_)
		{
			delete (parent_);
		}

		parent_ = 0;

		if (parent)
		{
			parent_ = parent->clone();
		}
	}

} //namespace rengine

