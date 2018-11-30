#version 430 core

struct Ray { vec3 origin, direction; };
struct Sphere { vec3 center; float radius; };
struct Cylinder { vec3 center; float p; vec3 direction; float radius; };
struct AABB { vec3 min; float p; vec3 max; float p1; };
struct Light { vec3 position; float p; vec3 color; float p1; };
struct Material {int reflectFlag; };

layout(binding = 0, rgba32f) uniform image2D framebuffer;
layout(std140, binding = 10) buffer debug { vec4 debug_out[]; };

layout(std140, binding = 1) readonly buffer AABB_buffer { AABB aabb[]; };
layout(std140, binding = 2) readonly buffer spheres_buffer { Sphere spheres[]; };
layout(std140, binding = 3) readonly buffer cylinder_buffer { Cylinder cylinders[]; };
layout(std140, binding = 4) readonly buffer lights_buffer { Light lights[]; };
layout(std140, binding = 5) readonly buffer material_buffer { Material materials[]; };


uniform vec3 eye;
uniform vec3 ray00, ray01, ray10, ray11;
uniform int num_aabb, num_spheres, num_cylinders, num_lights;


const float ambientStrength = 0.1f;
const float specularStrength = 0.5f;


#define MAX_SCENE_BOUNDS 100.0

#define AABB_PRIMITIVE 1
#define SPHERE_PRIMITIVE 2
#define CYLINDER_PRIMITIVE 3
#define RECURSION 15
#define EPSILON 0.00001

struct hitinfo {
    bool hit;
    float t;
    int primitiveIndex;
    int primitive_type;
};

hitinfo createHitInfo() {
    hitinfo i;
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
    if (cubePoint.x < aabb.min.x + EPSILON)
        return vec3(-1.0, 0.0, 0.0);
    else if (cubePoint.x > aabb.max.x - EPSILON)
        return vec3(1.0, 0.0, 0.0);
    else if (cubePoint.y < aabb.min.y + EPSILON)
        return vec3(0.0, -1.0, 0.0);
    else if (cubePoint.y > aabb.max.y - EPSILON)
        return vec3(0.0, 1.0, 0.0);
    else if (cubePoint.z < aabb.min.z + EPSILON)
        return vec3(0.0, 0.0, -1.0);
    else
        return vec3(0.0, 0.0, 1.0);


}

vec3 getRayHitPoint(Ray ray, float t) {
    return ray.origin + ray.direction * t;
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

bool intersectAABBes(Ray ray, inout hitinfo info) {
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

bool intersectSpheres(Ray ray, inout hitinfo info) {
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

bool intersectCylinder(Ray ray, inout hitinfo info) {
    float smallest = MAX_SCENE_BOUNDS;
    bool found = false;
    for(int i = 0; i < num_cylinders; ++i) {
        vec2 l = intersectCylinder(ray, cylinders[i]);
        if (l.x > 0.0 && l.x < l.t) {
            info.primitiveIndex = i;
            info.primitive_type = CYLINDER_PRIMITIVE;
            info.hit = true;
            info.t = l.x;
            found = true;
        }
    }
    return found;
}

void findIntersect(Ray ray, inout hitinfo hitInfo) {
    intersectSpheres(ray, hitInfo);
    intersectAABBes(ray, hitInfo);
    intersectCylinder(ray, hitInfo);
}




vec3 getNormal(hitinfo hitInfo, vec3 intersectionPoint) {
    switch(hitInfo.primitive_type) {
        case AABB_PRIMITIVE: return getNormalAABB(aabb[hitInfo.primitiveIndex], intersectionPoint); 
        case SPHERE_PRIMITIVE: return getNormalSphere(spheres[hitInfo.primitiveIndex], intersectionPoint); 
        case CYLINDER_PRIMITIVE: return getNormalCylinder(cylinders[hitInfo.primitiveIndex], intersectionPoint);
    }
}

bool isInShadow(hitinfo hitInfo, vec3 intersectionPoint, Light light) {
    bool test = hitInfo.hit && hitInfo.t < distance(light.position, intersectionPoint);
    return test;
}

vec3 PhongShadingPerLight(Light light, vec3 intersectionPoint, vec3 normal, hitinfo shadowHitInfo) {
    vec3 ambient = light.color * ambientStrength;
    if ( isInShadow(shadowHitInfo, intersectionPoint, light) ) { 
        return ambient; 
    }

    // diffuse
    vec3 lightDir = normalize(light.position - intersectionPoint);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color;


    const vec3 white = vec3(1); // TODO should be Material Component

    // specular
    vec3 viewDir = normalize(eye - intersectionPoint);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * light.color;
    return (ambient + diffuse + specular) * white;
}

vec3 lightning(vec3 intersectionPoint, vec3 normal, hitinfo hitInfo_cameraRay) {
    vec3 color = vec3(0);
    for(int i = 0; i < num_lights; ++i) {
        Light light = lights[i];
        Ray shadowRay = Ray(intersectionPoint, normalize(light.position - intersectionPoint));
        shadowRay.origin += shadowRay.direction * 0.0001; // Shift Ray
        hitinfo shadowHitInfo = createHitInfo();
        findIntersect(shadowRay, shadowHitInfo);

        color += PhongShadingPerLight(light, intersectionPoint, normal, shadowHitInfo);
    }
    return color;
}

float isMirror(hitinfo hitInfo) {
    if (hitInfo.primitiveIndex == 0 && hitInfo.primitive_type == AABB_PRIMITIVE ||
         (hitInfo.primitiveIndex == 0 && hitInfo.primitive_type == SPHERE_PRIMITIVE) ) 
    {
        return 1.0f;
    }
    else {
        return 0.0f;
    }
}

vec3 trace(Ray ray) {
    vec3 color = vec3(0);
    hitinfo hitInfo_cameraRay = createHitInfo();
    findIntersect(ray, hitInfo_cameraRay);
    if ( hitInfo_cameraRay.hit ) {
        vec3 intersectionPoint = getIntersectionPoint(ray, hitInfo_cameraRay.t);
        vec3 normal = getNormal(hitInfo_cameraRay, intersectionPoint);
        color += lightning(intersectionPoint, normal, hitInfo_cameraRay) * (1.0 - isMirror(hitInfo_cameraRay));

        if ( isMirror(hitInfo_cameraRay) == 1.0f) {
            float frac = 1.0;

            // Ray reflectedRay = Ray(intersectionPoint, normalize(ray.direction - 2*(ray.direction*normal) * normal));
            Ray reflectedRay = Ray(intersectionPoint, normalize(reflect(ray.direction, normal)));
            reflectedRay.origin += reflectedRay.direction * 0.001; // Shift Ray
            for (int i = 0; i < RECURSION; ++i) {
                hitinfo hi = createHitInfo();
                findIntersect(reflectedRay, hi);
                if ( !hi.hit ) { return vec3(0); }
                
                vec3 ip = getIntersectionPoint(reflectedRay, hi.t);
                vec3 normal = getNormal(hi, ip);
                color += lightning(ip, normal, hi) * frac;
                frac *= isMirror(hi);
                if ( isMirror(hi) != 1.0f ) {
                    return color;
                }

                // reflectedRay = Ray(ip, normalize(reflectedRay.direction - 2*(reflectedRay.direction*normal) * normal));
                reflectedRay = Ray(ip, normalize(reflect(reflectedRay.direction, normal)));
                reflectedRay.origin += reflectedRay.direction * 0.001; // Shift Ray
            }
        }
    }
    return color;
}


layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    if (pixel.x >= size.x || pixel.y >= size.y) { return; }
    vec2 pos = vec2(pixel) / vec2(size.x - 1, size.y - 1);
    vec3 dir = mix(mix(ray00, ray01, pos.y), mix(ray10, ray11, pos.y), pos.x);
    Ray ray = Ray(eye, dir);
    vec4 color = vec4(trace(ray), 0);
    imageStore(framebuffer, pixel, color);
}
