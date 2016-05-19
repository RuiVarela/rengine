// __!!rengine_copyright!!__ //

#include <rengine/windowing/Windowing.h>
#include <rengine/system/System.h>
#include <map>

namespace rengine
{
	class EventBinder : public InterfaceEventHandler, public SystemCommand::Handler
	{
	public:
		enum Commands
		{
			ClearBinding		= 0,
			AddBinding			= 1,
			ListBindings		= 2
		};

		typedef Uint InputType;
		typedef Uint EventType;

		typedef std::map<InputType, std::string> InputBindingMap; // input -> command
		typedef std::map<EventType, InputBindingMap> EventMap; // [key, mouse, scroll] -> InputBinding

		EventBinder();

		virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window);
		virtual void operator()(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments);

		void addBinding(EventType const& event, InputType const& input, std::string const& command);
		void clearEvent(EventType const& event);
		void clearBinding(EventType const& event, InputType const& input);

		Bool hasEvent(EventType const& event) const;
		Bool hasBinding(EventType const& event, InputType const& input) const;

		std::string const& getBinding(EventType const& event, InputType const& input) const;

		EventMap const& getEventMap() const;
		InputBindingMap const& getInputBindingMap(EventType const& event) const;

	private:
		EventMap event_map;

		void clearBinding(SystemCommand::Arguments const& arguments);
		void addBinding(SystemCommand::Arguments const& arguments);
		void listBindings(SystemCommand::Arguments const& arguments) const;
	};
} //namespace
