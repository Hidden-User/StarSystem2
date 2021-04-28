#include <iostream>
#include "StarSystem.h"
#include "d3dcompiler.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

HWND hWnd;
HINSTANCE hInstance;
MSG msg;

const wchar_t* windowName = L"Lab2";
const wchar_t *windowClassName = L"Lab2";

const char vertexShader[] = "cbuffer CB:register(b0){matrix proj;}struct POINT{float4 pos:SV_POSITION;float4 color:COLOR0;};POINT VS(float4 position:POSITION,float4 color:COLOR){POINT _out=(POINT)0;_out.pos=mul(position,proj);_out.color=color;return _out;}";
const char pixelShader[] = "struct POINT{float4 pos:SV_POSITION;float4 color:COLOR0;};float4 PS(POINT input):SV_Target{return input.color;}";

IDXGISwapChain* swapChain;
ID3D11RasterizerState* rasterState;
ID3D11Device* device;
ID3D11DeviceContext* context;
ID3D11RenderTargetView* targetView;
ID3D11Texture2D* depthStencil;
ID3D11DepthStencilState* DSS_zOn;
ID3D11DepthStencilState* DSS_zOff;
ID3D11DepthStencilView* depthStencilView;
ID3D11InputLayout* layout;
ID3D11Buffer* cBuff;

ID3D11VertexShader* vs;
ID3D11PixelShader* ps;

float backGrColor[] = { 0.7f, 0.7f, 0.7f, 1.0 };
cBuffer constB;

StarSystem ss;
std::mutex m_ss;
bool _stop;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void createWindow() {
	hInstance = GetModuleHandle(NULL);
	HCURSOR hCur = LoadCursor(hInstance, IDC_ARROW);

	LOGBRUSH logBr = { BS_SOLID, RGB(0,0,0), 0 };
	HBRUSH hbrBackGr = CreateBrushIndirect(&logBr);

	WNDCLASSEXA wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = hCur;
	wc.hbrBackground = hbrBackGr;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = (LPCSTR)windowClassName;
	wc.hIconSm = NULL;

	RegisterClassExA(&wc);

	hWnd = CreateWindowA(
		(LPCSTR)windowClassName,
		(LPCSTR)windowName,
		WS_OVERLAPPEDWINDOW,
		50,
		50,
		1280,
		800,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd) {
		throw 0;
	}

	ShowWindow(hWnd, SW_SHOW);

}

void startDX() {

	HRESULT hr = NULL;

	RECT rect = { 0 };
	GetClientRect(hWnd, &rect);
	DWORD wsX = rect.right - rect.left;
	DWORD wsY = rect.bottom - rect.top;

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Width = wsX;
	scd.BufferDesc.Height = wsY;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

	D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&scd,
		&swapChain,
		&device,
		NULL,
		&context
	);

	ID3D11Texture2D *backBuff = NULL;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuff);
	device->CreateRenderTargetView(backBuff, NULL, &targetView);

	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = wsX;
	descDepth.Height = wsY;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	device->CreateTexture2D(&descDepth, NULL, &depthStencil);

	D3D11_DEPTH_STENCIL_DESC dsd;
	ZeroMemory(&dsd, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = true;
	dsd.StencilReadMask = 0xFF;
	dsd.StencilWriteMask = 0xFF;
	dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	device->CreateDepthStencilState(&dsd, &DSS_zOn);
	dsd.DepthEnable = false;
	device->CreateDepthStencilState(&dsd, &DSS_zOff);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	descDSV.Flags = 0;
	device->CreateDepthStencilView(depthStencil, &descDSV, &depthStencilView);

	context->OMSetRenderTargets(1, &targetView, depthStencilView);

	D3D11_VIEWPORT vp;
	vp.Width = wsX * 1.0f;
	vp.Height = wsY * 1.0f;
	vp.MinDepth = 0.001f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	context->RSSetViewports(1, &vp);

	constB.projection = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0.0f, 1000.0f);

	D3D11_INPUT_ELEMENT_DESC inElDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	ID3DBlob* lBlob = NULL;
	ID3DBlob* errBlob = NULL;

	D3DCompile(vertexShader, sizeof(vertexShader), "vertexShader", NULL, NULL, "VS", "vs_4_1", NULL, NULL, &lBlob, &errBlob);

	if (errBlob != 0) {
		MessageBoxA(NULL, (LPCSTR)errBlob->GetBufferPointer(), "ERR", MB_ICONSTOP | MB_OK);
		errBlob->Release();
		errBlob = NULL;
	}

	device->CreateVertexShader(lBlob->GetBufferPointer(), lBlob->GetBufferSize(), NULL, &vs);

	hr = device->CreateInputLayout(inElDesc, 2, lBlob->GetBufferPointer(), lBlob->GetBufferSize(), &layout);

	if (FAILED(hr)) {
		std::cout << "create input layout error";
	}

	context->IASetInputLayout(layout);

	D3DCompile(pixelShader, sizeof(pixelShader), "pixelShader", NULL, NULL, "PS", "ps_4_1", NULL, NULL, &lBlob, &errBlob);

	if (errBlob != 0) {
		MessageBoxA(NULL, (LPCSTR)errBlob->GetBufferPointer(), "ERR", MB_ICONSTOP | MB_OK);
		errBlob->Release();
		errBlob = NULL;
	}

	hr = device->CreatePixelShader(lBlob->GetBufferPointer(), lBlob->GetBufferSize(), NULL, &ps);

	if (hr != S_OK) {
		MessageBoxA(hWnd, "ERR", "ERR", MB_OK);
	}

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = false;
	rasterDesc.FillMode = D3D11_FILL_SOLID;//!!!!
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	device->CreateRasterizerState(&rasterDesc, &rasterState);

	context->RSSetState(rasterState);

	lBlob->Release();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(cBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	hr = device->CreateBuffer(&bd, NULL, &cBuff);

	if (FAILED(hr)) {
		MessageBoxA(NULL, "error create const buff", "err", MB_ICONSTOP | MB_OK);
	}
}

void stopDX() {
	if (cBuff != NULL) {
		cBuff->Release();
	}
	if (ps != NULL) {
		ps->Release();
	}
	if (layout != NULL) {
		layout->Release();
	}
	if (vs != NULL) {
		vs->Release();
	}
	if (depthStencilView != NULL) {
		depthStencilView->Release();
	}
	if (DSS_zOff != NULL) {
		DSS_zOff->Release();
	}
	if (DSS_zOn != NULL) {
		DSS_zOn->Release();
	}
	if (depthStencil != NULL) {
		depthStencil->Release();
	}
	if (targetView != NULL) {
		targetView->Release();
	}
	if (swapChain != NULL) {
		swapChain->Release();
	}
	if (device != NULL) {
		device->Release();
	}
}

int main()
{
	_stop = false;
	timeBeginPeriod(1u);

	createWindow();
	startDX();

	auto func = [](void) {
		std::string str;
		unsigned index;
		float coef;
		bool in_ok = true;
		std::cout << "Please enter (+,-,*,&) (index) (value) to modify planet speed or ! to exit\n";
		while (IsWindow(hWnd) && !_stop)
		{
			std::cout << ">";

			std::cin >> str; switch (str.at(0))
			{
			default:
				std::cout << "Please enter (+,-,*,&) (index) (value) to modify planet speed or ! to exit\n";
				in_ok = false;
				break;
			case '+':
			case '-':
			case '*':
			case '&':
			case '!':
				break;
			}
			if (str.at(0) == '!') {
				_stop = true;
				break;
			}
			if (!in_ok) {
				in_ok = true;
				continue;
			}
			std::cin >> index >> coef;

			m_ss.lock();
			switch (str.at(0))
			{
			default:
				std::cout << "Please enter (+,-,*,&) (index) (value) to modify planet speed or ! to exit\n";
				break;
			case '+':
				ss.modifyPlanetSpeed(index, coef);
				break;
			case '-':
				ss.modifyPlanetSpeed(index, coef, 1u);
				break;
			case '*':
				ss.modifyPlanetSpeed(index, coef, 2u);
				break;
			case '&':
				ss.modifyPlanetSpeed(index, coef, 3u);
				break;
			}
			m_ss.unlock();

			str.clear();
			std::cin.get();
		}
	};

	std::thread thread(func);
	thread.detach();

	m_ss.lock();
	ss.initD3D(device, context, vs, ps);
	ss.init(5u);
	m_ss.unlock();
	while (true)
	{
		while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&msg);
		}
		m_ss.lock();
		ss.update();
		m_ss.unlock();
		if (_stop) {
			CloseWindow(hWnd);
			break;
		}
	}
	_stop = true;
	ss.stop();
	ss.update();

	Circle::g_m0.lock();
	stopDX();

	timeEndPeriod(1u);

	return 0;
}

StarSystem::StarSystem()
{
	this->planets = NULL;
	this->__planets = NULL;
}


StarSystem::~StarSystem()
{
	if (this->planets != NULL) {
		for (unsigned t = 0; t < this->countOfPlanet; t++) {
			if (this->planets[t] == NULL) {
				continue;
			}
			delete this->planets[t];
		}
		delete[] this->planets;
	}
	if (this->__planets != NULL) {
		for (unsigned t = 0; t < this->countOfPlanet; t++) {
			if (this->__planets[t] == NULL) {
				continue;
			}
			delete this->__planets[t];
		}
		delete[] this->__planets;
	}
}

void StarSystem::init(unsigned countOfPlanet)
{
	auto func = [](Planet* planet) {
		while (planet->isActive())
		{
			planet->update();
		}
	};

	this->countOfPlanet = countOfPlanet;
	this->planets = new Planet*[this->countOfPlanet];
	this->__planets = new std::thread*[this->countOfPlanet];

	for (unsigned t = 0; t < this->countOfPlanet; t++) {
		this->planets[t] = new Planet();
		(this->planets[t])->lock();
		(this->planets[t])->initD3D(this->device, this->context, this->vs, this->ps);
		(this->planets[t])->init((t * 65.0f) + 100.0f, ((rand() % 2) + 1.0f) * 10.0f, { (1.0f / this->countOfPlanet) * t,(1.0f / this->countOfPlanet) * t ,(1.0f / this->countOfPlanet) * t , 1.0f });
		(this->planets[t])->exist = true;
		this->__planets[t] = new std::thread(func, std::ref(this->planets[t]));
		(this->__planets[t])->detach();
	}

	this->circleInit(0.0f, 30.0f, { 1.0f, 1.0f, 0.0f, 1.0f });
}

void StarSystem::update()
{
	bool bb = true;
	cBuffer lCB;
	this->lock();
	Circle::g_m0.lock();

	context->OMSetDepthStencilState(DSS_zOff, 1);
	context->ClearRenderTargetView(targetView, backGrColor);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, (UINT8)0);

	lCB.projection = DirectX::XMMatrixTranspose(constB.projection);

	context->UpdateSubresource(cBuff, 0, NULL, &lCB, 0, 0);
	context->VSSetConstantBuffers(0, 1, &cBuff);

	context->VSSetShader(this->vs, NULL, NULL);
	context->PSSetShader(this->ps, NULL, NULL);

	Circle::g_m0.unlock();

	this->draw();

	Circle::g_m1.lock();
	for (unsigned t = 0; t < this->countOfPlanet; t++) {
		(this->planets[t])->waitDraw();
		(this->planets[t])->unlock();
	}

	while (true)
	{
		bb = true;
		for (unsigned t = 0; t < this->countOfPlanet; t++) {
			if (!(this->planets[t])->isActive()) {
				break;
			}
			if (!((this->planets[t])->drawSuccess())) {
				bb = false;
				break;
			}
		}
		if (bb) {
			break;
		}
	}

	for (unsigned t = 0; t < this->countOfPlanet; t++) {
		(this->planets[t])->lock();
	}
	Circle::g_m1.unlock();

	if (_stop) {
		this->unlock();
		return;
	}

	if (swapChain->Present(1, 0) != S_OK) {
		MessageBoxA(hWnd, "swapchain Err", "swapchain Err", MB_OK);
	}

	this->unlock();
}

void StarSystem::stop()
{
	for (unsigned t = 0; t < this->countOfPlanet; t++) {
		(this->planets[t])->stop();
	}
}

void StarSystem::modifyPlanetSpeed(unsigned index, float speed, unsigned type)
{
	this->lock();
	if (index >= this->countOfPlanet) {
		return;
	}
	switch (type)
	{
	default:
	case 0u:
		(this->planets[index])->setSpeed((this->planets[index])->getSpeed() + speed);
		break;
	case 1u:
		(this->planets[index])->setSpeed((this->planets[index])->getSpeed() - speed);
		break;
	case 2u:
		(this->planets[index])->setSpeed((this->planets[index])->getSpeed() * speed);
		break;
	case 3u:
		(this->planets[index])->setSpeed(speed);
		break;
	}
	this->unlock();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT res = NULL;
	switch (message)
	{
	default:
		res = DefWindowProc(hWnd, message, wParam, lParam);
		break;
	case WM_CLOSE:
		_stop = true;
		break;
	case WM_DESTROY:
		m_ss.lock();
		ss.stop();
		m_ss.unlock();
		PostQuitMessage(0);
		break;
	}
	return res;
}
