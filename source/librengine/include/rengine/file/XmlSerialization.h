// __!!rengine_copyright!!__ //

#ifndef __XML_XML_SERIALIZATION__
#define __XML_XML_SERIALIZATION__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <sstream>
#include <limits>

#include <rengine/lang/Lang.h>
#include <rengine/lang/exception/Exception.h>
#include <rengine/lang/Traits.h>

#define XML_SERIALIZE(archive, variable) rengine::serialize(archive, #variable, variable)

namespace rengine
{
	struct XmlArchiveData;

	class XmlArchive
	{
	public:
		enum Mode
		{
			Read,
			Write,
			None
		};

		XmlArchive();
		~XmlArchive();

		Bool open(std::string const& filename, Mode mode = Read);
		Bool save();

		Mode mode() const { return mode_; }

		//
		// main archive handling methods.
		// Should not be used directly.
		//

		//
		// beginNode
		// On Write : creates a node with "name".
		// On Read : reads the node with "name".
		// If no name specified it reads the "current sibling",
		// "current sibling" is advanced on each call to the funcion with no name.
		//
		void beginNode(std::string const& node_name = "");
		void endNode();
		std::string readText();
		void writeText(std::string const& text);
		Uint nodeElementCount() const;
	private:
		XmlArchiveData *data_;
		Mode mode_;
		std::string filename_;
	};

	//
	// Lexical cast for type conversion
	//
	template<typename OutputType, typename InputType>
	OutputType xml_lexical_cast(InputType const& value)
	{
		std::stringstream stream;

		if (std::numeric_limits<InputType>::is_specialized)
		{
			stream.precision(std::numeric_limits<InputType>::digits10 + 1);
		}

		stream << value;
		OutputType output;
		stream >> output;
		return output;
	}

	// forward declaration of serialization hook
	template<typename T> void serialize(XmlArchive& archive, std::string const name, T& value);

	//
	// Serialization for the default types
	//	Uint, Int, Real, Real64, Bool
	//	std::string, std::vector, std::list, std::map
	//
	namespace serialization
	{
		// declarations
		void serialize(XmlArchive& archive, std::string& value);
		void serialize(XmlArchive& archive, Int& value);
		void serialize(XmlArchive& archive, Uint& value);
		void serialize(XmlArchive& archive, Char& value);
		void serialize(XmlArchive& archive, Uchar& value);
		void serialize(XmlArchive& archive, Real64& value);
		void serialize(XmlArchive& archive, Real& value);
		void serialize(XmlArchive& archive, Bool& value);

		template<typename T> void serialize(XmlArchive& archive, T& object);
		template<typename T> void serialize(XmlArchive& archive, T* object);
		template<typename T> void serialize(XmlArchive& archive, std::vector<T>& value);
		template<typename T> void serialize(XmlArchive& archive, std::list<T>& value);
		template<typename KT, typename VT> void serialize(XmlArchive& archive, std::map<KT, VT>& value);

		//
		// Generic serialization. For implementing serialization inside the class.
		//
		template<typename T>
		void serialize(XmlArchive& archive, T& object)
		{
			object.serialize(archive);
		}

		template<typename T>
		void serialize(XmlArchive& archive, T* object)
		{
			object->serialize(archive);
		}

		//
		// Container serialization
		//
		template<typename T>
		void serialize(XmlArchive& archive, std::vector<T>& value)
		{
			if (archive.mode() == XmlArchive::Write)
			{
				for (typename std::vector< T >::size_type iterator = 0; iterator != value.size(); ++iterator)
				{
					rengine::serialize(archive, "element", value[iterator]);
				}
			}
			else if (archive.mode() == XmlArchive::Read)
			{
				Uint size = archive.nodeElementCount();
				value.resize(size);
				for (Uint iterator = 0; iterator != size; ++iterator)
				{
					archive.beginNode();
					rengine::serialization::serialize(archive, value[iterator]);
					archive.endNode();
				}
			}
		}

		template<typename T>
		void serialize(XmlArchive& archive, std::list<T>& value)
		{
			if (archive.mode() == XmlArchive::Write)
			{
				for (typename std::list< T >::iterator iterator = value.begin(); iterator != value.end(); ++iterator)
				{
					rengine::serialize(archive, "element", *iterator);
				}
			}
			else if (archive.mode() == XmlArchive::Read)
			{
				Uint size = archive.nodeElementCount();
				for (Uint iterator = 0; iterator != size; ++iterator)
				{
					value.push_back(T());
					archive.beginNode();
					rengine::serialization::serialize(archive, value.back());
					archive.endNode();
				}
			}
		}

		template<typename KT, typename VT>
		void serialize(XmlArchive& archive, std::map<KT, VT>& value)
		{
			if (archive.mode() == XmlArchive::Write)
			{
				for (typename std::map<KT, VT>::iterator iterator = value.begin(); iterator != value.end(); ++iterator)
				{
					archive.beginNode("element");
					archive.beginNode("key");
					typename RemoveConst<KT>::Type key_element = iterator->first;
					rengine::serialization::serialize(archive, key_element);
					archive.endNode();

					archive.beginNode("value");
					rengine::serialization::serialize(archive, iterator->second);
					archive.endNode();
					archive.endNode();

				}
			}
			else if (archive.mode() == XmlArchive::Read)
			{
				Uint size = archive.nodeElementCount();
				for (Uint iterator = 0; iterator != size; ++iterator)
				{
					archive.beginNode();
					archive.beginNode();
					KT map_key;
					rengine::serialization::serialize(archive, map_key);
					archive.endNode();

					archive.beginNode();
					VT map_value;
					rengine::serialization::serialize(archive, map_value);
					archive.endNode();
					archive.endNode();

					value[map_key] = map_value;
				}
			}
		}

	}

	//
	// Serialization hook
	// This is called to automatically begin and end a node
	//
	template<typename T>
	void serialize(XmlArchive& archive, std::string const name, T& value)
	{
		archive.beginNode(name);
		rengine::serialization::serialize(archive, value);
		archive.endNode();
	}

	//
	// Exceptions
	//
	class XmlArchiveException : public Exception
	{
	public:
		MetaException(XmlArchiveException);

		virtual ~XmlArchiveException() {}
	};
}

#endif //__XML_XML_SERIALIZATION__
