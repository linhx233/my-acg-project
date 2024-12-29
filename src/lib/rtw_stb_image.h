#ifndef RTW_STB_IMAGE_H
#define RTW_STB_IMAGE_H

// Disable strict warnings for this header from the Microsoft Visual C++ compiler.
#ifdef _MSC_VER
	#pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

class rtw_image {
  public:
	rtw_image() {}

	rtw_image(const char* image_filename) {
		// Loads image data from the specified file. If the RTW_IMAGES environment variable is
		// defined, looks only in that directory for the image file. If the image was not found,
		// searches for the specified image file first from the current directory, then in the
		// images/ subdirectory, then the _parent's_ images/ subdirectory, and then _that_
		// parent, on so on, for six levels up. If the image was not loaded successfully,
		// width() and height() will return 0.

		auto filename = std::string(image_filename);
		auto imagedir = getenv("RTW_IMAGES");

		// Hunt for the image file in some likely locations.
		if (imagedir && load(std::string(imagedir) + "/" + image_filename)) return;
		if (load(filename)) return;
		if (load("images/" + filename)) return;
		if (load("../images/" + filename)) return;
		if (load("../../images/" + filename)) return;
		if (load("../../../images/" + filename)) return;
		if (load("../../../../images/" + filename)) return;
		if (load("../../../../../images/" + filename)) return;
		if (load("../../../../../../images/" + filename)) return;

		std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
	}

	~rtw_image() {
		delete[] bdata;
		STBI_FREE(fdata);
	}

	bool load(const std::string& filename) {
		// Loads the linear (gamma=1) image data from the given file name. Returns true if the
		// load succeeded. The resulting data buffer contains the three [0.0, 1.0]
		// floating-point values for the first pixel (red, then green, then blue). Pixels are
		// contiguous, going left to right for the width of the image, followed by the next row
		// below, for the full height of the image.

		auto n = bytes_per_pixel; // Dummy out parameter: original components per pixel
		fdata = stbi_loadf(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
		if (fdata == nullptr) return false;

		bytes_per_scanline = image_width * bytes_per_pixel;
		convert_to_bytes();
		load_success=1;
		return true;
	}
	
	rtw_image(const aiTexture *tex) {        
		if(tex->mHeight!=0){
			std::clog<<"Using embedded texture"<<std::endl;
			int width=tex->mWidth,height=tex->mHeight;
			image_width=width,image_height=height;
			bdata=new unsigned char[width*height*bytes_per_pixel];
			int ptr=0;
			for(int i=0;i<height;i++)
				for(int j=0;j<width;j++){
					aiTexel c=tex->pcData[j*height+i];
					bdata[ptr++]=c.r;
					bdata[ptr++]=c.g;
					bdata[ptr++]=c.b;
				}
			bytes_per_scanline = image_width * bytes_per_pixel;
			load_success=1;
		}
		else{
			std::clog << "Compressed texture of size: " 
                      << tex->mWidth << " bytes" << std::endl;
			std::clog<<std::string(tex->achFormatHint)<<std::endl;
			std::clog<<tex->mFilename.C_Str()<<std::endl;
			const unsigned char* data = reinterpret_cast<const unsigned char*>(tex->pcData);
   			int channels;
			bdata = stbi_load_from_memory(data, tex->mWidth, &image_width, &image_height, &channels, 0);
    		if (!bdata) {
        		std::cerr << "Failed to load compressed texture: " << stbi_failure_reason() << std::endl;
        		return;
   			}

    		std::clog << "Loaded texture with dimensions: " << image_width << "x" << image_height << " and " << channels << " channels." << std::endl;

    		// Use the texture (e.g., upload to GPU)...

			bytes_per_scanline = image_width * bytes_per_pixel;
			load_success=1;
   			// Free the loaded data
		}
	}

	int width()  const { return load_success ? image_width : 0; }
	int height() const { return load_success ? image_height : 0; }

	const unsigned char* pixel_data(int x, int y) const {
		// Return the address of the three RGB bytes of the pixel at x,y. If there is no image
		// data, returns magenta.
		static unsigned char magenta[] = { 255, 0, 255 };
		if (bdata == nullptr) return magenta;

		x = clamp(x, 0, image_width);
		y = clamp(y, 0, image_height);

		return bdata + y*bytes_per_scanline + x*bytes_per_pixel;
	}

  private:
	bool 		   load_success	   = 0;
	const int      bytes_per_pixel = 3;
	float         *fdata = nullptr;         // Linear floating point pixel data
	unsigned char *bdata = nullptr;         // Linear 8-bit pixel data
	int            image_width = 0;         // Loaded image width
	int            image_height = 0;        // Loaded image height
	int            bytes_per_scanline = 0;

	static int clamp(int x, int low, int high) {
		// Return the value clamped to the range [low, high).
		if (x < low) return low;
		if (x < high) return x;
		return high - 1;
	}

	static unsigned char float_to_byte(float value) {
		if (value <= 0.0)
			return 0;
		if (1.0 <= value)
			return 255;
		return static_cast<unsigned char>(256.0 * value);
	}

	void convert_to_bytes() {
		// Convert the linear floating point pixel data to bytes, storing the resulting byte
		// data in the `bdata` member.

		int total_bytes = image_width * image_height * bytes_per_pixel;
		bdata = new unsigned char[total_bytes];

		// Iterate through all pixel components, converting from [0.0, 1.0] float values to
		// unsigned [0, 255] byte values.

		auto *bptr = bdata;
		auto *fptr = fdata;
		for (auto i=0; i < total_bytes; i++, fptr++, bptr++)
			*bptr = float_to_byte(*fptr);
	}
};

// Restore MSVC compiler warnings
#ifdef _MSC_VER
	#pragma warning (pop)
#endif

#endif