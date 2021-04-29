#include "Circle.h"

std::mutex Circle::g_m0;
std::mutex Circle::g_m1;

Circle::Circle()
{
	this->device = NULL;
	this->context = NULL;
	this->triangleBuff = NULL;
	this->indexBuff = NULL;
	this->indexes = NULL;
	this->triangleList = NULL;
	this->dw0 = timeGetTime();
}

Circle::~Circle()
{
	if (this->indexes != NULL) {
		delete[] this->indexes;
	}
	if (this->triangleList != NULL) {
		delete[] this->triangleList;
	}
	if (this->indexBuff != NULL) {
		(this->indexBuff)->Release();
	}
	if (this->triangleBuff != NULL) {
		(this->triangleBuff)->Release();
	}
}

void Circle::initD3D(ID3D11Device * device, ID3D11DeviceContext * context)
{
	this->device = device;
	this->context = context;
}

void Circle::circleInit(float dist, float size, F4 color)
{
	if (this->device == NULL || this->context == NULL) {
		return;
	}

	HRESULT hr = NULL;
	float angle = 0.0f;

	this->countOfTriangle = 180u;
	this->triangleList = new Triangle[this->countOfTriangle];
	this->indexes = new unsigned[this->countOfTriangle * 3];

	Triangle trg;
	trg.A.pos = { dist, 0.0f, 0.1f, 1.0f };
	trg.A.col = trg.B.col = trg.C.col = color;
	trg.B.pos.z = 0.1f;
	trg.B.pos.w = 1.0f;
	trg.C.pos.z = 0.1f;
	trg.C.pos.w = 1.0f;

	for (unsigned t = 0; t < this->countOfTriangle; t++) {
		trg.B.pos.x = (cosf(angle) * size) + dist;
		trg.B.pos.y = sinf(angle) * size;
		angle += oneAngleInRad * 2;
		trg.C.pos.x = (cosf(angle) * size) + dist;
		trg.C.pos.y = sinf(angle) * size;
		this->triangleList[t] = trg;
		this->indexes[t * 3] = t * 3;
		this->indexes[(t * 3) + 1] = (t * 3) + 1;
		this->indexes[(t * 3) + 2] = (t * 3) + 2;
	}

	D3D11_SUBRESOURCE_DATA _trgSR;
	_trgSR.pSysMem = this->triangleList;
	_trgSR.SysMemPitch = 0;
	_trgSR.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC tbd;
	tbd.Usage = D3D11_USAGE_DYNAMIC;
	tbd.ByteWidth = sizeof(Triangle) * this->countOfTriangle;
	tbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	tbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tbd.MiscFlags = 0;
	tbd.StructureByteStride = 0;

	hr = (this->device)->CreateBuffer(&tbd, &_trgSR, &this->triangleBuff);

	if (FAILED(hr)) {
		MessageBoxA(NULL, "error create buff", "err", MB_ICONSTOP | MB_OK);
	}

	D3D11_SUBRESOURCE_DATA _IndexesSR;

	_IndexesSR.pSysMem = this->indexes;
	_IndexesSR.SysMemPitch = 0;
	_IndexesSR.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(unsigned) * 3 * this->countOfTriangle;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	hr = (this->device)->CreateBuffer(&ibd, &_IndexesSR, &this->indexBuff);

	if (FAILED(hr)) {
		MessageBoxA(NULL, "error create buff", "err", MB_ICONSTOP | MB_OK);
	}

}

void Circle::move(float x, float y)
{
	for (unsigned t = 0; t < this->countOfTriangle; t++) {
		this->triangleList[t].A.pos.x += x;
		this->triangleList[t].A.pos.y += y;
		this->triangleList[t].B.pos.x += x;
		this->triangleList[t].B.pos.y += y;
		this->triangleList[t].C.pos.x += x;
		this->triangleList[t].C.pos.y += y;
	}
}

void Circle::draw()
{
	UINT stride = sizeof(_POINT);
	UINT offset = 0;
	D3D11_MAPPED_SUBRESOURCE mpsr = { 0 };

	Circle::g_m0.lock();

	(this->context)->Map(this->triangleBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mpsr);

	memcpy(mpsr.pData, this->triangleList, sizeof(Triangle) * this->countOfTriangle);

	(this->context)->Unmap(this->triangleBuff, 0);

	(this->context)->IASetVertexBuffers(0, 1, &this->triangleBuff, &stride, &offset);
	(this->context)->IASetIndexBuffer(this->indexBuff, DXGI_FORMAT_R32_UINT, 0);

	(this->context)->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	(this->context)->DrawIndexed(this->countOfTriangle * 3, 0, 0);

	this->drawS = true;

	Circle::g_m0.unlock();
	Circle::g_m1.lock();
	Circle::g_m1.unlock();
}

void Circle::waitDraw()
{
	this->drawS = false;
}

void Circle::lock()
{
	this->in_m.lock();
}

void Circle::unlock()
{
	this->in_m.unlock();
}

void Circle::stop()
{
	this->drawS = true;
}

bool Circle::drawSuccess()
{
	return this->drawS;
}

unsigned Circle::getDeltaTime()
{
	this->dw1 = timeGetTime();
	if (dw1 > dw0) {
		dw1 -= dw0;
		dw0 += dw1;
		return dw1;
	}
	dw0 = dw1;
	return dw1;
}
