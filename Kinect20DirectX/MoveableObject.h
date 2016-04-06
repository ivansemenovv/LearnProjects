#pragma once
#include <d3d11_1.h>
#include <directxmath.h>

using namespace DirectX;

class MoveableObject
{
public:
    MoveableObject::MoveableObject();

    void MoveAxisX(float step);
    void MoveAxisY(float step);
    void MoveAxisZ(float step);

    void RotateAxisX(float step);
    void RotateAxisY(float step);
    void RotateAxisZ(float step);

protected:
    XMMATRIX GetTransformation(XMMATRIX& originTransformationMatrix);

private:
    float m_xRotation = 0.0f;
    float m_yRotation = 0.0f;
    float m_zRotation = 0.0f;

    float m_xOffset = 0.0f;
    float m_yOffset = 0.0f;
    float m_zOffset = 0.0f;
};