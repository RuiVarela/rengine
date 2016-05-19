// __!!rengine_copyright!!__ //

#include <rengine/lang/Lang.h>
#include <rengine/string/CStringTokenizer.h>

#include <cstdio>
#include <cstdlib>

namespace rengine
{
	char* trimLeft(char * ch)
	{
		while (*ch == ' ')
		{
			++ch;
		}
		return ch;
	}

	unsigned int numberOfTokens(char const* ch)
	{
		int count = 0;

		if (ch != '\0')
		{
			++count;

			while (*ch != '\0')
			{
				if (*ch == ' ')
				{
					++count;
				}

				++ch;
			}
		}
		return count;
	}

	char* nextToken(char* ch)
	{
		while ((*ch != ' ') && (*ch != '\0'))
		{
			++ch;
		}

		return ++ch;
	}

	char* getToken(char* ch, unsigned int token)
	{
		for (; token; --token)
		{
			while (*ch != ' ')
			{
				++ch;
			}

			++ch;
		}

		return ch;
	}

	char* copyToken(char* dest, char* source)
	{
		while ((*source != ' ') && (*source != '\0'))
		{
			*dest = *source;
			++dest;
			++source;
		}
		*dest = '\0';

		if (*source == ' ')
			++source;

		return source;
	}

	char* readTextFile(char const* filename)
	{
		FILE *fp;
		char *content = NULL;

		size_t count = 0;

		if (filename != NULL)
		{
			fp = fopen(filename, "rt");

			if (fp != NULL)
			{

				fseek(fp, 0, SEEK_END);
				count = ftell(fp);
				rewind(fp);

				if (count > 0)
				{
					content = (char*) rg_malloc(sizeof(char) * (count + 1));
					count = fread(content, sizeof(char), count, fp);
					content[count] = '\0';
				}
				fclose(fp);
			}
		}
		return content;
	}

} // namespace rengine
