#include "../../win32includes.h"
#include <vector>

class DisplayCapturer {
public:
	enum Format {
		BMP = 0,
		PNG = 1,
		JPG = 2,
		GIF = 3,
		TIF = 4
	};

	static std::vector<char> CaptureScreen(Format);
};