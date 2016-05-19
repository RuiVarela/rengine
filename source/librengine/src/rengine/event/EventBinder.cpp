// __!!rengine_copyright!!__ //

#include <rengine/event/EventBinder.h>
#include <rengine/CoreEngine.h>
#include <rengine/string/String.h>
#include <rengine/outputstream/OutputStream.h>
#include <rengine/lang/debug/Debug.h>

namespace rengine
{
	EventBinder::EventBinder()
	{
	}

	void EventBinder::operator()(InterfaceEvent const& interface_event, GraphicsWindow* window)
	{
		Bool run_command = false;
		Uint input = 0;

		switch (interface_event.eventType())
		{
			case InterfaceEvent::EventPush:
			case InterfaceEvent::EventRelease:
			{
				run_command = true;
				input = interface_event.mouseButton();
			}
				break;
			case InterfaceEvent::EventScroll:
			{
				run_command = true;
				input = interface_event.scrollingMotion();
			}
				break;
			case InterfaceEvent::EventKeyUp:
			case InterfaceEvent::EventKeyDown:
			{
				run_command = true;
				input = Uint(interface_event.key());

				// enable inputs even when shift is down
				if ((input >= Uint('A')) && (input <= Uint('Z'))) // A -> Z
				{
					input = Uint( toLowercase( Int(input) ) );
				}
			}
				break;
			default:
				break;
		}

		if (run_command)
		{
			EventMap::const_iterator event_found = event_map.find(interface_event.eventType());
			if (event_found != event_map.end())
			{
				InputBindingMap::const_iterator input_found = event_found->second.find(input);

				if (input_found != event_found->second.end())
				{
					CoreEngine::instance()->system()(input_found->second);
				}
			}
		}
	}

	void EventBinder::addBinding(EventType const& event, InputType const& input, std::string const& command)
	{
		if (!hasEvent(event))
		{
			event_map[event] = InputBindingMap();
		}

		event_map[event][input] = command;
	}

	void EventBinder::clearEvent(EventType const& event)
	{
		event_map.erase(event);
	}

	void EventBinder::clearBinding(EventType const& event, InputType const& input)
	{
		if (hasEvent(event))
		{
			event_map[event].erase(input);
		}
	}

	Bool EventBinder::hasEvent(EventType const& event) const
	{
		return (event_map.find(event) != event_map.end());
	}

	Bool EventBinder::hasBinding(EventType const& event, InputType const& input) const
	{
		if (hasEvent(event))
		{
			InputBindingMap const& binding = event_map.find(event)->second;
			return (binding.find(input) != binding.end());
		}
		else
		{
			return false;
		}
	}

	std::string const& EventBinder::getBinding(EventType const& event, InputType const& input) const
	{
		RENGINE_ASSERT(hasBinding(event, input));
		return event_map.find(event)->second.find(input)->second;
	}

	EventBinder::EventMap const& EventBinder::getEventMap() const
	{
		return event_map;
	}

	EventBinder::InputBindingMap const& EventBinder::getInputBindingMap(EventType const& event) const
	{
		RENGINE_ASSERT(hasEvent(event));
		return event_map.find(event)->second;
	}
	//
	// System command handling
	//
	void EventBinder::operator()(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
	{
		switch (command)
		{
			case ClearBinding:
			clearBinding(arguments);
				break;
			case AddBinding:
			addBinding(arguments);
				break;
			case ListBindings:
			listBindings(arguments);
				break;
			default:
				break;
		}
	}

	void EventBinder::clearBinding(SystemCommand::Arguments const& arguments)
	{
		Bool show_syntax = false;
		std::string syntax_error;

		if (arguments.size() != 2)
		{
			syntax_error = "Invalid number of parameters";
			show_syntax = true;
		}

		// check event type
		std::string event_type;
		if (!show_syntax)
		{
			event_type = arguments[0]->asString();

			if ((event_type != "Push") && (event_type != "Release") && (event_type != "Scroll"))
			{
				show_syntax = true;
				syntax_error = "Invalid event type";
			}
		}

		//check input
		std::string input_string;
		Uint input = 0;
		Uint event = 0;
		if (!show_syntax)
		{
			input_string = arguments[1]->toString();

			if (event_type == "Scroll")
			{
				event = InterfaceEvent::EventScroll;
				if (!InterfaceEvent::scrollFromName(input_string, input))
				{
					show_syntax = true;
					syntax_error = "Invalid scroll type";
				}
			}
			else
			{
				if (InterfaceEvent::mouseButtonFromName(input_string, input))
				{
					if (event_type == "Push")
					{
						event = InterfaceEvent::EventPush;
					}
					else if (event_type == "Release")
					{
						event = InterfaceEvent::EventRelease;
					}
				}
				else
				{
					if (event_type == "Push")
					{
						event = InterfaceEvent::EventKeyDown;

					}
					else if (event_type == "Release")
					{
						event = InterfaceEvent::EventKeyUp;
					}

					if (!InterfaceEvent::keySymbolFromName(input_string, input))
					{
						show_syntax = true;
						syntax_error = "Invalid input";
					}
					else
					{
						if (input_string.size() == 1)
						{
							input = Uint( toLowercase( Int(input) ) );
						}
					}
				}
			}
		}


		if (show_syntax)
		{
			//CoreEngine::instance()->system().echo(arguments);
			CoreEngine::instance()->log() << "Bad Syntax : " << syntax_error << std::endl;
			CoreEngine::instance()->log() << "Arguments : <event> <input>" << std::endl;
			CoreEngine::instance()->log() << "<event> allowed values : Push, Release, Scroll" << std::endl;
			CoreEngine::instance()->log() << "Example: Push MouseLeftButton" << std::endl;
		}
		else
		{
			clearBinding(event, input);

			InterfaceEvent::eventName(event, event_type);

			CoreEngine::instance()->log() << "Input binding cleared : "
										  << event_type << ' '
										  << input_string << std::endl;
		}
	}

	void EventBinder::addBinding(SystemCommand::Arguments const& arguments)
	{
		Bool show_syntax = false;
		std::string syntax_error;

		if (arguments.size() < 3)
		{
			syntax_error = "Invalid number of parameters";
			show_syntax = true;
		}

		// check event type
		std::string event_type;
		if (!show_syntax)
		{
			event_type = arguments[0]->asString();

			if ((event_type != "Push") && (event_type != "Release") && (event_type != "Scroll"))
			{
				show_syntax = true;
				syntax_error = "Invalid event type";
			}
		}

		//check input
		std::string input_string;
		Uint input = 0;
		Uint event = 0;
		if (!show_syntax)
		{
			input_string = arguments[1]->toString();

			if (event_type == "Scroll")
			{
				event = InterfaceEvent::EventScroll;
				if (!InterfaceEvent::scrollFromName(input_string, input))
				{
					show_syntax = true;
					syntax_error = "Invalid scroll type";
				}
			}
			else
			{
				if (InterfaceEvent::mouseButtonFromName(input_string, input))
				{
					if (event_type == "Push")
					{
						event = InterfaceEvent::EventPush;
					}
					else if (event_type == "Release")
					{
						event = InterfaceEvent::EventRelease;
					}
				}
				else
				{
					if (event_type == "Push")
					{
						event = InterfaceEvent::EventKeyDown;

					}
					else if (event_type == "Release")
					{
						event = InterfaceEvent::EventKeyUp;
					}

					if (!InterfaceEvent::keySymbolFromName(input_string, input))
					{
						show_syntax = true;
						syntax_error = "Invalid input";
					}
					else
					{
						if (input_string.size() == 1)
						{
							input = Uint( toLowercase( Int(input) ) );
						}
					}
				}
			}
		}

		std::string command;
		if (!show_syntax)
		{
			for (SystemCommand::Arguments::size_type i = 2; i != arguments.size(); ++i)
			{
				command += " " + arguments[i]->toString();
			}
			trim(command);
		}

		if (show_syntax)
		{
			//CoreEngine::instance()->system().echo(arguments);
			CoreEngine::instance()->log() << "Bad Syntax : " << syntax_error << std::endl;
			CoreEngine::instance()->log() << "Arguments : <event> <input> <command>" << std::endl;
			CoreEngine::instance()->log() << "<event> allowed values : Push, Release, Scroll" << std::endl;
			CoreEngine::instance()->log() << "Example: Push MouseLeftButton echo hello world!" << std::endl;
		}
		else
		{
			addBinding(event, input, command);

			InterfaceEvent::eventName(event, event_type);

			CoreEngine::instance()->log() << "Input Binded : "
										  << event_type << ' '
										  << input_string << ' '
										  << command << std::endl;
		}
	}

	void EventBinder::listBindings(SystemCommand::Arguments const& arguments) const
	{
		CoreEngine::instance()->log() << "Input bindings:" << std::endl;

		for (EventMap::const_iterator event_iterator = event_map.begin();
			 event_iterator != event_map.end();
			 ++event_iterator)
		{
			std::string event_name;
			InterfaceEvent::eventName(event_iterator->first, event_name);

			CoreEngine::instance()->log() << "<> "<< event_name << std::endl;

			for (InputBindingMap::const_iterator binding_iterator = event_iterator->second.begin();
				 binding_iterator != event_iterator->second.end();
				 ++binding_iterator)
			{
				std::string input_name;

				switch (event_iterator->first)
				{
					case InterfaceEvent::EventPush:
					case InterfaceEvent::EventRelease:
					InterfaceEvent::mouseButtonName(binding_iterator->first, input_name);
						break;
					case InterfaceEvent::EventScroll:
					InterfaceEvent::scrollName(binding_iterator->first, input_name);
						break;
					case InterfaceEvent::EventKeyUp:
					case InterfaceEvent::EventKeyDown:
					InterfaceEvent::keySymbolName(binding_iterator->first, input_name);
						break;
					default:
					input_name = "Invalid Event";
						break;
				}

				CoreEngine::instance()->log() << "[" << input_name << "] " << binding_iterator->second << std::endl;
			}
		}
	}

} //namespace

