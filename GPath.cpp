#include "include/GPath.h"

//  = (1 - t) * A + t * B;
GPoint interpolate(const GPoint& A, const GPoint& B, float t) {
    return A + t * (B - A);
}

// float findMax(float arr[], int n) {
//     float mAx= arr[0];
//     for (int i = 1; i < n; i++) {
//         if (arr[i] > max) {
//             mAx= arr[i];
//         }
//     }
//     return max;
// }

// float findMin(float arr[], int n) {
//     float min = arr[0];
//     for (int i = 1; i < n; i++) {
//         if (arr[i] < min) {
//             min = arr[i];
//         }
//     }
//     return min;
// }

void GPath::addRect(const GRect& r, GPath::Direction dir) {
    float left = r.left;
    float right = r.right;
    float top = r.top;
    float bottom = r.bottom;

    this->moveTo(left, top);
    if (dir == kCW_Direction) {
        this->lineTo(right, top);
        this->lineTo(right, bottom);
        this->lineTo(left, bottom);
    }
    else if (dir == kCCW_Direction) {
        this->lineTo(left, bottom);
        this->lineTo(right, bottom);
        this->lineTo(right, top);
    }
}

void GPath::addPolygon(const GPoint pts[], int count) {
    if (count < 2) return;
    GPoint gp = pts[0]; 
    this->moveTo(gp);
    for (int i = 1; i < count; i++) {
        gp = pts[i];
        this->lineTo(gp);
    }
}
GRect GPath::bounds() const {
    int count = this->fPts.size();
    if (count == 0) return {0, 0, 0, 0};
    
    float min[] = {this->fPts[0].x, this->fPts[0].y};   // x index 0, y index 1
    float max[] = {this->fPts[0].x, this->fPts[0].y};

    GPoint pts[GPath::kMaxNextPoints];
    GPath::Edger edger(*this);

    while (auto v = edger.next(pts)) {
        float tx, ty;
        GPoint A, B, C, D, AB, BC, CD, ABC, BCD, ABCD;
        switch(*v) {
            case kMove:
                break;
            case kLine:
                min[0] = std::min({min[0], pts[0].x, pts[1].x});
                min[1] = std::min({min[1], pts[0].y, pts[1].y});
                max[0] = std::max({max[0], pts[0].x, pts[1].x});
                max[1] = std::max({max[1], pts[0].y, pts[1].y});
                break;
            case kQuad:
                A = pts[0];
                B = pts[1];
                C = pts[2];

                //  fprime = 2(A - 2 * B + C) * t + 2(B - A) = 0
                // tx = -2 * (B.x - A.x) / (2 * (A.x - 2 * B.x + C.x));
                // ty = -2 * (B.y - A.y) / (2 * (A.y - 2 * B.y + C.y));

                tx = (A.x - B.x) / (A.x - 2 * B.x + C.x);
                ty = (A.y - B.y) / (A.y - 2 * B.y + C.y);

                if (tx >= 0 && tx <= 1) {
                    AB = interpolate(A, B, tx);
                    BC = interpolate(B, C, tx);
                    ABC = interpolate(AB, BC, tx);
                    min[0] = std::min(min[0], ABC.x);
                    max[0] = std::max(max[0], ABC.x);
                    min[1] = std::min(min[1], ABC.y);
                    max[1] = std::max(max[1], ABC.y);
                }
                if (ty >= 0 && ty <= 1) {
                    AB = interpolate(A, B, ty);
                    BC = interpolate(B, C, ty);
                    ABC = interpolate(AB, BC, ty);
                    min[1] = std::min(min[1], ABC.y);
                    max[1] = std::max(max[1], ABC.y);
                    min[0] = std::min(min[0], ABC.x);
                    max[0] = std::max(max[0], ABC.x);
                }

                min[0] = std::min({min[0], pts[0].x, pts[2].x});
                min[1] = std::min({min[1], pts[0].y, pts[2].y});
                max[0] = std::max({max[0], pts[0].x, pts[2].x});
                max[1] = std::max({max[1], pts[0].y, pts[2].y});
                break;
            case kCubic:
                A = pts[0];
                B = pts[1];
                C = pts[2];
                D = pts[3];

                // f' = -3A(1 - 2t + t^2) + 3B(1 - 2t + t^2) - 6Bt + 6Bt^2 + 6Ct - 6Ct^2 - 3Ct^2 + 3Dt^2
                // f' = 3(-A + 3B - 3C + D)t^2 + 6(A - 2B + C)t + 3(-A + B)
                // f' = (-A + 3B - 3C + D)t^2 + 2(A - 2B + C)t + (-A + B)
                // t = (-b +- sqrt(b^2 - 4ac)) / 2a
                // if a = 0 then t = -c / b

                float Ax = -A.x + 3 * B.x - 3 * C.x + D.x;
                float Bx= 2 * (A.x - 2 * B.x + C.x);
                float Bx_square = Bx* Bx;
                float Cx = -A.x + B.x;

                float Ay= -A.y + 3 * B.y - 3 * C.y + D.y;
                float By = 2 * (A.y - 2 * B.y + C.y);
                float By_square = By * By;
                float Cy = -A.y + B.y;
                
                float txminus, tyminus;
                if (Ax == 0) {
                    tx = -Cx / Bx;
                    txminus = -1;
                }
                else {
                    tx = (-Bx + sqrt(Bx_square - 4 * Ax * Cx)) / (2 * Ax);
                    txminus = (-Bx - sqrt(Bx_square - 4 * Ax * Cx)) / (2 * Ax);
                }
                if (Ay == 0) {
                    ty = -Cy / By;
                    tyminus = -1;
                } else {
                    ty = (-By + sqrt(By_square - 4 * Ay* Cy)) / (2 * Ay);
                    tyminus = (-By - sqrt(By_square - 4 * Ay* Cy)) / (2 * Ay);
                }

                if (tx >= 0 && tx <= 1) {
                    AB = interpolate(A, B, tx);
                    BC = interpolate(B, C, tx);
                    CD = interpolate(C, D, tx);
                    ABC = interpolate(AB, BC, tx);
                    BCD = interpolate(BC, CD, tx);
                    ABCD = interpolate(ABC, BCD, tx);
                    min[0] = std::min(min[0], ABCD.x);
                    max[0] = std::max(max[0], ABCD.x);
                    min[1] = std::min(min[1], ABCD.y);
                    max[1] = std::max(max[1], ABCD.y);
                }
                if (ty >= 0 && ty <= 1) {
                    AB = interpolate(A, B, ty);
                    BC = interpolate(B, C, ty);
                    CD = interpolate(C, D, ty);
                    ABC = interpolate(AB, BC, ty);
                    BCD = interpolate(BC, CD, ty);
                    ABCD = interpolate(ABC, BCD, ty);
                    min[0] = std::min(min[0], ABCD.x);
                    max[0] = std::max(max[0], ABCD.x);
                    min[1] = std::min(min[1], ABCD.y);
                    max[1] = std::max(max[1], ABCD.y);
                }
                if (txminus >= 0 && txminus <= 1) {
                    AB = interpolate(A, B, txminus);
                    BC = interpolate(B, C, txminus);
                    CD = interpolate(C, D, txminus);
                    ABC = interpolate(AB, BC, txminus);
                    BCD = interpolate(BC, CD, txminus);
                    ABCD = interpolate(ABC, BCD, txminus);
                    min[0] = std::min(min[0], ABCD.x);
                    max[0] = std::max(max[0], ABCD.x);
                    min[1] = std::min(min[1], ABCD.y);
                    max[1] = std::max(max[1], ABCD.y);
                }
                if (tyminus >= 0 && tyminus <= 1) {
                    AB = interpolate(A, B, tyminus);
                    BC = interpolate(B, C, tyminus);
                    CD = interpolate(C, D, tyminus);
                    ABC = interpolate(AB, BC, tyminus);
                    BCD = interpolate(BC, CD, tyminus);
                    ABCD = interpolate(ABC, BCD, tyminus);
                    min[0] = std::min(min[0], ABCD.x);
                    max[0] = std::max(max[0], ABCD.x);
                    min[1] = std::min(min[1], ABCD.y);
                    max[1] = std::max(max[1], ABCD.y);
                }

                min[0] = std::min({min[0], pts[0].x, pts[3].x});
                min[1] = std::min({min[1], pts[0].y, pts[3].y});
                max[0] = std::max({max[0], pts[0].x, pts[3].x});
                max[1] = std::max({max[1], pts[0].y, pts[3].y});
                break;
        }
    }
    return { min[0], min[1], max[0], max[1] };
}

void GPath::transform(const GMatrix& m) {
    int count = this->fPts.size();
    GPoint* arr = this->fPts.data();
    m.mapPoints(arr, arr, count);
}

void GPath::addCircle(GPoint center, float radius, GPath::Direction dir) {
    // GPoint lastpoint = center;
    this->moveTo(center.x + radius, center.y);
    float factor = 0.551915f;
    // float factor = 0.55228474983079;
    if (dir == kCCW_Direction) {
        this->cubicTo(center.x + radius, center.y - radius * factor, 
                    center.x + radius * factor, center.y - radius, 
                    center.x, center.y - radius);
        this->cubicTo(center.x - radius * factor, center.y - radius, 
                    center.x - radius, center.y - radius * factor, 
                    center.x - radius, center.y);
        this->cubicTo(center.x - radius, center.y + radius * factor, 
                    center.x - radius * factor, center.y + radius, 
                    center.x, center.y + radius);
        this->cubicTo(center.x + radius * factor, center.y + radius, 
                    center.x + radius, center.y + radius * factor, 
                    center.x + radius, center.y);
    }
    else if (dir == kCW_Direction) {
        this->cubicTo(center.x + radius, center.y + radius * factor, 
                    center.x + radius * factor, center.y + radius,
                    center.x, center.y + radius);
        this->cubicTo(center.x - radius * factor, center.y + radius,
                    center.x - radius, center.y + radius * factor,
                    center.x - radius, center.y);
        this->cubicTo(center.x - radius, center.y - radius * factor,
                    center.x - radius * factor, center.y - radius,
                    center.x, center.y - radius);
        this->cubicTo(center.x + radius * factor, center.y - radius,
                    center.x + radius, center.y - radius * factor,
                    center.x + radius, center.y);
    }
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    GPoint A = src[0];
    GPoint B = src[1];
    GPoint C = src[2];
    GPoint AB = interpolate(A, B, t);
    GPoint BC = interpolate(B, C, t);
    GPoint ABC = interpolate(AB, BC, t);
    
    dst[0] = A;
    dst[1] = AB;
    dst[2] = ABC;
    dst[3] = BC;
    dst[4] = C;
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    GPoint A = src[0];
    GPoint B = src[1];
    GPoint C = src[2];
    GPoint D = src[3];

    GPoint AB = interpolate(A, B, t);
    GPoint BC = interpolate(B, C, t);
    GPoint CD = interpolate(C, D, t);
    GPoint ABC = interpolate(AB, BC, t);
    GPoint BCD = interpolate(BC, CD, t);
    GPoint ABCD = interpolate(ABC, BCD, t);

    dst[0] = A;
    dst[1] = AB;
    dst[2] = ABC;
    dst[3] = ABCD;
    dst[4] = BCD;
    dst[5] = CD;
    dst[6] = D;
}