#version 300 es

precision mediump float;

in vec2 fragTexCoord;

// Diffuse texture sampler
uniform sampler2D diffuseTex;

out vec4 outColor;

void main() {
  outColor = texture(diffuseTex, fragTexCoord);
}
