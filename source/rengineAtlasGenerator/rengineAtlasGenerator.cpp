#include <rengine/util/Bootstrap.h>

#include <rengine/image/ImageResourceLoader.h>
#include <rengine/algorithm/RectanglePacking.h>
#include <rengine/image/stb_image.h>

std::string const image_dir = convertFileNameToNativeStyle("unit_test_data/gui");
std::string const output_image = "dump";
Int const margin = 0;

//todo implement margin
typedef std::vector<std::string> Filenames;

int main(int argc, char *argv[])
{
	rengine::enableApplicationDebugger();
	std::cout << "Atlas generator" << std::endl;


	ImageResourceLoader loader;

	DirectoryContents directory_contents = getDirectoryContents(image_dir);

	Filenames filenames;
	MetaBinPack::MetaRectangles rectangles;

	for (DirectoryContents::iterator i = directory_contents.begin(); i != directory_contents.end(); ++i)
	{
		std::string name = getStrippedName(*i);
		std::string path = convertFileNameToNativeStyle(image_dir + "/" + *i);

		if (fileType(path) == FileRegular)
		{
			SharedPointer<Image> image =  loader.loadImplementation(path);

			if (image)
			{
				MetaBinPack::MetaRectangle rectangle;
				rectangle.originalWidth = image->getWidth() + 2 * margin;
				rectangle.originalHeight = image->getHeight() + 2 * margin;
				filenames.push_back(path);
				rectangles.push_back(rectangle);
			}
			else
			{
				std::cout << "Unable to load " << path << std::endl;
			}
		}
	}

	std::cout << rectangles.size() << " records loaded." << std::endl;

	MetaBinPack packer;
	Int width;
	Int height;

	if(packer.pack(rectangles, width, height))
	{
		std::cout << "Packed " << rectangles.size() << " [" << width << "x" << height << "]" << std::endl;

		for (Int i = 0; i != (Int)rectangles.size(); ++i)
		{
			std::cout << 
			stringPrinter("[%4d] Rect [%4dx%4d] To [%4d %4d %4dx%4d Rotated=%d]", i,
						  rectangles[i].originalWidth, rectangles[i].originalWidth, 
						  rectangles[i].x, rectangles[i].y, rectangles[i].width, rectangles[i].height, (Int)rectangles[i].rotated) << std::endl;
		}


		//
		// build image
		//
		Int channels = 4;
		Int total_size = width * height * channels;
		SharedArray<unsigned char> image = new unsigned char[total_size];
		memset(image.get(), 0, total_size);


		for (Int i = 0; i != (Int)rectangles.size(); ++i)
		{
			SharedPointer<Image> sourceImage =  loader.loadImplementation(filenames[i]);
			if (sourceImage.get() == 0)
			{	
				std::cout << "Unable to load " << filenames[i] << std::endl;
				continue;
			}

			for (Int x = 0; x != rectangles[i].width - 2 * margin; ++x)
			{
				for (Int y = 0; y != rectangles[i].height - 2 * margin; ++y)
				{
					Image::Color* color = 0;
					if (rectangles[i].rotated)
					{
						color = sourceImage->pixel(y, x);	 
					}
					else
					{
						color = sourceImage->pixel(x, y);	
					}

					Int x_pos = rectangles[i].x + x;
					Int y_pos = rectangles[i].y + y;
					

					image[(x_pos + margin + (y_pos + margin) * width) * 4 + 0] = color->r();
					
					if (sourceImage->getColorChannels() >= 3)
					{
						image[(x_pos + margin + (y_pos + margin) * width) * 4 + 1] = color->g();
						image[(x_pos + margin + (y_pos + margin) * width) * 4 + 2] = color->b();
					}
					else
					{
						image[(x_pos + margin + (y_pos + margin) * width) * 4 + 1] = color->r();
						image[(x_pos + margin + (y_pos + margin) * width) * 4 + 2] = color->r();
					}

					if (sourceImage->getColorChannels() >= 4)
					{
						image[(x_pos + margin + (y_pos + margin) * width) * 4 + 3] = color->a();
					}
					else
					{
						image[(x_pos + margin + (y_pos + margin) * width) * 4 + 3] = 255;
					}
				}
			}
		}

		int result = stbi_write_tga((output_image + ".tga").c_str(), width, height, channels, image.get());
	}
	else
	{
		std::cout << "Unable to pack rectangles..." << std::endl;
	}




	std::cin.get();
	return 0;
}