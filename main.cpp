#include <iostream>
#include <ostream>
#include "d3dApp.h"

class MyApp : public D3DApp
{
public:
	explicit MyApp(HINSTANCE hInstance);

	~MyApp() override;
	void initApp() override;
	void onResize() override;
	void updateScene(float dt) override;
	void drawScene() override;
};

MyApp::MyApp(HINSTANCE hInstance): D3DApp(hInstance)
{
}

MyApp::~MyApp()
{
	if (md3dDevice)
		md3dDevice->ClearState();
}

void MyApp::initApp()
{
	D3DApp::initApp();
}

void MyApp::onResize()
{
	D3DApp::onResize();
}

void MyApp::updateScene(float dt)
{
	D3DApp::updateScene(dt);
}

void MyApp::drawScene()
{
	D3DApp::drawScene();

	const D3DXCOLOR BLACK(0.0f, 0.0f, 0.0f, 1.0f);
	RECT r = { 5, 5, 0, 0 };

	mFont->DrawText(0, mFrameStats.c_str(), -1, &r, DT_NOCLIP, BLACK);

	mSwapChain->Present(0, 0);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MyApp app(hInstance);
	app.initApp();
	return app.run();
}
