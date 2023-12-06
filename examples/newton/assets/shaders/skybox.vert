#version 300 es

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec2 fragTexCoord;

void main() {
  fragTexCoord = inTexCoord;
  
  vec4 P = projMatrix * viewMatrix * vec4(inPosition, 1.0);
  gl_Position = P.xyww;
}