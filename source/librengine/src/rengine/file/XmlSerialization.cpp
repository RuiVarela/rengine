// __!!rengine_copyright!!__ //

#include <rengine/file/XmlSerialization.h>
#include <rengine/file/tinyxml.h>

#include <vector>

namespace rengine
{
	typedef std::pair<TiXmlElement*, Uint> XmlElementPair;
	struct XmlArchiveData
	{
		XmlArchiveData()
		{
		}
		~XmlArchiveData()
		{
		}

		TiXmlDocument document;
		std::vector< XmlElementPair> element_stack;
	};

	XmlArchive::XmlArchive() :
		data_(new XmlArchiveData()), mode_(None)
	{
	}

	XmlArchive::~XmlArchive()
	{
		delete (data_);
	}

	Bool XmlArchive::open(std::string const& filename, XmlArchive::Mode mode)
	{
		Bool state = false;
		data_->document.Clear();
		data_->element_stack.clear();

		if (mode == Read)
		{
			mode_ = Read;
			filename_ = filename;

			if (data_->document.LoadFile(filename_))
			{
				TiXmlNode* node = data_->document.FirstChild("root");
				if (node)
				{
					TiXmlElement* current_element = node->ToElement();
					if (current_element)
					{
						data_->element_stack.push_back(XmlElementPair(current_element, 0));
						state = true;
					}
				}
			}

		}
		else if (mode == Write)
		{
			mode_ = Write;
			filename_ = filename;

			TiXmlDeclaration *declaration = new TiXmlDeclaration("1.0", "UTF-8", "yes");
			data_->document.LinkEndChild(declaration);

			TiXmlElement *current_element = new TiXmlElement("root");
			data_->document.LinkEndChild(current_element);
			data_->element_stack.push_back(XmlElementPair(current_element, 0));
			state = true;
		}
		else
		{
			state = false;
		}

		return state;
	}

	void XmlArchive::beginNode(std::string const& node_name)
	{
		if (mode_ == Write)
		{
			TiXmlElement *current_element = new TiXmlElement(node_name);
			data_->element_stack.back().first->LinkEndChild(current_element);
			data_->element_stack.push_back(XmlElementPair(current_element, 0));
		}
		else if (mode_ == Read)
		{
			if (node_name != "")
			{
				TiXmlElement *current_element = data_->element_stack.back().first->FirstChildElement(node_name);
				if (!current_element)
				{
					throw XmlArchiveException(0001, "XmlElement with name " + node_name + " not found.");
				}
				data_->element_stack.push_back(XmlElementPair(current_element, 0));
			}
			else
			{
				Uint count = data_->element_stack.back().second;
				data_->element_stack.back().second++;

				// goto element number .. "count"
				TiXmlNode* node = data_->element_stack.back().first->FirstChild();
				if (node)
				{
					TiXmlElement *current_element = 0;
					for (current_element = node->ToElement(); (current_element && (count != 0)); current_element = current_element->NextSiblingElement())
					{
						count--;
					}

					if (current_element)
					{
						data_->element_stack.push_back(XmlElementPair(current_element, 0));
					}
				}
			}
		}
	}

	void XmlArchive::endNode()
	{
		data_->element_stack.pop_back();
	}

	Uint XmlArchive::nodeElementCount() const
	{
		Uint element_count = 0;

		TiXmlNode* node = data_->element_stack.back().first->FirstChild();
		if (node)
		{
			for (TiXmlElement *current_element = node->ToElement(); current_element; current_element = current_element->NextSiblingElement())
			{
				element_count++;
			}
		}

		return element_count;
	}

	void XmlArchive::writeText(std::string const& text)
	{
		TiXmlText *xml_text = new TiXmlText(text);
		data_->element_stack.back().first->LinkEndChild(xml_text);
	}

	std::string XmlArchive::readText()
	{
		if (data_->element_stack.back().first->GetText())
		{
			return data_->element_stack.back().first->GetText();
		}
		else
		{
			return std::string();
		}
	}

	Bool XmlArchive::save()
	{
		Bool state = false;

		if (mode_ == Write)
		{
			state = data_->document.SaveFile(filename_);
		}

		return state;
	}

	void serialization::serialize(XmlArchive& archive, std::string& value)
	{
		if (archive.mode() == XmlArchive::Write)
		{
			archive.writeText(value);
		}
		else if (archive.mode() == XmlArchive::Read)
		{
			value = archive.readText();
		}
	}

	template<typename T>
	void doSerialization(XmlArchive& archive, T& value)
	{
		if (archive.mode() == XmlArchive::Write)
		{
			std::string as_text = xml_lexical_cast<std::string> (value);
			serialization::serialize(archive, as_text);
		}
		else if (archive.mode() == XmlArchive::Read)
		{
			std::string as_text;
			serialization::serialize(archive, as_text);
			value = xml_lexical_cast<T> (as_text);
		}
	}
	template<typename T, typename Y>
	void doSerializationWithConversion(XmlArchive& archive, T& value)
	{
		Y temporary = Y(value);
		serialization::serialize(archive, temporary);
		value = T(temporary);
	}

	void serialization::serialize(XmlArchive& archive, int& value)
	{
		doSerialization(archive, value);
	}

	void serialization::serialize(XmlArchive& archive, Uint& value)
	{
		doSerialization(archive, value);
	}

	void serialization::serialize(XmlArchive& archive, Char& value)
	{
		doSerializationWithConversion<Char, Uint> (archive, value);
	}

	void serialization::serialize(XmlArchive& archive, Uchar& value)
	{
		doSerializationWithConversion<Uchar, Uint> (archive, value);
	}

	void serialization::serialize(XmlArchive& archive, Real64& value)
	{
		doSerialization(archive, value);
	}

	void serialization::serialize(XmlArchive& archive, Real& value)
	{
		doSerialization(archive, value);
	}

	void serialization::serialize(XmlArchive& archive, Bool& value)
	{
		doSerialization(archive, value);
	}
}
