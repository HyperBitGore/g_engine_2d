#include "../../include/g_engine/util/matrix.hpp"
#include "../../include/g_engine/util/vector.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>

static int s_pass = 0;
static int s_fail = 0;
static const float EPS = 1e-4f;

static bool near(float a, float b) { return std::fabs(a - b) < EPS; }

#define CHECK(name, expr) \
    do { \
        if (expr) { std::printf("  PASS  %s\n", name); ++s_pass; } \
        else      { std::printf("  FAIL  %s\n", name); ++s_fail; } \
    } while(0)

// -------------------------------------------------------------------------
// vec2
// -------------------------------------------------------------------------
static void test_vec2() {
    std::printf("=== vec2 ===\n");
    gore::vec2 a(1.0f, 2.0f), b(3.0f, 4.0f);

    auto s = a + b;
    CHECK("add",           near(s.x, 4) && near(s.y, 6));
    auto d = b - a;
    CHECK("subtract",      near(d.x, 2) && near(d.y, 2));
    auto m = a * 3.0f;
    CHECK("scalar mul",    near(m.x, 3) && near(m.y, 6));
    auto cm = a * b;
    CHECK("component mul", near(cm.x, 3) && near(cm.y, 8));
    CHECK("dot product",   near(a.dotProduct(b), 11.0f));
    CHECK("length",        near(gore::vec2(3,4).length(), 5.0f));
    auto n = gore::vec2(3,4).normalize();
    CHECK("normalize",     near(n.x, 0.6f) && near(n.y, 0.8f));
    auto neg = -a;
    CHECK("negate",        near(neg.x, -1) && near(neg.y, -2));
}

// -------------------------------------------------------------------------
// vec3
// -------------------------------------------------------------------------
static void test_vec3() {
    std::printf("=== vec3 ===\n");
    gore::vec3 a(1,2,3), b(4,5,6);

    auto s = a + b;
    CHECK("add",           near(s.x,5) && near(s.y,7) && near(s.z,9));
    auto d = b - a;
    CHECK("subtract",      near(d.x,3) && near(d.y,3) && near(d.z,3));
    auto m = a * 2.0f;
    CHECK("scalar mul",    near(m.x,2) && near(m.y,4) && near(m.z,6));
    CHECK("dot product",   near(a.dotProduct(b), 32.0f));
    CHECK("length",        near(gore::vec3(1,0,0).length(), 1.0f));
    auto n = gore::vec3(0,3,4).normalize();
    CHECK("normalize",     near(n.x,0) && near(n.y,0.6f) && near(n.z,0.8f));
    auto neg = -a;
    CHECK("negate",        near(neg.x,-1) && near(neg.y,-2) && near(neg.z,-3));
}

// -------------------------------------------------------------------------
// vec4
// -------------------------------------------------------------------------
static void test_vec4() {
    std::printf("=== vec4 ===\n");
    gore::vec4 a(1,2,3,4), b(5,6,7,8);

    auto s = a + b;
    CHECK("add",        near(s.x,6)&&near(s.y,8)&&near(s.z,10)&&near(s.w,12));
    auto d = b - a;
    CHECK("subtract",   near(d.x,4)&&near(d.y,4)&&near(d.z,4)&&near(d.w,4));
    auto m = a * 2.0f;
    CHECK("scalar mul", near(m.x,2)&&near(m.y,4)&&near(m.z,6)&&near(m.w,8));
    CHECK("dot product",near(a.dotProduct(b), 70.0f));
    CHECK("length",     near(gore::vec4(1,0,0,0).length(), 1.0f));
    auto n = gore::vec4(0,0,3,4).normalize();
    CHECK("normalize",  near(n.z,0.6f) && near(n.w,0.8f));
    auto neg = -a;
    CHECK("negate",     near(neg.x,-1)&&near(neg.y,-2)&&near(neg.z,-3)&&near(neg.w,-4));
}

// -------------------------------------------------------------------------
// matrix construction / identity
// -------------------------------------------------------------------------
static void test_matrix_construction() {
    std::printf("=== matrix construction ===\n");

    gore::matrix z(3, 3, 0.0f);
    bool all_zero = true;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (!near(z[i][j], 0)) all_zero = false;
    CHECK("zero fill", all_zero);

    gore::matrix id = gore::matrix::generateIdentity(4, 4);
    bool diag_ok = near(id[0][0],1)&&near(id[1][1],1)&&near(id[2][2],1)&&near(id[3][3],1);
    bool off_ok  = near(id[0][1],0)&&near(id[1][0],0)&&near(id[0][3],0);
    CHECK("identity diagonal", diag_ok);
    CHECK("identity off-diag", off_ok);

    // copy constructor
    gore::matrix cp = id;
    CHECK("copy ctor", near(cp[0][0],1) && near(cp[3][3],1) && near(cp[0][1],0));

    // operator=
    gore::matrix assigned(4,4);
    assigned = id;
    CHECK("operator= rows", assigned.numRows() == 4);
    CHECK("operator= vals", near(assigned[1][1],1) && near(assigned[0][1],0));
}

// -------------------------------------------------------------------------
// matrix * vec
// -------------------------------------------------------------------------
static void test_matrix_vec_multiply() {
    std::printf("=== matrix * vec ===\n");

    gore::matrix id3 = gore::matrix::generateIdentity(3, 3);
    gore::matrix id4 = gore::matrix::generateIdentity(4, 4);

    gore::matrix id2 = gore::matrix::generateIdentity(2, 2);
    auto v2 = id2 * gore::vec2(3, 7);
    CHECK("identity * vec2", near(v2.x,3) && near(v2.y,7));

    auto v3b = id3 * gore::vec3(1,2,3);
    CHECK("identity * vec3", near(v3b.x,1) && near(v3b.y,2) && near(v3b.z,3));

    auto v4 = id4 * gore::vec4(1,2,3,4);
    CHECK("identity * vec4", near(v4.x,1)&&near(v4.y,2)&&near(v4.z,3)&&near(v4.w,4));
}

// -------------------------------------------------------------------------
// translate
// -------------------------------------------------------------------------
static void test_translate() {
    std::printf("=== translate ===\n");

    // 4x4: applying translation to homogeneous point (0,0,0,1)
    gore::matrix id4 = gore::matrix::generateIdentity(4, 4);
    gore::matrix t4  = id4.translate(gore::vec3(5,6,7));
    gore::vec4   p4  = t4 * gore::vec4(0,0,0,1);
    CHECK("translate vec3 col", near(t4[0][3],5) && near(t4[1][3],6) && near(t4[2][3],7));
    CHECK("translate vec3 point", near(p4.x,5)&&near(p4.y,6)&&near(p4.z,7)&&near(p4.w,1));

    // 3x3: applying translation to homogeneous 2D point (0,0,1)
    gore::matrix id3 = gore::matrix::generateIdentity(3, 3);
    gore::matrix t3  = id3.translate(gore::vec2(3,4));
    CHECK("translate vec2 col", near(t3[0][2],3)&&near(t3[1][2],4));
}

// -------------------------------------------------------------------------
// rotate (vec3 axis — Rodrigues)
// -------------------------------------------------------------------------
static void test_rotate_3d() {
    std::printf("=== rotate 3D (Rodrigues) ===\n");
    const float PI = 3.14159265358979f;

    gore::matrix id = gore::matrix::generateIdentity(4, 4);

    // 90° around Z: (1,0,0,1) -> (0,1,0,1)
    gore::matrix rz = id.rotate(gore::vec3(0,0,1), PI/2.0f);
    gore::vec4 pz   = rz * gore::vec4(1,0,0,1);
    CHECK("rot Z 90 x->y", near(pz.x,0)&&near(pz.y,1)&&near(pz.z,0));

    // 90° around X: (0,1,0,1) -> (0,0,1,1)
    gore::matrix rx = id.rotate(gore::vec3(1,0,0), PI/2.0f);
    gore::vec4 px   = rx * gore::vec4(0,1,0,1);
    CHECK("rot X 90 y->z", near(px.x,0)&&near(px.y,0)&&near(px.z,1));

    // 90° around Y: (1,0,0,1) -> (0,0,-1,1)
    gore::matrix ry = id.rotate(gore::vec3(0,1,0), PI/2.0f);
    gore::vec4 py   = ry * gore::vec4(1,0,0,1);
    CHECK("rot Y 90 x->-z", near(py.x,0)&&near(py.y,0)&&near(py.z,-1));

    // 0° rotation = identity
    gore::matrix r0 = id.rotate(gore::vec3(0,0,1), 0.0f);
    CHECK("rot 0 deg = identity", near(r0[0][0],1)&&near(r0[1][1],1)&&near(r0[0][1],0));

    // Zero-length axis = no-op
    gore::matrix rn = id.rotate(gore::vec3(0,0,0), PI/2.0f);
    CHECK("rot zero axis = no-op", near(rn[0][0],1)&&near(rn[1][1],1));
}

// -------------------------------------------------------------------------
// rotate (vec2 pivot)
// -------------------------------------------------------------------------
static void test_rotate_2d() {
    std::printf("=== rotate 2D (pivot) ===\n");
    const float PI = 3.14159265358979f;

    gore::matrix id = gore::matrix::generateIdentity(3, 3);

    // 90° CCW around pivot (1,1): point (2,1) -> (1,2)
    gore::matrix r = id.rotate(gore::vec2(1,1), PI/2.0f);
    // apply via matrix * homogeneous vec3
    // manual: row0*(2,1,1), row1*(2,1,1)
    float rx = r[0][0]*2 + r[0][1]*1 + r[0][2]*1;
    float ry = r[1][0]*2 + r[1][1]*1 + r[1][2]*1;
    CHECK("pivot rot (2,1) -> (1,2)", near(rx,1.0f) && near(ry,2.0f));

    // 0° around any pivot = identity
    gore::matrix r0 = id.rotate(gore::vec2(5,5), 0.0f);
    CHECK("pivot rot 0 deg = identity", near(r0[0][0],1)&&near(r0[1][1],1)&&near(r0[0][1],0));
}

// -------------------------------------------------------------------------
// scale
// -------------------------------------------------------------------------
static void test_scale() {
    std::printf("=== scale ===\n");

    gore::matrix id4 = gore::matrix::generateIdentity(4, 4);
    gore::matrix s4  = id4.scale(gore::vec3(2,3,4));
    gore::vec4 p4    = s4 * gore::vec4(1,1,1,1);
    CHECK("scale vec3 diagonal", near(s4[0][0],2)&&near(s4[1][1],3)&&near(s4[2][2],4));
    CHECK("scale vec3 point",    near(p4.x,2)&&near(p4.y,3)&&near(p4.z,4)&&near(p4.w,1));

    gore::matrix id3 = gore::matrix::generateIdentity(3, 3);
    gore::matrix s3  = id3.scale(gore::vec2(5,6));
    CHECK("scale vec2 diagonal", near(s3[0][0],5)&&near(s3[1][1],6)&&near(s3[2][2],1));
}

// -------------------------------------------------------------------------
// inverse
// -------------------------------------------------------------------------
static void test_inverse() {
    std::printf("=== inverse ===\n");

    gore::matrix m(4,4);
    m[0][0]=1; m[0][1]=2; m[0][2]=3; m[0][3]=4;
    m[1][0]=0; m[1][1]=1; m[1][2]=0; m[1][3]=2;
    m[2][0]=0; m[2][1]=0; m[2][2]=1; m[2][3]=3;
    m[3][0]=0; m[3][1]=0; m[3][2]=0; m[3][3]=1;

    gore::matrix inv = m.inverse();
    gore::matrix prod = m * inv;

    bool is_identity = true;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            float expected = (i == j) ? 1.0f : 0.0f;
            if (!near(prod[i][j], expected)) is_identity = false;
        }
    CHECK("M * M_inv = identity", is_identity);

    gore::matrix id4 = gore::matrix::generateIdentity(4,4);
    gore::matrix id_inv = id4.inverse();
    bool id_inv_ok = true;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            float expected = (i == j) ? 1.0f : 0.0f;
            if (!near(id_inv[i][j], expected)) id_inv_ok = false;
        }
    CHECK("identity inverse = identity", id_inv_ok);
}

// -------------------------------------------------------------------------
// matrix multiply
// -------------------------------------------------------------------------
static void test_matrix_multiply() {
    std::printf("=== matrix multiply ===\n");

    gore::matrix id = gore::matrix::generateIdentity(4, 4);
    gore::matrix a(4,4);
    a[0][0]=1; a[0][1]=2; a[0][2]=0; a[0][3]=0;
    a[1][0]=0; a[1][1]=1; a[1][2]=3; a[1][3]=0;
    a[2][0]=0; a[2][1]=0; a[2][2]=1; a[2][3]=4;
    a[3][0]=0; a[3][1]=0; a[3][2]=0; a[3][3]=1;

    gore::matrix r = a * id;
    bool ok = true;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (!near(r[i][j], a[i][j])) ok = false;
    CHECK("A * I = A", ok);

    gore::matrix r2 = id * a;
    bool ok2 = true;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (!near(r2[i][j], a[i][j])) ok2 = false;
    CHECK("I * A = A", ok2);
}

// -------------------------------------------------------------------------
// lookat
// -------------------------------------------------------------------------
static void test_lookat() {
    std::printf("=== lookat ===\n");

    // Camera at (0,0,5) looking at origin, up = world Y
    gore::matrix lk = gore::matrix::lookat(
        gore::vec3(0,0,5), gore::vec3(0,0,0), gore::vec3(0,1,0));

    // Origin should land at (0,0,-5) in view space
    gore::vec4 p0 = lk * gore::vec4(0,0,0,1);
    CHECK("lookat origin z=-5",  near(p0.x,0)&&near(p0.y,0)&&near(p0.z,-5)&&near(p0.w,1));

    // A point at camera height, 1 unit right, same Z as camera -> (1,0,0)
    gore::vec4 p1 = lk * gore::vec4(1,0,5,1);
    CHECK("lookat side point",   near(p1.x,1)&&near(p1.y,0)&&near(p1.z,0)&&near(p1.w,1));

    // Camera at origin looking toward +X
    gore::matrix lk2 = gore::matrix::lookat(
        gore::vec3(0,0,0), gore::vec3(1,0,0), gore::vec3(0,1,0));

    // The target (1,0,0) should be straight ahead (0,0,-1)
    gore::vec4 p2 = lk2 * gore::vec4(1,0,0,1);
    CHECK("lookat target ahead", near(p2.x,0)&&near(p2.y,0)&&near(p2.z,-1)&&near(p2.w,1));

    // Rotation rows must be orthonormal
    gore::vec3 r(lk[0][0], lk[0][1], lk[0][2]);
    gore::vec3 u(lk[1][0], lk[1][1], lk[1][2]);
    gore::vec3 d(lk[2][0], lk[2][1], lk[2][2]);
    CHECK("lookat |right|=1",    near(r.length(), 1));
    CHECK("lookat |up|=1",       near(u.length(), 1));
    CHECK("lookat |dir|=1",      near(d.length(), 1));
    CHECK("lookat right.up=0",   near(r.dotProduct(u), 0));
    CHECK("lookat right.dir=0",  near(r.dotProduct(d), 0));
    CHECK("lookat up.dir=0",     near(u.dotProduct(d), 0));
}
int main () {
// -------------------------------------------------------------------------
    test_vec2();
    test_vec3();
    test_vec4();
    test_matrix_construction();
    test_matrix_vec_multiply();
    test_translate();
    test_rotate_3d();
    test_rotate_2d();
    test_scale();
    test_inverse();
    test_matrix_multiply();
    test_lookat();

    std::printf("\n%d passed, %d failed\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}