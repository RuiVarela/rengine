// __!!rengine_copyright!!__ //

#ifndef __RENGINE_EVENTMANAGER_H__
#define __RENGINE_EVENTMANAGER_H__

#include <rengine/event/EventBinder.h>
#include <vector>

namespace rengine
{
	class EventManager : public InterfaceEventHandler
	{
	public:
		EventManager() {};
		~EventManager() {};

		Uint numberOfGuiEventHandlers() const { return gui_event_handler_.size(); }

		void addGuiEventHandler(SharedPointer<InterfaceEventHandler> const input_handler);

		virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window);

		void configure();

		EventBinder& eventBinder();
		EventBinder const& eventBinder() const;

		void shutdown();
	private:
		std::vector< SharedPointer<InterfaceEventHandler> > gui_event_handler_;
		SharedPointer<EventBinder> event_binder_;
	};

} // end of namespace

#endif // __RENGINE_EVENTMANAGER_H__
