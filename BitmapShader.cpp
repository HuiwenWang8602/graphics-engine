#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"


class BitmapShader : public GShader {
public:
    BitmapShader(const GBitmap& device, const GMatrix& matrix, const GTileMode mode) : fBitmap(device), localMatrix(matrix), mode(mode) {}
    bool isOpaque() {
        return fBitmap.isOpaque();
    }
    bool setContext(const GMatrix &ctm) {
        GMatrix tmp = ctm * localMatrix;
        if (!tmp.invert()) {
            return false;
        }
        localInv = *tmp.invert();
        localInv = GMatrix::Scale(1.0f / fBitmap.width(), 1.0f / fBitmap.height()) * localInv;
        return true;
    }
    void shadeRow(int x, int y, int count, GPixel row[]) {
        GPoint gp = localInv * GPoint( {x + 0.5f, y + 0.5f} );
        float fx = gp.x;
        float fy = gp.y;
        float a = localInv[0];
        float b = localInv[1];
        float xbounds = 1.0f - 1.0f / fBitmap.width();
        float ybounds = 1.0f - 1.0f / fBitmap.height();
        if (b == 0) {
            float iy;
            switch (mode) {
                    case GTileMode::kClamp:
                        iy = std::clamp(fy, 0.0f, ybounds);
                        break;
                    case GTileMode::kRepeat: {
                        iy = fy - GFloorToInt(fy);
                        break;
                    }
                    case GTileMode::kMirror: {
                        iy = fy * 0.5f;
                        iy = iy - GFloorToInt(iy);
                        if (iy > 0.5f)
                            iy = 1 - iy;
                        iy *= 2;
                        break;
                    }
            }
            iy *= fBitmap.height();
            for (int i = 0; i < count; i++) {
                float ix;
                switch (mode) {
                    case GTileMode::kClamp:
                        ix = std::clamp(fx, 0.0f, xbounds);
                        break;
                    case GTileMode::kRepeat: {
                        ix = fx - GFloorToInt(fx);
                        break;
                    }
                    case GTileMode::kMirror: {
                        ix = fx * 0.5f;
                        ix = ix - GFloorToInt(ix);
                        if (ix > 0.5f)
                            ix = 1 - ix;
                        ix *= 2;
                        break;
                    }
                }
                ix *= fBitmap.width();
                row[i] = *fBitmap.getAddr(GFloorToInt(ix), GFloorToInt(iy));
                fx += a;
            }
        } else {
            for (int i = 0; i < count; i++) {
                float ix, iy;
                switch (mode) {
                    case GTileMode::kClamp:
                        ix = std::clamp(fx, 0.0f, xbounds);
                        iy = std::clamp(fy, 0.0f, ybounds);
                        break;
                    case GTileMode::kRepeat: {
                        ix = fx - GFloorToInt(fx);
                        iy = fy - GFloorToInt(fy);
                        break;
                    }
                    case GTileMode::kMirror: {
                        ix = fx * 0.5f;
                        ix = ix - GFloorToInt(ix);
                        if (ix > 0.5f)
                            ix = 1 - ix;
                        ix *= 2;
                        iy = fy * 0.5f;
                        iy = iy - GFloorToInt(iy);
                        if (iy > 0.5f)
                            iy = 1 - iy;
                        iy *= 2;
                        break;
                    }
                }
                ix *= fBitmap.width();
                iy *= fBitmap.height();
                row[i] = *fBitmap.getAddr(GFloorToInt(ix), GFloorToInt(iy));
                fx += a;
                fy += b;
            }
        }
    }
private:
    const GBitmap fBitmap;
    const GMatrix localMatrix;
    const GTileMode mode;
    GMatrix localInv;

    int modulo(int x, int m) {
        return (x % m + m) % m;
    }
};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap &bitmap, const GMatrix &matrix, const GTileMode mode) {
    if (!bitmap.pixels())
        return nullptr;
    return std::unique_ptr<GShader>(new BitmapShader(bitmap, matrix, mode));
}