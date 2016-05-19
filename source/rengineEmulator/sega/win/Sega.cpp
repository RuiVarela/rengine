#include <rengine/lang/debug/Debug.h>
#include <rengine/file/Zip.h>
#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>
#include <rengine/system/System.h>
#include <rengine/math/Math.h>
#include <rengine/state/BaseStates.h>
#include <rengine/state/Program.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/outputstream/Log.h>
#include <rengine/outputstream/OutputStream.h>

#include <sstream>
#include <cstdio>

#include <AL/al.h>
#include <AL/alc.h>

#include "Sega.h"

#include "shared.h"
#include "m68kcpu.h"

using namespace rengine;

m68ki_cpu_core m68ki_cpu;
_zbank_memory_map zbank_memory_map[256];
_m68k_memory_map m68k_memory_map[256];
int log_error   = 1;


extern "C"
{
	unsigned long computeCrc(unsigned long crc, unsigned char* data, unsigned int size);
	unsigned long crc32(unsigned long crc, unsigned char* data, unsigned int size)
	{
		crc = 0xffffffffL;
		return computeCrc(crc, data, size);
	}

	void uncompress(Bytef* out, unsigned long* outbytes, Bytef* in, unsigned long inbytes)
	{
		if (inbytes >= (STATE_SIZE - 4))
		{
			error("uncompress/compress2 using a buffer too big");
		}
		else
		{
			*outbytes = inbytes;
			memcpy(out, in, inbytes);
		}
	}

	void compress2(Bytef *out, unsigned long* outbytes, Bytef* in, unsigned long inbytes, int level)
	{
		uncompress(out, outbytes, in, inbytes);
	}

	void osd_input_Update(void)
	{
		Sega::instance->updateInput();
	}

	int load_archive(char* const filename)
	{
		char* buffer = 0;
		int data_size = 0;
		SharedArray<Uint8> data = Sega::instance->readFile(filename, data_size);

		if (data && data_size)
		{
			memcpy(cart.rom, data.get(), data_size);
		}

		return data_size;
	}

	void error(char *format, ...)
	{
		/*
		if (!log_error) return;
		va_list ap;
		va_start(ap, format);
		if(error_log) vfprintf(error_log, format, ap);
		va_end(ap);
		*/
	}


} // extern c


/* audio "exact" samplerate, measured on real hardware */
//#define SAMPLERATE 22050
//#define SAMPLERATE 44100
#define SAMPLERATE 48000
//#define SAMPLERATE_48KHZ 48044



Sega* Sega::instance = 0;

#define ADD_VARIABLE(description, default_value, var, prefix) \
{ \
	SharedVariable redirect = new RedirectVariable(); \
	redirect->size = sizeof(config.var); \
	redirect->address = (void*) &(config.var); \
	redirect->variable = new SystemVariable(prefix #var, default_value); \
	redirect->variable->setDescription(description); \
	redirect->variable->setHandler(this); \
	CoreEngine::instance()->system().registerVariable(redirect->variable); \
	variables[redirect->variable->name()] = redirect; \
}

#define COPY_INT_VAR(type) type value = (type)copy.asInt(); memcpy(found->second->address, &value, found->second->size);
#define COPY_FLT_VAR(type) type value = (type)copy.asFloat(); memcpy(found->second->address, &value, found->second->size);
#define ADD_SYSTEM_VARIABLE(description, default_value, var) ADD_VARIABLE(description, default_value, var, "sega_system_")
#define ADD_SOUND_VARIABLE(description, default_value, var) ADD_VARIABLE(description, default_value, var, "sega_sound_")
#define ADD_VIDEO_VARIABLE(description, default_value, var) ADD_VARIABLE(description, default_value, var, "sega_video_")
#define ADD_INPUT_VARIABLE(description, default_value, var) ADD_VARIABLE(description, default_value, var, "sega_input_")

class SegaVariableManager : SystemVariable::Handler
{
public:
	struct RedirectVariable
	{
		int size;
		void* address;
		SharedPointer<SystemVariable> variable;
	};


	typedef SharedPointer<RedirectVariable> SharedVariable;
	typedef std::map<std::string, SharedVariable> Variables;

	SegaVariableManager()
	{
	}

	~SegaVariableManager()
	{
	}

	Bool operator()(SystemVariable& variable, SystemVariable::Arguments const& arguments)
	{
		Variables::iterator found = variables.find(variable.name());

		SystemVariable copy(variable);
		copy.setHandler(0);
		CoreEngine::instance()->system().changeSystemVariable(&copy, arguments);

		if (found != variables.end())
		{
			if (variable.type() == Variable::IntType)
			{
				if	(found->second->size == 1)		{ COPY_INT_VAR(Int8) }
				else if (found->second->size == 2)	{ COPY_INT_VAR(Int16) }
				else if (found->second->size == 4)	{ COPY_INT_VAR(Int32) }
				else if (found->second->size == 8)	{ COPY_INT_VAR(Int64) }
			}
			else if (variable.type() == Variable::FloatType) { COPY_FLT_VAR(Real) }
			else if (variable.type() == Variable::StringType) 
			{
				std::string value = copy.asString();
				memset(found->second->address, 0, found->second->size);
				memcpy(found->second->address, value.c_str(), minimum<int>(value.size(), found->second->size - 1)); 
			}
		}


		return true;
	}




	void install()
	{
		//ADD_SOUND_VARIABLE("PSG Volume", 150, psg_preamp);
		//ADD_SOUND_VARIABLE("FM Volume", 100, fm_preamp);
		//ADD_SOUND_VARIABLE("High-Quality FM [1/0]", 1, hq_fm);
		//ADD_SOUND_VARIABLE("PSG Noise Boost [1/0]", 0, psgBoostNoise);
		//ADD_SOUND_VARIABLE("Filtering [0 none, 1 LOW-PASS, 2 3-BAND EQ]", 1, filter);
		//ADD_SOUND_VARIABLE("Low-Pass filter Rate", 50, lp_range); // for LOW-PASS
		//ADD_SOUND_VARIABLE("EQ Lowest Frequency", 880, low_freq);
		//ADD_SOUND_VARIABLE("EQ Highest Frequency", 5000, high_freq);
		//ADD_SOUND_VARIABLE("Low Gain", 1, lg); // for 3-BAND EQ
		//ADD_SOUND_VARIABLE("Middle Gain", 1, mg);
		//ADD_SOUND_VARIABLE("High Gain", 1, hg);
		//ADD_SOUND_VARIABLE("High Gain", 1, hg);
		//ADD_SOUND_VARIABLE("FIR low-pass filtering", 0.995f, rolloff); // for High-Quality FM
		//ADD_SOUND_VARIABLE("FM Resolution [< 15]", 14, dac_bits); // FM Resolution

		ADD_SYSTEM_VARIABLE("Console Region [0 AUTO, 1 USA, 2 EUR, 3 JAPAN]", 0, region_detect);
		ADD_SYSTEM_VARIABLE("System Lockups [1/0]", 0, force_dtack);
		ADD_SYSTEM_VARIABLE("68k Address Error", 1, addr_error);
		ADD_SYSTEM_VARIABLE("System TMSS", 1, tmss);
		ADD_SYSTEM_VARIABLE("Lock On [0 None, 1 GGenie, 2 ActionReplay, 3 S&k]", 0, lock_on);
		ADD_SYSTEM_VARIABLE("Rom Type", 0, romtype);

		//ADD_VIDEO_VARIABLE("Borders [0 None, 1 Vertical, 2 Horizontal, 3 Both]", 3, overscan);
		//ADD_VIDEO_VARIABLE("Display [0 Original, 1 Interlaced, 2 Progressive]", 0, render);



		//
		// Input variables
		//

		ADD_INPUT_VARIABLE("Port 0", 1, port[0]);
		ADD_INPUT_VARIABLE("Port 1", 0, port[1]);
		
#define RENGINE_GEN_PAD(n, param) \
	ADD_INPUT_VARIABLE("PadType", 0, input[n].padtype); \
	ADD_INPUT_VARIABLE("Pad MODE state", 0, input[n].mode); \
	ADD_INPUT_VARIABLE("Pad START state", 0, input[n].start); \
	ADD_INPUT_VARIABLE("Pad RIGHT state", 0, input[n].right); \
	ADD_INPUT_VARIABLE("Pad LEFT state", 0, input[n].left); \
	ADD_INPUT_VARIABLE("Pad DOWN state", 0, input[n].down); \
	ADD_INPUT_VARIABLE("Pad UP state", 0, input[n].up); \
	ADD_INPUT_VARIABLE("Pad A state", 0, input[n].a); \
	ADD_INPUT_VARIABLE("Pad B state", 0, input[n].b); \
	ADD_INPUT_VARIABLE("Pad C state", 0, input[n].c); \
	ADD_INPUT_VARIABLE("Pad X state", 0, input[n].x); \
	ADD_INPUT_VARIABLE("Pad Y state", 0, input[n].y); \
	ADD_INPUT_VARIABLE("Pad Z state", 0, input[n].z); 


		RENGINE_REPEAT(MAX_INPUTS, RENGINE_GEN_PAD, _ )

#undef RENGINE_GEN_PAD


	}

	void uninstall()
	{
		for (Variables::iterator i = variables.begin(); i != variables.end(); ++i)
		{
			CoreEngine::instance()->system().unregisterVariable(i->second->variable);
		}
		variables.clear();
	}

	Variables variables;
};

struct Sega::PrivateImplementation
{
	PrivateImplementation()
	{
		enabled = false;
		context = 0;
		device = 0;

		for (Uint i = 0; i != number_of_buffers; ++i)
		{
			buffer[i] = 0;;
		}
		source = 0;

		format = AL_FORMAT_STEREO16;
		//format = AL_FORMAT_STEREO8;
		audiobuffer_size = 0;
	}

	~PrivateImplementation()
	{
	}

	static unsigned int const number_of_buffers = 5;
	ALenum format;

	bool enabled;
	ALCcontext *context;
	ALCdevice *device;
	ALuint buffer[number_of_buffers];
	ALuint source;
	SharedArray<Int16> audiobuffer;
	unsigned int audiobuffer_size;
	typedef std::vector<ALuint> AvailableBuffers;
	AvailableBuffers available_buffers;

	void ALCheck()
	{
		int error = alGetError();

		if(error != AL_NO_ERROR)
		{
			RLOG_ERROR("sega", " AL error raised");
		}
	}

	void ALEmpty()
	{
		if (enabled)
		{

			int queued;

			alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);

			while(queued--)
			{
				ALuint buffer;

				alSourceUnqueueBuffers(source, 1, &buffer);
				ALCheck();
			}
		}
	}

};

Sega::Sega() 
{ 
	m_pimpl = new PrivateImplementation();

	set_config_defaults();
	m_variable_manager = new SegaVariableManager();

	RENGINE_ASSERT(Sega::instance == 0);
	Sega::instance = this;
}

Sega::~Sega() 
{
	m_variable_manager.reset();

	Sega::instance = 0;
}

rengine::SharedArray<Uint8> Sega::readFile(std::string const& filename, int& size)
{
	SharedArray<Uint8> data;

	rengine::Zip zip;

	if (!zip.fileExists(filename))
	{
		zip.close();
		zip.load(GENESIS_DATABASE);
	}

	if (!zip.fileExists(filename))
	{
		zip.close();
		zip.load(MASTERSYSTEM_DATABASE);
	}

	if (!zip.fileExists(filename))
	{
		zip.close();
		zip.load(GAMEGEAR_DATABASE);
	}

	if (!zip.fileExists(filename))
	{
		zip.close();
		zip.load(SG1000_DATABASE);
	}

	if (zip.fileExists(filename))
	{
		Zip::FileData file_data = zip.read(filename);
		data = file_data.data;
		size = file_data.size;
	}

	zip.close();

	return data;
}

void Sega::shutdown()
{	
	/* shutdown emulation */
	system_shutdown();
	audio_shutdown();

	if(cart.rom)
	{
		free(cart.rom);
	}


	m_variable_manager->uninstall();

	m_start_time = -1.0;
	m_rendered_frames = 0;
	m_framerate = 0.0;

	//
	// Audio Shutdown
	//

	if (m_pimpl->enabled)
	{
		alSourceStop(m_pimpl->source); 
	}

	alDeleteBuffers(PrivateImplementation::number_of_buffers, m_pimpl->buffer);
	alDeleteSources(1, &m_pimpl->source);

	m_pimpl->source = 0;

	for (Uint i = 0; i != PrivateImplementation::number_of_buffers; ++i)
	{
		m_pimpl->buffer[i] = 0;
	}

	if (m_pimpl->context)
	{
		alcMakeContextCurrent(NULL);			//Disable context
		alcDestroyContext(m_pimpl->context);	//Release context(s)
		m_pimpl->context = 0;
	}

	if (m_pimpl->device)
	{
		alcCloseDevice(m_pimpl->device);	//Close device
		m_pimpl->device = 0;
	}

	m_pimpl->available_buffers.clear();



}

void Sega::loadBios()
{
	/* clear BIOS detection flag */
	config.tmss &= ~2;

	/* open BIOS file */
	int size = 0;
	SharedArray<Uint8> data = Sega::instance->readFile(OS_ROM, size);

	if (!data.get()) return;

	/* read file */
	if (size <= (int)sizeof(bios_rom))
		memcpy(bios_rom, data.get(), size);

	/* check ROM file */
	if (!strncmp((char *)(bios_rom + 0x120),"GENESIS OS", 10))
	{
		/* valid BIOS detected */
		config.tmss |= 2;

#ifdef LSB_FIRST
		/* Byteswap ROM */
		for(Uint i = 0; i < sizeof(bios_rom); i += 2)
		{
			int temp = bios_rom[i];
			bios_rom[i] = bios_rom[i + 1];
			bios_rom[i + 1] = temp;
		}
#endif
	}
}

void Sega::init()
{
	//vdp_pal = 1;

	CoreEngine::instance()->log() << std::endl << "Sega Port types" << std::endl
								  <<" (0) unconnected port"<< std::endl
								  <<" (1) single 3-buttons or 6-buttons Control Pad"<< std::endl
								  <<" (2) Sega Mouse"<< std::endl
								  <<" (3) Sega Menacer (port B only)"<< std::endl
								  <<" (4) Konami Justifiers (port B only)"<< std::endl
								  <<" (5) XE-A1P analog controller (port A only)"<< std::endl
								  <<" (6) Sega Activator"<< std::endl
								  <<" (7) single 2-buttons Control Pad (Master System)"<< std::endl
								  <<" (8) Sega Light Phaser (Master System)"<< std::endl
								  <<" (9) Sega Paddle Control (Master System)"<< std::endl
								  <<"(10) Sega Sports Pad (Master System)"<< std::endl
								  <<"(11) Multi Tap -- Sega TeamPlayer"<< std::endl
								  <<"(12) Multi Tap -- EA 4-Way Play (use both ports)"<< std::endl;

	CoreEngine::instance()->log() << std::endl << "Sega Pad types" << std::endl
								  <<"  (0) 3-buttons Control Pad (fixed ID for Team Player)"<< std::endl
								  <<"  (1) 6-buttons Control Pad (fixed ID for Team Player)"<< std::endl
								  <<"  (2) 2-buttons Control Pad"<< std::endl
								  <<"  (3) Sega Mouse"<< std::endl
								  <<"  (4) Sega Light Phaser, Menacer or Konami Justifiers"<< std::endl
								  <<"  (5) Sega Paddle Control"<< std::endl
								  <<"  (6) Sega Sports Pad"<< std::endl
								  <<"  (7) PICO tablet"<< std::endl
								  <<"  (8) Terebi Oekaki tablet"<< std::endl
								  <<"  (9) XE-A1P analog controller"<< std::endl
								  <<" (10) Activator "<< std::endl
								  <<"(255) unconnected device (fixed ID for Team Player)"<< std::endl;


	SharedPointer<SystemCommand> action;
	
	action = new SystemCommand("segaHardReset", HardReset, this);
	action->setDescription("Performs a hard reset");
	CoreEngine::instance()->system().registerCommand(action);

	action = new SystemCommand("segaSoftReset", SoftReset, this);
	action->setDescription("Performs a soft reset");
	CoreEngine::instance()->system().registerCommand(action);

	action = new SystemCommand("segaToggleBorderEmulation", ToggleBorderEmulation, this);
	action->setDescription("Toggle border emulation");
	CoreEngine::instance()->system().registerCommand(action);

	action = new SystemCommand("segaSaveGameState", SaveGame, this);
	action->setDescription("Save game state");
	CoreEngine::instance()->system().registerCommand(action);

	action = new SystemCommand("segaLoadGameState", LoadGame, this);
	action->setDescription("Load game state");
	CoreEngine::instance()->system().registerCommand(action);


	m_start_time = -1.0;
	m_rendered_frames = 0;

	m_variable_manager->install();
	CoreEngine::instance()->system()("/runScript " DEFAULT_SEGA_SCRIPT);



	m_framerate = 0;
	cart.rom = 0;

	/* allocate cart.rom here (10 MBytes) */
	cart.rom = (uint8*)malloc(MAXROMSIZE);
	if (!cart.rom)
	{
		return;
	}

	//
	// Video Setup
	//

	// allocate global work bitmap 
	memset(&bitmap, 0, sizeof (bitmap));
	bitmap.width        = 512;
	bitmap.height       = 512;
#if defined(USE_8BPP_RENDERING)
	bitmap.pitch        = (bitmap.width * 1);
#elif defined(USE_15BPP_RENDERING)
	bitmap.pitch        = (bitmap.width * 2);
#elif defined(USE_16BPP_RENDERING)
	bitmap.pitch        = (bitmap.width * 2);
#elif defined(USE_32BPP_RENDERING)
	bitmap.pitch        = (bitmap.width * 4);
#endif
	bitmap.viewport.changed = 3;



	image = new Image(bitmap.width, bitmap.height, 4);
	image->zeroImage();

	texture = new Texture2D(image);
	texture->setFlags(texture->getFlags() & ~Texture2D::ReleaseImage);

	quadrilateral = new Quadrilateral();
	quadrilateral->setCornersVertex(Vector3D(0.0f, 0.0f, 0.0f), Vector3D(Real(bitmap.width), Real(bitmap.height), 0.0f));

	// flip image
	quadrilateral->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, -1.0f));
	quadrilateral->states()->setTexture(texture);

	SharedPointer<Program> decal_program = CoreEngine::instance()->resourceManager().load<Program>(DEFAULT_SEGA_EFFECT);

	quadrilateral->states()->setCapability(DrawStates::DepthTest, DrawStates::Off);
	quadrilateral->states()->setCapability(DrawStates::Blend, DrawStates::Off);
	quadrilateral->states()->setCapability(DrawStates::CullFace, DrawStates::Off);
	quadrilateral->states()->setProgram(decal_program);
	quadrilateral->setDrawMode(Drawable::DynamicDraw);

	bitmap.data = image->getData();


	//
	// Audio Setup
	//
	m_pimpl->enabled = true;

	m_pimpl->device = alcOpenDevice(NULL);
	if (!m_pimpl->device)
	{
		m_pimpl->enabled = false;
	}

	if (m_pimpl->enabled)
	{
		//Create context(s)
		m_pimpl->context = alcCreateContext(m_pimpl->device, NULL);

		if (!m_pimpl->context)
		{
			m_pimpl->enabled = false;
		}
	}

	if (m_pimpl->enabled)
	{
		//Set active context
		alcMakeContextCurrent(m_pimpl->context);
	}

	if (m_pimpl->enabled)
	{
		// Load wav data into a buffer.
		alGenBuffers(PrivateImplementation::number_of_buffers, m_pimpl->buffer);
		if (alGetError() != AL_NO_ERROR)
		{
			m_pimpl->enabled = false;
		}
	}

	if (m_pimpl->enabled)
	{
		alGenSources(1, &m_pimpl->source);

		if (alGetError() != AL_NO_ERROR)
		{
			m_pimpl->enabled = false;
		}
	}

	if (m_pimpl->enabled)
	{
		ALfloat source_pos[] = { 0.0, 0.0, 0.0 };							// Position of the source sound.
		ALfloat source_vel[] = { 0.0, 0.0, 0.0 };							// Velocity of the source sound.
		ALfloat listener_pos[] = { 0.0, 0.0, 0.0 };							// Position of the listener.
		ALfloat listener_vel[] = { 0.0, 0.0, 0.0 };							// Velocity of the listener.
		ALfloat listener_ori[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };		// Orientation of the listener. (first 3 elements are "at", second 3 are "up")

		alSourcef (m_pimpl->source, AL_PITCH,    1.0f);
		alSourcef (m_pimpl->source, AL_GAIN,     1.0f);
		alSourcefv(m_pimpl->source, AL_POSITION, source_pos);
		alSourcefv(m_pimpl->source, AL_VELOCITY, source_vel);
		alSourcef (m_pimpl->source, AL_ROLLOFF_FACTOR,  0.0          );
		alSourcei (m_pimpl->source, AL_SOURCE_RELATIVE, AL_TRUE      );
		alSourcei (m_pimpl->source, AL_LOOPING,  FALSE);


		alListenerfv(AL_POSITION,    listener_pos);
		alListenerfv(AL_VELOCITY,    listener_vel);
		alListenerfv(AL_ORIENTATION, listener_ori);

		// Do another error check and return.
		if (alGetError() != AL_NO_ERROR)
		{
			m_pimpl->enabled = false;
		}
	}

	for (Uint i = 0; i != PrivateImplementation::number_of_buffers; ++i)
	{
		m_pimpl->available_buffers.push_back(m_pimpl->buffer[i]);
	}
}

void Sega::loadRom(std::string const& rom)
{
	for (Uint i = 0; i != 2; ++i)
	{
		input.system[i] = config.port[i];
	}

	// load ROM file
	load_rom((char*)rom.c_str());

	/* BIOS support */
	loadBios();

	bool const vsynch = true;

	m_framerate = 50.0f;
	if (vdp_pal)
	{
		m_framerate = 50.0f;
	}
	else if (vsynch)
	{
		m_framerate = 60.0f;
	}

	audio_init(SAMPLERATE, m_framerate);

	/* System Power ON */
	system_init();
	system_reset();



	filename = rom;

	m_start_time = -1.0;
	m_rendered_frames = 0;
}

// gets mouse absolute values in sega viewport coordinates
void GetMouseAbsolutePos(int joynum, int& mouse_x, int& mouse_y, bool& mouse_left, bool& mouse_middle, bool& mouse_right){ }


// gets mouse relative values in sega viewport coordinates
void GetMouseRelativePos(int joynum, int& mouse_x, int& mouse_y, bool& mouse_left, bool& mouse_middle, bool& mouse_right){}

void Sega::updateInput()
{
	//
	// todo
	//
	
	int mouse_x = 0;
	int mouse_y = 0;
	bool mouse_left = false;
	bool mouse_middle = false;
	bool mouse_right = false;

	for(int joynum = 0; joynum != MAX_INPUTS; ++joynum)
	{
		input.pad[joynum] = 0;

		switch (input.dev[joynum])
		{
		case DEVICE_LIGHTGUN:
			{
				/* get mouse (absolute values) */
				GetMouseAbsolutePos(joynum, mouse_x,  mouse_y, mouse_left, mouse_middle, mouse_right);

				/* Calculate X Y axis values */
				input.analog[joynum][0] = mouse_x;
				input.analog[joynum][1] = mouse_x;

				/* TRIGGER, B, C (Menacer only), START (Menacer & Justifier only) */
				if(mouse_left)					input.pad[joynum] |= INPUT_A;
				if(mouse_right)					input.pad[joynum] |= INPUT_B;
				if(mouse_middle)				input.pad[joynum] |= INPUT_C; 
				if(config.input[joynum].start)	input.pad[joynum] |= INPUT_START;
				break;
			}

		case DEVICE_PADDLE:
			{
				/* get mouse (absolute values) */
				GetMouseAbsolutePos(joynum, mouse_x,  mouse_y, mouse_left, mouse_middle, mouse_right);

				/* Range is [0;256], 128 being middle position */
				input.analog[joynum][0] = ((float)mouse_x / (float)bitmap.viewport.w) * 256.0f;

				/* Button I -> 0 0 0 0 0 0 0 I*/
				if(mouse_left) input.pad[joynum] |= INPUT_B;

				break;
			}

		case DEVICE_SPORTSPAD:
			{
				/* get mouse (relative values) */
				GetMouseRelativePos(joynum, mouse_x,  mouse_y, mouse_left, mouse_middle, mouse_right);

				/* Range is [0;256] */
				input.analog[joynum][0] = (unsigned char)(-mouse_x & 0xFF);
				input.analog[joynum][1] = (unsigned char)(-mouse_y & 0xFF);

				/* Buttons I & II -> 0 0 0 0 0 0 II I*/
				if(mouse_left) input.pad[joynum] |= INPUT_B;
				if(mouse_right) input.pad[joynum] |= INPUT_C;

				break;
			}

		case DEVICE_MOUSE:
			{
				/* get mouse (relative values) */
				GetMouseRelativePos(joynum, mouse_x,  mouse_y, mouse_left, mouse_middle, mouse_right);

				/* Sega Mouse range is [-256;+256] */
				input.analog[joynum][0] = mouse_x * 2;
				input.analog[joynum][1] = mouse_y * 2;

				/* Vertical movement is upsidedown */
				if (!config.invert_mouse)
					input.analog[joynum][1] = 0 - input.analog[joynum][1];

				/* Start,Left,Right,Middle buttons -> 0 0 0 0 START MIDDLE RIGHT LEFT */
				if(mouse_left)						input.pad[joynum] |= INPUT_B;
				if(mouse_right)						input.pad[joynum] |= INPUT_C;
				if(mouse_middle)					input.pad[joynum] |= INPUT_A;
				if(config.input[joynum].start)		input.pad[joynum] |= INPUT_START;

				break;
			}

		case DEVICE_XE_A1P:
			{
				/* A,B,C,D,Select,START,E1,E2 buttons -> E1(?) E2(?) START SELECT(?) A B C D */
				//if(keystate[SDLK_a])  input.pad[joynum] |= INPUT_START;
				//if(keystate[SDLK_s])  input.pad[joynum] |= INPUT_A;
				//if(keystate[SDLK_d])  input.pad[joynum] |= INPUT_C;
				//if(keystate[SDLK_f])  input.pad[joynum] |= INPUT_Y;
				//if(keystate[SDLK_z])  input.pad[joynum] |= INPUT_B;
				//if(keystate[SDLK_x])  input.pad[joynum] |= INPUT_X;
				//if(keystate[SDLK_c])  input.pad[joynum] |= INPUT_MODE;
				//if(keystate[SDLK_v])  input.pad[joynum] |= INPUT_Z;

				/* Left Analog Stick (bidirectional) */
				//if(keystate[SDLK_UP])     input.analog[joynum][1]-=2;
				//else if(keystate[SDLK_DOWN])   input.analog[joynum][1]+=2;
				//else input.analog[joynum][1] = 128;
				//if(keystate[SDLK_LEFT])   input.analog[joynum][0]-=2;
				//else if(keystate[SDLK_RIGHT])  input.analog[joynum][0]+=2;
				//else input.analog[joynum][0] = 128;

				/* Right Analog Stick (unidirectional) */
				//if(keystate[SDLK_KP8])    input.analog[joynum+1][0]-=2;
				//else if(keystate[SDLK_KP2])   input.analog[joynum+1][0]+=2;
				//else if(keystate[SDLK_KP4])   input.analog[joynum+1][0]-=2;
				//else if(keystate[SDLK_KP6])  input.analog[joynum+1][0]+=2;
				//else input.analog[joynum+1][0] = 128;

				/* Limiters */
				if (input.analog[joynum][0] > 0xFF) input.analog[joynum][0] = 0xFF;
				else if (input.analog[joynum][0] < 0) input.analog[joynum][0] = 0;
				if (input.analog[joynum][1] > 0xFF) input.analog[joynum][1] = 0xFF;
				else if (input.analog[joynum][1] < 0) input.analog[joynum][1] = 0;
				if (input.analog[joynum+1][0] > 0xFF) input.analog[joynum+1][0] = 0xFF;
				else if (input.analog[joynum+1][0] < 0) input.analog[joynum+1][0] = 0;
				if (input.analog[joynum+1][1] > 0xFF) input.analog[joynum+1][1] = 0xFF;
				else if (input.analog[joynum+1][1] < 0) input.analog[joynum+1][1] = 0;

				break;
			}

		case DEVICE_PICO:
			{
				/* get mouse (absolute values) */
				GetMouseAbsolutePos(joynum, mouse_x,  mouse_y, mouse_left, mouse_middle, mouse_right);

				/* Calculate X Y axis values */
				input.analog[0][0] = 0x3c  + (mouse_x * (0x17c-0x03c+1)) / bitmap.viewport.w;
				input.analog[0][1] = 0x1fc + (mouse_y * (0x2f7-0x1fc+1)) / bitmap.viewport.h;

				/* Map mouse buttons to player #1 inputs */
				if(mouse_middle) pico_current++;
				if(mouse_right) input.pad[0] |= INPUT_B;
				if(mouse_left) input.pad[0] |= INPUT_A;

				break;
			}

		case DEVICE_TEREBI:
			{
				/* get mouse (absolute values) */
				GetMouseAbsolutePos(joynum, mouse_x,  mouse_y, mouse_left, mouse_middle, mouse_right);

				/* Calculate X Y axis values range is [0;250] */
				input.analog[0][0] = ((float)mouse_x / (float)bitmap.viewport.w) * 250.0f;
				input.analog[0][1] = ((float)mouse_y / (float)bitmap.viewport.h) * 250.0f;

				/* Map mouse buttons to player #1 inputs */
				if(mouse_right) input.pad[0] |= INPUT_B;

				break;
			}

		case DEVICE_ACTIVATOR:
			{
				//if(keystate[SDLK_g])  input.pad[joynum] |= INPUT_ACTIVATOR_7L;
				//if(keystate[SDLK_h])  input.pad[joynum] |= INPUT_ACTIVATOR_7U;
				//if(keystate[SDLK_j])  input.pad[joynum] |= INPUT_ACTIVATOR_8L;
				//if(keystate[SDLK_k])  input.pad[joynum] |= INPUT_ACTIVATOR_8U;
			}

		default:
			{
				if(config.input[joynum].a)		input.pad[joynum] |= INPUT_A;
				if(config.input[joynum].b)		input.pad[joynum] |= INPUT_B;
				if(config.input[joynum].c)		input.pad[joynum] |= INPUT_C;
				if(config.input[joynum].start)  input.pad[joynum] |= INPUT_START;
				if(config.input[joynum].x)		input.pad[joynum] |= INPUT_X;
				if(config.input[joynum].y)		input.pad[joynum] |= INPUT_Y;
				if(config.input[joynum].z)		input.pad[joynum] |= INPUT_Z;
				if(config.input[joynum].mode)	input.pad[joynum] |= INPUT_MODE;

				if(config.input[joynum].up)     
					input.pad[joynum] |= INPUT_UP;
				else if(config.input[joynum].down)   
					input.pad[joynum] |= INPUT_DOWN;

				if(config.input[joynum].left)   
					input.pad[joynum] |= INPUT_LEFT;
				else if(config.input[joynum].right)  
					input.pad[joynum] |= INPUT_RIGHT;

				break;
			}
		}
	}
}

void Sega::updateAudio()
{
	if (m_pimpl->enabled)
	{
		int processed;
		alGetSourcei(m_pimpl->source, AL_BUFFERS_PROCESSED, &processed);
		while (processed--)
		{
			ALuint buffer;
			alSourceUnqueueBuffers(m_pimpl->source, 1, &buffer);
			m_pimpl->ALCheck();
			m_pimpl->available_buffers.push_back(buffer);
		}


		int size = audio_update();
		if (size && !m_pimpl->available_buffers.empty())
		{
			unsigned int buffered_size = size * 2;

			if (m_pimpl->audiobuffer_size < buffered_size)
			{
				m_pimpl->audiobuffer_size = buffered_size;
				m_pimpl->audiobuffer = new Int16[m_pimpl->audiobuffer_size];

				if (!m_pimpl->audiobuffer)
				{
					m_pimpl->audiobuffer_size = 0;
				}
			}

			if (m_pimpl->audiobuffer_size)
			{
				for(int i = 0; i < size; ++i) 
				{
					m_pimpl->audiobuffer[i * 2 + 0] = snd.buffer[0][i];
					m_pimpl->audiobuffer[i * 2 + 1] = snd.buffer[1][i];
				}

				ALuint buffer = m_pimpl->available_buffers.back();
				m_pimpl->available_buffers.pop_back();

				alBufferData(buffer, m_pimpl->format, (Uchar*)m_pimpl->audiobuffer.get(), buffered_size * sizeof(Int16), SAMPLERATE);

				m_pimpl->ALCheck();

				alSourceQueueBuffers(m_pimpl->source, 1, &buffer);
				m_pimpl->ALCheck();


				ALint state;
				alGetSourcei(m_pimpl->source, AL_SOURCE_STATE, &state) ;
				if (state != AL_PLAYING)
				{
					alSourcePlay(m_pimpl->source);
				}
			}

		}
	}

}

void Sega::updateVideo()
{
	int vwidth = 0;
	int vheight = 0;

	int update = bitmap.viewport.changed & 1;

	/* check if display has changed during frame */
	if (update)
	{
		/* Clear update flags */
		bitmap.viewport.changed &= ~1;

		/* update texture size */
		vwidth = bitmap.viewport.w + (2 * bitmap.viewport.x);
		vheight = bitmap.viewport.h + (2 * bitmap.viewport.y);


		int xCrop = 0;

		if (system_hw == SYSTEM_SMS2)
		{
			xCrop = 8;
		}

		vwidth -= xCrop;

		if (vwidth && vheight)
		{
			Rectanglei source(0, 0, vwidth, vheight);
			Rectanglei destination(0, 0, 
				Int(CoreEngine::instance()->mainWindow()->contextOptions().width),
				Int(CoreEngine::instance()->mainWindow()->contextOptions().height));

			Rectanglei letterbox = Image::letterbox(source, destination);

			quadrilateral->setCornersVertex(Vector3D(Real(letterbox.left()) , Real(letterbox.bottom()), 0.0f), 
											Vector3D(Real(letterbox.right()), Real(letterbox.top())   , 0.0f));

			Real xCropPos = Real(xCrop) / Real(bitmap.width);
			Real height_factor = Real(vheight) / Real(bitmap.height);
			Real width_factor = Real(vwidth) / Real(bitmap.width);
			quadrilateral->setCornersTextureCoordinates(Vector2D(xCropPos, -(1.0f - height_factor)), Vector2D(xCropPos + width_factor, -1.0f));
		}


		/* audio & video resynchronization */
		//audioStarted = 0;
	}

	texture->changeFlags() |= Texture2D::ImageDataChanged;
}

void Sega::update()
{
	if ((m_start_time >= 0.0) && (m_framerate > 0.0))
	{
		Real64 simulation_time = CoreEngine::instance()->frameGlobalTime() - m_start_time;
		Uint64 supposed_frame = simulation_time / (1.0 / m_framerate);
		Int64 difference = 0;

		if (supposed_frame > m_rendered_frames)
		{
			difference = supposed_frame - m_rendered_frames;
		}

		if (m_rendered_frames != supposed_frame)
		{
			m_rendered_frames = supposed_frame;

			bool skip = false;

			if (skip)
			{
				system_frame(1);	// skip frame 
			}
			else
			{
				system_frame(0);	// render frame
				updateVideo();		// update video
			}


			updateAudio();			// update audio

			if (bitmap.viewport.changed & 4)		// check interlaced mode change
			{
				// in original 60hz modes, audio is synced with framerate
				if (!config.render && !vdp_pal)
				{
					uint8 *state = (uint8*) malloc(YM2612GetContextSize());
					if (state)
					{

						memcpy(state, YM2612GetContextPtr(), YM2612GetContextSize());	  // save YM2612 context
						audio_init(SAMPLERATE, interlaced ? 59.94 : (1000000.0/16715.0)); // framerate has changed, reinitialize audio timings
						sound_init();
						YM2612Restore(state);											 // restore YM2612 context 
						free(state);
					}
				}
				bitmap.viewport.changed &= ~4;	// clear flag
			}
		}

	}
	else
	{
		m_start_time = CoreEngine::instance()->frameGlobalTime();
		m_rendered_frames = 0;
	}
}

void Sega::render()
{
	float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
	float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;
	CoreEngine::instance()->renderEngine().setViewport(0, 0, width, height);
	
	Matrix projection = Matrix::ortho2D(0, width, 0, height);
	CoreEngine::instance()->renderEngine().pushDrawStates();

	quadrilateral->states()->getProgram()->uniform("mvp").set( Matrix::ortho2D(0, width, 0, height) );
	CoreEngine::instance()->renderEngine().draw( *quadrilateral );

	CoreEngine::instance()->renderEngine().popDrawStates();
}

void Sega::saveGame(int slot)
{
	uint8* savebuffer = 0;
	int savebuffer_size = 0;

	std::stringstream filename_stream;
	filename_stream << DEFAULT_SEGA_SAVE_PATH <<  filename;

	if (slot >= 0)
	{
		savebuffer = (uint8*) malloc(STATE_SIZE);
		if (savebuffer)
		{
			savebuffer_size = state_save(savebuffer);
			filename_stream << "_slot_" << slot << ".gen";
		}
	}
	else if (sram.on)
	{
		if (crc32(0, &sram.sram[0], 0x10000) != sram.crc)
		{
			savebuffer_size = 0x10000;
			savebuffer = (uint8*) malloc(savebuffer_size);
			if (savebuffer)
			{
				memcpy(savebuffer, sram.sram, savebuffer_size);
				sram.crc = crc32(0, sram.sram, savebuffer_size);
				filename_stream << ".srm";
			}
		}
	}

	if (savebuffer && savebuffer_size)
	{
		FILE *fp = fopen(filename_stream.str().c_str(), "wb");
		if (fp)
		{
			int chunk = 1024 * 2;
			int done = 0;

			while (savebuffer_size > 0)
			{
				if (savebuffer_size < chunk)
				{
					chunk = savebuffer_size;
				}

				fwrite(savebuffer + done, chunk, 1, fp);
				savebuffer_size -= chunk;
				done += chunk;
			}

			fclose(fp);
		}
	}

	if (savebuffer)
	{
		free(savebuffer);
	}
}

void Sega::loadGame(int slot)
{
	uint8* savebuffer = 0;
	int savebuffer_size = 0;

	std::stringstream filename_stream;
	filename_stream << DEFAULT_SEGA_SAVE_PATH << filename;

	if (slot >= 0)
	{
		filename_stream << "_slot_" << slot << ".gen";
		savebuffer_size = STATE_SIZE;
		savebuffer = (uint8*) malloc(STATE_SIZE);
	}
	else if (sram.on)
	{
		filename_stream <<".srm" ;
		savebuffer_size = 0x10000;
		savebuffer = (uint8*) malloc(savebuffer_size);
	}

	if (savebuffer && savebuffer_size)
	{
		FILE *fp = fopen(filename_stream.str().c_str(), "rb");
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			savebuffer_size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			int chunk = 1024 * 2;
			int done = 0;

			while (savebuffer_size > 0)
			{
				if (savebuffer_size < chunk)
				{
					chunk = savebuffer_size;
				}

				fread(savebuffer + done, chunk, 1, fp);
				savebuffer_size -= chunk;
				done += chunk;
			}

			savebuffer_size = done;
			fclose(fp);
		}
	}


	if (slot > 0)
	{
		state_load(savebuffer);
	}
	else if (savebuffer && savebuffer_size)
	{
		memcpy(sram.sram, savebuffer, 0x10000);
		sram.crc = crc32(0, sram.sram, 0x10000);
	}

	if (savebuffer)
	{
		free(savebuffer);
	}
}

void Sega::operator()(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
{
	if (command == HardReset)
	{
		system_init();
        system_reset();
	}
	else if (command == SoftReset)
	{
		gen_reset(0);
	}
	else if (command == ToggleBorderEmulation)
	{
		config.overscan =  (config.overscan + 1) & 3;
		if (system_hw == SYSTEM_GG)
		{
			bitmap.viewport.x = (config.overscan & 2) ? 14 : -48;
		}
		else
		{
			bitmap.viewport.x = (config.overscan & 2) * 7;
		}
		bitmap.viewport.changed = 3;
	}
	else if (command == SaveGame)
	{
		saveGame(1);
	}
	else if (command == LoadGame)
	{
		loadGame(1);
	}
}

