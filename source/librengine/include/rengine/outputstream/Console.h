// __!!rengine_copyright!!__ //

#ifndef __RENGINE_CONSOLE__
#define __RENGINE_CONSOLE__

#include <rengine/state/Texture.h>
#include <rengine/text/HudWriter.h>
#include <rengine/geometry/Drawable.h>
#include <rengine/windowing/Windowing.h>
#include <rengine/outputstream/OutputStream.h>

namespace rengine
{
	class SystemFeed;
	class Quadrilateral;

	class Console : public Drawable, public InterfaceEventHandler
	{
	public:
		enum State
		{
			uninitialized,
			initialized,
			closed,
			open,
			opening,
			closing
		};

		// Constructors
		Console();
		~Console();
		void initialize(const Int lines_number, std::string const& background_filename,
											    std::string const& floating_background_filename,
											    std::string const& effect);

		virtual void prepareDrawing(RenderEngine& render_engine);
		virtual void unprepareDrawing(RenderEngine& render_engine);
		virtual void updateUniforms(RenderEngine& render_engine);
		virtual void draw(RenderEngine& render_engine);

		// Events
		virtual void operator()(rengine::InterfaceEvent const& interface_event, rengine::GraphicsWindow* window);
		void execute();
		State state() const;
		void toggle();

		// Command Buffer
		void getPrvCommand();
		void getNextCommand();
		void addCommand();

		// Message Buffer
		Int nextMesssage() const;
		void moveToNextMessage();
		void addMessage(Char *const message);
		void addMessage(std::string const &message);
		void clearMessageBuffer();

		// Input Buffer
		Int nextChar() const;
		Int prevChar() const;
		void moveRight();
		void moveLeft();
		void removeChar();
		void removePrevChar();
		void setChar(const Char ch);
		void setInputBuffer(Char const* message);
		void setInputBuffer(std::string const &message);
		void clearInputBuffer();

		void registerSystemFeed(SystemFeed *system_feed);

		void shutdown();
	private:

		Int line_char_n;
		Int lines_n;
		Char *lines;
		Char *input_buffer;

		Int current_char;
		Int current_message;
		SharedPointer<HudWriter> hud_writer;
		SharedPointer<Quadrilateral> background;

		Int visible_lines;
		Real char_height;
		Real char_width;
		Int visible_index;
		Int window_height;
		Int window_width;

		Real current_height;

		State current_state;

		const Real toggling_speed;
		SystemFeed* command_handler;

		Char *commands;
		Int visible_command;
		Int current_command;
		Int number_of_commands;
		bool show_carret;
	};

	struct ConsolePrinter : public StringPrinter
	{
		ConsolePrinter(Console* console);
		virtual ~ConsolePrinter();

		virtual void operator()(std::string const& string_data);
		Console* printer;
	};

} //namespace rengine

#endif //__RENGINE_CONSOLE__
