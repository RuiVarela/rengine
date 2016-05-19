#pragma once

#include "OS.h"
#include <vector>
#include <string>

class imSquared;

class Menu
{
public :
	typedef std::vector<std::string > Elements;
	Menu();

	void SetElements(Elements const& elements) { m_elements = elements; }
	void SetTitle(std::string const& title) { m_title = title; }
	std::string const& GetTitle() const { return m_title; }
	int Selected() const { return m_selected; }

	void Show();
	void Hide();

	bool activated();
	void Click(imSquared* game);
	void update(imSquared* game);
	void render(imSquared* game);
private :
	std::string m_title;
	Elements m_elements;

	int GetYPos(int element);

	int m_letter_height;
	int m_top;
	int m_selected;
	bool m_show;
};

