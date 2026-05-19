#include "Model.h"

#include "Math.h"

const int Model::reserved = 100;

Model::Model(Vec3 position, Vec3 rotation, size_t texId, const Triangle3DList& tris)
    : position(position), rotation(rotation), texId(texId), tris(tris)
{
    this->tris.reserve(reserved);
}

void Model::pitch(float deg)
{
    rotation.x += Math::degToRad(deg);
}

void Model::yaw(float deg)
{
    rotation.y += Math::degToRad(deg);
}

void Model::roll(float deg)
{
    rotation.z += Math::degToRad(deg);
}

void Model::setPosition(const Vec3& position)
{
    this->position = position;
}

void Model::setRotation(const Vec3& rotation)
{
    this->rotation = rotation;
}

void Model::move(const Vec3& dist)
{
    position += dist;
}

Vec3 Model::getPosition() const
{
    return position;
}

Vec3 Model::getRotation() const
{
    return rotation;
}

size_t Model::getSize() const
{
    return tris.size();
}

size_t Model::getTextureIndex() const
{
    return texId;
}

const Triangle3& Model::getTriangle(size_t index) const
{
    return tris[index];
}

Triangle3& Model::getTriangle(size_t index)
{
    return tris[index];
}

void Model::addTriangle(const Triangle3& triangle)
{
    tris.push_back(triangle);
}