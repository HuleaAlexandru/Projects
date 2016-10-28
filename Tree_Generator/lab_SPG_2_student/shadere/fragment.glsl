#version 330
layout(location = 0) out vec4 out_color;

in vec3 world_position;
in vec3 normal;
in vec2 texCoord;

uniform sampler2D myTexture;
uniform sampler2D myAlphaTexture;
uniform int mod;

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float shininess;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform vec3 constants;

void main(){
	
	if (mod == 0) {
		out_color = vec4(0, 0, 0, 1);
	}
	else if (mod == 1) {
		vec4 tex = texture(myTexture, texCoord);
		
		vec3 ambient = ambientStrength * lightColor;
		
		vec3 N = normalize(normal);
		vec3 L = lightPos - world_position;
		L = normalize(L);
		float diff = max(dot(N, L), 0.0);
		vec3 diffuse = diff * lightColor * diffuseStrength;
		
		vec3 V = viewPos - world_position;
		V = normalize(V);
		vec3 H = normalize(L + V);
		float spec = pow(max(dot(N, H), 0.0), shininess);
		vec3 specular = spec * lightColor * specularStrength;
		
		float distance = length(lightPos - world_position);
		float atenuare = 1. / (constants.x + constants.y * distance + constants.z * distance * distance);
		
		vec3 light = (ambient + diffuse + specular) * vec3(tex) * atenuare;
		out_color = vec4(light, 1); 
	}
	else if (mod == 2) {
		vec4 texAlpha = texture(myAlphaTexture, texCoord);
		if (texAlpha.r < 0.6 && texAlpha.g < 0.6 && texAlpha.b < 0.6)
			discard;
		else {
			vec4 tex = texture(myTexture, texCoord);
			
			vec3 ambient = ambientStrength * lightColor;
			
			vec3 N = normalize(normal);
			vec3 L = lightPos - world_position;
			L = normalize(L);
			float diff = max(dot(N, L), 0);
			vec3 diffuse = diff * lightColor;
			
			vec3 V = viewPos - world_position;
			V = normalize(V);
			vec3 H = normalize(L + V);
			float spec = pow(max(dot(N, H), 0.0), shininess);
			vec3 specular = spec * lightColor * specularStrength;
				
			float distance = length(lightPos - world_position);
			float atenuare = 1. / (constants.x + constants.y * distance + constants.z * distance * distance);	
			
			vec3 light = (ambient + diffuse + specular) * vec3(tex) * atenuare;
			out_color = vec4(light, 1);  
		}
	}
}