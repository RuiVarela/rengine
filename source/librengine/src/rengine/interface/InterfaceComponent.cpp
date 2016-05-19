// __!!rengine_copyright!!__ //

#include <rengine/interface/InterfaceComponent.h>

namespace rengine
{

	InterfaceComponent::InterfaceComponent() 
	{
	}

	InterfaceComponent::~InterfaceComponent()
	{
	}

	void InterfaceComponent::setMessage(String const& message) 
	{
		_text.setText(message); 
	}

	String const& InterfaceComponent::getMessage() const
	{ 
		return _text.getText(); 
	};

	Text& InterfaceComponent::getText()  
	{ 
		return _text; 
	}

	Text const& InterfaceComponent::getText() const 
	{
		return _text;
	}

	void InterfaceComponent::setWidth(Vector2D const& width)
	{ 
		_width = width;
	}

	Vector2D const& InterfaceComponent::getWidth() const
	{ 
		return _width; 
	}

	void InterfaceComponent::setPosition(Vector3D const& position)
	{ 
		_position = position; 
	}

	Vector3D const& InterfaceComponent::getPosition() const
	{ 
		return _position;
	}

	void InterfaceComponent::setColor(Vector4D const& color) 
	{ 
		_color = color;
	}

	Vector4D const& InterfaceComponent::getColor() const
	{ 
		return _color;
	}
}