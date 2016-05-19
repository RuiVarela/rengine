// __!!rengine_copyright!!__ //

#ifndef __RENGINE_TOKEN__

namespace rengine
{
	// Remove spaces at the beginning of the string
	char* trimLeft(char * string);

	// counts the number of tokens in a string, an empty string has 0 token
	unsigned int numberOfTokens(char const* string);

	// moves to the next token
	char* nextToken(char* string);

	// moves to the "token" token
	char* getToken(char* string, unsigned int const token);

	// copy the next token, and moves to the next token
	char* copyToken(char* dest, char* source);

	//reads a text file, you need to clean up memory
	char* readTextFile(char const* filename);
}

#endif// __RENGINE_TOKEN__
