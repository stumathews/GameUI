#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <D3DX10.h>
#include <windows.h>
#include <dinput.h>
#include <iostream>
#include <ostream>

#define KEYPRESSED(buffer, key) (buffer[key] & 0x80)

HINSTANCE g_hInstance = nullptr;
HWND g_hWindow = nullptr;
D3D10_DRIVER_TYPE g_d3d10DriverType = D3D10_DRIVER_TYPE_HARDWARE;
IDXGISwapChain* g_pdx10SwapChain = nullptr;
ID3D10Device* g_id3dDevice = nullptr;
ID3D10RenderTargetView* g_pd3d10RenderTargetView = nullptr;
LPDIRECTINPUTDEVICE8 g_lpInputDevice;
ID3D10EffectTechnique* g_pd3d10EffectTechnique = nullptr;
ID3D10Effect* g_id3dEffect = nullptr;

struct TriangleVertex
{
	D3DXVECTOR3 Location;
	D3DXVECTOR3 Color;
};

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

// {C1C315B8-E544-4B5C-A7F5-BC1DF0391975}
static const GUID g_ApplicationGUID = { 0xc1c315b8, 0xe544, 0x4b5c, { 0xa7, 0xf5, 0xbc, 0x1d, 0xf0, 0x39, 0x19, 0x75 } };


bool SetupD3DInput()
{
	LPDIRECTINPUT8 lpdi;
	HRESULT hResult;

	hResult = DirectInput8Create(g_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&lpdi, NULL);
	if(FAILED(hResult))
	{
		// do something
	}

	hResult = lpdi->CreateDevice(GUID_SysKeyboard,
		&g_lpInputDevice, NULL);
	if (FAILED(hResult))
	{
		g_lpInputDevice->Release();
		return FALSE;
	}

	hResult = g_lpInputDevice->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(hResult))
	{
		g_lpInputDevice->Release();
		return FALSE;
	}

	hResult = g_lpInputDevice->SetCooperativeLevel(g_hWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if(FAILED(hResult))
	{
		g_lpInputDevice->Release();
		return FALSE;
	}

	g_lpInputDevice->Acquire();
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

	hResult = g_id3dDevice->CreateRenderTargetView(pBackBufferSurface, 
												nullptr, 
												&g_pd3d10RenderTargetView);

	pBackBufferSurface->Release();

	if (FAILED(hResult))
		return hResult;

	g_id3dDevice->OMSetRenderTargets(1,
										&g_pd3d10RenderTargetView, 
										nullptr);

	D3D10_VIEWPORT view_port;
	SecureZeroMemory(&view_port, sizeof(view_port));
	
	view_port.TopLeftX = 0;
	view_port.TopLeftY = 0;
	view_port.Width = rectangle_width;
	view_port.Height = rectangle_height;
	view_port.MinDepth = 0.0f;
	view_port.MaxDepth = 1.0f;

	g_id3dDevice->RSSetViewports(1,
								&view_port);

	// make vertex buffer 
	D3D10_BUFFER_DESC vert_buffer_desc;
	vert_buffer_desc.Usage = D3D10_USAGE_DEFAULT;
	vert_buffer_desc.ByteWidth = sizeof(TriangleVertex) * 3;
	vert_buffer_desc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vert_buffer_desc.CPUAccessFlags = 0;
	vert_buffer_desc.MiscFlags = 0;

	TriangleVertex array_of_vertex_data[] =
	{
		{D3DXVECTOR3(0.0f, 1.0f, 1.0f)},
		{D3DXVECTOR3(1.0f, -1.0f, 1.0f)},
		{D3DXVECTOR3(-1.0f, -1.0f, 1.0f)}
	};

	D3D10_SUBRESOURCE_DATA sub_resource_data;
	sub_resource_data.pSysMem = array_of_vertex_data;
	sub_resource_data.SysMemPitch = 0;
	sub_resource_data.SysMemSlicePitch = 0;

	ID3D10Buffer* vertex_buffer[2] = { nullptr, nullptr };
	hResult = g_id3dDevice->CreateBuffer(&vert_buffer_desc,
										&sub_resource_data,
										&vertex_buffer[0]);

	if (FAILED(hResult))
		return hResult;

	// make index buffer
	D3D10_BUFFER_DESC index_buffer_desc;
	index_buffer_desc.Usage = D3D10_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = sizeof(TriangleVertex) * 3;
	index_buffer_desc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	index_buffer_desc.CPUAccessFlags = 0;
	index_buffer_desc.MiscFlags = 0;

	UINT array_of_index_data[] = { 1, 2, 3, 4 };

	D3D10_SUBRESOURCE_DATA index_sub_resource_data;
	index_sub_resource_data.pSysMem = array_of_index_data;
	index_sub_resource_data.SysMemPitch = 0;
	index_sub_resource_data.SysMemSlicePitch = 0;

	ID3D10Buffer* indexBuffer = nullptr;
	g_id3dDevice->CreateBuffer(&index_buffer_desc, 
								&index_sub_resource_data,
								&indexBuffer);

	UINT start_input_slot = 0;
	UINT number_buffers_in_array = 1;
	UINT offset_value = 0;
	UINT stride_value = sizeof(TriangleVertex);

	g_id3dDevice->IASetVertexBuffers(start_input_slot,
		number_buffers_in_array,
		vertex_buffer,
		&stride_value,
		&offset_value);

	hResult = D3DX10CreateEffectFromFile(L"effect.fx",
		NULL,
		NULL,
		"fx_4_0",
		D3D10_SHADER_ENABLE_STRICTNESS,
		0,
		g_id3dDevice,
		NULL,
		NULL,
		&g_id3dEffect,
		NULL,
		NULL);


	if (FAILED(hResult))
		return hResult;

	g_pd3d10EffectTechnique = g_id3dEffect->GetTechniqueByName("Triangle");

	D3D10_PASS_DESC pass_desc;
	g_pd3d10EffectTechnique->GetPassByIndex(0)->GetDesc(&pass_desc);

	// define input layout

	D3D10_INPUT_ELEMENT_DESC input_layout_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	int number_of_inputs = sizeof(input_layout_desc) / sizeof(input_layout_desc[0]);

	ID3D10InputLayout* p_input_layout = nullptr;
	hResult = g_id3dDevice->CreateInputLayout(input_layout_desc, 
												number_of_inputs,
												pass_desc.pIAInputSignature, 
												pass_desc.IAInputSignatureSize, 
												&p_input_layout);

	if (FAILED(hResult))
		return hResult;
	
	
	g_id3dDevice->IASetInputLayout(p_input_layout);
	g_id3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Rasterizer state object
	
	ID3D10RasterizerState* general_rasterizer_state;
	D3D10_RASTERIZER_DESC rasterizer_state_desc;
	g_id3dDevice->CreateRasterizerState(&rasterizer_state_desc,
		&general_rasterizer_state);

	// depth stencil state object
	
	D3D10_DEPTH_STENCIL_DESC depth_stencil_desc;
	ID3D10DepthStencilState* p_depth_stencil_state;
	g_id3dDevice->CreateDepthStencilState(&depth_stencil_desc, &p_depth_stencil_state);

	// Blend state object
	D3D10_BLEND_DESC blend_state_desc;
	ID3D10BlendState* p_blend_state = nullptr;
	g_id3dDevice->CreateBlendState(&blend_state_desc, &p_blend_state);
	g_id3dDevice->OMSetBlendState(p_blend_state, 0, 0xffffffff);

	// sample state object

	D3D10_SAMPLER_DESC sampler_state_desc;
	ID3D10SamplerState* p_sampler_state = nullptr;

	g_id3dDevice->CreateSamplerState(&sampler_state_desc, &p_sampler_state);
	
	return S_OK;
}



void RenderScene()
{
	float clear_buffer_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	g_id3dDevice->ClearRenderTargetView(
		g_pd3d10RenderTargetView,
		clear_buffer_color);

	D3D10_TECHNIQUE_DESC technique;
	g_pd3d10EffectTechnique->GetDesc(&technique);
	for(int i = 0; i < technique.Passes; i++)
	{
		g_pd3d10EffectTechnique->GetPassByIndex(i)->Apply(0);
		g_id3dDevice->Draw(3, 0);
	}

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

void GetInput()
{
	char g_KeyboardBuffer[256];
	const HRESULT hResult = g_lpInputDevice->GetDeviceState(sizeof(g_KeyboardBuffer), static_cast<LPVOID>(&g_KeyboardBuffer));
	if (FAILED(hResult))
	{
		// Let do the worst thing and assume its ok and this will not be run. Bad.
		return;
	}
	
	if (KEYPRESSED(g_KeyboardBuffer, DIK_SPACE))
	{
		std::cout << "S key was pressed" << std::endl;
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	if (FAILED(InitWindow(hInstance, nCmdShow))) 
	{
		DeviceCleanup();
		return 0;
	}

	InitD3D10();
	SetupD3DInput();
	
	MSG msg = { nullptr};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			GetInput();
			RenderScene();
		}
	}
	DeviceCleanup();
	
	return static_cast<int>(msg.wParam);
}
