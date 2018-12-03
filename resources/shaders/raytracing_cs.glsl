#version 430 core

struct Ray { vec3 origin, direction; };
struct Sphere { vec3 center; float radius; };
struct Cylinder { vec3 center; float p; vec3 direction; float radius; };
struct AABB { vec3 min; float p; vec3 max; float p1; };
struct Light { vec3 position; float p; vec3 color; float p1; };
struct Material { vec3 color; float reflectance; };

layout(binding = 0, rgba32f) uniform image2D framebuffer;
layout(std140, binding = 10) buffer debug { vec4 debug_out[]; };

layout(std140, binding = 1) readonly buffer AABB_buffer { AABB aabb[]; };
layout(std140, binding = 2) readonly buffer spheres_buffer { Sphere spheres[]; };
layout(std140, binding = 3) readonly buffer cylinder_buffer { Cylinder cylinders[]; };
layout(std140, binding = 4) readonly buffer lights_buffer { Light lights[]; };
layout(std140, binding = 5) readonly buffer material_buffer { Material materials[]; };
layout(std140, binding = 6) readonly buffer startMaterialIndex { int primitiveMaterialBegin[]; };


uniform vec3 cameraPosition;
uniform vec3 ray00, ray01, ray10, ray11;
uniform int num_aabb, num_spheres, num_cylinders, num_lights;
uniform ivec4 ps;


const float ambientStrength = 0.1f;
const float specularStrength = 0.5f;


#define MAX_SCENE_BOUNDS 100.0

#define AABB_PRIMITIVE 1
#define SPHERE_PRIMITIVE 2
#define CYLINDER_PRIMITIVE 3
#define NUM_REFLECTION 4

struct Hitinfo {
    bool hit;
    float t;
    int primitiveIndex;
    int primitive_type;
};

Hitinfo createHitInfo() {
    Hitinfo i;
    i.t = MAX_SCENE_BOUNDS;
    i.hit = false;
    return i;
}

vec3 getIntersectionPoint(Ray r, float t) {
    return r.origin + r.direction * t;
}

// NORMALS FROM SHAPES
vec3 getNormalSphere(Sphere sphere, vec3 spherePoint) {
    vec3 normal = spherePoint - sphere.center;
    return normalize(normal);
}

vec3 getNormalCylinder(Cylinder c, vec3 cylinderPoint) {
    vec3 proj = c.center + ((cylinderPoint - c.center) * c.direction) * c.direction;
    vec3 normal = cylinderPoint - proj;
    return normalize(normal);
}

vec3 getNormalAABB(AABB aabb, vec3 cubePoint) {
    const float e = 0.001f;
    if (cubePoint.x < aabb.min.x + e) return vec3(-1.0,  0.0,  0.0);
    if (cubePoint.x > aabb.max.x - e) return vec3( 1.0,  0.0,  0.0);
    if (cubePoint.y < aabb.min.y + e) return vec3( 0.0, -1.0,  0.0);
    if (cubePoint.y > aabb.max.y - e) return vec3( 0.0,  1.0,  0.0);
    if (cubePoint.z < aabb.min.z + e) return vec3( 0.0,  0.0, -1.0);
    return vec3(0.0, 0.0, 1.0);
}

vec3 getNormal(Hitinfo hitInfo, vec3 intersectionPoint) {
    switch(hitInfo.primitive_type) {
        case AABB_PRIMITIVE: return getNormalAABB(aabb[hitInfo.primitiveIndex], intersectionPoint); 
        case SPHERE_PRIMITIVE: return getNormalSphere(spheres[hitInfo.primitiveIndex], intersectionPoint); 
        case CYLINDER_PRIMITIVE: return getNormalCylinder(cylinders[hitInfo.primitiveIndex], intersectionPoint);
    }
}

// INTERSECTION POINT RAY x SHAPE
vec2 intersectAABB(Ray ray, const AABB b) {
    // Taktez prevzany kod z dole uveneho github-u.
    vec3 invDir = 1.0f / ray.direction;
    vec3 tMin = (b.min - ray.origin) * invDir;
    vec3 tMax = (b.max - ray.origin) * invDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

bool intersectAABBes(Ray ray, inout Hitinfo info) {
    bool found = false;
    for (int i = 0; i < num_aabb; i++) {
        vec2 l = intersectAABB(ray, aabb[i]);
        if (l.x > 0.0 && l.x < l.y && l.x < info.t) {
            info.primitiveIndex = i;
            info.primitive_type = AABB_PRIMITIVE;
            info.t = l.x;
            info.hit = true;
            found = true;
        }
    }
    return found;
}

void solveQuadratic(float A, float B, float C, out float x1, out float x2) {
    float D = B*B - 4*A*C;
    if (D < 0.0f) { 
        x1 = -1.0f; 
        x2 = -1.0f; 
    }
    else if ( D == 0.0f) {
        float x = (-B - sqrt(D)) / (2*A);
        x1 = x; 
        x2 = MAX_SCENE_BOUNDS;
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
    
    float t1, t2;
    solveQuadratic(a, b, c, t1, t2);
    return vec2(min(t1,t2), max(t1,t2));
};



bool intersectSpheres(Ray ray, inout Hitinfo info) {
    bool found = false;
    for(int i = 0; i < num_spheres; ++i) {
        vec2 l = intersectSphere(ray, spheres[i]);
        if (l.x > 0.0  && l.x < info.t ) {
            info.t = l.x;
            info.primitiveIndex = i;
            info.primitive_type = SPHERE_PRIMITIVE;
            info.hit = true;
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

bool intersectCylinder(Ray ray, inout Hitinfo info) {
    float smallest = MAX_SCENE_BOUNDS;
    bool found = false;
    for(int i = 0; i < num_cylinders; ++i) {
        vec2 l = intersectCylinder(ray, cylinders[i]);
        if (l.x > 0.0  && l.x < info.t ) {
            info.primitiveIndex = i;
            info.primitive_type = CYLINDER_PRIMITIVE;
            info.hit = true;
            info.t = l.x;
            found = true;
        }
    }
    return found;
}

void findIntersect(Ray ray, inout Hitinfo hitInfo) {
    intersectSpheres(ray, hitInfo);
    intersectAABBes(ray, hitInfo);
    intersectCylinder(ray, hitInfo);
}


bool isInShadow(Hitinfo hitInfo, vec3 intersectionPoint, Light light) {
    bool test = hitInfo.hit && hitInfo.t < distance(light.position, intersectionPoint);
    return test;
}

vec3 PhongShadingPerLight(Light light, vec3 intersectionPoint, vec3 normal, Hitinfo shadowHitInfo, Material material) {
    vec3 ambient = light.color * ambientStrength;
    if ( isInShadow(shadowHitInfo, intersectionPoint, light) ) { return ambient; }

    // diffuse
    vec3 lightDir = normalize(light.position - intersectionPoint);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color;

    // specular
    vec3 viewDir = normalize(cameraPosition - intersectionPoint);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * light.color;
    return (ambient + diffuse + specular) * material.color;
}

vec3 getColor(vec3 intersectionPoint, vec3 normal, Material material) {
    vec3 color = vec3(0);
    for(int i = 0; i < num_lights; ++i) {
        Light light = lights[i];
        Ray shadowRay = Ray(intersectionPoint, normalize(light.position - intersectionPoint));
        shadowRay.origin += shadowRay.direction * 0.0001; // Shift Ray
        Hitinfo shadowHitInfo = createHitInfo();
        findIntersect(shadowRay, shadowHitInfo);


        color += PhongShadingPerLight(light, intersectionPoint, normal, shadowHitInfo, material);
    }
    return color;
}

vec3 trace(Ray ray) {
    vec3 color = vec3(0);
    Hitinfo hitInfoCamera = createHitInfo();
    findIntersect(ray, hitInfoCamera);
    if ( hitInfoCamera.hit ) {
        vec3 intersectionPoint = getIntersectionPoint(ray, hitInfoCamera.t);
        vec3 normal = getNormal(hitInfoCamera, intersectionPoint);
        int materialBaseIndex = ps[hitInfoCamera.primitive_type-1];
        Material material = materials[materialBaseIndex + hitInfoCamera.primitiveIndex];

        color += getColor(intersectionPoint, normal, material) * (1.0 - material.reflectance);

        if ( material.reflectance > 0.0f) {
            float frac = material.reflectance;

            Ray reflectedRay = Ray(intersectionPoint, normalize(reflect(ray.direction, normal)));
            reflectedRay.origin += reflectedRay.direction * 0.001; // Shift Ray
            for (int i = 0; i < NUM_REFLECTION; ++i) {
                Hitinfo hitInfoReflected = createHitInfo();
                findIntersect(reflectedRay, hitInfoReflected);
                if ( !hitInfoReflected.hit ) { return color; }
                
                vec3 ip = getIntersectionPoint(reflectedRay, hitInfoReflected.t);
                vec3 normal = getNormal(hitInfoReflected, ip);
                const int startIndex = ps[hitInfoReflected.primitive_type-1];
                Material reflectedMaterial = materials[startIndex + hitInfoReflected.primitiveIndex];
                color += getColor(ip, normal, reflectedMaterial) * frac;
                frac *= reflectedMaterial.reflectance;
                if ( frac < 0.1f ) { return color; }

                reflectedRay = Ray(ip, normalize(reflect(reflectedRay.direction, normal)));
                reflectedRay.origin += reflectedRay.direction * 0.001; // Shift Ray
            }
        }
    }
    return color;
}


layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {
    // Vypocet aktualne pocitaneho pixelu s vypocetem kolize s AABB
    // https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-I%29
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    if (pixel.x >= size.x || pixel.y >= size.y) { return; }
    vec2 position = vec2(pixel) / vec2(size.x , size.y);
    vec3 direction = mix(mix(ray00, ray01, position.y), mix(ray10, ray11, position.y), position.x);
    Ray ray = Ray(cameraPosition, direction);
    vec4 color = vec4(trace(ray), 0);
    imageStore(framebuffer, pixel, color);
}
