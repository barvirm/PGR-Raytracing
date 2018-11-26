#version 450

in vec2 texCoords;
out vec4 color;

uniform sampler2D tex;
uniform vec4 c;

void main() {
    color = c;//texture(tex, texCoords);
}
