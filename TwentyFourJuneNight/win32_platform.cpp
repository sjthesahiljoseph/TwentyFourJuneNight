
#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static


// TODO(SJtheSahilJoseph): This is global variable for now.
global_variable bool Running = true;

LRESULT CALLBACK main_window_callback(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	LRESULT result = 0;

	switch (Msg)
	{

	case WM_SIZE:
	{
		OutputDebugStringA("WM_SIZE executed this.\n");
	} break;

	case WM_PAINT:
	{
		OutputDebugStringA("WM_PAINT executed this.\n");

		PAINTSTRUCT paint;

		HDC DeviceContext = BeginPaint(hWnd, &paint);

		LONG width = paint.rcPaint.right - paint.rcPaint.left;
		LONG height = paint.rcPaint.bottom - paint.rcPaint.top;

		int X = paint.rcPaint.left;
		int Y = paint.rcPaint.top;

		local_persist DWORD Operation = WHITENESS;

		PatBlt(DeviceContext, X, Y, width, height, Operation);

		if (Operation == WHITENESS)
		{
			Operation = BLACKNESS;
		}
		else
		{
			Operation = WHITENESS;
		}

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
	window_class.lpfnWndProc = main_window_callback;
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




