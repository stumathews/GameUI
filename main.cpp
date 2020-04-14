#include <D3DX10.h>
#include <windows.h>

HINSTANCE g_hInstance = nullptr;
HWND g_hWindow = nullptr;
D3D10_DRIVER_TYPE g_d3d10DriverType = D3D10_DRIVER_TYPE_HARDWARE;
IDXGISwapChain* g_pdx10SwapChain = nullptr;
ID3D10Device* g_id3dDevice = nullptr;
ID3D10RenderTargetView* g_pd3d10RenderTargetView = nullptr;

LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC device_context;
	PAINTSTRUCT paint_struct;

	switch (msg)
	{
	case WM_PAINT:
		device_context = BeginPaint(hWindow, &paint_struct);
		EndPaint(hWindow, &paint_struct);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWindow, msg, wParam, lParam);

	}
	return 0;
}

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	g_hInstance = hInstance;
	RECT rectangle = { 0, 0 ,400,300 };
	WNDCLASSEX window;

	window.cbSize = sizeof(WNDCLASSEX);
	window.style = CS_HREDRAW | CS_VREDRAW;
	window.lpfnWndProc = WndProc;
	window.cbClsExtra = 0;
	window.cbWndExtra = 0;
	window.hInstance = hInstance;
	window.hIcon = nullptr;
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.hbrBackground = (HBRUSH)(1);
	window.lpszMenuName = nullptr;
	window.lpszClassName = L"WindowClass";
	window.hIconSm = nullptr;

	if (!RegisterClassEx(&window))
		return E_FAIL;

	AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE);

	g_hWindow = CreateWindow(
		L"WindowClass",
		L"Setting up a window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rectangle.right - rectangle.left,
		rectangle.bottom - rectangle.top,
		NULL, NULL, hInstance, NULL);

	if (!g_hWindow)
		return E_FAIL;

	ShowWindow(g_hWindow, nCmdShow);

	return S_OK;
}

HRESULT InitD3D10()
{
	RECT rectangle;
	GetClientRect(g_hWindow, &rectangle);

	const UINT rectangle_width = rectangle.right - rectangle.left;
	const UINT rectangle_height = rectangle.bottom - rectangle.top;

	DXGI_SWAP_CHAIN_DESC swap_chain_desc;

	SecureZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));

	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferDesc.Width = rectangle_width;
	swap_chain_desc.BufferDesc.Height = rectangle_height;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.OutputWindow = g_hWindow;
	swap_chain_desc.Windowed = TRUE;
	
	
	HRESULT hResult = D3D10CreateDeviceAndSwapChain(nullptr,
	                                                g_d3d10DriverType,
	                                                nullptr,
	                                                0, D3D10_SDK_VERSION, &swap_chain_desc,
	                                                &g_pdx10SwapChain, &g_id3dDevice);

	if (FAILED(hResult))
		return hResult;

	ID3D10Texture2D* pBackBufferSurface;
	hResult = g_pdx10SwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D),
		(LPVOID*)&pBackBufferSurface);

	if (FAILED(hResult))
		return hResult;

	hResult = g_id3dDevice->CreateRenderTargetView(pBackBufferSurface, nullptr,
		&g_pd3d10RenderTargetView);

	pBackBufferSurface->Release();

	if (FAILED(hResult))
		return hResult;

	g_id3dDevice->OMSetRenderTargets(1,
		&g_pd3d10RenderTargetView, nullptr);

	D3D10_VIEWPORT view_port;
	SecureZeroMemory(&view_port, sizeof(view_port));
	
	view_port.TopLeftX = 0;
	view_port.TopLeftY = 0;
	view_port.Width = rectangle_width;
	view_port.Height = rectangle_height;
	view_port.MinDepth = 0.0f;
	view_port.MaxDepth = 1.0f;

	g_id3dDevice->RSSetViewports(1, &view_port);
	
	return S_OK;
}

void RenderScene()
{
	float clear_buffer_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

	g_id3dDevice->ClearRenderTargetView(
		g_pd3d10RenderTargetView,
		clear_buffer_color);

	g_pdx10SwapChain->Present(0, 0);
}

void DeviceCleanup()
{
	if(g_id3dDevice)
	{
		g_id3dDevice->ClearState();
		g_id3dDevice->Release();
	}

	if(g_pd3d10RenderTargetView)
		g_pd3d10RenderTargetView->Release();

	if(g_pdx10SwapChain)
		g_pdx10SwapChain->Release();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	InitD3D10();
	
	MSG msg = { nullptr };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			RenderScene();
		}
	}
	DeviceCleanup();
	
	return static_cast<int>(msg.wParam);
}
