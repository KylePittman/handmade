/*
	Create Windows Entry Point
*/

#include <windows.h>
#include <stdint.h>

#define local_persist static
#define global_variable static
#define internal static

// TODO(KP): This is global for now
global_variable bool running;


global_variable BITMAPINFO bitmapInfo;
global_variable void *bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;
global_variable int bytesPerPixel = 4;


internal void renderWeirdGradient(int xOffset, int yOffset){
	int width = bitmapWidth;
	int height = bitmapHeight;
	
	int pitch = bytesPerPixel*width;
	uint8_t *row = (uint8_t *)bitmapMemory;
	for(int y = 0; y < height; y++)	{
		uint32_t *pixel = (uint32_t *)row;
		for(int x = 0; x < width; x++)
		{
			uint8_t r = 0;
			uint8_t g = (y+yOffset);
			uint8_t b = (x+xOffset);
			
			
			*pixel++ = (r << 16 | g << 8 | b);
		}
		row+=pitch;
	}
}

internal void Win32ResizeDIBSection(int width, int height){	
	
	if(bitmapMemory){
		VirtualFree(bitmapMemory, NULL, MEM_RELEASE);
	}
	
	bitmapWidth = width;
	bitmapHeight = height;
	
	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
	bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biSizeImage = 0;
	bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	bitmapInfo.bmiHeader.biClrImportant = 0;

	int bitmapMemorySize = bitmapWidth * bitmapHeight * bytesPerPixel;
	bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindow(HDC context, RECT *clientRect, int x, int y, int width, int height){
	int windowWidth = clientRect->right - clientRect->left;
	int windowHeight = clientRect->bottom - clientRect->top;
	
	StretchDIBits(context, 
					/*x, y, width, height,
					x, y, width, height,*/
					0, 0, bitmapWidth, bitmapHeight,
					0, 0, windowWidth, windowHeight,
					bitmapMemory,
					&bitmapInfo,
					DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK 
Win32MainWindowCallback(HWND   window,
				UINT   message,
				WPARAM wParam,
				LPARAM lParam)
{
	LRESULT result = 0;
	
	switch(message)
	{		
		case WM_SIZE:
		{
			RECT clientRect;
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			Win32ResizeDIBSection(width, height);
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
			
			RECT clientRect;
			GetClientRect(window, &clientRect);
			Win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
			
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
	
		//TODO(KP): Check if redraws are necessary
		windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
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
			while(running){
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
					if(message.message == WM_QUIT){
						running = false;
					}
					
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				renderWeirdGradient(xOffset,yOffset);
				HDC context = GetDC(window);
				RECT clientRect;
				GetClientRect(window, &clientRect);
				
				int windowWidth = clientRect.right - clientRect.left;
				int windowHeight = clientRect.bottom - clientRect.top;
				
				Win32UpdateWindow(context, &clientRect, 0, 0, windowWidth, windowHeight);
				xOffset++;
				yOffset++;
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