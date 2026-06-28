#include "globals.hlsli"
#include "ShaderInterop_Postprocess.h"

PUSHCONSTANT(postprocess, PostProcess);

Texture2D<float4> input : register(t0);
RWTexture2D<float4> output : register(u0);

float hash21(float2 p)
{
	p = frac(p * float2(123.34, 456.21));
	p += dot(p, p + 45.32);
	return frac(p.x * p.y);
}

[numthreads(POSTPROCESS_BLOCKSIZE, POSTPROCESS_BLOCKSIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x >= (uint)postprocess.resolution.x || DTid.y >= (uint)postprocess.resolution.y)
		return;

	const float2 uv = (DTid.xy + 0.5f) * postprocess.resolution_rcp;

	const float time       = postprocess.params0.x;
	const float vignetteAmt = postprocess.params0.y;
	const float grainAmt   = postprocess.params0.z;
	const float redGrade   = postprocess.params0.w;

	const float pulse      = postprocess.params1.x;
	const float desat      = postprocess.params1.y;
	const float scanAmt    = postprocess.params1.z;
	const float dread      = postprocess.params1.w;

	float3 color = input[DTid.xy].rgb;

	float luma = dot(color, float3(0.299, 0.587, 0.114));
	color = lerp(color, luma.xxx, desat);

	float3 graded = color * float3(1.18, 0.90, 0.74);
	graded += float3(0.07, 0.005, 0.0) * luma;
	color = lerp(color, graded, redGrade);

	color *= (0.95 + 0.06 * pulse);

	float2 d = uv - 0.5;
	float vig = smoothstep(1.15, 0.30, length(d) * 1.35);
	float3 edge = color * float3(0.16, 0.03, 0.03);
	color = lerp(edge, color, lerp(1.0, vig, vignetteAmt));

	float scan = sin(uv.y * postprocess.resolution.y * 1.5 + time * 6.0) * 0.5 + 0.5;
	color *= 1.0 - scanAmt * (1.0 - scan);

	float g = hash21(uv * postprocess.resolution.xy + frac(time) * 137.0);
	color += (g - 0.5) * grainAmt;

	color = lerp(color, float3(0.02, 0.0, 0.0), saturate(dread));

	output[DTid.xy] = float4(max(color, 0.0), 1.0);
}
