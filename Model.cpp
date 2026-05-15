#include "Model.h"

#include "Math.h"

const int Model::reserved = 100;

Model::Model(Vec3 position, Vec3 rotation, size_t textureIndex, std::initializer_list<Triangle3> triangleList)
    : position(position), rotation(rotation), textureIndex(textureIndex), triangleList(triangleList)
{
    this->triangleList.reserve(reserved);
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

void Model::move(Vec3 distance)
{
    position += distance;
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
    return triangleList.size();
}

size_t Model::getTextureIndex() const
{
    return textureIndex;
}

const Triangle3& Model::getTriangle(size_t index) const
{
    return triangleList[index];
}

Triangle3& Model::getTriangle(size_t index)
{
    return triangleList[index];
}

void Model::addTriangle(const Triangle3& triangle)
{
    triangleList.push_back(triangle);
}