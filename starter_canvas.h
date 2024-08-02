/*
 *  Copyright 2024 <me>
 */

#ifndef _g_starter_canvas_h_
#define _g_starter_canvas_h_

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GPaint.h"
#include "include/GBitmap.h"
#include "include/GMath.h"
#include "GEdge.h"
#include "GBlend.h"
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GPath.h"
#include "TriColorShader.h"
#include "ComposeShader.h"
#include "ProxyShader.h"
#include <stack>
#include <list>
#include <cmath>

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {
        ctm.push(GMatrix());
    }

    void clear(const GColor& color) override;
    void drawRect(const GRect& rect, const GPaint& color) override;
    void drawConvexPolygon(const GPoint points[], int count, const GPaint& color) override;

    // uint8_t GDiv255(unsigned prod);
    void clip(GPoint p0, GPoint p1, std::vector<GEdge> &edges);
    std::vector<GEdge> makeEdges(const GPoint points[], int count);
    GPixel premul(const GColor& color);
    template <typename Proc> void blitRow(int x1, int y, int count, GPixel src, GShader* shader, Proc blend);

    void save() override;
    void restore() override;
    void concat(const GMatrix&) override; 

    void drawPath(const GPath& path, const GPaint&) override;
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                              int count, const int indices[], const GPaint& paint) override;
    void drawQuad(const GPoint verts[4], const GColor colors[4], int level, const GPaint&);
    void drawQuad(const GPoint verts[4], const GPoint texs[4], int level, const GPaint&);
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                              int level, const GPaint&) override;
    void drawTriangle(const GPoint pts[3], GPaint paint) {
        drawConvexPolygon(pts, 3, paint);
    }
    void drawTriangleColor(const GPoint pts[3], GColor colors[3]) {
        TriColorShader triColor(pts, colors);
        GPaint p(&triColor);

        this->drawTriangle(pts, p);
    }
    void drawTriangleWithTex(const GPoint pts[3], const GPoint tex[3], GShader* originalShader) {
        GMatrix P, T, invT;
        P = compute_basis(pts);
        T = compute_basis(tex);

        invT = *T.invert();
        ProxyShader proxy(originalShader, P * invT);
        GPaint p(&proxy);

        this->drawTriangle(pts, p);
    }
    GMatrix compute_basis(const GPoint pts[3]) {
        return GMatrix(pts[2] - pts[0], pts[1] - pts[0], pts[0]);
    }
    void drawComposeTriangle(const GPoint pts[3], const GColor colors[3], const GPoint texs[3], GShader* shader) {
        GMatrix P, T, invT;
        P = compute_basis(pts);
        T = compute_basis(texs);

        invT = *T.invert();
        ComposeShader compose(new TriColorShader(pts, colors), new ProxyShader(shader, P * invT));
        GPaint p(&compose);

        this->drawTriangle(pts, p);
    
    }

private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
    std::stack<GMatrix> ctm;
    void addEdge(GPoint p0, GPoint p1, int w, std::vector<GEdge> &edges);
    GPoint interpolate(const GPoint& A, const GPoint& B, float t) {
        return A + t * (B - A);
    };
    GPoint bilerp(const GPoint pts[4], float u, float v) {
        return (1-u) * (1-v) * pts[0] + u * (1-v) * pts[1] + u * v * pts[2] + (1-u) * v * pts[3];
    }
    GColor bilerp(const GColor colors[4], float u, float v) {
        return (1-u) * (1-v) * colors[0] + u * (1-v) * colors[1] + u * v * colors[2] + (1-u) * v * colors[3];
    }
};

#endif