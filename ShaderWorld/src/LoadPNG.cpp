#include <string>

#include "Week9Application.h"
#include "LoadPNG.h"

#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;

// From: http://msdn.microsoft.com/en-us/library/windows/desktop/ms533843%28v=vs.85%29.aspx
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

bool ImageLoaderInit = false;

char * loadPNG(const char * filename, int & width, int & height)
{
	if (!ImageLoaderInit)
	{
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR           gdiplusToken;

		// Initialize GDI+.
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		ImageLoaderInit = true;
	}

	std::string fname = filename;
	Image originalimage(std::wstring(fname.begin(), fname.end()).c_str());
	if (originalimage.GetLastStatus() != Gdiplus::Status::Ok)
	{
		return NULL;
	}


	Bitmap image(512, 512);
	Graphics graphics(&image);
	graphics.DrawImage(&originalimage, 0, 0, 512, 512);

	width = image.GetWidth();
	height = image.GetHeight();

	BitmapData* bitmapData = new BitmapData;
	Rect rect(0, 0, width, height);
	image.LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, bitmapData);
	char * data = new char[width * height * 4];
	for (int row = 0; row < height; row++)
	{
		memcpy(data + row * width * 4, (char *)(bitmapData->Scan0) + row * bitmapData->Stride, width * 4);
	}
	image.UnlockBits(bitmapData);
	delete bitmapData;

	return data;
}

char * loadAtlas(char * filenames[], int numfiles, int & width, int & height)
{
	if (!ImageLoaderInit)
	{
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR           gdiplusToken;

		// Initialize GDI+.
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		ImageLoaderInit = true;
	}

	Bitmap image(512 * numfiles, 512);
	for (int i = 0; i < numfiles; i++)
	{
		std::string fname = filenames[i];
		Image originalimage(std::wstring(fname.begin(), fname.end()).c_str());

		Graphics graphics(&image);
		graphics.DrawImage(&originalimage, 512 * i, 0, 512, 512);
	}

	width = image.GetWidth();
	height = image.GetHeight();

	BitmapData* bitmapData = new BitmapData;
	Rect rect(0, 0, width, height);
	image.LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, bitmapData);
	char * data = new char[width * height * 4];
	for (int row = 0; row < height; row++)
	{
		memcpy(data + row * width * 4, (char *)(bitmapData->Scan0) + row * bitmapData->Stride, width * 4);
	}
	image.UnlockBits(bitmapData);
	delete bitmapData;

	return data;
}

void savePNG(char * filename, char * data, int & width, int & height)
{
	if (!ImageLoaderInit)
	{
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR           gdiplusToken;

		// Initialize GDI+.
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		ImageLoaderInit = true;
	}

	Bitmap image(width, height);

	BitmapData* bitmapData = new BitmapData;
	Rect rect(0, 0, width, height);
	image.LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, bitmapData);
	for (int row = 0; row < height; row++)
	{
		memcpy((char *)(bitmapData->Scan0) + row * bitmapData->Stride, data + row * width * 4, width * 4);
	}
	image.UnlockBits(bitmapData);
	delete bitmapData;

	CLSID pngClsid;
	GetEncoderClsid(L"image/png", &pngClsid);
	std::string fname = filename;
	image.Save(std::wstring(fname.begin(), fname.end()).c_str(), &pngClsid, NULL);
}

