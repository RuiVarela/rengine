#include "OS.h"

extern "C"
{
	void OS_Initialize()
	{

    }

	void OS_Finalize()
	{
	}

	double OS_ElapsedTime()
	{
        
        NSTimeInterval ts = ([[NSDate date] timeIntervalSince1970]);
		return ts;
	}

	char* OS_GetFilePath(const char* filename)
	{
        int size = 0;
        char* output = 0;
        
        NSString *original = [[NSString alloc] initWithCString:filename encoding:NSASCIIStringEncoding];
        NSString *path = [original substringToIndex:[original length] - 4];
        NSString *ext = [original substringFromIndex:[original length] - 3];
        
        NSString *filePath = [[NSBundle mainBundle] pathForResource:path ofType:ext];
        
        if (filePath != 0) 
        {
            size = [filePath length];
            output = (char*)OS_Malloc(size + 1);
            
            for (int i = 0; i != size; ++i) 
                output[i] = (char)[filePath characterAtIndex:i];
            
            output[size] = 0;
        }
        
        //[filePath release];
        
        [original release];
       // [path release];
       // [ext release];

		return output;
	}
}