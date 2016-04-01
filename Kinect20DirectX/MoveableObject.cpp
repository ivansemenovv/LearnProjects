#include "MoveableObject.h"

MoveableObject::MoveableObject() :
    m_yRotation(0.0f),
    m_xRotation(0.0f),
    m_zRotation(0.0f),
    m_yOffset(0.0f),
    m_xOffset(0.0f),
    m_zOffset(0.0f) 
{}

void MoveableObject::MoveAxisX(float step)
{
    m_xOffset += step;
}

void MoveableObject::MoveAxisY(float step)
{
    m_yOffset += step;
}

void MoveableObject::MoveAxisZ(float step)
{
    m_zOffset += step;
}

void MoveableObject::RotateAxisX(float step)
{
    m_xRotation += step;
}

void MoveableObject::RotateAxisY(float step)
{
    m_yRotation += step;
}

void MoveableObject::RotateAxisZ(float step)
{
    m_zRotation += step;
}

XMMATRIX MoveableObject::GetTransformation()
{
    XMMATRIX transform;

    auto viewTranslationMatrix = XMMatrixTranslation(m_xOffset, m_yOffset, m_zOffset);
    transform = XMMatrixIdentity() * viewTranslationMatrix * XMMatrixRotationX(m_xRotation) * XMMatrixRotationY(m_yRotation) * XMMatrixRotationZ(m_zRotation);

    return transform;
}
