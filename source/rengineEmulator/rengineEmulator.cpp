#include <rengine/util/Bootstrap.h>

#include "Sega.h"


class MainScene : public Scene, public InterfaceEventHandler
{
public:
	MainScene() 
	{
		m_sega = new Sega();
	}
	
	virtual ~MainScene() 
	{
		m_sega.reset();
	}

	virtual void init()
	{				
		m_sega->init();

		std::string rom = "";
	
		
		//
		// game gear
		//
		rom = "Sonic_the_Hedgehog_2_(JUE)_[!].gg";
		rom = "Sonic_the_Hedgehog_(JUE)_(V1.1)_[!].gg";
		rom = "Sonic_the_Hedgehog_-_Triple_Trouble_(UE).gg";
		rom = "Zoop.gg";
		rom = "Zool.gg";

		//
		// master system
		//
		rom = "Gauntlet_(UE)_[!].sms";
		rom = "Dr._Robotnik's_Mean_Bean_Machine_(UE)_[!].sms";
		rom = "Sonic_the_Hedgehog_(UE)_[!].sms";
		rom = "Pac_Mania_(UE)_[!].sms";	
		rom = "Back_to_the_Future_2_(UE)_[!].sms";
		rom = "Blade_Eagle_3D_(UE)_[!].sms";
		rom = "Sonic_the_Hedgehog_2_(UE)_[!].sms";
		rom = "Y's_-_The_Vanished_Omens_(UE)_[!].sms";
		rom = "Addams_Family,_The_(UE)_[!].sms";
		rom = "Phantasy_Star_(Korean)_[!].sms";
		rom = "Back_to_the_Future_3_(UE)_[!].sms";
		rom = "Shinobi_(UE)_[!].sms";

		
		//
		// SG1000
		//
		rom = "Sega_Galaga_(SG-1000)_[!].sg";
		rom = "Super_Tank_(SG-1000)_[!].sg";

		//
		// genesis
		//
		rom = "Robocop_3_(UE)_[!].bin";
		rom = "Pac-Attack_(U)_[!].bin";
		rom = "Shinobi_3_-_Return_of_the_Ninja_Master_(U)_[!].bin";
		rom = "Super_Hang-On_(REV_01)_(JUE)_[!].bin";
		rom = "Virtua_Fighter_2_(F)_[!].bin";
		
		rom = "Michael_Jackson's_Moonwalker_(REV_01)_(JUE)_[!].bin";
		rom = "Street_Fighter_2_Special_Champion_Edition_(U)_[!].bin";
		rom = "Sonic_the_Hedgehog_(JUE)_[!].bin";
		rom = "Sonic_the_Hedgehog_3_(U)_[!].bin";

		m_sega->loadRom(rom);
		m_sega->loadGame(-1);
	}

	virtual void shutdown()
	{
		m_sega->saveGame(-1);
		m_sega->shutdown();
	}

	virtual void update()
	{
		m_sega->update();
	}

	virtual void render()
	{
		CoreEngine::instance()->renderEngine().clearBuffers();
		CoreEngine::instance()->renderEngine().setClearColor(Vector4D(0.0f, 0.0f, 0.0f, 1.0f));

		m_sega->render();
	}

	virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window) { }

private:
	SharedPointer<Sega> m_sega;

};

RENGINE_BOOT();