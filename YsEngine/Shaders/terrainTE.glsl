#version 460 core

layout (quads, fractional_odd_spacing, ccw) in;

uniform sampler2D heightSampler;
uniform mat4 PVM;
uniform float HEIGHT_SCALE;

in vec2 TextureCoord[];

out float Height;
out vec2 TexCoord_global;
out vec2 TexCoord_local;
out vec3 FragPos;

void main()
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec2 t00 = TextureCoord[0];
	vec2 t01 = TextureCoord[1];
	vec2 t10 = TextureCoord[2];
	vec2 t11 = TextureCoord[3];

	vec2 t0 = (t01 - t00) * u + t00;
	vec2 t1 = (t11 - t10) * u + t10;
	vec2 texCoord = (t1 - t0) * v + t0;

	Height = texture(heightSampler, texCoord).x * HEIGHT_SCALE - 16.0;

	vec4 p00 = gl_in[0].gl_Position;
	vec4 p01 = gl_in[1].gl_Position;
	vec4 p10 = gl_in[2].gl_Position;
	vec4 p11 = gl_in[3].gl_Position;

	vec4 uVec = p01 - p00;
	vec4 vVec = p10 - p00;
	vec4 normal = normalize(vec4(cross(vVec.xyz, uVec.xyz), 0));

	vec4 p0 = (p01 - p00) * u + p00;
	vec4 p1 = (p11 - p10) * u + p10;
	vec4 p = (p1 - p0) * v + p0;

	p += normal * Height;

	gl_Position = PVM * p;
	TexCoord_global = texCoord;
	TexCoord_local = gl_TessCoord.xy;
	FragPos = p.xyz;
}