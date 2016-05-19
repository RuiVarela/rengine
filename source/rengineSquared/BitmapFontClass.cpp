#include <fstream>
#include <string>
#include "BitmapFontClass.h"

#include "imSquared.h"

using namespace std;

#define MAX_TEX_ID 9876

CBitmapFont::CBitmapFont() 
{
	TexID = MAX_TEX_ID;
}

CBitmapFont::~CBitmapFont() { }

bool CBitmapFont::Load(const char *fname)
{
	Unload();

	char *dat,*img;
	fstream in;
	unsigned long fileSize;
	char bpp;
	int ImgX,ImgY;


	fileSize = OS_ReadFileBinary(fname, &dat);
	if (!dat || !fileSize) return false;

	// Check ID is 'BFF2'
	if((unsigned char)dat[0]!=0xBF || (unsigned char)dat[1]!=0xF2)
	{
		OS_Free(dat);
		return false;
	}

	// Grab the rest of the header
	memcpy(&ImgX,&dat[2],sizeof(int));
	memcpy(&ImgY,&dat[6],sizeof(int));
	memcpy(&CellX,&dat[10],sizeof(int));
	memcpy(&CellY,&dat[14],sizeof(int));
	bpp=dat[18];
	Base=dat[19];

	// Check filesize
	if(fileSize!=((MAP_DATA_OFFSET)+((ImgX*ImgY)*(bpp/8))))
		return false;

	// Calculate font params
	RowPitch=ImgX/CellX;
	ColFactor=(float)CellX/(float)ImgX;
	RowFactor=(float)CellY/(float)ImgY;
	YOffset=CellY;

	// Determine blending options based on BPP
	switch(bpp)
	{
	case 8: // Greyscale
		RenderStyle=BFG_RS_ALPHA;
		break;

	case 24: // RGB
		RenderStyle=BFG_RS_RGB;
		break;

	case 32: // RGBA
		RenderStyle=BFG_RS_RGBA;
		break;

	default: // Unsupported BPP
		delete [] dat;
		return false;
		break;
	}

	// Allocate space for image
	img=new char[(ImgX*ImgY)*(bpp/8)];

	if(!img)
	{
		delete [] dat;
		return false;
	}

	// Grab char widths
	memcpy(Width,&dat[WIDTH_DATA_OFFSET],256);

	// Grab image data
	memcpy(img,&dat[MAP_DATA_OFFSET],(ImgX*ImgY)*(bpp/8));

	// Create Texture
	glGenTextures(1,&TexID);
	glBindTexture(GL_TEXTURE_2D,TexID);
	// Fonts should be rendered at native resolution so no need for texture filtering
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Stop chararcters from bleeding over edges
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	// Tex creation params are dependent on BPP
	switch(RenderStyle)
	{
	case BFG_RS_ALPHA:
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,ImgX,ImgY,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,img);
		break;

	case BFG_RS_RGB:
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,ImgX,ImgY,0,GL_RGB,GL_UNSIGNED_BYTE,img);
		break;

	case BFG_RS_RGBA:
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ImgX,ImgY,0,GL_RGBA,GL_UNSIGNED_BYTE,img);
		break;
	}

	// Clean up
	delete [] img;
	OS_Free(dat);

	return true;
}

void CBitmapFont::Unload()
{
	if(TexID != MAX_TEX_ID)
	{
		glDeleteTextures(1, &TexID);
		TexID = MAX_TEX_ID;
	}
}

void CBitmapFont::Bind(int width, int height)
{

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
    
#ifdef RENGINE_PLATFORM
	glOrtho(0, width, 0, height, -1, 1);
#else
    glOrthof(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f);
#endif 
    
    
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
    

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D,TexID);

	switch(RenderStyle)
	{
	case BFG_RS_ALPHA: // 8Bit
		glBlendFunc(GL_SRC_ALPHA,GL_SRC_ALPHA);
		glEnable(GL_BLEND);
		break;

	case BFG_RS_RGB:   // 24Bit
		glDisable(GL_BLEND);
		break;

	case BFG_RS_RGBA:  // 32Bit
		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		break;
	}
}
void CBitmapFont::Unbind()
{
	//glLoadIdentity();
	
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}    

void CBitmapFont::Print(char ch, float x, float y)
{
	static float vertex[4 * 2];
	static float coords[4 * 2];

	int Row = (ch - Base) / RowPitch;
	int Col = (ch - Base) - Row * RowPitch;

	float U = Col * ColFactor;
	float V = Row * RowFactor;
	float U1 = U + ColFactor;
	float V1 = V + RowFactor;

	float offset = 1.0;

	int start = 0;

	for (int i = start; i != 5; ++i)
	{
		float offset_x = 0.0f;
		float offset_y = 0.0f;

		if (i == 0)
		{
			offset_x = -offset;
			offset_y = -offset;
		}
		else if (i == 1)
		{
			offset_x = offset;
			offset_y = -offset;
		}
		else if (i == 2)
		{
			offset_x = offset;
			offset_y = offset;
		}
		else if (i == 3)
		{
			offset_x = -offset;
			offset_y = offset;
		}


		if (i < 4)
		{
			glColor4f(0.0f, 0.0f, 0.0f, 1.0);
		}
		else
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0);
		}

		vertex[0 * 2 + 0] = offset_x + x;
		vertex[0 * 2 + 1] = offset_y + y;
		coords[0 * 2 + 0] = U;
		coords[0 * 2 + 1] = V1;

		vertex[1 * 2 + 0] = offset_x + x + (float)CellX;
		vertex[1 * 2 + 1] = offset_y + y;
		coords[1 * 2 + 0] = U1;
		coords[1 * 2 + 1] = V1;

		vertex[2 * 2 + 0] = offset_x + x;
		vertex[2 * 2 + 1] = offset_y + y + (float)CellY;
		coords[2 * 2 + 0] = U;
		coords[2 * 2 + 1] = V;

		vertex[3 * 2 + 0] = offset_x + x + (float)CellX;
		vertex[3 * 2 + 1] = offset_y  + y + (float)CellY;
		coords[3 * 2 + 0] = U1;
		coords[3 * 2 + 1] = V;


		glVertexPointer(2, GL_FLOAT, 0, vertex);
		glEnableClientState(GL_VERTEX_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 0, coords);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	/*
	glBegin(GL_QUADS);

	glTexCoord2f(U, V1);  glVertex2i(x, y);
	glTexCoord2f(U1,V1);  glVertex2i(x + CellX, y);
	glTexCoord2f(U1,V );  glVertex2i(x + CellX, y + CellY);
	glTexCoord2f(U, V );  glVertex2i(x, y + CellY);
	
	glEnd();*/
}

void CBitmapFont::Print(std::string const& text, int x, int y)
{
	for(int i = 0; i != text.size(); ++i)
	{
		Print(text[i], x, y);
		x += Width[text[i]];
	}
}
