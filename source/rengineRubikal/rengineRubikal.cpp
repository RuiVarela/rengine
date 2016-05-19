#include <rengine/util/Bootstrap.h>

#include "RubiksCubeRenderer.h"
#include "Algorithm.h"

#ifdef RENGINE_WITH_OPENCV
#include "CubeDetector.h"
#endif //RENGINE_WITH_OPENCV

Vector3D const initial_position(0.0f, 0.0f, 0.7f);
typedef std::list<RubiksCube::CubeMove> MovesList;

namespace rengine
{
	namespace serialization
	{
		template <>
		RENGINE_INLINE void serialize<RubiksCube::CubeFace>(XmlArchive& archive, RubiksCube::CubeFace& cube_face)
		{
			for (unsigned int i = 0; i != 3; ++i)
			{
				for (unsigned int j = 0; j != 3; ++j)
				{
					std::stringstream string_stream;
					string_stream << "coordinate_" << i << "_" << j;

					int color_int = int(cube_face.color[i][j]);
					rengine::serialize(archive, string_stream.str(), color_int);
					cube_face.color[i][j] = RubiksCube::FaceColor(color_int);
				}
			}
		}

	}
}

class MainScene : public Scene, public InterfaceEventHandler, public SystemCommand::Handler
{
public:
	enum Commands
	{
		MoveCommand				= 0,
		ShuffleCommand			= 1,
		SolveCross				= 2,
		SolveCorners        	= 3,
		SolveMiddleLayer    	= 4,
		SolveBottomCross		= 5,
		SolveBottomCorners  	= 6,
		SolveBottomCubeCorners 	= 7,
		SolveBottomEdges	 	= 8,
		SaveCubeFaces			= 9,
		LoadCubeFaces			= 10
	};

	MainScene() {}
	virtual ~MainScene() {}

	virtual void init()
	{
		rubics_cube = new RubiksCubeRenderer();


		camera = new OrbitCamera();
		//camera->setPosition(initial_position);
		CoreEngine::instance()->setCamera(camera);
		camera->connect();


		SharedPointer<SystemVariable> cube_depth = new SystemVariable("cube_depth", 0);
		cube_depth->setDescription("Current cube ring depth");
		CoreEngine::instance()->system().registerVariable(cube_depth);

		SharedPointer<SystemCommand> move_command = new SystemCommand("cubeMove", MoveCommand, this);
		move_command->setDescription("Performs a rubik cube movement");
		CoreEngine::instance()->system().registerCommand(move_command);


		SharedPointer<SystemCommand> shuffle_command = new SystemCommand("cubeShuffle", ShuffleCommand, this);
		shuffle_command->setDescription("Shuffles the cube");
		CoreEngine::instance()->system().registerCommand(shuffle_command);

		SharedPointer<SystemCommand> solve_cross_command = new SystemCommand("cubeSolveCross", SolveCross, this);
		solve_cross_command->setDescription("Solves the top cross");
		CoreEngine::instance()->system().registerCommand(solve_cross_command);

		SharedPointer<SystemCommand> solve_corners_command = new SystemCommand("cubeSolveCorners", SolveCorners, this);
		solve_corners_command->setDescription("Solves the top corners");
		CoreEngine::instance()->system().registerCommand(solve_corners_command);

		SharedPointer<SystemCommand> solve_middle_layer = new SystemCommand("cubeSolveMiddleLayer", SolveMiddleLayer, this);
		solve_corners_command->setDescription("Solves the middle layer");
		CoreEngine::instance()->system().registerCommand(solve_middle_layer);

		SharedPointer<SystemCommand> solve_bottom_cross = new SystemCommand("cubeSolveBottomCross", SolveBottomCross, this);
		solve_bottom_cross->setDescription("Solves the bottom cross");
		CoreEngine::instance()->system().registerCommand(solve_bottom_cross);

		SharedPointer<SystemCommand> solve_bottom_corners = new SystemCommand("cubeSolveBottomCorners", SolveBottomCorners, this);
		solve_bottom_corners->setDescription("Solves the bottom corners");
		CoreEngine::instance()->system().registerCommand(solve_bottom_corners);

		SharedPointer<SystemCommand> solve_bottom_cube_corners = new SystemCommand("cubeSolveBottomCubeCorners", SolveBottomCubeCorners, this);
		solve_bottom_cube_corners->setDescription("Solves the bottom cube corners");
		CoreEngine::instance()->system().registerCommand(solve_bottom_cube_corners);

		SharedPointer<SystemCommand> solve_bottom_edges = new SystemCommand("cubeSolveBottomEdges", SolveBottomEdges, this);
		solve_bottom_edges->setDescription("Solves the bottom edges");
		CoreEngine::instance()->system().registerCommand(solve_bottom_edges);

		SharedPointer<SystemCommand> load_cube_faces = new SystemCommand("cubeLoadFaces", LoadCubeFaces, this);
		load_cube_faces->setDescription("Loads cube faces from xml");
		CoreEngine::instance()->system().registerCommand(load_cube_faces);

		SharedPointer<SystemCommand> save_cube_faces = new SystemCommand("cubeSaveFaces", SaveCubeFaces, this);
		save_cube_faces->setDescription("Saves cube faces to XML");
		CoreEngine::instance()->system().registerCommand(save_cube_faces);


#ifdef RENGINE_WITH_OPENCV
		cube_detector_hook = new CubeDetectorHook(rubics_cube);
#endif //RENGINE_WITH_OPENCV


		CoreEngine::instance()->system()("/runScript data/scripts/rubikal_startup.rss");
		//CoreEngine::instance()->console().setInputBuffer("/cubeMove front 0 1");

		states.setState(new Depth);
		states.setState(new CullFace);
		//states.setState(new PolygonMode(PolygonMode::FrontAndBack, PolygonMode::Line));


		//rubics_cube->serializeTo(std::cout);
	}

	virtual void shutdown()
	{
	}

	virtual void update()
	{
		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;

		float const fovy = 45.0f;
		float const aspect = width / height;
		float const z_near = 0.01f;
		float const z_far = 1000.0f;

		CoreEngine::instance()->camera()->setProjectionAsPerspective(fovy, aspect, z_near, z_far);
		//	CoreEngine::instance()->camera()->setProjectionAsOrtho(0.0f, width, 0.0f, height, -100.0f, 100.0f);

		if ((rubics_cube->state() == RubiksCubeRenderer::Idle) && !moves_fifo.empty())
		{
			RubiksCube::CubeMove move = moves_fifo.front();
			moves_fifo.pop_front();
			rubics_cube->startMove(move.movement, move.depth, move.turns);
		}

#ifdef RENGINE_WITH_OPENCV
		cube_detector_hook->update();
		if (cube_detector_hook->hasDetectedCube())
		{
			CubeDetector::FacesDetected faces_detected = cube_detector_hook->detectedFaces();

			RENGINE_ASSERT(faces_detected.size() == 6);

			RubiksCube::CubeFace face;
			RubiksCube::CubeFaces input;

			for (unsigned int i = 0; i != faces_detected.size(); ++i)
			{
				for (unsigned int j = 0; j != 3; ++j)
				{
					for (unsigned int k = 0; k != 3; ++k)
					{
						//	std::cout << "faces_detected[" << i << "].square[" << j << "][" << k << "] " << faces_detected[i].square[j][k] << std::endl;
						face.color[j][k] = RubiksCube::FaceColor( faces_detected[i].square[j][k] );
					}
				}

				input.push_back(face);
			}

			RENGINE_ASSERT(input.size() == 6);


			RubiksCube::CubeFaces corrected;
			RubiksCubeFaceAligner aligner;

			if (aligner.makeCubeFromFacesThreaded(input, corrected) )
			{
				CoreEngine::instance()->log() << "Cube mirrored successfully" << std::endl;

				// the the detected faces
				rubics_cube->setSetup(corrected);
				// update renderer geometry
				rubics_cube->synchGeometry();

			}
			else
			{
				CoreEngine::instance()->log() << "Unable to mirror cube, invalid faces! " << std::endl;
			}





			cube_detector_hook->SetOff();
		}
#endif //RENGINE_WITH_OPENCV

		rubics_cube->update( float(CoreEngine::instance()->frameGlobalTime()) );
	}


	virtual void render()
	{
		CoreEngine::instance()->renderEngine().clearBuffers();

		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;
		CoreEngine::instance()->renderEngine().setViewport(0, 0, width, height);

		CoreEngine::instance()->renderEngine().pushDrawStates();
		//	CoreEngine::instance()->renderEngine().reportDrawStates();

		CoreEngine::instance()->renderEngine().apply(states);
		CoreEngine::instance()->renderEngine().draw(*rubics_cube);

#ifdef RENGINE_WITH_OPENCV
		cube_detector_hook->render( CoreEngine::instance()->renderEngine() );
#endif //RENGINE_WITH_OPENCV


		CoreEngine::instance()->renderEngine().popDrawStates();
	}

	virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window)
	{
		if (interface_event.eventType() == InterfaceEvent::EventKeyDown)
		{
			if (interface_event.key() == InterfaceEvent::KeySpace)
			{
				//			camera->setPosition(initial_position);
				//			camera->setPitch(0.0f);
				//			camera->setYaw(0.0f);
				//			camera->setRoll(0.0f);
			}
			else if (interface_event.key() == 'p')
			{
				togglePolygonMode();
			}
		}
	}

	virtual void operator()(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
	{
		if (command == MoveCommand)
		{
			handleMoveCommand(command, arguments);
		}
		else if (command == SaveCubeFaces)
		{
			saveCubeFaces(command, arguments);
		}
		else if (command == LoadCubeFaces)
		{
			loadCubeFaces(command, arguments);
		}
		else
		{
			bool do_optimization = true;
			SharedPointer<RubiksCubeAlgorithm> algorithm;

			if (command == ShuffleCommand)
			{
				do_optimization = false;
				int turns = parseCommand(command, arguments);

				if (turns)
				{
					algorithm = new RubiksCubeShuffle(turns);
				}
			}
			else if (command == SolveCross)
			{
				algorithm = new RubiksCubeCross();
			}
			else if (command == SolveCorners)
			{
				algorithm = new RubiksCubeCorners();
			}
			else if (command == SolveMiddleLayer)
			{
				algorithm = new RubiksCubeMiddleLayer();
			}
			else if (command == SolveBottomCross)
			{
				algorithm = new RubiksCubeBottomCross();
			}
			else if (command == SolveBottomCorners)
			{
				algorithm = new RubiksCubeBottomCorners();
			}
			else if (command == SolveBottomCubeCorners)
			{
				algorithm = new RubiksCubeBottomCubeCorners();
			}
			else if (command == SolveBottomEdges)
			{
				algorithm = new RubiksCubeBottomEdges();
			}

			if (algorithm)
			{
				RubiksCube::CubeMoves moves = algorithm->operator()( *rubics_cube );
				RubiksCube::CubeMoves optimized_moves = moves;

				if (do_optimization)
				{
					RubiksCubeOptimizer optimizer;
					optimized_moves = optimizer(moves);
				}

				unsigned int removed_moves = (unsigned int)(moves.size() - optimized_moves.size());
				CoreEngine::instance()->log() << "Cube Algorithm applied [" << optimized_moves.size() << "] moves. [" << removed_moves << "] moves removed due optimization." << std::endl;

				std::copy(optimized_moves.begin(), optimized_moves.end(), std::back_inserter(moves_fifo));
			}
		}
	}




	void saveCubeFaces(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
	{
		string syntax_error;
		string filename;
		bool show_syntax = false;

		if ((arguments.size() == 1) && (arguments[0]->type() == Variable::StringType))
		{
			filename = arguments[0]->asString();
		}
		else
		{
			show_syntax = true;
			syntax_error = "Invalid filename";
		}

		if (show_syntax)
		{
			CoreEngine::instance()->system().echo(arguments);
			CoreEngine::instance()->log() << "Bad Syntax : " << syntax_error << std::endl;
			CoreEngine::instance()->log() << "Arguments : <filename>" << std::endl;
		}
		else
		{
			//
			// Read faces fom cube
			//
			RubiksCube::CubeFaces faces;
			for (unsigned int f = 0; f != 6; ++f)
			{
				RubiksCube::CubeFace face;

				for (unsigned int x = 0; x != rubics_cube->getCubeLength(); ++x)
				{
					for (unsigned int y = 0; y != rubics_cube->getCubeLength(); ++y)
					{
						face.color[x][y] = rubics_cube->cell(f, x, y);
					}
				}

				faces.push_back(face);
			}

			//
			// save to file
			//
			std::string error_msg;
			try
			{
				XmlArchive archive;
				if (archive.open(filename, XmlArchive::Write))
				{
					XML_SERIALIZE(archive, faces);
					if (!archive.save())
					{
						error_msg = "Unable to save file : " + filename;
					}
				}
				else
				{
					error_msg = "Unable to open serializer for file : " + filename;
				}
			}
			catch (Exception caught)
			{
				error_msg = caught.message();
			}

			if (error_msg.empty())
			{
				error_msg = "Cube saved to " + filename;

			}

			CoreEngine::instance()->log() << error_msg << std::endl;
		}
	}


	void loadCubeFaces(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
	{
		string syntax_error;
		string filename;
		bool show_syntax = false;

		if ((arguments.size() == 1) && (arguments[0]->type() == Variable::StringType))
		{
			filename = arguments[0]->asString();
		}
		else
		{
			show_syntax = true;
			syntax_error = "Invalid filename";
		}

		if (show_syntax)
		{
			CoreEngine::instance()->system().echo(arguments);
			CoreEngine::instance()->log() << "Bad Syntax : " << syntax_error << std::endl;
			CoreEngine::instance()->log() << "Arguments : <filename>" << std::endl;
		}
		else
		{
			//
			// Read faces fom cube
			//
			RubiksCube::CubeFaces faces;

			//
			// save to file
			//
			std::string error_msg;
			try
			{
				XmlArchive archive;
				if (archive.open(filename))
				{
					XML_SERIALIZE(archive, faces);
				}
				else
				{
					error_msg = "Unable to open serializer for file : " + filename;
				}
			}
			catch (Exception caught)
			{
				error_msg = caught.message();
			}

			if (error_msg.empty())
			{
				RubiksCubeFaceAligner aligner;


				//			rengine::Timer timer;
				//			timer.start();
				//			RubiksCube::CubeFaces corrected;
				//			if (aligner.makeCubeFromFacesThreaded(faces, corrected))
				//			{
				//				CoreEngine::instance()->log() << "Found [" << timer.elapsedTime() << "]" << std::endl;
				//			}
				//			else
				//			{
				//				CoreEngine::instance()->log() << "Failed [" << timer.elapsedTime() << "]" << std::endl;
				//			}

				if ( aligner.isValidCube(faces) )
				{
					rubics_cube->setSetup(faces);

					// update renderer geometry
					rubics_cube->synchGeometry();

					error_msg = "Cube loaded from " + filename;
				}
				else
				{
					error_msg = "Unable to load cube from " + filename + " : invalid faces";
				}
			}

			CoreEngine::instance()->log() << error_msg << std::endl;
		}
	}

	void handleMoveCommand(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
	{
		string syntax_error;
		string rubik_command;

		int depth = 0;
		int turns = 1;

		bool show_syntax = false;

		if ((arguments.size() > 0) && (arguments[0]->type() == Variable::StringType))
		{
			rubik_command = arguments[0]->asString();
		}
		else
		{
			show_syntax = true;
			syntax_error = "Invalid rubic command type";
		}

		if ((rubik_command != "front") &&
			(rubik_command != "right") &&
			(rubik_command != "up") )
		{
			show_syntax = true;
			syntax_error = "Rubik command not supported";
		}

		if (!show_syntax && (arguments.size() > 1))
		{
			if (arguments[1]->type() == Variable::IntType)
			{
				depth = arguments[1]->asInt();
			}
			else
			{
				show_syntax = true;
				syntax_error = "Rubik command not supported";
			}
		}

		if (!show_syntax && (arguments.size() > 2))
		{
			if (arguments[2]->type() == Variable::IntType)
			{
				turns = arguments[2]->asInt();
			}
			else
			{
				show_syntax = true;
				syntax_error = "Invalid turns";
			}
		}

		if ((depth < 0) || ((unsigned int)(depth) >= rubics_cube->getCubeLength()))
		{
			show_syntax = true;
			syntax_error = "Invalid depth, must be lower than " + lexical_cast<string>(rubics_cube->getCubeLength());
		}

		if (show_syntax)
		{
			CoreEngine::instance()->system().echo(arguments);
			CoreEngine::instance()->log() << "Bad Syntax : " << syntax_error << std::endl;
			CoreEngine::instance()->log() << "Arguments : <rubic command> <depth> <turns>" << std::endl;
			CoreEngine::instance()->log() << "<rubic command> allowed values : front, right, up" << std::endl;
		}
		else
		{
			RubiksCube::Movement movement_type = RubiksCube::Front;
			if (rubik_command == "front") { movement_type = RubiksCube::Front; }
			else if (rubik_command == "right") { movement_type = RubiksCube::Right; }
			else if (rubik_command == "up") { movement_type = RubiksCube::Up; }

			RubiksCube::CubeMove move;
			move.movement = movement_type;
			move.depth = depth;
			move.turns = turns;

			moves_fifo.push_back(move);
		}
	}

	int parseCommand(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
	{
		string syntax_error;
		bool show_syntax = false;

		int turns = 15;

		if ((arguments.size() > 0) && (arguments[0]->type() == Variable::IntType))
		{
			if (turns > 0)
			{
				turns = arguments[0]->asInt();
			}
			else
			{
				show_syntax = true;
				syntax_error = "Turns must be a positive number";
			}
		}
		else if (arguments.size() > 0)
		{
			show_syntax = true;
			syntax_error = "Invalid turns.";
		}

		if (show_syntax)
		{
			CoreEngine::instance()->system().echo(arguments);
			CoreEngine::instance()->log() << "Bad Syntax : " << syntax_error << std::endl;
			CoreEngine::instance()->log() << "Arguments : <turns>" << std::endl;

			turns = 0;
		}

		return turns;
	}

	void togglePolygonMode()
	{
		if (!states.hasState(DrawStates::PolygonMode))
		{
			states.setState(new PolygonMode(PolygonMode::FrontAndBack, PolygonMode::Fill));
		}

		PolygonMode* polygon_mode = dynamic_cast<PolygonMode*> (states.getState(DrawStates::PolygonMode).first.get());
		switch (polygon_mode->getMode(PolygonMode::FrontAndBack))
		{
		case PolygonMode::Fill:
			{
				polygon_mode->setMode(PolygonMode::FrontAndBack, PolygonMode::Line);
				break;
			}
		case PolygonMode::Line:
			{
				polygon_mode->setMode(PolygonMode::FrontAndBack, PolygonMode::Point);
				break;
			}
		case PolygonMode::Point:
			{
				polygon_mode->setMode(PolygonMode::FrontAndBack, PolygonMode::Fill);
				break;
			}
		}
	}

private:
	DrawStates states;
	SharedPointer<OrbitCamera> camera;

	SharedPointer<RubiksCubeRenderer> rubics_cube;
	MovesList moves_fifo;

#ifdef RENGINE_WITH_OPENCV
	SharedPointer<CubeDetectorHook> cube_detector_hook;
#endif //RENGINE_WITH_OPENCV
};

RENGINE_BOOT();
