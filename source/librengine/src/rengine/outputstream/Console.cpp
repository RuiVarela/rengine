// __!!rengine_copyright!!__ //

#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>
#include <rengine/outputstream/Console.h>
#include <rengine/system/SystemFeed.h>
#include <rengine/lang/exception/BaseExceptions.h>
#include <rengine/time/Timer.h>
#include <rengine/math/Math.h>

#include <rengine/string/String.h>
#include <rengine/string/CStringTokenizer.h>

#include <rengine/resource/ResourceManager.h>
#include <rengine/geometry/BaseShapes.h>
#include <rengine/state/BaseStates.h>
#include <rengine/state/Program.h>

#include <cstring>

using namespace std;

namespace rengine
{
	Console::Console() :
		lines(0),
		input_buffer(0),
		current_state(uninitialized),
		toggling_speed(3.0f),
		command_handler(0)
	{
		char_height = 0.0f;
		char_width = 0.0f;
	}

	Console::~Console()
	{
		shutdown();
	}

	void Console::shutdown()
	{
		if (lines != 0)
		{
			delete[] lines;
			delete[] input_buffer;
			delete[] commands;
			current_state = uninitialized;

			lines = 0;
			input_buffer = 0;
			commands = 0;

			hud_writer = 0;
			background = 0;
		}
	}

	void Console::initialize(const Int lines_number, std::string const &background_filename, std::string const &floating_background_filename, std::string const& effect)
	{
		try
		{
			shutdown();

			OpaqueProperties options;

			SharedPointer<Texture2D> background_texture = CoreEngine::instance()->resourceManager().load<Texture2D>(background_filename);
			if (!background_texture)
			{
				throw GraphicsException(302, "Empty Background Image");
			}

			SharedPointer<Texture2D> floating_background_texture = CoreEngine::instance()->resourceManager().load<Texture2D>(floating_background_filename);
			if (!floating_background_texture)
			{
				throw GraphicsException(303, "Empty Floating Background Image");
			}

			background = new Quadrilateral();
			background->setDrawMode(Drawable::DynamicDraw);
			background->states()->setTexture(background_texture);
			background->states()->setTexture(floating_background_texture);

			SharedPointer<Program> program = CoreEngine::instance()->resourceManager().load<Program>(effect);
			if (!program)
			{
				throw GraphicsException(306, "Unable to load Console effect: " + effect);
			}

			background->states()->setProgram(program);

			//create the draw states
			states();

			if (lines_number <= 0)
				throw GraphicsException(304, "Invalid Lines Number");

			hud_writer = new HudWriter();
			lines_n = lines_number;

			Font::Glyph* glyph = hud_writer->getFont()->glyph('x');
			if (glyph == 0)
			{
				throw GraphicsException(305, "Invalid Font For HudWriter");
			}

			Real width = Real(CoreEngine::instance()->mainWindow()->contextOptions().width);
			Real height = Real(CoreEngine::instance()->mainWindow()->contextOptions().height);

			char_height = glyph->dimension().y();
			char_width = glyph->dimension().x();

			visible_lines = Int( (height * 0.5f) / char_height );
			line_char_n = Int( width / char_width );
			window_height = height;
			window_width = width;

			lines = new Char[lines_n * (line_char_n + 1)];
			input_buffer = new Char[line_char_n + 1]; // the last byte will always be \0

			number_of_commands = Int(lines_n * 0.25f);
			commands = new Char[number_of_commands * (line_char_n + 1)];
			visible_command = 0;
			current_command = 0;

			for (Int i = 0; i != number_of_commands; ++i)
			{
				// initialize the commands
				commands[i * (line_char_n + 1)] = '\0';
			}

			clearMessageBuffer();
			clearInputBuffer();

			input_buffer[line_char_n] = '\0'; // the last byte will always be \0

			current_state = closed;

			current_height = (Real) height;

			show_carret = true;
		}
		catch (std::bad_alloc bad_allocation)
		{
			throw GraphicsException(301, "Error Allocating memory - Console initialization");
		}
	}

	void Console::operator()(rengine::InterfaceEvent const& interface_event, rengine::GraphicsWindow* window)
	{
		switch (interface_event.keySymbol())
		{
			case rengine::InterfaceEvent::KeyRight:
			moveRight();
				break;
			case rengine::InterfaceEvent::KeyLeft:
			moveLeft();
				break;
			case rengine::InterfaceEvent::KeyEnd:
			visible_index = current_message;
				break;
			case rengine::InterfaceEvent::KeyPageUp:
			{
				if (current_message != ((lines_n + visible_index - visible_lines) % (lines_n)))
				{
					Bool overlap = (visible_index > current_message) && (((lines_n + visible_index - visible_lines) % (lines_n)) < current_message);
					if (overlap)
						visible_index = ((current_message + visible_lines) % lines_n);
					else
						visible_index = ((lines_n + visible_index - visible_lines) % (lines_n));
				}
			}
				break;
			case rengine::InterfaceEvent::KeyPageDown:
			{
				if (current_message != visible_index)
				{
					Bool overlap = (visible_index < current_message) && (((visible_index + visible_lines) % (lines_n)) > current_message);

					if (overlap)
						visible_index = current_message;
					else
						visible_index = ((visible_index + visible_lines) % (lines_n));
				}
			}
				break;
			case rengine::InterfaceEvent::KeyUp:
			getPrvCommand();
				break;
			case rengine::InterfaceEvent::KeyDown:
			getNextCommand();
				break;
			case rengine::InterfaceEvent::KeyDelete:
			removeChar();
				break;
			case rengine::InterfaceEvent::KeyBackSpace:
			removePrevChar();
				break;
			case rengine::InterfaceEvent::KeyReturn:
			execute();
				break;

			case rengine::InterfaceEvent::KeyShiftLeft:
			case rengine::InterfaceEvent::KeyShiftRight:
			case rengine::InterfaceEvent::KeyControlLeft:
			case rengine::InterfaceEvent::KeyControlRight:
				break;

			default:
			setChar(Char(interface_event.key()));
				break;
		}
	}

	void Console::setChar(const Char ch)
	{
		if (input_buffer[current_char] == '\0')
		input_buffer[current_char + 1] = '\0';

		input_buffer[current_char] = ch;

		if (current_char != (line_char_n - 1))
		moveRight();
	}

	void Console::removeChar()
	{
		if (input_buffer[current_char] != '\0')
		strcpy(&input_buffer[current_char], &input_buffer[current_char + 1]);
	}

	void Console::removePrevChar()
	{
		if(current_char != 0)
		moveLeft();

		removeChar();
	}

	void Console::addMessage(Char *const message)
	{
		std::string message_as_string(message);
		std::string::size_type new_line_position = message_as_string.rfind('\n');

		if (new_line_position == std::string::npos)
		{
			strncpy(&lines[current_message * (line_char_n + 1)], message, line_char_n);

			visible_index = nextMesssage();
			moveToNextMessage();

			if (Int(strlen(message))> line_char_n)
			addMessage(&message[line_char_n]);
		}
		else
		{
			std::string first = message_as_string.substr(0, new_line_position);
			addMessage(first);

			if (message_as_string.length() - new_line_position> 1)
			{
				std::string second = message_as_string.substr(new_line_position + 1, message_as_string.length() - new_line_position);
				addMessage(second);
			}
		}
	}

	void Console::addMessage(std::string const &message)
	{
		Char *c_str = new Char[message.size() + 1];
		strcpy(c_str, message.c_str());

		addMessage(c_str);
		delete[] c_str;
	}

	void Console::toggle()
	{
		switch (current_state)
		{
			case open:
			case opening:
			current_state = closing;
				break;

			case closed:
			case closing:
			case initialized:
			current_state = opening;
				break;

			default:
			throw GraphicsException(301, "Ilegal Console State : " + lexical_cast<string> (current_state));
				break;
		}
	}

	void Console::execute()
	{
		addCommand();

		if(command_handler)
		{
			addMessage(std::string("-> ") + input_buffer);
			(*command_handler)(std::string(input_buffer));
		}

		clearInputBuffer();
	}

	void Console::clearMessageBuffer()
	{
		for (Int i = 0; i != lines_n; ++i ) // initialize the messages
		lines[i * (line_char_n + 1)] = '\0';

		visible_index = current_message = 0;
	}

	void Console::prepareDrawing(RenderEngine& render_engine)
	{

	}

	void Console::unprepareDrawing(RenderEngine& render_engine)
	{

	}

	void Console::updateUniforms(RenderEngine& render_engine)
	{
		Real total_time = (Real) CoreEngine::instance()->timer().elapsedTime();

		Real64 delta_time = CoreEngine::instance()->frameDeltaTime();
		if (rengine::equivalent(delta_time, 0.0))
		delta_time = 1-6;

		if (rengine::equivalent(total_time, 0.0f))
		total_time = 1-6;

		if (current_state >= open)
		{
			Font::Glyph* glyph = hud_writer->getFont()->glyph('_');
			Real y_advance = 3.0f;

			if (glyph)
			{
				y_advance += Real(glyph->dimension().y()) - glyph->bearingY();
			}

			background->setCornersVertex(Vector3D(0.0f, current_height - y_advance, 0.0f),
										 Vector3D(window_width, current_height + window_height - y_advance, 0.0f));

			background->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));


			if (background->states()->hasState(DrawStates::Program))
			{
				Matrix projection = Matrix::ortho2D(0, Real(CoreEngine::instance()->mainWindow()->contextOptions().width),
											        0, Real(CoreEngine::instance()->mainWindow()->contextOptions().height));

				Program* program = dynamic_cast<ProgramUnit*>( background->states()->getState(DrawStates::Program).first.get() )->get().get();
				program->uniform("projection").set(projection);
				program->uniform("background").set(0);
				program->uniform("floating_background").set(1);
				program->uniform("time").set(total_time);
			}

			show_carret = (((Uint) (total_time * 2.0f) % 2) == 0);

			if (current_state == opening)
			{
				current_height -= (Real) window_height * Real(delta_time) * toggling_speed;
				if ((current_height - 4) <= window_height * 0.5)
				{
					current_height = window_height * 0.5f - 4;
					current_state = open;
				}
			}
			else if(current_state == closing)
			{
				current_height += (Real) window_height * Real(delta_time) * toggling_speed;
				if (current_height >= window_height)
				{
					current_height = (Real) window_height;
					current_state = closed;
				}
			}
		}
	}

	void Console::draw(RenderEngine& render_engine)
	{
		if (current_state >= open)
		{
			render_engine.draw(*background);

			Int current_vi = lines_n + visible_index;
			Int line_char_n_ = line_char_n + 1;

			hud_writer->clear();

			for (Int i = 1; i != (visible_lines + 1); ++i)
			{
				hud_writer->write(Vector2D(0.0f, current_height + Real(i * char_height)), String(&lines[((current_vi - i) % (lines_n)) * (line_char_n_)]));
			}

			hud_writer->write(Vector2D(0.0f, current_height), std::string(input_buffer));
			if (show_carret)
			{
				hud_writer->write(Vector2D(Real(current_char * char_width), current_height), "_");
			}
			render_engine.draw(*hud_writer);
		}
	}

	void Console::getPrvCommand()
	{
		Int prev = ((number_of_commands + visible_command - 1) % (number_of_commands));

		if ((prev != current_command) && (commands[prev * (line_char_n + 1)] != '\0'))
		{
			visible_command = prev;

			strcpy(input_buffer,&commands[prev * (line_char_n + 1)]);
			current_char = Int(strlen(input_buffer));

			//when is the last one
			if (current_char == line_char_n)
			--current_char;
		}
	}

	void Console::getNextCommand()
	{
		Int next = ((visible_command + 1) % (number_of_commands));

		if ((next != current_command) && (commands[next * (line_char_n + 1)] != '\0'))
		{
			visible_command = next;

			strcpy(input_buffer,&commands[next * (line_char_n + 1)]);
			current_char = Int(strlen(input_buffer));

			//when is the last one
			if (current_char == line_char_n)
			--current_char;
		}
	}

	void Console::addCommand()
	{
		if (input_buffer[0] != '\0')
		{
			strcpy(&commands[current_command * (line_char_n + 1)],input_buffer);
			current_command = ((current_command + 1) % (number_of_commands));
			visible_command = current_command;
		}
	}

	Int Console::nextChar() const
	{
		return ((current_char + 1) % (line_char_n));
	}

	Int Console::prevChar() const
	{
		return ((line_char_n + current_char - 1) % (line_char_n));
	}

	void Console::moveRight()
	{
		current_char = nextChar();
	}
	void Console::moveLeft()
	{
		current_char = prevChar();
	}

	void Console::setInputBuffer(Char const* message)
	{
		Int length = Int(strlen(message));
		if ((length > 0) && (length <= line_char_n))
		{
			strcpy(input_buffer, message);
			current_char = 0;
		}
	}

	void Console::setInputBuffer(std::string const &message)
	{
		setInputBuffer(message.c_str());
	}

	void Console::clearInputBuffer()
	{
		input_buffer[0] = '\0';
		current_char = 0;
	}

	Int Console::nextMesssage() const
	{
		return ((current_message + 1) % (lines_n));
	}

	void Console::moveToNextMessage()
	{
		current_message = nextMesssage();
	}

	Console::State Console::state() const
	{
		return current_state;
	}

	void Console::registerSystemFeed(SystemFeed *system_feed)
	{
		command_handler = system_feed;
	}

	//
	// ConsolePrinter
	//
	ConsolePrinter::ConsolePrinter(Console* console)
		:printer(console)
	{
	}

	ConsolePrinter::~ConsolePrinter()
	{
	}

	void ConsolePrinter::operator()(std::string const& string_data)
	{
		printer->addMessage(string_data);
	}
} // namespace rengine
