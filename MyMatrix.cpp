#include "include/GMatrix.h"
#include <cmath>
#include <optional>


GMatrix::GMatrix() {
        fMat[0] = 1;    fMat[2] = 0;    fMat[4] = 0;
        fMat[1] = 0;    fMat[3] = 1;    fMat[5] = 0;
};

GMatrix GMatrix::Translate(float tx, float ty) {
    float a = 1; float c = 0; float e = tx;
    float b = 0; float d = 1; float f = ty;
    return GMatrix(a, c, e, b, d, f);
}
GMatrix GMatrix::Scale(float sx, float sy) {
    float a = sx; float c = 0; float e = 0;
    float b = 0; float d = sy; float f = 0;
    return GMatrix(a, c, e, b, d, f);
}
GMatrix GMatrix::Rotate(float radians) {
    float a = cosf(radians); float c = -1 * sinf(radians); float e = 0;
    float b = -1 * c; float d = a; float f = 0;
    return GMatrix(a, c, e, b, d, f);
}

GMatrix GMatrix::Concat(const GMatrix& first, const GMatrix& second) {
    float a = second[0] * first[0] + second[1] * first[2];
    float b = second[0] * first[1] + second[1] * first[3];
    float c = second[2] * first[0] + second[3] * first[2];
    float d = second[2] * first[1] + second[3] * first[3];
    float e = second[4] * first[0] + second[5] * first[2] + first[4];
    float f = second[4] * first[1] + second[5] * first[3] + first[5];
    return GMatrix(a, c, e, b, d, f);
}

std::optional<GMatrix> GMatrix::invert() const {
    float det = fMat[0] * fMat[3] - fMat[1] * fMat[2];
    if (det == 0) {
        return std::nullopt;
    }
    float a = fMat[3] / det;
    float b = -1 * fMat[1] / det;
    float c = -1 * fMat[2] / det;
    float d = fMat[0] / det;
    float e = (fMat[2] * fMat[5] - fMat[3] * fMat[4]) / det;
    float f = (fMat[1] * fMat[4] - fMat[0] * fMat[5]) / det;
    return GMatrix(a, c, e, b, d, f);
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; i++) {
        float x = src[i].x * fMat[0] + src[i].y * fMat[2] + fMat[4];
        float y = src[i].x * fMat[1] + src[i].y * fMat[3] + fMat[5];
        dst[i] = {x, y};
    }
}