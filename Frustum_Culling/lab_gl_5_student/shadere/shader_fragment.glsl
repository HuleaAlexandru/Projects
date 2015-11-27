#version 330
layout(location = 0) out vec4 out_color;

uniform sampler2D textura1;

uniform vec3 light_position;
uniform vec3 direction_position;
uniform float cut_off;
uniform int spot_exponent;
uniform int camera;

in vec3 world_pos;
in vec3 world_normal;

in vec2 texcoord;

void main(){

	vec3 tex1 = texture(textura1, texcoord).xyz;
	if (camera == 0) {
		float spotEffect = 0;
		vec3 L = normalize(light_position - world_pos);
		float nL = max (0.0, dot(world_normal, L));	
		if (nL > 0.0) {
			spotEffect = dot(direction_position, -L);
			if (spotEffect > cos(cut_off)) {
				spotEffect = spotEffect * 79 / 80;
				spotEffect = pow(spotEffect, spot_exponent);
				tex1.r = clamp(tex1.r + spotEffect, 0, 1);
				tex1.g = clamp(tex1.g + spotEffect, 0, 1);
			}
		}
	}

	out_color = vec4(tex1, 1);
	
}