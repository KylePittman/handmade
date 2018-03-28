/*
	Create Windows Entry Point
*/

#include <windows.h>

#define local_persist static
#define global_variable static
#define internal static

// TODO(KP): This is global for now
global_variable bool running;


global_variable BITMAPINFO bitmapInfo;
global_variable void *bitmapMemory;
global_variable HBITMAP bitmapHandle;
global_variable HDC bitmapDeviceContext;

internal void Win32ResizeDIBSection(int width, int height){
	
	
	if(bitmapHandle){
		DeleteObject(bitmapHandle);
	}
	
	if(!bitmapDeviceContext){
		// TODO(KP): Recreate?
		bitmapDeviceContext = CreateCompatibleDC(0);
	}
	
	//TODO(KP): BulletProof this
	
	
	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biSizeImage = 0;
	bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	bitmapInfo.bmiHeader.biClrImportant = 0;
	
	
	bitmapHandle = CreateDIBSection(bitmapDeviceContext, &bitmapInfo,
									DIB_RGB_COLORS,
									&bitmapMemory,
									0, 0);
	
}

internal void Win32UpdateWindow(HDC context, int x, int y, int width, int height){
	StretchDIBits(context, 
					x, y, width, height,
					x, y, width, height,
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
			Win32UpdateWindow(deviceContext, x, y, width, height);
			
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
		HWND windowHandle = CreateWindowEx(	0, windowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW|WS_VISIBLE, 
											CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,	CW_USEDEFAULT,
											0, 0, hInstance, 0);
		if(windowHandle){
			running = true;
			while(running){
				MSG message;
				BOOL messageResult = GetMessage(&message, 0, 0, 0);
				if(messageResult > 0){
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				else{
					break;
				}
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