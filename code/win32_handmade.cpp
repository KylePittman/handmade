/*
	Create Windows Entry Point
*/

#include <windows.h>
#include <stdint.h>
#include <XInput.h>

#define local_persist static
#define global_variable static
#define internal static

struct Win32_offscreen_buffer{
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int pitch;
	int bytesPerPixel;
};

struct Win32WindowDimensions{
	int width;
	int height;
};
// TODO(KP): This is global for now
global_variable bool running;

global_variable Win32_offscreen_buffer backBuffer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
        return(0);
}

global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
        return(0);
}

global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

internal void Win32LoadXInput(void){
	HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
	if(XInputLibrary){
		XInputGetState = (x_input_get_state *) GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *) GetProcAddress(XInputLibrary, "XInputSetState");
	}
}

internal Win32WindowDimensions Win32GetDimensions(HWND window){
	Win32WindowDimensions wd;
	RECT clientRect;
	GetClientRect(window, &clientRect);
	wd.width = clientRect.right - clientRect.left;
	wd.height = clientRect.bottom - clientRect.top;
	return wd;
}

internal void renderWeirdGradient(Win32_offscreen_buffer buffer, int xOffset, int yOffset){
	
	uint8_t *row = (uint8_t *)buffer.memory;
	for(int y = 0; y < buffer.height; y++)	{
		uint32_t *pixel = (uint32_t *)row;
		for(int x = 0; x < buffer.width; x++)
		{
			uint8_t r = 0;
			uint8_t g = (y+yOffset);
			uint8_t b = (x+xOffset);
			
			
			*pixel++ = (r << 16 | g << 8 | b);
		}
		row+=buffer.pitch;
	}
}

internal void Win32ResizeDIBSection(Win32_offscreen_buffer *buffer, int width, int height){	
	
	if(buffer->memory){
		VirtualFree(buffer->memory, NULL, MEM_RELEASE);
	}
	
	buffer->width = width;
	buffer->height = height;
	buffer->bytesPerPixel = 4;
	
	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;
	buffer->info.bmiHeader.biSizeImage = 0;
	buffer->info.bmiHeader.biXPelsPerMeter = 0;
	buffer->info.bmiHeader.biYPelsPerMeter = 0;
	buffer->info.bmiHeader.biClrImportant = 0;

	int bitmapMemorySize = buffer->width * buffer->height * buffer->bytesPerPixel;
	buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	
	buffer->pitch = buffer->bytesPerPixel*buffer->width;
}

internal void Win32BufferToWindow(HDC context, int windowWidth, int windowHeight, Win32_offscreen_buffer buffer, int x, int y, int width, int height){	
	StretchDIBits(context, 
					0, 0, windowWidth, windowHeight,
					0, 0, buffer.width, buffer.height,
					buffer.memory,
					&buffer.info,
					DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK 
Win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam){
	LRESULT result = 0;
	
	switch(message)
	{		
		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZE\n");
			
		} break;
		case WM_DESTROY:
		{
			// TODO(KP): Handle as error
			running = false;
			OutputDebugStringA("WM_DESTROY\n");
		} break;
		case WM_CLOSE:
		{
			// TODO(KP): Handle with message to user?
			running = false;
			OutputDebugStringA("WM_CLOSE\n");
		} break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(window, &paint);
			
			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int height = paint.rcPaint.bottom - y;
			int width = paint.rcPaint.right - x;
			
			Win32WindowDimensions wd = Win32GetDimensions(window);
			Win32BufferToWindow(deviceContext, wd.width, wd.height, backBuffer, x, y, width, height);
			
			EndPaint(window, &paint);
		} break;
		default:
		{
			//OutputDebugStringA("default\n");
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}
	return(result);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS windowClass = {};
	
	Win32LoadXInput();
	
	Win32ResizeDIBSection(&backBuffer, 1280, 720);
	
		windowClass.style = CS_HREDRAW|CS_VREDRAW;
		windowClass.lpfnWndProc = Win32MainWindowCallback;
		windowClass.hInstance = hInstance;
		//windowClass.hIcon = ;
		windowClass.lpszClassName = "HandmadeHeroWindowClass";
		
	if(RegisterClass(&windowClass)){
		HWND window = CreateWindowEx(	0, windowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE, 
											CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,	CW_USEDEFAULT,
											0, 0, hInstance, 0);
		if(window){
			running = true;
			
			int xOffset = 0;
			int yOffset = 0;
			XINPUT_VIBRATION vibration;
			while(running){
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
					if(message.message == WM_QUIT){
						running = false;
					}
					
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				
				for(DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++){
					XINPUT_STATE controllerState;
					if(XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS){
						//NOTE: Controller is connected
						//TODO(KP): See if controllerState.dwPacketNumber increases too rapidly
						XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
						
						//NOTE: Commented sections are unused by handmade hero, but could be used by future programs
						
						bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
						bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
						//bool leftThumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
						//bool rightThumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
						bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool aButton = (pad->wButtons & XINPUT_GAMEPAD_A);
						bool bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
						bool xButton = (pad->wButtons & XINPUT_GAMEPAD_X);
						bool yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);
						
						int16_t leftStickX = pad->sThumbLX;
						int16_t leftStickY = pad->sThumbLY;
						
						//int16_t rightStickX = pad->sThumbRX;
						//int16_t rightStickY = pad->sThumbRY;
						
						//uint8_t leftTrigger = pad->bLeftTrigger;
						//uint8_t rightTrigger = pad->bRightTrigger;
						
						if(aButton){
							yOffset++;
						}
						if(yButton){
							yOffset--;
						}
						if(xButton){
							xOffset--;
						}
						if(bButton){
							xOffset++;
						}
						if(leftShoulder){
							vibration.wLeftMotorSpeed = 60000;
						}
						else{
							vibration.wLeftMotorSpeed = 0;
						}
						if(rightShoulder){
							vibration.wRightMotorSpeed = 60000;
						}
						else{
							vibration.wRightMotorSpeed = 0;
						}
						
						XInputSetState(0, &vibration);
					}
					else{
						//NOTE: Controller isn't connected
					}
				}
				renderWeirdGradient(backBuffer, xOffset, yOffset);
				HDC context = GetDC(window);
				Win32WindowDimensions wd = Win32GetDimensions(window);
				
				Win32BufferToWindow(context, wd.width, wd.height, backBuffer, 0, 0, wd.width, wd.height);
				ReleaseDC(window, context);
			}
		}
		else{
			//TODO(KP): Logging
		}
	}
	else{
		//TOFO(KP): Logging
	}
		
	return(0);
}