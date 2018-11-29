#version 430 core

struct Ray { vec3 origin, direction; };
struct Sphere { vec3 center; float radius; };
struct Cylinder { vec3 center, direction; float radius; };
struct AABB { vec3 min; float padding0; vec3 max; float padding1; };

layout(binding = 0, rgba32f) uniform image2D framebuffer;
layout(std140, binding = 10) buffer debug { vec4 debug_out[]; };

layout(std140, binding = 1) buffer AABB_buffer { AABB aabb[]; };
layout(std140, binding = 2) buffer spheres_buffer { Sphere spheres[]; };
layout(std140, binding = 3) buffer cylinder_buffer { Cylinder cylinders[]; };


uniform vec3 eye;
uniform vec3 ray00, ray01, ray10, ray11;
uniform int num_aabb, num_spheres, num_cylinders;


#define MAX_SCENE_BOUNDS 100.0

#define AABB_PRIMITIVE 0.3f
#define SPHERE_PRIMITIVE 0.5f
#define CYLINDER_PRIMITIVE 0.7f


struct hitinfo {
    vec2 lambda;
    int bi;
    float primitive_type;
};

// NORMALS FROM SHAPES
vec3 getNormalSphere(vec3 origin, vec3 spherePoint) {
    vec3 normal = spherePoint - origin;
    return normalize(normal);
}

vec3 getNormalCylinder(vec3 origin, vec3 direction, float radius, vec3 cylinderPoint) {
    vec3 proj = origin + ((cylinderPoint - origin) * direction) * direction;
    vec3 normal = cylinderPoint - proj;
    return normalize(normal);
}

vec3 getRayHitPoint(Ray ray, float t) {
    return ray.origin + ray.direction * t;
}

void shiftRay(Ray ray, float amount) {
    ray.origin += ray.direction * amount;
}


// INTERSECTION POINT RAY x SHAPE
vec2 intersectAABB(Ray ray, const AABB b) {
    vec3 invDir = 1.0f / ray.direction;
    vec3 tMin = (b.min - ray.origin) * invDir;
    vec3 tMax = (b.max - ray.origin) * invDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

bool intersectAABBes(Ray ray, out hitinfo info) {
    float smallest = MAX_SCENE_BOUNDS;
    bool found = false;
    for (int i = 0; i < num_aabb; i++) {
        vec2 lambda = intersectAABB(ray, aabb[i]);
        if (lambda.x > 0.0 && lambda.x < lambda.y && lambda.x < smallest) {
            info.lambda = lambda;
            info.bi = i;
            info.primitive_type = AABB_PRIMITIVE;
            smallest = lambda.x;
            found = true;

        }
    }
    return found;
}

void solveQuadratic(float A, float B, float C, out float x1, out float x2) {
    float D = B*B - 4*A*C;
    if (D < 0.0f) { x1 = -1.0f; x2 = -1.0f; }
    else if ( D == 0.0f) {
        float x = (-B - sqrt(D)) / (2*A);
        x1 = x; x2 = MAX_SCENE_BOUNDS;
    }
    else {
        x1 = (-B - sqrt(D)) / (2*A);
        x2 = (-B + sqrt(D)) / (2*A);
    }
}

vec2 intersectSphere(Ray ray, const Sphere s) {
	float r2 = s.radius * s.radius;
	float a = dot(ray.direction, ray.direction);
	float b = 2 * dot(ray.direction, (ray.origin - s.center));
	float c = dot((ray.origin - s.center), (ray.origin - s.center)) - r2;
	float D = b * b - 4 * a * c;
    
    float t1, t2;
    solveQuadratic(a, b, c, t1, t2);
    return vec2(min(t1,t2), max(t1,t2));
};

bool intersectSpheres(Ray ray, out hitinfo info) {
    float smallest = MAX_SCENE_BOUNDS;
    bool found = false;
    for(int i = 0; i < num_spheres; ++i) {
        vec2 l = intersectSphere(ray, spheres[i]);
        if (l.x > 0.0 && l.x < smallest) {
            info.lambda = l;
            info.bi = i;
            info.primitive_type = SPHERE_PRIMITIVE;
            smallest = l.x;
            found = true;
        }
    }
    return found;
}

vec2 intersectCylinder(Ray r, const Cylinder c) {

    vec3 cc = r.origin - c.center;
    vec3 b = r.direction;

    float bd = dot(b,c.direction);
    float b2 = dot(b,b);
    float cb = dot(cc,b);
    float cd = dot(cc,c.direction);
    float c2 = dot(cc,cc);

    float A = b2 - bd*bd;
    float B = 2*cb - 2*cd*bd;
    float C = c2 - cd*cd - c.radius*c.radius;

    float t1,t2;
    solveQuadratic(A, B, C, t1, t2);
    return vec2(min(t1,t2), max(t1,t2));

}

bool intersectCylinder(Ray ray, out hitinfo info) {
    float smallest = MAX_SCENE_BOUNDS;
    bool found = false;
    for(int i = 0; i < num_cylinders; ++i) {
        vec2 l = intersectCylinder(ray, cylinders[i]);
        if (l.x > 0.0 && l.x < smallest) {
            info.lambda = l;
            info.bi = i;
            info.primitive_type = CYLINDER_PRIMITIVE;
            smallest = l.x;
            found = true;
        }
    }
    return found;
}


vec4 trace(Ray ray) {
    hitinfo i;
    if (
        intersectAABBes(ray, i) ||
        intersectSpheres(ray, i) ||
        intersectCylinder(ray, i)
    ) {
        vec4 gray = vec4(i.bi * i.primitive_type / 10.0 + 0.8);
        return vec4(gray.rgb, 1.0);
    }
    return vec4(0.0, 0.0, 0.0, 1.0);
}


layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    debug_out[gl_GlobalInvocationID.x] = vec4(aabb[0].min.xyz, 0.0f);
    if (pixel.x >= size.x || pixel.y >= size.y) { return; }
    vec2 pos = vec2(pixel) / vec2(size.x - 1, size.y - 1);
    vec3 dir = mix(mix(ray00, ray01, pos.y), mix(ray10, ray11, pos.y), pos.x);
    Ray ray = Ray(eye, dir);
    vec4 color = trace(ray);
    imageStore(framebuffer, pixel, color);
}
