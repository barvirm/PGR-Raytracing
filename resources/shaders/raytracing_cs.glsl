#version 430 core

layout(binding = 0, rgba32f) uniform image2D framebuffer;
layout(std140, binding = 3) buffer debug { vec4 debug_out[]; };

uniform vec3 eye;
uniform vec3 ray00, ray01, ray10, ray11;


#define MAX_SCENE_BOUNDS 100.0
#define NUM_BOXES 2

struct Ray { vec3 origin, direction; };
struct AABB { vec3 min, max; };

const AABB boxes[] = {
    // The ground 
    {vec3(-5.0, -0.1, -5.0), vec3(5.0, 0.0, 5.0)},
    // Box in the middle 
    {vec3(-0.5, 0.0, -0.5), vec3(0.5, 1.0, 0.5)}
};

struct hitinfo {
    vec2 lambda;
    int bi;
};

vec2 intersectBox(Ray ray, const AABB b) {
    vec3 invDir = 1.0f / ray.direction;
    vec3 tMin = (b.min - ray.origin) * invDir;
    vec3 tMax = (b.max - ray.origin) * invDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

bool intersectBoxes(Ray ray, out hitinfo info) {
    float smallest = MAX_SCENE_BOUNDS;
    bool found = false;
    for (int i = 0; i < NUM_BOXES; i++) {
        vec2 lambda = intersectBox(ray, boxes[i]);
        if (lambda.x > 0.0 && lambda.x < lambda.y && lambda.x < smallest) {
            info.lambda = lambda;
            info.bi = i;
            smallest = lambda.x;
            found = true;
        }
    }
    return found;
}

vec4 trace(Ray ray) {
    hitinfo i;
    if (intersectBoxes(ray, i)) {
        vec4 gray = vec4(i.bi / 10.0 + 0.8);
        return vec4(gray.rgb, 1.0);
    }
    return vec4(0.0, 0.0, 0.0, 1.0);
}


layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    debug_out[gl_GlobalInvocationID.x] = vec4(pixel, size);
    if (pixel.x >= size.x || pixel.y >= size.y) { return; }
    vec2 pos = vec2(pixel) / vec2(size.x - 1, size.y - 1);
    vec3 dir = mix(mix(ray00, ray01, pos.y), mix(ray10, ray11, pos.y), pos.x);
    Ray ray = Ray(eye, dir);
    vec4 color = trace(ray);
    imageStore(framebuffer, pixel, color);
}
