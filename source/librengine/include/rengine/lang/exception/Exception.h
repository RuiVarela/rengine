// __!!rengine_copyright!!__ //

#ifndef __RENGINE_EXCEPTION_H__
#define __RENGINE_EXCEPTION_H__

#include <rengine/lang/Lang.h>

#include <exception>
#include <string>
#include <iosfwd>

namespace rengine
{
	#define MetaExceptionCopy(Class) \
			Class(Class const& exception) { initialize(exception.type(), exception.code(), exception.message(), exception.parent()); } \
			virtual Class* clone() const { return new Class(*this); } \
			Class const& operator=(Class const& exception) { setParent(0); initialize(exception.type(), exception.code(), exception.message(), exception.parent()); return *this; }

	#define MetaExceptionConstructor(Class) \
			Class() { initialize(#Class, code_not_defined, "", 0); } \
			Class(CodeType const code, std::string const& message) { initialize(#Class, code, message, 0); } \
			Class(std::string const& message) { initialize(#Class, code_not_defined, message, 0); } \
			Class(CodeType const code) { initialize(#Class, code, "", 0); } \
			Class(CodeType const code, std::string message, Exception const& parent) { initialize(#Class, code, message, &parent); } \
			Class(std::string message, Exception const& parent) { initialize(#Class, code_not_defined, message, &parent); } \
			Class(CodeType code, Exception const& parent) { initialize(#Class, code, "", &parent); }

	#define MetaException(Class) \
			MetaExceptionConstructor(Class) \
			MetaExceptionCopy(Class)

	class Exception
	{
	public:
		typedef Uint CodeType;
		static Uint const code_not_defined = 9999999;
	
		virtual ~Exception() { setParent(0); }

		MetaExceptionConstructor(Exception);
		MetaExceptionCopy(Exception)


		virtual void serialize(std::ostream& out) const;

		//
		// This will clone parent if parent is not null
		// the current Exception parent will be deleted
		//
		void setParent(Exception const* parent);
		Exception const* parent() const { return parent_; }
		bool hasParent() const { return (parent_ != 0); }

		void setCode(CodeType const code) { code_ = code; }
		CodeType const& code() const { return code_; }

		void setMessage(std::string const& message) { message_ = message; }
		std::string const& message() const { return message_; }

		std::string type() const { return type_; }

	private:	
		CodeType code_;
		std::string message_;
		Exception* parent_;
		std::string type_;
	protected:
		void initialize(std::string const& type_name, CodeType const code, std::string const& message, Exception const* parent);
	};

	RENGINE_INLINE std::ostream& operator<<(std::ostream& out, Exception const& exception)
	{
		exception.serialize(out);
		return out;
	}

} // namespace rengine


#endif // __RENGINE_EXCEPTION_H__
