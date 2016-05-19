#pragma once

#define BFG_RS_NONE  0x0      // Blend flags
#define BFG_RS_ALPHA 0x1
#define BFG_RS_RGB   0x2
#define BFG_RS_RGBA  0x4


#define WIDTH_DATA_OFFSET  20 // Offset to width data with BFF file
#define MAP_DATA_OFFSET   276 // Offset to texture image data with BFF file

class CBitmapFont
 {
  public:
   CBitmapFont();
   ~CBitmapFont();
   
   bool Load(const char *fname);  
   void Unload();
   
   void Bind(int width, int height);     
   void Unbind();
   
   void Print(std::string const& text, int x, int y); 
   void Print(char ch, float x, float y);
   
   float Scale;
   int CellX,CellY,YOffset,RowPitch;
   char Base;
   char Width[256];   
   unsigned int TexID;
   float RowFactor,ColFactor;
   int RenderStyle;
 };