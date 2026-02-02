
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

// TODO(SJtheSahilJoseph): Implement Sine ourselves.
#include <math.h>

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

typedef float real32;
typedef double real64;

typedef int32 bool32;

#define PI32 3.14159265359f

struct win32_offscreen_buffer
{
	// NOTE(SJtheSahilJoseph): Pixels are always 32 bit wide. Memory Order: BB GG RR XX. Little Endian 0x XX RR GG BB.
	BITMAPINFO Info;

	void* Memory;

	int Width;
	int Height;
	int Pitch;

	// NOTE(SJtheSahilJoseph): I will use BytesPerPixel from here.
	int BytesPerPixel;
};

// TODO(SJtheSahilJoseph): This is global variable for now.
global_variable bool Running = true;

global_variable win32_offscreen_buffer GlobalBackBuffer;

// TODO(SJtheSahilJoseph): Just experimenting thing.
global_variable int XOffset;
global_variable int YOffset;

global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

struct win32_window_dimension
{
	int Width;
	int Height;
};


// NOTE(SJtheSahilJoseph): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(SJtheSahilJoseph): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


internal void
Win32LoadXInput(void)
{

	// TODO(SJtheSahilJoseph): Test this on Windows 8 stuff.
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");

	if (!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("xinput1_3.dll");
	}

	if (XInputLibrary)
	{
		XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
	}

}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
	// NOTE(SJtheSahilJoseph): Load the library.

	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

	if (DSoundLibrary)
	{

		// NOTE(SJtheSahilJoseph): Get a DirectSound Object.
		direct_sound_create* DirectSoundCreate = (direct_sound_create*)GetProcAddress(DSoundLibrary, "DirectSoundCreate");

		// TODO(SJtheSahilJoseph): Double check if that works on XP or 7 or 8 or whatever.
		LPDIRECTSOUND DirectSound;

		if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
		{
			WAVEFORMATEX WavFormat = {};

			WavFormat.wFormatTag = WAVE_FORMAT_PCM;
			WavFormat.nChannels = 2;
			WavFormat.nSamplesPerSec = SamplesPerSecond;
			WavFormat.wBitsPerSample = 16;
			WavFormat.nBlockAlign = (WavFormat.nChannels * WavFormat.wBitsPerSample) / 8;
			WavFormat.nAvgBytesPerSec = WavFormat.nSamplesPerSec * WavFormat.nBlockAlign;
			WavFormat.cbSize = 0;


			if (!SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				// NOTE(SJtheSahilJoseph): Create a Primary Buffer.

				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
				BufferDescription.dwSize = sizeof(BufferDescription);

				if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
				{


					if (SUCCEEDED(PrimaryBuffer->SetFormat(&WavFormat)))
					{
						// NOTE(SJtheSahilJoseph): Now we have finally set the format.
					}
					else
					{
						// TODO(SJtheSahilJoseph): Diagnostic
					}
				}
				else
				{
					// TODO(SJtheSahilJoseph): Diagnostic
				}
			}
			else
			{
				// TODO(SJtheSahilJoseph): Diagnostic.
			}



			// NOTE(SJtheSahilJoseph): Create a Secondary Buffer.



			DSBUFFERDESC BufferDescription = {};
			BufferDescription.dwFlags = 0;
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.dwBufferBytes = BufferSize;
			BufferDescription.lpwfxFormat = &WavFormat;


			if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0)))
			{

				// NOTE(SJtheSahilJoseph): Start it playing.


			}

		}
		else
		{
			// TODO(SJtheSahilJoseph): Diagnostic.
		}
	}
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);

	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return Result;
}

internal void
RenderWeirdGradient(win32_offscreen_buffer* Buffer, int BlueOffset, int GreenOffset)
{
	// TODO(SJtheSahilJoseph): Pass by Value or Pass by Address? Buffer or *Buffer?

	uint8* Row = (uint8*)Buffer->Memory;

	for (int Y = 0; Y < Buffer->Height; Y++)
	{

		uint32* Pixel = (uint32*)Row;

		for (int X = 0; X < Buffer->Width; X++)
		{

			uint8 Blue = (X + BlueOffset);
			uint8 Green = (Y + GreenOffset);
			uint8 Red = 0;

			*Pixel++ = ((Green << 8) | Blue);

		}

		Row += Buffer->Pitch;
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

	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);


	// TODO(SJtheSahilJoseph): Probably wanna clear this to black.


	Buffer->Pitch = Width * Buffer->BytesPerPixel;


}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer* Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{

	// TODO(SJtheSahilJoseph): Aspect Ratio Correction.
	// TODO(SJtheSahilJoseph): Experiment with Stretch Modes.

	StretchDIBits(DeviceContext,
		//X, Y, Width, Height,
		// X, Y, Width, Height,
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory, &Buffer->Info, DIB_RGB_COLORS, SRCCOPY);
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

		win32_window_dimension Dimension = Win32GetWindowDimension(hWnd);

		Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);

		EndPaint(hWnd, &paint);
	} break;

	case WM_DESTROY:
	{
		// TODO(SJtheSahilJoseph): Handle this as an error - recreate window?
		Running = false;
		PostQuitMessage(0);
	} break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32 VKeyCode = wParam;

		bool WasDown = ((lParam & (1 << 30)) != 0);
		bool IsDown = ((lParam & (1 << 31)) == 0);

		if (VKeyCode == VK_UP)
		{
			if (IsDown) {
				YOffset += 20;
			}
		}

		else if (VKeyCode == VK_DOWN)
		{
			if (IsDown) {
				YOffset -= 20;
			}
		}

		else if (VKeyCode == VK_LEFT)
		{
			if (IsDown) {
				XOffset += 20;
			}
		}

		else if (VKeyCode == VK_RIGHT)
		{
			if (IsDown) {
				XOffset -= 20;
			}
		}

		else if (VKeyCode == 'Q')
		{

		}

		else if (VKeyCode == 'E')
		{

		}

		else if (VKeyCode == VK_ESCAPE)
		{
			if (IsDown)
			{
				Running = false;
			}

			if (WasDown)
			{
				Running = false;
			}
		}

		else if (VKeyCode == VK_SPACE)
		{

		}

		bool32 AltKeyWasDown = (lParam & (1 << 29));
		if ((VKeyCode == VK_F4) && AltKeyWasDown)
		{
			Running = false;
		}

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

struct win32_sound_output
{
	int SamplesPerSecond;
	int ToneHz;
	int16 ToneVolume;
	uint32 RunningSampleIndex;
	int WaveCounter;
	int WavePeriod;
	int BytesPerSample;
	int SecondaryBufferSize;
	real32 tSine;
	int LatencySampleCount;
};

internal void Win32FillSoundBuffer(win32_sound_output* SoundOutput, DWORD BytesToLock, DWORD BytesToWrite)
{

	VOID* Region1;
	DWORD Region1Size;
	VOID* Region2;
	DWORD Region2Size;


	if (SUCCEEDED(GlobalSecondaryBuffer->Lock(BytesToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
	{


		// TODO(SJtheSahilJoseph): Assert that Region1Size/Region2Size is valid.

		DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
		int16* SampleOut = (int16*)Region1;

		for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; SampleIndex++)
		{
			real32 SineValue = sinf(SoundOutput->tSine);

			int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);

			*SampleOut++ = SampleValue;
			*SampleOut++ = SampleValue;
			
			SoundOutput->tSine += (real32)2.0f * PI32 * (real32)1.0f / (real32)SoundOutput->WavePeriod;
			
			SoundOutput->RunningSampleIndex++;
		}

		SampleOut = (int16*)Region2;
		DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;

		for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; SampleIndex++)
		{
			real32 SineValue = sinf(SoundOutput->tSine);

			int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);

			*SampleOut++ = SampleValue;
			*SampleOut++ = SampleValue;

			SoundOutput->tSine += (real32)2.0f * PI32 * (real32)1.0f / (real32)SoundOutput->WavePeriod;

			SoundOutput->RunningSampleIndex++;
		}

		GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);

	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

	Win32LoadXInput();

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
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			0, 0, hInstance, 0
		);

		if (Window)
		{
			// NOTE(SJtheSahilJoseph): Since we specified CS_OWNDC, we can just get one device context and use it forever.
			// Because we are not sharing it with anyone.
			HDC DeviceContext = GetDC(Window);

			MSG Msg;

			Running = true;

			// TODO(SJtheSahilJoseph): I'm now creating globals to experiment.
			//int XOffset = 0;
			//int YOffset = 0;

			win32_sound_output SoundOutput = {};

			// TODO(SJtheSahilJoseph): Make this like 60 seconds?
			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.ToneHz = 256;
			SoundOutput.ToneVolume = 2000;
			SoundOutput.RunningSampleIndex = 0;
			SoundOutput.WaveCounter = 0;
			SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
			SoundOutput.BytesPerSample = (sizeof(int16) * 2);
			SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
			SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;

			Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);

			Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount * SoundOutput.SamplesPerSecond);

			GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

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

				// TODO(SJtheSahilJoseph): Should we pull this more frequently.
				DWORD dwResult;
				for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++)
				{
					XINPUT_STATE ControllerState;

					ZeroMemory(&ControllerState, sizeof(XINPUT_STATE));

					dwResult = XInputGetState(ControllerIndex, &ControllerState);

					if (dwResult == ERROR_SUCCESS)
					{
						XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;

						bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
						bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool LeftThumb = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
						bool RightThumb = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
						bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
						bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
						bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
						bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 StickLX = Pad->sThumbLX;
						int16 StickLY = Pad->sThumbLY;
						int16 StickRX = Pad->sThumbRX;
						int16 StickRY = Pad->sThumbRY;


					}
					else
					{
						// Controller is not connected
					}
				}
				RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

				// NOTE(SJtheSahilJoseph): DirectSound output test.

				DWORD PlayCursor;
				DWORD WriteCursor;

				if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
				{

					DWORD BytesToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
					DWORD BytesToWrite;

					DWORD TargetCursor = PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample);

					// TODO(SJtheSahilJoseph): Change this to using a lower latency offset from the play cursor when we actually start having sound effects.
					if (BytesToLock > TargetCursor)
					{
						BytesToWrite = (SoundOutput.SecondaryBufferSize - BytesToLock);
						BytesToWrite += TargetCursor;
					}
					else
					{
						BytesToWrite = TargetCursor - BytesToLock;
					}

					Win32FillSoundBuffer(&SoundOutput, BytesToLock, BytesToWrite);

				}

				win32_window_dimension Dimension = Win32GetWindowDimension(Window);

				Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);

				ReleaseDC(Window, DeviceContext);

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




