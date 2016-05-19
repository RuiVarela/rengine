#include <stdlib.h>
#include <stdio.h>

#include "OS.h"

extern "C"
{	
	void* OS_Malloc(int size)
	{
		return malloc(size);
	}

	void OS_Free(void* pointer)
	{
		if(pointer)
			free(pointer);
	}

	int OS_ReadFileBinary(const char* filename, char** output)
	{
		int result = -1;
		char* real_filename = OS_GetFilePath(filename);

		if (real_filename == 0) return -1;

		FILE *file;
		file = fopen(real_filename, "rb");
		if (file)
		{
			char *buffer;
			unsigned long len;

			fseek(file, 0, SEEK_END);
			len = ftell(file);
			fseek(file, 0, SEEK_SET);

			buffer = (char*) OS_Malloc(len + 1);

			if (buffer)
			{
				if(fread(buffer, len, 1, file) == 1)
				{
					result = len;
					*output = buffer;
				}
				else
				{
					OS_Free(buffer);
				}
			}

			fclose(file);
		}
	
		OS_Free(real_filename);
		return result;
	}
}