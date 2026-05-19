#pragma once

#include <vector>

#include "Vector.h"
#include "Triangle.h"

class Model
{
public:

    Model(Vec3 position, Vec3 rotation, size_t texId, const Triangle3DList& tris);

    void pitch(float deg);
    void yaw(float deg);
    void roll(float deg);

    void setPosition(const Vec3& position);
    void setRotation(const Vec3& rotation);

    void move(const Vec3& dist);

    Vec3 getPosition() const;
    Vec3 getRotation() const;
    size_t getSize() const;
    size_t getTextureIndex() const;
    const Triangle3& getTriangle(size_t index) const;
    Triangle3& getTriangle(size_t index);

    void addTriangle(const Triangle3& triangle);

private:

    static const int reserved;

    Vec3 position;
    Vec3 rotation;

    size_t texId;

    Triangle3DList tris;
};