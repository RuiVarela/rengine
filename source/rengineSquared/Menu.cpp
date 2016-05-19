
#include "imSquared.h"
#include "Menu.h"
#include <sstream>
#include <iomanip>

Menu::Menu()
{
	m_title = "none";
	m_letter_height = -1;
	m_top = -1;
	m_selected = -1;
	m_show = false;
}

bool Menu::activated()
{
	return m_show;
}
void Menu::Show()
{
	m_selected = -1;
	m_show = true;
}

void Menu::Hide()
{
	m_show = false;
}

void Menu::Click(imSquared* game)
{
	if (m_letter_height <= 0) return;
	
	for (imSquared::Points::const_iterator p = game->m_touches.begin(); p != game->m_touches.end(); ++p)
	{
		float rescaled = ((p->y + 1.0f) * 0.5f) * game->m_configuration.screenHeight;
		for (int i = 0; i != m_elements.size(); ++i)
		{
			int low = GetYPos(i);
			int high = low + m_letter_height;

			if ((rescaled >= low) && (rescaled <= high))
			{
				m_selected = i;

				DPrintf("Menu::Click %s\n", m_elements[i].c_str());
				return;
			}
		}
	}
}

void Menu::update(imSquared* game)
{
	if (m_letter_height <= 0) return;
}

static void DrawRect(int x, int y, int width, int height)
{
	float squareVertices[12] = {
		x, y, 0.0,
		x + width, y, 0.0,
		x, y + height, 0.0,
		x + width, y + height, 0.0
	};

	glVertexPointer(3, GL_FLOAT, 0, squareVertices);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
}

int Menu::GetYPos(int element)
{
	return m_top - element * m_letter_height - m_letter_height;
}

void Menu::render(imSquared* game)
{
	glClearColor(0.0, 0.0, 0.0, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	m_letter_height = game->m_font.CellY;
	m_top = game->m_configuration.screenHeight - 5 - m_letter_height;
	int x = 5;

	//
	// Go fot the header
	//
	{
		game->m_font.Bind(game->m_configuration.screenWidth, game->m_configuration.screenHeight);
		game->m_font.Print(m_title.c_str(), x, m_top);
		game->m_font.Unbind();
	}

	int maxElements = m_top / m_letter_height;


	//
	// draw the lines
	//
	glLineWidth(1.0f);

	x = 20;
	int box_border = 2;
	int box_width = game->m_configuration.screenWidth - box_border - box_border;



	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glDisable(GL_TEXTURE_2D);
	glColor4f(0.3, 0.0, 0.0, 1.0);
	for (int i = 0; i != m_elements.size(); ++i)
	{
			


		int currentY = GetYPos(i);
		DrawRect(box_border, currentY, box_width, m_letter_height);
	}

	//
	// highlight selected
	//
	if ((m_selected >= 0) && (m_elements.size()))
	{
		glColor4f(0.9,0.9, 0.9, 1.0);
		int currentY = GetYPos(m_selected);
		DrawRect(box_border, currentY, box_width, m_letter_height);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	game->m_font.Bind(game->m_configuration.screenWidth, game->m_configuration.screenHeight);
	for (int i = 0; i != m_elements.size(); ++i)
	{
		int currentY = GetYPos(i);

		game->m_font.Print(m_elements[i], x, currentY);
	}
	game->m_font.Unbind();

	

	//
	// Go for the header
	//
	glColor4f(0.4, 0.4, 0.4, 0.4);
	DrawRect(box_border, m_top, box_width, m_letter_height);

	x = 5;
	{
		game->m_font.Bind(game->m_configuration.screenWidth, game->m_configuration.screenHeight);
		game->m_font.Print(m_title.c_str(), x, m_top);
		game->m_font.Unbind();
	}

}

