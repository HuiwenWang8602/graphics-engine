/*
 *  Copyright 2024 <me>
 */

#include "starter_canvas.h"
# include "iostream"

// static inline uint32_t absvalue(uint32_t x) {
//     uint32_t m = x >> 31;
//     return (x ^ m) - m;
// }

/* Given two points, clips the two points and adds to list of edges */
void MyCanvas::clip(GPoint p0, GPoint p1, std::vector<GEdge> &edges) {
    int top = 0;
    int left = 0;
    int right = fDevice.width();
    int bot = fDevice.height();
    int w = 1;
    // if (GRoundToInt(p0.y) == GRoundToInt(p1.y)) return;
    if (p0.y > p1.y) {  // make sure p0 above p1
        std::swap(p0, p1);
        w = -1;     // w = 1 means up to down, w = -1 is down to up
    }
    if (p1.y < top) return;
    if (p0.y < top) {
        p0.x = p0.x + (top - p0.y) * (p1.x - p0.x) / (p1.y - p0.y); 
        p0.y = top;         
    }
    if (p0.y > bot) return;
    if (p1.y > bot) {
        p1.x = p0.x + (bot - p0.y) * (p1.x - p0.x) / (p1.y - p0.y);
        p1.y = bot;
    }
    // do x clipping
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }
    if (p1.x < left) {  // projects entire line segment to left
        p0.x = left;
        p1.x = left;
    }
    else if (p0.x < left) { // bend line segment
        GPoint temp;
        temp.x = left;
        temp.y = p0.y;
        p0.y = p0.y + (left - p0.x) * (p1.y - p0.y) / (p1.x - p0.x);
        p0.x = left;
        addEdge(temp, p0, w, edges);
    }
    if (p0.x > right) { // project
        p0.x = right;
        p1.x = right;
    }
    else if (p1.x > right) {    // bend line segment 
        GPoint temp;
        temp.x = right;
        temp.y = p1.y;
        p1.y = p0.y + (right - p0.x) * (p1.y - p0.y) / (p1.x - p0.x);
        p1.x = right;
        addEdge(temp, p1, w, edges);
    }
    addEdge(p0, p1, w, edges);
}

void MyCanvas::addEdge(GPoint p0, GPoint p1, int w, std::vector<GEdge> &edges) {
    GEdge e = GEdge::makeGEdge(p0, p1, w);
    if (e.top != e.bot) edges.push_back(e);
}

/* Returns sorted edges with clipping done from points */
std::vector<GEdge> MyCanvas::makeEdges(const GPoint points[], int count) {
    std::vector<GEdge> edges;
    for (int i = 0; i < count - 1; i++) {
        clip(points[i], points[i+1], edges);
    }
    clip(points[count - 1], points[0], edges);
    std::sort(edges.begin(), edges.end());
    return edges;
}

void MyCanvas::clear(const GColor& color) {
    int width = fDevice.width();
    int height = fDevice.height();
    GPixel* dst = fDevice.getAddr(0,0);

    GPixel src = premul(color);

    for (int i = 0; i < width * height; i++) {  // access every pixel in bitmap
        *dst = src;
        dst++;
    }
}

void MyCanvas::drawPath(const GPath& path, const GPaint& paint) {
    int count = path.countPoints();
    if (count < 3) return;

    GPath newpath(path);
    newpath.transform(ctm.top());

    std::vector<GEdge> edges;
    GPoint pts[GPath::kMaxNextPoints];
    GPath::Edger edger(newpath);
    std::vector<GPoint> bitmap_pts;
    while (auto v = edger.next(pts)) {
        switch(*v) {
            case GPath::kMove: {
                break;
            }
            case GPath::kLine: {
                clip(pts[0], pts[1], edges); 
                // bitmap_pts.push_back(pts[0]);
                // bitmap_pts.push_back(pts[1]);
                break;
            }
            case GPath::kQuad: {
                // Number of segments = sqrt(|error| / tolerance)
                GPoint A = pts[0];
                GPoint B = pts[1];
                GPoint C = pts[2];
                GPoint E = A - 2 * B + C;
                // E = E * 0.25; 
                int num_segs = GCeilToInt(sqrt(E.length()));
                float t_inc = 1.0f / num_segs;
                float t = 0;

                GPoint gp = A;
                for (int i = 1; i < num_segs; i++) {
                    t += t_inc;
                    
                    GPoint AB = interpolate(A, B, t);
                    GPoint BC = interpolate(B, C, t);
                    GPoint ABC = interpolate(AB, BC, t);

                    clip(gp, ABC, edges);
                    // bitmap_pts.push_back(gp);
                    // bitmap_pts.push_back(ABC);
                    gp = ABC;
                }
                clip(gp, C, edges); 
                // bitmap_pts.push_back(gp);
                // bitmap_pts.push_back(C);
                break;
            }
            case GPath::kCubic: {
                GPoint A = pts[0];
                GPoint B = pts[1];
                GPoint C = pts[2];
                GPoint D = pts[3];
                GPoint E0 = A - 2 * B + C;
                GPoint E1 = B - 2 * C + D;
                GPoint E = {std::max(abs(E0.x), abs(E1.x)), std::max(abs(E0.y), abs(E1.y))};
                int num_segs = GCeilToInt(sqrt(3 * E.length()));

                float t_inc = 1.0f / num_segs;
                float t = 0;

                GPoint gp = A;
                for (int i = 1; i < num_segs; i++) {
                    t += t_inc;

                    GPoint AB = interpolate(A, B, t);
                    GPoint BC = interpolate(B, C, t);
                    GPoint CD = interpolate(C, D, t);
                    GPoint ABC = interpolate(AB, BC, t);
                    GPoint BCD = interpolate(BC, CD, t);
                    GPoint ABCD = interpolate(ABC, BCD, t);

                    clip(gp, ABCD, edges);
                    // bitmap_pts.push_back(gp);
                    // bitmap_pts.push_back(ABCD);
                    gp = ABCD;
                }
                clip(gp, D, edges);
                // bitmap_pts.push_back(gp);
                // bitmap_pts.push_back(D);
                break;
            }
        }
    }

    // std::vector<GEdge> edges;
    // for (int i = 0; i < bitmap_pts.size(); i += 2) {
    //     clip(bitmap_pts[i], bitmap_pts[i+1], edges);
    // }
    std::sort(edges.begin(), edges.end());

    if (edges.size() < 2) {
        return;
    }

    GPixel src = premul(paint.getColor());
    GBlendMode mode = paint.getBlendMode();
    GShader* shader = paint.getShader();
    if (shader == nullptr) {
        int src_alpha = GPixel_GetA(src);
        if (src_alpha == 0) mode = check0(mode);
        else if (src_alpha == 255) mode = check255(mode);
    } else {
        if (shader->isOpaque()) mode = check255(mode);
    } 
    if (mode == GBlendMode::kDst) return;

    GRect bounds = newpath.bounds();
    int top = GRoundToInt(bounds.top);
    int bot = GRoundToInt(bounds.bottom);

    int i = 0;
    while (i < edges.size() && edges[i].isValid(top)) {
        i++;
    }

    std::sort(edges.begin(), edges.begin() + i, [] (GEdge e1, GEdge e2){
        return e1.x < e2.x;
    });
    
    for (int y = top; y < bot; y++) {
        int i = 0;
        int w = 0;
        int L;
        while (i < edges.size() && edges[i].isValid(y)) {
            int x = GRoundToInt(edges[i].x);
            if (w == 0) {
                L = x;
            }
            w += edges[i].w;
            if (w == 0) {
                int R = x;
                switch (mode) {
                    case GBlendMode::kClear: blitRow(L, y, R - L, src, shader, clear_mode); break;
                    case GBlendMode::kSrc: blitRow(L, y, R - L, src, shader, src_mode); break;
                    case GBlendMode::kDst: break;
                    case GBlendMode::kSrcOver: blitRow(L, y, R - L, src, shader, src_over_mode); break;
                    case GBlendMode::kDstOver: blitRow(L, y, R - L, src, shader, dst_over_mode); break;
                    case GBlendMode::kSrcIn: blitRow(L, y, R - L, src, shader, src_in_mode); break;
                    case GBlendMode::kDstIn: blitRow(L, y, R - L, src, shader, dst_in_mode); break;
                    case GBlendMode::kSrcOut: blitRow(L, y, R - L, src, shader, src_out_mode); break;
                    case GBlendMode::kDstOut: blitRow(L, y, R - L, src, shader, dst_out_mode); break;
                    case GBlendMode::kSrcATop: blitRow(L, y, R - L, src, shader, src_a_top_mode); break;
                    case GBlendMode::kDstATop: blitRow(L, y, R - L, src, shader, dst_a_top_mode); break;
                    case GBlendMode::kXor: blitRow(L, y, R - L, src, shader, xor_mode); break;
                }
            }

            if (edges[i].isValid(y+1)) {
                edges[i].updateX();
                i += 1;
            } else {
                edges.erase(edges.begin() + i); 
            }
        }

        assert(w == 0);

        while (i < edges.size() && edges[i].isValid(y+1)) {
            i++;
        }

        std::sort(edges.begin(), edges.begin() + i, [] (GEdge e1, GEdge e2){
            return e1.x < e2.x;
        });
    }
}

void MyCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint& color) {
    if (count < 3) return;

    GPoint newpoints[count];
    ctm.top().mapPoints(newpoints, points, count);
    
    std::vector<GEdge> edges = makeEdges(newpoints, count);   
    // assert(edges.size() >= 2);
    if (edges.size() < 2) {
        return;
    }

    GPixel src = premul(color.getColor());
    GBlendMode mode = color.getBlendMode();
    GShader* shader = color.getShader();
    

    if (shader == nullptr) {
        int src_alpha = GPixel_GetA(src);
        if (src_alpha == 0) mode = check0(mode);
        else if (src_alpha == 255) mode = check255(mode);
    } else {
        if (shader->isOpaque()) mode = check255(mode);
    } 

    if (mode == GBlendMode::kDst) return;
    // BlendProc proc = gProcs[(int)mode];
    
    int p1 = 0;
    int p2 = 1;
    GEdge e1 = edges[p1];
    GEdge e2 = edges[p2];
    int top = e1.top;
    int bot = edges[edges.size() - 1].bot;

    for (int y = top; y < bot; y++) {
        if(!e1.isValid(y)) {
            p1 = std::max(p1 + 1, p2 + 1);
            e1 = edges[p1];
        } 
        if(!e2.isValid(y)) {
            p2 = std::max(p1 + 1, p2 + 1);
            e2 = edges[p2];
        } 
        int x1 = GRoundToInt(e1.x);
        int x2 = GRoundToInt(e2.x);
        e1.updateX();
        e2.updateX();
        if (x1 > x2) std::swap(x1, x2);
        int count = x2 - x1;

        // blitRow(x1, y, count, src, shader, proc);
        switch (mode) {
            case GBlendMode::kClear: blitRow(x1, y, count, src, shader, clear_mode); break;
            case GBlendMode::kSrc: blitRow(x1, y, count, src, shader, src_mode); break;
            case GBlendMode::kDst: break;
            case GBlendMode::kSrcOver: blitRow(x1, y, count, src, shader, src_over_mode); break;
            case GBlendMode::kDstOver: blitRow(x1, y, count, src, shader, dst_over_mode); break;
            case GBlendMode::kSrcIn: blitRow(x1, y, count, src, shader, src_in_mode); break;
            case GBlendMode::kDstIn: blitRow(x1, y, count, src, shader, dst_in_mode); break;
            case GBlendMode::kSrcOut: blitRow(x1, y, count, src, shader, src_out_mode); break;
            case GBlendMode::kDstOut: blitRow(x1, y, count, src, shader, dst_out_mode); break;
            case GBlendMode::kSrcATop: blitRow(x1, y, count, src, shader, src_a_top_mode); break;
            case GBlendMode::kDstATop: blitRow(x1, y, count, src, shader, dst_a_top_mode); break;
            case GBlendMode::kXor: blitRow(x1, y, count, src, shader, xor_mode); break;
        }
    }
}  

template <typename Proc> void MyCanvas::blitRow(int x1, int y, int count, GPixel src, GShader* shader, Proc blend) {
    if (shader == nullptr) {
        for (int i = 0; i < count; i++) {
            GPixel* dst = fDevice.getAddr(x1 + i, y);
            *dst = blend(src, *dst);
        }
    } else {
       if (!(shader->setContext(ctm.top()))) {
            return;
        }
        GPixel row[count];
        shader->shadeRow(x1, y, count, row);

        for (int i = 0; i < count; i++) {
            GPixel* dst = fDevice.getAddr(x1 + i, y);
            *dst = blend(row[i], *dst);
        }
    } 
}

void MyCanvas::drawRect(const GRect& rect, const GPaint& color) {
    GPoint points[4];
    float left, right, top, bottom;
    left = rect.left;
    right = rect.right;
    top = rect.top;
    bottom = rect.bottom;
    points[0] = {left, top};
    points[1] = {right, top};
    points[2] = {right, bottom};
    points[3] = {left, bottom};
    drawConvexPolygon(points, 4, color);
}

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) {
    GShader* shader = paint.getShader();
    int n = 0;
    GPoint pts[3];
    GColor shaderColors[3];
    GPoint shaderTexs[3];
    if (colors != nullptr && texs != nullptr) {
        for (int i = 0; i < count; i++) {
            pts[0] = verts[indices[n]];
            pts[1] = verts[indices[n + 1]];
            pts[2] = verts[indices[n + 2]];
            shaderColors[0] = colors[indices[n]];
            shaderColors[1] = colors[indices[n + 1]];
            shaderColors[2] = colors[indices[n + 2]];
            shaderTexs[0] = texs[indices[n]];
            shaderTexs[1] = texs[indices[n + 1]];
            shaderTexs[2] = texs[indices[n + 2]];
            drawComposeTriangle(pts, shaderColors, shaderTexs, shader);
            n += 3;
        }
    } else if (colors != nullptr) {
        for (int i = 0; i < count; i++) {
            pts[0] = verts[indices[n]];
            pts[1] = verts[indices[n + 1]];
            pts[2] = verts[indices[n + 2]];
            shaderColors[0] = colors[indices[n]];
            shaderColors[1] = colors[indices[n + 1]];
            shaderColors[2] = colors[indices[n + 2]];
            drawTriangleColor(pts, shaderColors);
            n += 3;
        }
    } else if (texs != nullptr && shader != nullptr) {
        for (int i = 0; i < count; i++) {
            pts[0] = verts[indices[n]];
            pts[1] = verts[indices[n + 1]];
            pts[2] = verts[indices[n + 2]];
            shaderTexs[0] = texs[indices[n]];
            shaderTexs[1] = texs[indices[n + 1]];
            shaderTexs[2] = texs[indices[n + 2]];
            drawTriangleWithTex(pts, shaderTexs, shader);
            n += 3;
        }
    }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {
    if (texs == nullptr && colors == nullptr) return;
    else if (texs == nullptr) {
        this->drawQuad(verts, colors, level, paint);
        return;
    } else if (colors == nullptr) {
        this->drawQuad(verts, texs, level, paint);
        return;
    }
    GPoint quadpts[4];
    GColor quadcolor[4];
    GPoint quadtex[4];

    float t_inc = 1.f / (level + 1.f);
    float u = 0;
    float v = 0;
    for (int i = 0; i <= level; i++) {
        float u2 = u + t_inc;
        v = 0;
        for (int j = 0; j <= level; j++) {
            float v2 = v + t_inc;
            quadpts[0] = bilerp(verts, u, v);
            quadpts[1] = bilerp(verts, u2, v);
            quadpts[2] = bilerp(verts, u2, v2);
            quadpts[3] = bilerp(verts, u, v2);

            quadcolor[0] = bilerp(colors, u, v);
            quadcolor[1] = bilerp(colors, u2, v);
            quadcolor[2] = bilerp(colors, u2, v2);
            quadcolor[3] = bilerp(colors, u, v2);

            quadtex[0] = bilerp(texs, u, v);
            quadtex[1] = bilerp(texs, u2, v);
            quadtex[2] = bilerp(texs, u2, v2);
            quadtex[3] = bilerp(texs, u, v2);

            int indices[6] = {0, 1, 3, 1, 3, 2};
            drawMesh(quadpts, quadcolor, quadtex, 2, indices, paint);

            v += t_inc;
        }
        u += t_inc;
    }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GPoint texs[4], int level, const GPaint& paint) {
    GPoint quadpts[4];
    GPoint quadtex[4];

    float t_inc = 1.f / (level + 1.f);
    float u = 0;
    float v = 0;
    for (int i = 0; i <= level; i++) {
        float u2 = u + t_inc;
        v = 0;
        for (int j = 0; j <= level; j++) {
            float v2 = v + t_inc;
            quadpts[0] = bilerp(verts, u, v);
            quadpts[1] = bilerp(verts, u2, v);
            quadpts[2] = bilerp(verts, u2, v2);
            quadpts[3] = bilerp(verts, u, v2);

            quadtex[0] = bilerp(texs, u, v);
            quadtex[1] = bilerp(texs, u2, v);
            quadtex[2] = bilerp(texs, u2, v2);
            quadtex[3] = bilerp(texs, u, v2);

            int indices[6] = {0, 1, 3, 1, 3, 2};
            drawMesh(quadpts, nullptr, quadtex, 2, indices, paint);

            v += t_inc;
        }
        u += t_inc;
    }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], int level, const GPaint& paint) {
    GPoint quadpts[4];
    GColor quadcolor[4];

    float t_inc = 1.f / (level + 1.f);
    float u = 0;
    float v = 0;
    for (int i = 0; i <= level; i++) {
        float u2 = u + t_inc;
        v = 0;
        for (int j = 0; j <= level; j++) {
            float v2 = v + t_inc;
            quadpts[0] = bilerp(verts, u, v);
            quadpts[1] = bilerp(verts, u2, v);
            quadpts[2] = bilerp(verts, u2, v2);
            quadpts[3] = bilerp(verts, u, v2);

            quadcolor[0] = bilerp(colors, u, v);
            quadcolor[1] = bilerp(colors, u2, v);
            quadcolor[2] = bilerp(colors, u2, v2);
            quadcolor[3] = bilerp(colors, u, v2);

            int indices[6] = {0, 1, 3, 1, 3, 2};
            drawMesh(quadpts, quadcolor, nullptr, 2, indices, paint);

            v += t_inc;
        }
        u += t_inc;
    }
}

// void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {
//     // A -- B
//     // |    |
//     // D -- C
//     if (colors == nullptr && texs == nullptr) return;
//     if (colors == nullptr) {
//         drawQuad(verts, texs, level, paint);
//         return;
//     }
//     else if (texs == nullptr) {
//         drawQuad(verts, colors, level, paint);
//         return;
//     }
//     float t_inc = 1.f / (level + 1.f);
//     GVector AB_inc = t_inc * (verts[1] - verts[0]);
//     GVector BC_inc = t_inc * (verts[2] - verts[1]);
//     GVector DC_inc = t_inc * (verts[2] - verts[3]);
//     GVector AD_inc = t_inc * (verts[3] - verts[0]);
//     GPoint A, B, C, D;
//     A = verts[0];
//     B = A + AB_inc;
//     C = B + BC_inc;
//     D = A + AD_inc;

//     GColor ABcolor_inc = t_inc * (colors[1] - colors[0]);
//     GColor BCcolor_inc = t_inc * (colors[2] - colors[1]);
//     GColor DCcolor_inc = t_inc * (colors[2] - colors[3]);
//     GColor ADcolor_inc = t_inc * (colors[3] - colors[0]);
//     GColor colorA, colorB, colorC, colorD;
//     colorA = colors[0];
//     colorB = colorA + ABcolor_inc;
//     colorC = colorB + BCcolor_inc;
//     colorD = colorA + ADcolor_inc;

//     GVector ABtex_inc = t_inc * (verts[1] - verts[0]);
//     GVector BCtex_inc = t_inc * (verts[2] - verts[1]);
//     GVector DCtex_inc = t_inc * (verts[2] - verts[3]);
//     GVector ADtex_inc = t_inc * (verts[3] - verts[0]);
//     GPoint texA, texB, texC, texD;
//     texA = texs[0];
//     texB = texA + ABtex_inc;
//     texC = texB + BCtex_inc;
//     texD = texA + ADtex_inc;

//     for (int i = 0; i <= level; i++) {
//         A = verts[0] + i * AD_inc;
//         B = A + AB_inc;
//         C = B + BC_inc;
//         D = A + AD_inc;
//         colorA = colors[0] + i * ADcolor_inc;
//         colorB = colorA + ABcolor_inc;
//         colorC = colorB + BCcolor_inc;
//         colorD = colorA + ADcolor_inc;
//         texA = texs[0] + i * ADtex_inc;
//         texB = texA + ABtex_inc;
//         texC = texB + BCtex_inc;
//         texD = texA + ADtex_inc;
//         for (int j = 0; j <= level; j++) {
//             int indices[6] = {0, 1, 3, 1, 3, 2};
//             GPoint quadpts[4] = {A, B, C, D};
//             GColor quadcolors[4] = {colorA, colorB, colorC, colorD};
//             GPoint quadtex[4] = {texA, texB, texC, texD};
//             drawMesh(quadpts, quadcolors, quadtex, 2, indices, paint);
//             A += AB_inc;
//             B += AB_inc;
//             C += DC_inc;
//             D += DC_inc;
//             colorA += ABcolor_inc;
//             colorB += ABcolor_inc;
//             colorC += DCcolor_inc;
//             colorD += DCcolor_inc;
//             texA += ABtex_inc;
//             texB += ABtex_inc;
//             texC += DCtex_inc;
//             texD += DCtex_inc;
//         }
//         A += AD_inc;
//         B += BC_inc;
//         C += BC_inc;
//         D += AD_inc;
//         colorA += ADcolor_inc;
//         colorB += BCcolor_inc;
//         colorC += BCcolor_inc;
//         colorD += ADcolor_inc;
//         texA += ADtex_inc;
//         texB += BCtex_inc;
//         texC += BCtex_inc;
//         texD += ADtex_inc;
//     }
// }

// void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], int level, const GPaint& paint) {
//     float t_inc = 1.f / (level + 1.f);
//     GVector AB_inc = t_inc * (verts[1] - verts[0]);
//     GVector BC_inc = t_inc * (verts[2] - verts[1]);
//     GVector DC_inc = t_inc * (verts[2] - verts[3]);
//     GVector AD_inc = t_inc * (verts[3] - verts[0]);
//     GPoint A, B, C, D;
//     A = verts[0];
//     B = A + AB_inc;
//     C = B + BC_inc;
//     D = A + AD_inc;

//     GColor ABcolor_inc = t_inc * (colors[1] - colors[0]);
//     GColor BCcolor_inc = t_inc * (colors[2] - colors[1]);
//     GColor DCcolor_inc = t_inc * (colors[2] - colors[3]);
//     GColor ADcolor_inc = t_inc * (colors[3] - colors[0]);
//     GColor colorA, colorB, colorC, colorD;
//     colorA = colors[0];
//     colorB = colorA + ABcolor_inc;
//     colorC = colorB + BCcolor_inc;
//     colorD = colorA + ADcolor_inc;

//     for (int i = 0; i <= level; i++) {
//         A = verts[0] + i * AD_inc;
//         B = A + AB_inc;
//         C = B + BC_inc;
//         D = A + AD_inc;
//         colorA = colors[0] + i * ADcolor_inc;
//         colorB = colorA + ABcolor_inc;
//         colorC = colorB + BCcolor_inc;
//         colorD = colorA + ADcolor_inc;

//         for (int j = 0; j <= level; j++) {
//             int indices[6] = {0, 1, 3, 1, 3, 2};
//             GPoint quadpts[4] = {A, B, C, D};
//             GColor quadcolors[4] = {colorA, colorB, colorC, colorD};
//             drawMesh(quadpts, quadcolors, nullptr, 2, indices, paint);
//             A += AB_inc;
//             B += AB_inc;
//             C += DC_inc;
//             D += DC_inc;
//             colorA += ABcolor_inc;
//             colorB += ABcolor_inc;
//             colorC += DCcolor_inc;
//             colorD += DCcolor_inc;
//         }
//         A += AD_inc;
//         B += BC_inc;
//         C += BC_inc;
//         D += AD_inc;
//         colorA += ADcolor_inc;
//         colorB += BCcolor_inc;
//         colorC += BCcolor_inc;
//         colorD += ADcolor_inc;
//     }
// }

// void MyCanvas::drawQuad(const GPoint verts[4], const GPoint texs[4], int level, const GPaint& paint) {
//     float t_inc = 1.f / (level + 1.f);
//     GVector AB_inc = t_inc * (verts[1] - verts[0]);
//     GVector BC_inc = t_inc * (verts[2] - verts[1]);
//     GVector DC_inc = t_inc * (verts[2] - verts[3]);
//     GVector AD_inc = t_inc * (verts[3] - verts[0]);
//     GPoint A, B, C, D;
//     A = verts[0];
//     B = A + AB_inc;
//     C = B + BC_inc;
//     D = A + AD_inc;

//     GVector ABtex_inc = t_inc * (verts[1] - verts[0]);
//     GVector BCtex_inc = t_inc * (verts[2] - verts[1]);
//     GVector DCtex_inc = t_inc * (verts[2] - verts[3]);
//     GVector ADtex_inc = t_inc * (verts[3] - verts[0]);
//     GPoint texA, texB, texC, texD;
//     texA = texs[0];
//     texB = texA + ABtex_inc;
//     texC = texB + BCtex_inc;
//     texD = texA + ADtex_inc;

//     for (int i = 0; i <= level; i++) {
//         A = verts[0] + i * AD_inc;
//         B = A + AB_inc;
//         C = B + BC_inc;
//         D = A + AD_inc;
//         texA = texs[0] + i * ADtex_inc;
//         texB = texA + ABtex_inc;
//         texC = texB + BCtex_inc;
//         texD = texA + ADtex_inc;

//         for (int j = 0; j <= level; j++) {
//             int indices[6] = {0, 1, 3, 1, 3, 2};
//             GPoint quadpts[4] = {A, B, C, D};
//             GPoint quadtex[4] = {texA, texB, texC, texD};
//             drawMesh(quadpts, nullptr, quadtex, 2, indices, paint);
//             A += AB_inc;
//             B += AB_inc;
//             C += DC_inc;
//             D += DC_inc;
//             texA += ABtex_inc;
//             texB += ABtex_inc;
//             texC += DCtex_inc;
//             texD += DCtex_inc;
//         }
//         A += AD_inc;
//         B += BC_inc;
//         C += BC_inc;
//         D += AD_inc;
//         texA += ADtex_inc;
//         texB += BCtex_inc;
//         texC += BCtex_inc;
//         texD += ADtex_inc;
//     }
// }

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    return "ehe?"; // name of artwork`
}

/* turns 0-1 unpremul color to 0-255 premul form */
GPixel MyCanvas::premul(const GColor& color) {  
    float a = color.a * 255;   
    int r = GRoundToInt(color.r * a);
    int g = GRoundToInt(color.g * a);
    int b = GRoundToInt(color.b * a);
    return GPixel_PackARGB(GRoundToInt(a), r, g, b);
}

void MyCanvas::save() {
    ctm.push(ctm.top());
}

void MyCanvas::restore() {
    ctm.pop(); 
}

void MyCanvas::concat(const GMatrix& matrix) {
    GMatrix tmp = ctm.top();
    ctm.pop();
    ctm.push(GMatrix::Concat(tmp, matrix));
}