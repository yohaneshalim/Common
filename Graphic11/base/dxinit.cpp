
#include "stdafx.h"
#include "dxinit.h"

using namespace graphic;


bool graphic::InitDirectX11(const HWND hWnd, const float width, const float height
	, OUT ID3D11Device **pd3dDevice
	, OUT ID3D11DeviceContext **pImmediateContext
	, OUT IDXGISwapChain **pSwapChain 
	, OUT ID3D11RenderTargetView **pRenderTargetView
	, OUT ID3D11DepthStencilView **pDepthStencilView
)
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = (UINT)width;
	sd.BufferDesc.Height = (UINT)height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	//sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	//sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, pSwapChain, pd3dDevice, &featureLevel, pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return false;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = (*pSwapChain)->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return false;

	hr = (*pd3dDevice)->CreateRenderTargetView(pBackBuffer, NULL, pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return false;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = (UINT)width;
	descDepth.Height = (UINT)height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	ID3D11Texture2D *pDepthStencil = NULL;
	hr = (*pd3dDevice)->CreateTexture2D(&descDepth, NULL, &pDepthStencil);
	if (FAILED(hr))
		return false;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = (*pd3dDevice)->CreateDepthStencilView(pDepthStencil, &descDSV, pDepthStencilView);
	if (FAILED(hr))
		return false;
	pDepthStencil->Release();

	(*pImmediateContext)->OMSetRenderTargets(1, pRenderTargetView, *pDepthStencilView);

	return true;
}


//------------------------------------------------------------
// Direct3D11 Deferred Context Initialize
bool graphic::InitDirectX11SwapChain(
	ID3D11Device *pd3dDevice
	, const HWND hWnd
	, const float width
	, const float height
	, OUT IDXGISwapChain **pSwapChain
	, OUT ID3D11RenderTargetView **pRenderTargetView
	, OUT ID3D11DepthStencilView **pDepthStencilView
)
{
	HRESULT hr = S_OK;
	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}

	if (FAILED(hr))
		return false;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = (UINT)width;
	sd.BufferDesc.Height = (UINT)height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	//sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	//sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	hr = dxgiFactory->CreateSwapChain(pd3dDevice, &sd, pSwapChain);

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = (*pSwapChain)->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return false;

	hr = pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return false;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = (UINT)width;
	descDepth.Height = (UINT)height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	ID3D11Texture2D *pDepthStencil = NULL;
	hr = pd3dDevice->CreateTexture2D(&descDepth, NULL, &pDepthStencil);
	if (FAILED(hr))
		return false;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = pd3dDevice->CreateDepthStencilView(pDepthStencil, &descDSV, pDepthStencilView);
	if (FAILED(hr))
		return false;

	pDepthStencil->Release();

	//(*pImmediateContext)->OMSetRenderTargets(1, pRenderTargetView, *pDepthStencilView);

	SAFE_RELEASE(dxgiFactory);

	return true;
}


bool graphic::InitDirectX11Deferred(
	ID3D11Device *pd3dDevice
	, const HWND hWnd
	, const float width
	, const float height
	, OUT ID3D11DeviceContext **pDeferredContext
	, OUT IDXGISwapChain **pSwapChain
	, OUT ID3D11RenderTargetView **pRenderTargetView
	, OUT ID3D11DepthStencilView **pDepthStencilView
)
{
	assert(0);

	//HRESULT hr = S_OK;
	//// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	//IDXGIFactory* dxgiFactory = nullptr;
	//{
	//	IDXGIDevice* dxgiDevice = nullptr;
	//	hr = pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
	//	if (SUCCEEDED(hr))
	//	{
	//		IDXGIAdapter* adapter = nullptr;
	//		hr = dxgiDevice->GetAdapter(&adapter);
	//		if (SUCCEEDED(hr))
	//		{
	//			hr = adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory));
	//			adapter->Release();
	//		}
	//		dxgiDevice->Release();
	//	}
	//}

	//if (FAILED(hr))
	//	return false;

	//DXGI_SWAP_CHAIN_DESC sd;
	//ZeroMemory(&sd, sizeof(sd));
	//sd.BufferCount = 1;
	//sd.BufferDesc.Width = width;
	//sd.BufferDesc.Height = height;
	//sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//sd.BufferDesc.RefreshRate.Numerator = 60;
	//sd.BufferDesc.RefreshRate.Denominator = 1;
	////sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//sd.OutputWindow = hWnd;
	//sd.SampleDesc.Count = 1;
	//sd.SampleDesc.Quality = 0;
	//sd.Windowed = TRUE;
	////sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//hr = dxgiFactory->CreateSwapChain(pd3dDevice, &sd, pSwapChain);

	//// Create a render target view
	//ID3D11Texture2D* pBackBuffer = NULL;
	//hr = (*pSwapChain)->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	//if (FAILED(hr))
	//	return false;

	//hr = pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, pRenderTargetView);
	//pBackBuffer->Release();
	//if (FAILED(hr))
	//	return false;

	//// Create depth stencil texture
	//D3D11_TEXTURE2D_DESC descDepth;
	//ZeroMemory(&descDepth, sizeof(descDepth));
	//descDepth.Width = width;
	//descDepth.Height = height;
	//descDepth.MipLevels = 1;
	//descDepth.ArraySize = 1;
	//descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//descDepth.SampleDesc.Count = 1;
	//descDepth.SampleDesc.Quality = 0;
	//descDepth.Usage = D3D11_USAGE_DEFAULT;
	//descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//descDepth.CPUAccessFlags = 0;
	//descDepth.MiscFlags = 0;
	//hr = pd3dDevice->CreateTexture2D(&descDepth, NULL, pDepthStencil);
	//if (FAILED(hr))
	//	return false;

	//// Create the depth stencil view
	//D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	//ZeroMemory(&descDSV, sizeof(descDSV));
	//descDSV.Format = descDepth.Format;
	//descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//descDSV.Texture2D.MipSlice = 0;
	//hr = pd3dDevice->CreateDepthStencilView(*pDepthStencil, &descDSV, pDepthStencilView);
	//if (FAILED(hr))
	//	return false;

	////hr = pd3dDevice->CreateDeferredContext(0, pDeferredContext);
	////if (FAILED(hr))
	////	return false;

	//(*pImmediateContext)->OMSetRenderTargets(1, pRenderTargetView, *pDepthStencilView);

	//SAFE_RELEASE(dxgiFactory);

	return false;
}

