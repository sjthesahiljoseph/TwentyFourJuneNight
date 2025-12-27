
#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

// TODO(SJtheSahilJoseph): This is global variable for now.
global_variable bool Running = true;

global_variable BITMAPINFO BitmapInfo;

global_variable void* BitmapMemory;

global_variable HBITMAP BitmapHandle;

global_variable HDC BitmapDeviceContext;

internal void Win32ResizeDIBSection(int Width, int Height)
{

	// TODO(SJtheSahilJoseph): Bulletproof this.
	// Maybe don't free first, Free after, then free first if that fails.

	// TODO(SJtheSahilJoseph): Free our DIBSection.
	if (BitmapHandle)
	{
		DeleteObject(BitmapHandle);
	}
	
	if (!BitmapDeviceContext)
	{
		// TODO(SJtheSahilJoseph): Should we recreate these under certain special circumstances.

		BitmapDeviceContext = CreateCompatibleDC(0);
	}

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	BitmapInfo.bmiHeader.biSizeImage = 0;
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biClrImportant = 0;

	HBITMAP BitmapHandle = CreateDIBSection(BitmapDeviceContext, &BitmapInfo, DIB_RGB_COLORS,
							 &BitmapMemory, NULL, NULL);

}

internal void
Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
	StretchDIBits(DeviceContext, X, Y, Width, Height, X, Y, Width, Height,
			  		  BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK Win32MainWindowCallback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	LRESULT result = 0;

	switch (Msg)
	{

	case WM_SIZE:
	{
		RECT ClientRect;
		GetClientRect(hWnd, &ClientRect);

		LONG Width = ClientRect.right - ClientRect.left;
		LONG Height = ClientRect.bottom - ClientRect.top;

		Win32ResizeDIBSection(Width, Height);
	} break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;

		HDC DeviceContext = BeginPaint(hWnd, &paint);


		LONG Width = paint.rcPaint.right - paint.rcPaint.left;
		LONG Height = paint.rcPaint.bottom - paint.rcPaint.top;

		int X = paint.rcPaint.left;
		int Y = paint.rcPaint.top;

		Win32UpdateWindow(DeviceContext, X, Y, Width, Height);

		local_persist DWORD Operation = WHITENESS;

		PatBlt(DeviceContext, X, Y, Width, Height, Operation);

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
	window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = Win32MainWindowCallback;
	window_class.hInstance = hInstance;
	window_class.hIcon = 0;
	window_class.hCursor = 0;
	window_class.lpszMenuName = 0;
	window_class.lpszClassName = "Twenty Four June Night";

	if (RegisterClassA(&window_class))
	{

		HWND window_handle = CreateWindowExA(
			0,
			window_class.lpszClassName,
			window_class.lpszClassName,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, 1200, 720,
			0, 0, hInstance, 0
		);

		if (window_handle)
		{
			MSG Msg;

			Running = true;

			while (Running)
			{
				BOOL message_result = GetMessage(&Msg, 0, 0, 0);

				if (message_result > 0)
				{
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}

				else
				{
					Running = false;
				}

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




