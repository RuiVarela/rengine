// __!!rengine_copyright!!__ //

#include <rengine/CoreEngine.h>
#include <rengine/event/EventManager.h>
#include <rengine/outputstream/Console.h>

namespace rengine
{
	void EventManager::configure()
	{
		event_binder_ = new EventBinder();

		SharedPointer<SystemCommand> clear_binding = new SystemCommand("bindingClear", EventBinder::ClearBinding, event_binder_.get());
		clear_binding->setDescription("Clears a input binding");
		CoreEngine::instance()->system().registerCommand(clear_binding);

		SharedPointer<SystemCommand> add_binding = new SystemCommand("bindingAdd", EventBinder::AddBinding, event_binder_.get());
		add_binding->setDescription("Adds a input binding");
		CoreEngine::instance()->system().registerCommand(add_binding);

		SharedPointer<SystemCommand> list_bindings = new SystemCommand("bindingList", EventBinder::ListBindings, event_binder_.get());
		list_bindings->setDescription("Lists all input bindings");
		CoreEngine::instance()->system().registerCommand(list_bindings);


		CoreEngine::instance()->eventManager().addGuiEventHandler(event_binder_);
	}

	void EventManager::addGuiEventHandler(SharedPointer<InterfaceEventHandler> const gui_event_handler)
	{
		gui_event_handler_.push_back( gui_event_handler );
	}

	void EventManager::operator()(InterfaceEvent const& interface_event, GraphicsWindow* window)
	{
		if (interface_event.eventType() == InterfaceEvent::EventKeyDown)
		{
			if (interface_event.key() == '\\')
			{
				CoreEngine::instance()->console().toggle();
			}

			if ( CoreEngine::instance()->console().state() == Console::open )
			{
				CoreEngine::instance()->console()(interface_event, window);
			}
			else
			{
				// TODO : Escape quits?
				if (interface_event.keySymbol() == InterfaceEvent::KeyEscape)
				{
					CoreEngine::instance()->requestShutdown();
				}
			}
		}
		else if (interface_event.eventType() == InterfaceEvent::EventWindowClose)
		{
			CoreEngine::instance()->requestShutdown();
		}
		else if (interface_event.eventType() == InterfaceEvent::EventWindowResize)
		{
			/*
			CoreEngine::instance()->log() << "EventWindowResize : Window "
					<< interface_event.windowX() << ' ' << interface_event.windowY() << ' '
					<< interface_event.windowWidth() << ' ' << interface_event.windowHeight() << std::endl;
			*/
		}


		if (CoreEngine::instance()->console().state() == Console::closed)
		{
			for (Uint i = 0; i != numberOfGuiEventHandlers(); ++i )
			{
				gui_event_handler_[i]->operator ()(interface_event, window);
			}
		}
	}

	EventBinder& EventManager::eventBinder()
	{
		return *event_binder_;
	}

	EventBinder const& EventManager::eventBinder() const
	{
		return *event_binder_;
	}

	void EventManager::shutdown()
	{
		event_binder_ = 0;
		gui_event_handler_.clear();
	}

} // namespace rengine
