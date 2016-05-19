// __!!rengine_copyright!!__ //

#ifndef __RENGINE_INTERFACE_COMPONENT_H__
#define __RENGINE_INTERFACE_COMPONENT_H__

#include <rengine/geometry/BaseShapes.h>
#include <rengine/text/Text.h>

namespace rengine
{
    class InterfaceComponent : public Quadrilateral
    {
        public:
			InterfaceComponent();
            virtual ~InterfaceComponent();

			void setMessage(String const& message);
			String const& getMessage() const;

			Text& getText();
			Text const& getText() const;

			void setWidth(Vector2D const& width);
			Vector2D const& getWidth() const;

			void setPosition(Vector3D const& position);
			Vector3D const& getPosition() const;

			void setColor(Vector4D const& color);
			Vector4D const& getColor() const;
        private:
			Vector2D _width;
			Vector3D _position;
			Vector4D _color;
			Text _text;
    };

}// end of namespace

#endif // __RENGINE_INTERFACE_COMPONENT_H__
