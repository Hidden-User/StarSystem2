#pragma once
#include <thread>
#include <mutex>
#include "d3d11.h"
#include "DirectXMath.h"
#include <string>
#include "timeapi.h"

struct F2
{
	float x;
	float y;
};

struct F3
{
	float x;
	float y;
	float z;
};

struct F4
{
	float x;
	float y;
	float z;
	float w;
};

struct _POINT
{
	F4 pos;
	F4 col;
};

struct Triangle
{
	_POINT A;
	_POINT B;
	_POINT C;
};

struct cBuffer
{
	DirectX::XMMATRIX projection;
};

const float oneAngleInRad = 0.0174533f;

class Circle
{
public:
	Circle();
	~Circle();

	void initD3D(ID3D11Device* device, ID3D11DeviceContext* context);
	void circleInit(float dist, float size, F4 color);

	virtual void update() = 0;
	void move(float x, float y);
	void draw();
	void waitDraw();

	void lock();
	void unlock();
	void stop();

	bool drawSuccess();
	unsigned getDeltaTime();

	static std::mutex g_m0;
	static std::mutex g_m1;

protected:

	ID3D11DeviceContext* context;
	ID3D11Device* device;

private:

	Triangle* triangleList;
	unsigned* indexes;
	unsigned countOfTriangle;

	DWORD dw0;
	DWORD dw1;
	bool drawS;

	ID3D11Buffer* triangleBuff;
	ID3D11Buffer* indexBuff;

	std::mutex in_m;
};

