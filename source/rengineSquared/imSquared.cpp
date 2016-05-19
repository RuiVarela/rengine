#include "imSquared.h"

#include <sstream>
#include <iomanip>
#include <cmath>

#include "BitmapLoader.h"


#define PROP_TEST(name, code) else if (startsWith(line, #name ":")) { replace(line, #name ":", ""); trim(line); code; }
#define PROP_TEST_INT(obj, name) PROP_TEST(name, obj.name = lcast<int>(line, 0) )
#define PROP_TEST_FLT(obj, name) PROP_TEST(name, obj.name = lcast<float>(line, 0) )
#define PROP_TEST_STR(obj, name) PROP_TEST(name, obj.name = line )    

//#ifdef RENGINE_PLATFORM
//
//    #define COMBINE_(expression_0, expression_1) expression_0##expression_1
//    #define COMBINE(expression_0, expression_1) COMBINE_(expression_0, expression_1)
//
//
//	//#define PROP_TEST_INT(obj, name) PROP_TEST(name, COMBINE(obj., name) = lcast<int>(line, 0) )
// //   #define PROP_TEST_FLT(obj, name) PROP_TEST(name, COMBINE(obj., name) = lcast<float>(line, 0) )
// //   #define PROP_TEST_STR(obj, name) PROP_TEST(name, COMBINE(obj., name) = line )
//
//	#define PROP_TEST_INT(obj, name) PROP_TEST(name, obj.name = lcast<int>(line, 0) )
//    #define PROP_TEST_FLT(obj, name) PROP_TEST(name, obj.name = lcast<float>(line, 0) )
//    #define PROP_TEST_STR(obj, name) PROP_TEST(name, obj.name = line )
//
//#else
//
//
//
//#endif 

 

//
// math helpers
//

template<typename T>
 T minimum(T const left, T const right) { return ((left < right) ? left: right); }

template<typename T>
T maximum(T const left, T const right) { return ((left > right) ? left : right); }

//
// String Helpers
//

static std::vector<std::string> split(std::string const& input, std::string const& delimiter)
{
    std::vector<std::string> output;
    
    if (input.size() != 0)
    {
        std::string::size_type start = 0;
        
        do
        {
            std::string::size_type end = input.find(delimiter, start);
            if (end == std::string::npos)
            {
                end = input.size();
            }
            
            std::string selection = input.substr(start, end - start);
            if (selection != "")
            {
                output.push_back(selection);
            }
            
            start = end + delimiter.size();
            
        }
        while (start < input.size());
        
    }
    return output;
}


template<typename OutputType, typename InputType>
OutputType lcast(InputType const& value, OutputType const& default_value)
{
	std::stringstream stream;
	//skip leading whitespace characters on input
	stream.unsetf(std::ios::skipws);

	//set the correct precision
	if (std::numeric_limits<OutputType>::is_specialized)
	{
		stream.precision(std::numeric_limits<OutputType>::digits10 + 1);
	}

	stream << value;

	OutputType output;
	stream >> output;

	if (!stream)
	{
		output = default_value;
	}

	return output;
}

static void replace(std::string &target, std::string const& match, std::string const& replace_string)
{
	std::string::size_type position = target.find(match);

	while (position != std::string::npos)
	{
		target.replace(position, match.size(), replace_string);
		position = target.find(match, position + replace_string.size());
	}
}

static bool startsWith(std::string const& string, std::string const& prefix)
{
	return string.find(prefix) == std::string::size_type(0);
}

static void trimLeft(std::string &string)
{
	std::string whitespaces(" \t\f\v\n\r");
	std::string::size_type pos = string.find_first_not_of(whitespaces);

	if (pos != std::string::npos)
	{
		string = string.substr(pos);
		//string.erase(0, pos);
	}
	else
	{
		string.clear();
	}
}

static void trimRight(std::string &string)
{
	std::string whitespaces(" \t\f\v\n\r");
	std::string::size_type pos = string.find_last_not_of(whitespaces);

	if (pos != std::string::npos)
	{
		string.erase(pos + 1);
	}
	else
	{
		string.clear();
	}
}

static void trim(std::string &string)
{
	trimLeft(string);
	trimRight(string);
}


//
// Texture Loader
//

int LoadTexture(std::string const& name)
{
	GLuint TexID = 0;

	char* fileData = 0;
	int fileDataSize = OS_ReadFileBinary(name.c_str(), &fileData);

	if (!fileData || !fileDataSize) return 0;
	
	CBitmap bitmap;
	if (bitmap.Load(fileData))
	{
		unsigned int width = bitmap.GetWidth(); 
		unsigned int height = bitmap.GetHeight(); 
		void* data = bitmap.GetBits();

		
		// Create Texture
		glGenTextures(1,&TexID);
		glBindTexture(GL_TEXTURE_2D,TexID);
		
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}


	OS_Free(fileData);

	return TexID;
}

//
// imsquared
//

imSquared::Figure::Figure()
{
	markedCount = 0;
	width = 0;
	height = 0;
	score = 10;
	hits = 0;
	color.red = 0.0f;
	color.green = 0.0f;
	color.blue = 1.0f;
	color.alpha = 1.0f;
	sound = "default sound";
}

#define FLT_N_INIT -100012.0

imSquared::Level::Level()
{
	for (int i = 0; i != max_hardness; ++i)
	{
		speed[i] = FLT_N_INIT;
		clear_factor[i] = FLT_N_INIT;
		speed_increment_per_second[i] = FLT_N_INIT;
		score_factor[i] = FLT_N_INIT;
		max_expected_score[i] = 0;
		max_expected_score_with_multiplier[i] = 0;
		bonus[i] = 0;
	}

	
	speed_max = 0.1;
	total_figures = 0;

}

imSquared::imSquared()
{
	m_textureId = 0;
	OS_Initialize();
}

imSquared::~imSquared()
{
	if (m_textureId) glDeleteTextures(1, &m_textureId);

	m_matrix.clear();

	OS_Finalize();
}

int imSquared::score()
{
    return m_score;
}

bool imSquared::ended()
{
	return m_ended;
}

bool imSquared::completed()
{
    return (m_score > (m_levels[m_currentLevel].clear_factor[m_configuration.hardness] * m_levels[m_currentLevel].max_expected_score[m_configuration.hardness]));
}

int imSquared::indexFromFigureName(std::string const& name)
{
	for (int i = 0; i != m_figures.size(); ++i)
	{
		if (m_figures[i].name == name)
			return i;
	}

	return -1;
}

int imSquared::indexFromLevelName(std::string const& name)
{
	for (int i = 0; i != m_levels.size(); ++i)
	{
		if (m_levels[i].name == name)
			return i;
	}

	return -1;
}

void imSquared::loadDatabase()
{
	//
	// load figures
	//
	{
		char *data = 0;
		OS_ReadFileBinary("square.fig", &data);	

		std::stringstream buffer;
		if (data)
		{
			buffer << data;
			OS_Free(data);

			int emit_count = 1;
			Figure figure;

			std::string line;
			while (!std::getline(buffer, line).eof())
			{
				trim(line);
				if(line.empty()) continue;

				if (line == "#")
				{
					m_figures.push_back(figure);
					figure = Figure();
					emit_count = 1;
				}
				PROP_TEST_STR(figure, name)
				PROP_TEST_STR(figure, sound)
				PROP_TEST_INT(figure, score)
				PROP_TEST_INT(figure, hits)
				PROP_TEST(emit_count, emit_count = lcast<int>(line, 1))
				PROP_TEST(color, 
				{
					std::vector<std::string> splitted = split(line, " ");
					if (splitted.size() == 4)
					{
						figure.color.red = lcast<int>(splitted[0], 0) / 255.0f;
						figure.color.green = lcast<int>(splitted[1], 0) / 255.0f;
						figure.color.blue = lcast<int>(splitted[2], 0) / 255.0f;
						figure.color.alpha = lcast<int>(splitted[3], 0) / 255.0f;
					}
				})
				else
				{
					for (int i = 0; i != emit_count; ++i)
						figure.lines.push_back(line);
				}
			}
		}
		else
		{
			DPrintf("Unable to load figure file");
		}
	}

	for (Figures::iterator figure = m_figures.begin(); figure != m_figures.end(); ++figure)
	{
		figure->markedCount = 0;
		figure->width = 0;
		figure->height = figure->lines.size();

		for (FigureLines::iterator line = figure->lines.begin(); line != figure->lines.end(); ++line)
		{
			if ( (int)line->size() > figure->width)
				figure->width = line->size();

			for (int x = 0; x < figure->width; x++)
			{
				if ((*line)[x] == 'X')
					figure->markedCount++;
			}
		}
	}

	//
	// debug
	//
	DPrintf("************** FIGURES **************\n");
	for (Figures::iterator figure = m_figures.begin(); figure != m_figures.end(); ++figure)
	{
		DPrintf("****************************\n");
		DPrintf("name: %s\n", figure->name.c_str());
		DPrintf("width: %d\n", figure->width);
		DPrintf("height: %d\n",figure->height);
		DPrintf("score: %d\n", figure->score);
		DPrintf("hits: %d\n", figure->hits);
		DPrintf("color: %.2f %.2f %.2f %.2f\n", figure->color.red, figure->color.green, figure->color.blue, figure->color.alpha);
		DPrintf("sound: %s\n", figure->sound.c_str());
		DPrintf("markedCount: %d\n",figure->markedCount);

		for (FigureLines::iterator line = figure->lines.begin(); line != figure->lines.end(); ++line)
			DPrintf("[%s]\n", line->c_str());
	}


	//
	// load levels
	// 
	{
		char *data = 0;
		OS_ReadFileBinary("square.lvl", &data);	

		std::stringstream buffer;
		if (data)
		{
			buffer << data;
			OS_Free(data);


			Level level;

			std::string line;

			while (!std::getline(buffer, line).eof())
			{
				trim(line);
				if(line.empty()) continue;

				if (line == "#")
				{
					m_levels.push_back(level);
					level = Level();
				}
				PROP_TEST_STR(level, name)
				PROP_TEST_STR(level, type)
				PROP_TEST_INT(level, figure_spacing)
				PROP_TEST_INT(level, bonus[0])
				PROP_TEST_INT(level, bonus[1])
				PROP_TEST_INT(level, bonus[2])
				PROP_TEST_FLT(level, score_factor[0])
				PROP_TEST_FLT(level, score_factor[1])
				PROP_TEST_FLT(level, score_factor[2])
				PROP_TEST_FLT(level, speed[0])
				PROP_TEST_FLT(level, speed[1])
				PROP_TEST_FLT(level, speed[2])
				PROP_TEST_FLT(level, clear_factor[0])
				PROP_TEST_FLT(level, clear_factor[1])
				PROP_TEST_FLT(level, clear_factor[2])
				PROP_TEST_FLT(level, speed_increment_per_second[0])
				PROP_TEST_FLT(level, speed_increment_per_second[1])
				PROP_TEST_FLT(level, speed_increment_per_second[2])
				PROP_TEST_FLT(level, speed_max)
				PROP_TEST_INT(level, total_figures)
				PROP_TEST(figure, {
					int figureIndex = indexFromFigureName(line);
					if (figureIndex == -1) 
						EPrintf("Figure not found [%s] for level!", line.c_str());
					else
						level.figures.push_back(figureIndex);
				})


			}
		}

		for (Levels::iterator level = m_levels.begin(); level != m_levels.end(); ++level)
		{
			if (level->type != "random puzzle")
			{
				level->total_figures = level->figures.size();
			}

			if (level->type == "continuous")
			{
				level->figure_spacing = 0;
			}

			
			//
			// set default values
			//
			if (level->score_factor[0] == FLT_N_INIT) level->score_factor[0] = 1.0;
			if (level->score_factor[1] == FLT_N_INIT) level->score_factor[1] = level->score_factor[0];
			if (level->score_factor[2] == FLT_N_INIT) level->score_factor[2] = level->score_factor[1];

			if (level->speed[0] == FLT_N_INIT) level->speed[0] = 1.0;
			if (level->speed[1] == FLT_N_INIT) level->speed[1] = level->speed[0];
			if (level->speed[2] == FLT_N_INIT) level->speed[2] = level->speed[1];

			if (level->speed_increment_per_second[0] == FLT_N_INIT) level->speed_increment_per_second[0] = 0.0;
			if (level->speed_increment_per_second[1] == FLT_N_INIT) level->speed_increment_per_second[1] = level->speed_increment_per_second[0];
			if (level->speed_increment_per_second[2] == FLT_N_INIT) level->speed_increment_per_second[2] = level->speed_increment_per_second[1];

			if (level->clear_factor[0] == FLT_N_INIT) level->clear_factor[0] = 0.5;
			if (level->clear_factor[1] == FLT_N_INIT) level->clear_factor[1] = level->clear_factor[0];
			if (level->clear_factor[2] == FLT_N_INIT) level->clear_factor[2] = level->clear_factor[1];

			if (level->bonus[0] == FLT_N_INIT) level->bonus[0] = 0.5;
			if (level->bonus[1] == FLT_N_INIT) level->bonus[1] = level->bonus[0];
			if (level->bonus[2] == FLT_N_INIT) level->bonus[2] = level->bonus[1];


			//
			// compute max expected scores
			//
			int multiplier = 1;
			for (int hard = 0; hard != max_hardness; ++hard)
			{
				level->max_expected_score[hard] = 0;
				level->max_expected_score_with_multiplier[hard] = 0;
			}
			
			for (FigureIndex::iterator figure = level->figures.begin(); figure != level->figures.end(); ++figure)
			{
				for (int hard = 0; hard != max_hardness; ++hard)
				{
					level->max_expected_score[hard] += 
													  (float)m_figures[*figure].markedCount * 
													  (float)m_figures[*figure].score * 
												      (float)level->score_factor[hard];

					level->max_expected_score_with_multiplier[hard] += 
													  (float)m_figures[*figure].markedCount * 
													  (float)m_figures[*figure].score * 
												      (float)level->score_factor[hard] * 
													  (float)multiplier;
				}
				multiplier++;
			}
		}

		
		//
		// debug
		//
		DPrintf("************** LEVELS **************\n");
		for (Levels::iterator level = m_levels.begin(); level != m_levels.end(); ++level)
		{
			DPrintf("****************************\n");
			DPrintf("name: %s\n", level->name.c_str());
			DPrintf("type: %s\n", level->type.c_str());
			DPrintf("figure_spacing: %d\n", level->figure_spacing);
			DPrintf("speed: %.3f %.3f %.3f\n", level->speed[0], level->speed[1], level->speed[2]);
			DPrintf("speed_increment_per_second: %.3f %.3f %.3f\n", level->speed_increment_per_second[0], level->speed_increment_per_second[1], level->speed_increment_per_second[2]);
			DPrintf("speed_max: %.3f\n", level->speed_max);
			DPrintf("total_figures: %d\n", level->total_figures);
			DPrintf("clear_factor: %.3f %.3f %.3f\n", level->clear_factor[0], level->clear_factor[1], level->clear_factor[2]);
			DPrintf("score_factor: %.3f %.3f %.3f\n", level->score_factor[0], level->score_factor[1], level->score_factor[2]);
			DPrintf("bonus: %.3d %.3d %.3d\n", level->bonus[0], level->bonus[1], level->bonus[2]);
			DPrintf("max_expected_score: %d %d %d\n", level->max_expected_score[0], level->max_expected_score[1], level->max_expected_score[2]);
			DPrintf("max_expected_score_with_multiplier: %d %d %d\n", level->max_expected_score_with_multiplier[0], level->max_expected_score_with_multiplier[1], level->max_expected_score_with_multiplier[2]);

			for (FigureIndex::iterator index = level->figures.begin(); index != level->figures.end(); ++index)
			{
				if (*index != -1)
					DPrintf("figure [%d %s]\n", *index, m_figures[*index].name.c_str());
			}
		}
	}
}

void imSquared::shutdown()
{
	m_touches.clear();

	m_figureGeneration = -1;
	m_currentFigure = -1;
	m_currentFigureLine = 0;
	m_currentFigureOffset = 0;
	m_currentLevel = 0;

	m_multiplier = 1;
	m_score = 0;

	m_textureId = 0;
	m_font.Unload();

	m_figures.clear();
	m_figuresOnBoard.clear();
	m_levels.clear();
}

void imSquared::reboot()
{
	Configuration configuration = m_configuration;
		m_menu.Hide();
	if (m_configuration.menus_enabled)
	{
		configuration.level = "";
	}


	initialize(configuration);
}

void imSquared::initialize(Configuration const& configuration)
{
	shutdown();

    m_ended = false;
	m_configuration = configuration;

	//
	// Load Figure Database
	//
	loadDatabase();

	//
	// Load font
	//
	m_font.Load("font.bff");
	
	m_currentLevel = indexFromLevelName(m_configuration.level);
	
	//if (m_currentLevel == -1)
	//{
	//	DPrintf("Invalid level %s", m_configuration.level.c_str());
	//	m_levels.clear();
	//	m_figures.clear();
	//	return;
	//}

	
	//
	// Show menu
	//
	if ((m_currentLevel == -1) && (m_configuration.menus_enabled))
	{
		ShowMainMenu();
		return;
	}



	m_speed = m_levels[m_currentLevel].speed[m_configuration.hardness];



	//
	// Setup Geometry
	//
	m_matrix.clear();

    m_translation = 0.0;
	m_lastUpdate = OS_ElapsedTime();
	    
	m_configuration.squareWidth = (float)m_configuration.screenWidth / (float)m_configuration.columns;
    m_configuration.squareWidth = ((float)m_configuration.squareWidth / (float)m_configuration.screenWidth) * 2.0;
    
	m_configuration.squareHeight = (float)m_configuration.screenHeight / (float)(m_configuration.rows - 1);
    m_configuration.squareHeight = ((float)m_configuration.squareHeight / (float)m_configuration.screenHeight) * 2.0;
       
    
	//
	// Compute Square positions
	//
	for (int currentLine = 0; currentLine != m_configuration.rows; ++currentLine)
	{
		SquareElements row;
		for (int currentColumn = 0; currentColumn != m_configuration.columns; ++currentColumn)
        {
			SquareElement cell;

			if (m_configuration.texture_enabled)
			{
				float color_factor = 1.0;

				cell.color.red = color_factor;
				cell.color.green = color_factor;
				cell.color.blue = color_factor;
				cell.color.alpha = 1.0f;
			}
			else
			{
				cell.color.red = (0.5f + sin((float)m_lastUpdate - currentLine * m_speed) * 0.3f);
				cell.color.green = cell.color.red;
				cell.color.blue = cell.color.red;
				cell.color.alpha = 1.0f;
			}

			cell.texture = 0;


			cell.state = Idle;
			cell.figure = -1;
			cell.hasPiece = false;
            cell.vertices[0].x = -1.0f + currentColumn * m_configuration.squareWidth;
            cell.vertices[0].y = -1.0f + (currentLine - 1) * m_configuration.squareHeight;
            
            cell.vertices[1].x = -1.0f + (currentColumn + 1) * m_configuration.squareWidth;
            cell.vertices[1].y = -1.0f + (currentLine - 1) * m_configuration.squareHeight;

            cell.vertices[2].x = -1.0f + currentColumn * m_configuration.squareWidth;
            cell.vertices[2].y = -1.0f + currentLine * m_configuration.squareHeight;

            cell.vertices[3].x = -1.0f + (currentColumn + 1) * m_configuration.squareWidth;
            cell.vertices[3].y = -1.0f + currentLine * m_configuration.squareHeight;

			
			row.push_back(cell);
			
        }

		m_matrix.push_back(row);
	}

	//
	// Compute Lines
	//
	m_lines.clear();
	for (int currentLine = 0; currentLine != (m_configuration.rows + 1); ++currentLine)
	{
		Line line;
		line.start.x = -1.0f;
		line.start.y = -1.0f + (currentLine - 1) * m_configuration.squareHeight;
		line.end.x = 1.0f;
		line.end.y = line.start.y;
		m_lines.push_back(line);
	}

	for (int currentColumn = 0; currentColumn != (m_configuration.columns + 1); ++currentColumn)
	{
		Line line;
		line.start.x = -1.0f + currentColumn * m_configuration.squareWidth;
		line.start.y = -1.0f - m_configuration.squareHeight;
		line.end.x = line.start.x;
		line.end.y = 1.0f;
		m_lines.push_back(line);
	}
    


	//
	// GL Setup
	//
	glDisable(GL_BLEND);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
	//glClearDepth(1.0f);								// TODO: check on GLES
	glDisable(GL_DEPTH_TEST);						

	glViewport(0, 0, configuration.screenWidth, configuration.screenHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//
	// Load textures
	//
	m_textureId = LoadTexture("square.bmp");
}

void imSquared::applyColors()
{
	for (SquareMatrix::iterator row = m_matrix.begin(); row != m_matrix.end(); ++row)
	{
		for (SquareElements::iterator cell = row->begin(); cell != row->end(); ++cell)
		{


			if (cell->state == Idle)
			{
				cell->texture = m_textureId;
			}
			else if (cell->state == Marked)
			{
				cell->color = cell->markedColor;
			}
			else if (cell->state == Hit)
			{
				cell->color.red = 0.0f;
				cell->color.green = 1.0f;
				cell->color.blue = 0.0f;
				cell->color.alpha = 1.0f;
			}
			else if (cell->state == Miss)
			{
				cell->color.red = 1.0f;
				cell->color.green = 0.0f;
				cell->color.blue = 0.0f;
				cell->color.alpha = 1.0f;
			}
		}
	}
}

void imSquared::update()
{
	if (!UpdateLevelChaining())
	{
		return;
	}

	if (m_menu.activated())
	{
		m_menu.update(this);
		return;
	}


	if (m_levels.empty()) return;

	double currentTime = OS_ElapsedTime();
	double updateDelta = currentTime - m_lastUpdate;
	
	m_translation += (updateDelta / m_speed) * m_configuration.squareHeight;
	//DPrintf("m_translation %0.5f\n", m_translation);

	if (m_translation > m_configuration.squareHeight)
	{
		processLeavingFigures();

		for (int currentColumn = 0; currentColumn != m_configuration.columns; ++currentColumn)
		{
			for (int currentLine = (m_configuration.rows - 1); currentLine >= 0; --currentLine)
			{
				if (currentLine > 0)
				{
					Point vertices[4];
					memcpy(vertices, m_matrix[currentLine][currentColumn].vertices, sizeof(vertices));
					memcpy( &(m_matrix[currentLine][currentColumn]), &(m_matrix[currentLine - 1][currentColumn]), sizeof(SquareElement));
					memcpy(m_matrix[currentLine][currentColumn].vertices, vertices, sizeof(vertices));
				}
				else
				{
					m_matrix[currentLine][currentColumn].figureGeneration = -1;
					m_matrix[currentLine][currentColumn].state = Idle;
					m_matrix[currentLine][currentColumn].hasPiece = false;

					if (m_configuration.texture_enabled)
					{
						float color_factor = 1.0f;
						m_matrix[currentLine][currentColumn].color.red = color_factor;
						m_matrix[currentLine][currentColumn].color.green = color_factor;
						m_matrix[currentLine][currentColumn].color.blue = color_factor;
					
					}
					else
					{
						m_matrix[currentLine][currentColumn].color.red = (0.5f + sin((float)currentTime) * 0.3f);
						m_matrix[currentLine][currentColumn].color.green = m_matrix[currentLine][currentColumn].color.red;
						m_matrix[currentLine][currentColumn].color.blue = m_matrix[currentLine][currentColumn].color.red;
					}
				}
			}
		}

		while (m_translation > m_configuration.squareHeight)
			m_translation -= m_configuration.squareHeight;

		processFigures();
	}

	m_speed += updateDelta * m_levels[m_currentLevel].speed_increment_per_second[m_configuration.hardness];
	if (m_speed < m_levels[m_currentLevel].speed_max) 
		m_speed = m_levels[m_currentLevel].speed_max;

	m_lastUpdate = OS_ElapsedTime();
	processTouches();
	applyColors();
    
    if ((!m_levels.empty() && (m_figureGeneration > m_levels[m_currentLevel].total_figures) && m_figuresOnBoard.empty()))
    {
        m_ended = true;
        m_score += m_levels[m_currentLevel].bonus[m_configuration.hardness];
    }
}

bool imSquared::pointHitSquare(Point const& point, SquareElement const& square)
{
	// Test point againt corner 1 and corner 4

	return (
		(point.x > square.vertices[0].x) &&
		(point.y > (square.vertices[0].y + m_translation)) &&
		(point.x < square.vertices[3].x) &&
		(point.y < (square.vertices[3].y + m_translation))
		);
}

void imSquared::processTouches()
{
	if (m_levels.empty()) return;

	//
	// check hits
	//
	for (Points::const_iterator i = m_touches.begin(); i != m_touches.end(); ++i)
		for (SquareMatrix::iterator row = m_matrix.begin(); row != m_matrix.end(); ++row)
			for (SquareElements::iterator cell = row->begin(); cell != row->end(); ++cell)
			{
				if (cell->state == Idle)
				{
					if (pointHitSquare(*i, *cell))
					{
						cell->state = Miss;

						if (cell->figure >= 0)
						{
							int computed = (float)m_figures[cell->figure].score * 
										   (float)m_levels[m_currentLevel].score_factor[m_configuration.hardness] * 
										   (float)m_configuration.missScoreFactor;
							m_score += computed;
							DPrintf("Miss Score: %d\n", computed);
							m_multiplier = 1;

							for(FiguresOnBoard::iterator iterator = m_figuresOnBoard.begin(); iterator != m_figuresOnBoard.end(); ++iterator)
								iterator->multiplied = true;
						}
					}
				}
				else if (cell->state == Marked)
				{
					if (pointHitSquare(*i, *cell))
					{
						cell->state = Hit;

						addHitToFigureOnBoard(cell->figureGeneration, 1);
						if (cell->figure >= 0)
						{
							int computed = (float)m_figures[cell->figure].score * 
										   (float)m_levels[m_currentLevel].score_factor[m_configuration.hardness] * 
										   (float)m_multiplier;
							m_score += computed;
							DPrintf("Hit Score: %d %d\n", m_multiplier, computed);
						}
					}
				}
			}


	//
	// check multiplicators
	//

	for(FiguresOnBoard::iterator iterator = m_figuresOnBoard.begin(); iterator != m_figuresOnBoard.end(); ++iterator)
	{
		if ((iterator->hitCount == iterator->markedCount) && !iterator->multiplied)
		{
			m_multiplier++;
			iterator->multiplied = true;
		}
	}
}

void imSquared::setTouches(Points const& touches)
{
	m_touches = touches;

	if (m_menu.activated())
	{
		m_menu.Click(this);
		return;
	}

	processTouches();
}

Menu& imSquared::menu()
{
	return m_menu;
}

void imSquared::processLeavingFigures()
{

	//
	// check for squares not pressed on last line
	//
	for(SquareElements::iterator i = m_matrix.rbegin()->begin(); i != m_matrix.rbegin()->end(); ++i)
	{
		if (i->state == Marked)
		{
			m_multiplier = 1;
		}
	}
}

void imSquared::addHitToFigureOnBoard(int figureGeneration, int hit)
{
	for(FiguresOnBoard::iterator iterator = m_figuresOnBoard.begin(); iterator != m_figuresOnBoard.end(); ++iterator)
	{
		if (iterator->figureGeneration == figureGeneration)
		{
			iterator->hitCount += hit;
		}
	}
}

bool imSquared::isFigureOnMatrix(int figureGeneration)
{
	for (SquareMatrix::iterator row = m_matrix.begin(); row != m_matrix.end(); ++row)
		for (SquareElements::iterator cell = row->begin(); cell != row->end(); ++cell)
		{
			if ((cell->figureGeneration == figureGeneration) && (cell->hasPiece))
				return true;
		}

		return false;
}


void imSquared::processFigures()
{
    if (m_figures.empty()) return;


	//
	// Check leaving figure
	//
	FiguresOnBoard::iterator iterator = m_figuresOnBoard.begin();
	while(iterator != m_figuresOnBoard.end())
	{
		if (!isFigureOnMatrix(iterator->figureGeneration))
		{
			DPrintf("Removed figureOnBoard %d\n", iterator->figureGeneration);
			iterator = m_figuresOnBoard.erase(iterator);	
		}
		else
		{
			++iterator;
		}
	}

	    

	//
	// Create new Figures
	//
    if (m_currentFigure == -1)
    {
		m_figureGeneration++;

		if (m_figureGeneration < m_levels[m_currentLevel].total_figures)
		{	
			m_lastFigure = -1;
			m_currentFigureLine = 0;

			if (m_levels[m_currentLevel].type == "random puzzle")
			{
				m_currentFigure = rand() % int(m_figures.size());
			}
			else
			{
				m_currentFigure = m_levels[m_currentLevel].figures[m_figureGeneration];
			}
		
			
			m_currentFigureOffset = rand() % (m_configuration.columns - m_figures[m_currentFigure].width + 1);

			FigureOnBoard figureOnBoard;
			figureOnBoard.figure = m_currentFigure;
			figureOnBoard.figureGeneration = m_figureGeneration;
			figureOnBoard.hitCount = 0;
			figureOnBoard.markedCount = m_figures[m_currentFigure].markedCount;
			figureOnBoard.multiplied = false;
			m_figuresOnBoard.push_back(figureOnBoard);
			DPrintf("Generated figureOnBoard %d\n", figureOnBoard.figureGeneration);
					
		}
		else
			m_currentFigure--;
    }
	
	if (m_currentFigure < 0)
	{
		for(SquareElements::iterator i = m_matrix[0].begin(); i != m_matrix[0].end(); ++i)
			i->figure = m_lastFigure;

		++m_currentFigure;
		return;
	}
    
	//
	// Feed Figure Lines to board
	//
	if (m_currentFigureLine < m_figures[m_currentFigure].height) 
	{
		for (int x = 0; x < m_figures[m_currentFigure].width; x++)
		{
			if (m_figures[m_currentFigure].lines[m_currentFigureLine][x] == 'X')
			{
				m_matrix[0][m_currentFigureOffset + x].state = Marked;
				m_matrix[0][m_currentFigureOffset + x].markedColor = m_figures[m_currentFigure].color;
				m_matrix[0][m_currentFigureOffset + x].figure = m_currentFigure;
				m_matrix[0][m_currentFigureOffset + x].hasPiece = true;
				m_matrix[0][m_currentFigureOffset + x].figureGeneration = m_figureGeneration;
			}
		}

		for(SquareElements::iterator i = m_matrix[0].begin(); i != m_matrix[0].end(); ++i)
		{
			if (!i->hasPiece)
			{
				i->figure = m_currentFigure;
			}
		}

		m_lastFigure = m_currentFigure;
		++m_currentFigureLine;
	}
	

	if (m_currentFigureLine >= m_figures[m_currentFigure].height) 
	{
		m_currentFigure = -1 - 1 * m_levels[m_currentLevel].figure_spacing;
	}
}

void imSquared::render()
{
	if (m_menu.activated())
	{
		m_menu.render(this);
		return;
	}


	glClearColor(0.0, 0.0, 0.0, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glTranslatef(0.0, m_translation, 0.0);
    
	if (m_configuration.texture_enabled)
	{
		glEnable(GL_TEXTURE_2D);
	
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
	}

	static const GLfloat texture_coords[] = {
        0.0, 1.0,
        1.0, 1.0,
        0.0, 0.0,
        1.0, 0.0
    };

    //
    // draw squares
    //   
	for (SquareMatrix::const_iterator row = m_matrix.begin(); row != m_matrix.end(); ++row)
	{
		for (SquareElements::const_iterator cell = row->begin(); cell != row->end(); ++cell)
		{
			glColor4f(cell->color.red, cell->color.green, cell->color.blue, cell->color.alpha);

			if (m_configuration.texture_enabled)
			{
				glBindTexture(GL_TEXTURE_2D, cell->texture);

				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(2, GL_FLOAT, 0, texture_coords);
			}


			glVertexPointer(3, GL_FLOAT, 0, (float*)& cell->vertices);
			glEnableClientState(GL_VERTEX_ARRAY);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glDisableClientState(GL_VERTEX_ARRAY);

			if (m_configuration.texture_enabled)
			{
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
		}
	}  

	if (m_configuration.texture_enabled)
	{
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
	}

	//
	// draw lines
	//
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	glLineWidth(5.0f);
    

	for (Lines::const_iterator line = m_lines.begin(); line != m_lines.end(); ++line)
	{
		glVertexPointer(3, GL_FLOAT, 0, (float*)& (*line));
		glEnableClientState(GL_VERTEX_ARRAY);
		glDrawArrays(GL_LINES, 0, 2);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

    float scale = 2.0f;



    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glScalef(scale, scale, scale);
    
	//
	// draw text
	//

	{
		std::stringstream messagestream; 
		messagestream << std::setfill('0') << std::setw(8) << m_score;
		std::string message = messagestream.str();

		m_font.Bind(m_configuration.screenWidth, m_configuration.screenHeight);
		m_font.Print(message, 5, 5);
		m_font.Unbind();
	}

	{
		std::stringstream messagestream; 
		messagestream << std::setfill('0') << std::setw(3) << m_multiplier;
		std::string message = messagestream.str();

		m_font.Bind(m_configuration.screenWidth, m_configuration.screenHeight);
		m_font.Print(message, 5, m_configuration.screenHeight * 0.5 - 5 - (m_font.CellY));
		m_font.Unbind();
	}

}
void imSquared::ShowMainMenu()
{
	Menu::Elements elements;

	for (Levels::iterator level = m_levels.begin(); level != m_levels.end(); ++level)
	{
		elements.push_back(level->name);
	}
	m_menu.SetElements(elements);
	m_menu.SetTitle(IMS_MAIN_MENU_TITLE);
	m_menu.Show();
}

bool imSquared::UpdateLevelChaining()
{
	if (!m_configuration.menus_enabled) return true;

	if (m_menu.activated())
	{
		if (m_menu.Selected() != -1)
		{
			std::string level = m_levels[m_menu.Selected()].name;
			Configuration configuration = m_configuration;
			configuration.level = level;
			m_menu.Hide();
			initialize(configuration);

			return false;
		}
	}
	else 
	{
		if (ended())
		{
			ShowMainMenu();
			return false;
		}
	}


	return true;
}
