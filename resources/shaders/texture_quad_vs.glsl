#version 450

in vec2 vert;

out vec2 texCoords;

void main() {
    gl_Position = vec4(vert, 0.0, 1.0);
    texCoords = vert * 0.5 + vec2(0.5);
}