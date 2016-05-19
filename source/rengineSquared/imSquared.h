#pragma once 

#include <vector>
#include <string>
#include "OS.h"
#include "BitmapFontClass.h"
#include "Menu.h"

#define IMS_MAIN_MENU_TITLE "Main Menu"

class imSquared
{
public:
	enum Device
	{
		Pc,
		iPad,
		iPhone
	};

	struct Color
	{
		Color()
			:red(0.0f), green(0.0f), blue(0.0f), alpha(1.0f)
		{}

		float red;
		float green;
		float blue;
		float alpha;
	};

	struct Point
	{
		Point()
			:x(0.0f), y(0.0f), z(0.0f)
		{}

		float x;
		float y;
		float z;
	};
	typedef std::vector<Point> Points;

	struct Line
	{
		Point start;
		Point end;
	};
	typedef std::vector<Line> Lines;
	
	enum State
	{
		Idle,
		Marked,

		Hit, // marked and ok
		Miss // marked but not ok
	};

	// watch out.. this is beeing copied with memcpy
	struct SquareElement
	{
		unsigned int texture;
		Color color;
		Color markedColor;
		Point vertices[4];
		State state;
		bool hasPiece;
		int figureGeneration;
		int figure;
	};
	typedef std::vector<SquareElement> SquareElements;
	typedef std::vector<SquareElements> SquareMatrix;


	typedef std::vector<std::string> FigureLines;	
	struct Figure
	{
		Figure();

		FigureLines lines;
		int markedCount;
		int width;
		int height;
		int score;
		int hits;
		Color color;
		std::string sound;
		std::string name;
	};
	typedef std::vector<Figure> Figures;

	struct FigureOnBoard
	{
		int figure;
		int markedCount;
		int hitCount;
		int figureGeneration;
		bool multiplied;
	};
	typedef std::vector<FigureOnBoard> FiguresOnBoard;

	typedef std::vector<int> FigureIndex;
	static int const max_hardness = 3;
	struct Level
	{
		Level();

		std::string name;
		std::string type;
		int figure_spacing;
		float speed[max_hardness];						// time to move one square in seconds
		float speed_increment_per_second[max_hardness];
		float speed_max;
		int total_figures;
		float score_factor[max_hardness];
		float clear_factor[max_hardness];
		int bonus[max_hardness];
		int max_expected_score[max_hardness];
		int max_expected_score_with_multiplier[max_hardness];
		FigureIndex figures;
	};
	typedef std::vector<Level> Levels;


	struct Configuration
	{
		Configuration()
			:screenWidth(0), screenHeight(0), 
			 columns(0), rows(0), device(imSquared::Pc), 
			 squareWidth(0), squareHeight(0), 
			 missScoreFactor(-0.2f), hardness(0),
			 texture_enabled(true), menus_enabled(false)
		{
		}

		int screenWidth;
		int screenHeight;
		int columns;
		int rows;
		Device device;

		float squareWidth;
		float squareHeight;

		float missScoreFactor;

		std::string level;
		int hardness;

		bool texture_enabled;
		bool menus_enabled;
	};

	imSquared();
	~imSquared();

	void reboot();
	void initialize(Configuration const& configuration);
    void shutdown();

    
    int score();
	bool ended();
    bool completed();
    
	void update();
	void render();
	void setTouches(Points const& touches);
	
	Menu& menu();

private:
	bool UpdateLevelChaining();
	void ShowMainMenu();

	Configuration m_configuration;
	SquareMatrix m_matrix;
	Lines m_lines;
	Points m_touches;
	float m_translation;
	double m_lastUpdate;
   
	void applyColors();
	void processTouches();
	bool pointHitSquare(Point const& point, SquareElement const& square);

	void processFigures();
	void loadDatabase();
	Figures m_figures;
	int m_lastFigure;
	int m_currentFigure;
	int m_currentFigureLine;
	int m_currentFigureOffset;
	int m_multiplier;
	int m_figureGeneration;

	float m_speed;
	Levels m_levels;
	int m_currentLevel;
	int indexFromFigureName(std::string const& name);
	int indexFromLevelName(std::string const& name);

	void processLeavingFigures();
	void addHitToFigureOnBoard(int figureGeneration, int hit);
	bool isFigureOnMatrix(int figureGeneration);
	FiguresOnBoard m_figuresOnBoard;

	int m_score;
	CBitmapFont m_font;

    bool m_ended;

	unsigned int m_textureId;

	Menu m_menu;

	friend class Menu;
};
