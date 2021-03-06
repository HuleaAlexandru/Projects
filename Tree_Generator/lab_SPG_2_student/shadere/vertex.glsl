#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;	
layout(location = 2) in vec2 in_texCoord;

out vec3 world_position;
out vec3 normal;
out vec2 texCoord;

uniform mat4 model_matrix, view_matrix, projection_matrix;

void main(){
	world_position = vec3(model_matrix * vec4(in_position,1));
	normal = normalize(mat3(model_matrix) * in_normal);
	texCoord = in_texCoord;
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position,1); 
}
