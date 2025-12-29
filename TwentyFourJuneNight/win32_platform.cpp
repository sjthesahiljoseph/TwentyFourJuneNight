
#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_offscreen_buffer
{
	BITMAPINFO Info;

	void* Memory;

	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

// TODO(SJtheSahilJoseph): This is global variable for now.
global_variable bool Running = true;

global_variable win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension
{
	int Width;
	int Height;
};

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);

	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return Result;
}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int BlueOffset, int GreenOffset)
{
	// TODO(SJtheSahilJoseph): Pass by Value or Pass by Address? Buffer or *Buffer?

	uint8* Row = (uint8*)Buffer.Memory;

	for (int Y = 0; Y < Buffer.Height; Y++)
	{

		uint32* Pixel = (uint32*)Row;

		for (int X = 0; X < Buffer.Width; X++)
		{

			uint8 Blue = (X + BlueOffset);
			uint8 Green = (Y + GreenOffset);
			uint8 Red = 0;

			*Pixel++ = ((Green << 8) | Blue);

		}

		Row += Buffer.Pitch;
	}

}

internal void Win32ResizeDIBSection(win32_offscreen_buffer* Buffer, int Width, int Height)
{

	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	// NOTE(SJtheSahilJoseph): When the biHeight is negative, this is the clue to Windows to treat this bitmap as top-down, not bottom-up. Meaning that the first three bytes of the image are the color for the top-left pixel in the bitmap. Not the bottom-left.
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;
	Buffer->Info.bmiHeader.biSizeImage = 0;
	Buffer->Info.bmiHeader.biXPelsPerMeter = 0;
	Buffer->Info.bmiHeader.biYPelsPerMeter = 0;
	Buffer->Info.bmiHeader.biClrUsed = 0;
	Buffer->Info.bmiHeader.biClrImportant = 0;

	int BitmapMemorySize = ((Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel);

	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);


	// TODO(SJtheSahilJoseph): Probably wanna clear this to black.


	Buffer->Pitch = Width * Buffer->BytesPerPixel;


}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer Buffer, int X, int Y, int Width, int Height)
{

	// TODO(SJtheSahilJoseph): Aspect Ratio Correction.

	StretchDIBits(DeviceContext, 
		//X, Y, Width, Height,
		// X, Y, Width, Height,
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer.Width, Buffer.Height,
		Buffer.Memory, &Buffer.Info, DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK Win32MainWindowCallback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	LRESULT result = 0;

	switch (Msg)
	{

	case WM_SIZE:
	{

	} break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;

		HDC DeviceContext = BeginPaint(hWnd, &paint);


		LONG Width = paint.rcPaint.right - paint.rcPaint.left;
		LONG Height = paint.rcPaint.bottom - paint.rcPaint.top;

		int X = paint.rcPaint.left;
		int Y = paint.rcPaint.top;

		win32_window_dimension Dimension = Win32GetWindowDimension(hWnd);

		Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer, X, Y, Width, Height);

		EndPaint(hWnd, &paint);
	} break;

	case WM_DESTROY:
	{
		// TODO(SJtheSahilJoseph): Handle this as an error - recreate window?
		Running = false;
		PostQuitMessage(0);
	} break;

	case WM_CLOSE:
	{
		// TODO(SJtheSahilJoseph): Handle this with a message to the user?
		Running = false;
		DestroyWindow(hWnd);
	} break;

	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_SIZE executed this.\n");
	} break;

	default:
	{
		result = DefWindowProcA(hWnd, Msg, wParam, lParam);
	} break;

	}

	return result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

	WNDCLASSA window_class = {};

	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = Win32MainWindowCallback;
	window_class.hInstance = hInstance;
	window_class.hIcon = 0;
	window_class.hCursor = 0;
	window_class.lpszMenuName = 0;
	window_class.lpszClassName = "Twenty Four June Night";

	if (RegisterClassA(&window_class))
	{

		HWND Window = CreateWindowExA(
			0,
			window_class.lpszClassName,
			window_class.lpszClassName,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, 1200, 720,
			0, 0, hInstance, 0
		);

		if (Window)
		{
			MSG Msg;

			Running = true;

			int XOffset = 0;
			int YOffset = 0;

			while (Running)
			{

				while (PeekMessage(&Msg, 0, 0, 0, PM_REMOVE))
				{

					if (Msg.message == WM_QUIT)
					{
						Running = false;
					}

					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}

				RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

				HDC DeviceContext = GetDC(Window);

				win32_window_dimension Dimension = Win32GetWindowDimension(Window);

				Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer, 0, 0, Dimension.Width, Dimension.Height);

				ReleaseDC(Window, DeviceContext);

				XOffset++;
				YOffset++;

			}

		}

		else
		{
			OutputDebugStringA("Error: window_handle.\n");
		}

	}
	else
	{
		OutputDebugStringA("Error: register_class.\n");
	}

	return 0;
}




