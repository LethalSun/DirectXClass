Texture2D texDiffuse;
SamplerState samLinear;
 
cbuffer ConstantBuffer
{
	float4x4 wvp;
	float4x4 world;

	float4 lightDir;
	float4 lightColor;

	float4 ambient;
};

struct VertexIn
{
	float3 pos:POSITION;
	float4 color:COLOR;

	float3 normal:NORMAL;
	float2 tex:TEXCOORD;
};

struct VertexOut
{
	float4 pos:SV_POSITION; //:SV_POSITION 이 꼭 붙여야 한다. 
	float4 color:COLOR0;//여러개의 색을 가질때 뒤에 숫자를 붙인다 안붙이면 0이 있는것과 같음

	float4 normal:NORMAL;
	float2 tex:TEXCOORD;
};

VertexOut VS(VertexIn vIn)
{
	VertexOut vOut;
	vOut.pos = mul(float4(vIn.pos, 1.f),wvp);
	vOut.color = vIn.color;

	vOut.normal = mul(float4(vIn.normal, 0.0f), world);
	vOut.tex = vIn.tex;
	return vOut;
}

float4 PS(VertexOut vOut) : SV_TARGET//: SV_TARGET 을 꼭붙여야 한다.
{
	//float4 finalColor = 0;
	//finalColor = saturate(((dot(-lightDir, vOut.normal)+1)/2)*lightColor);
	//
	//float4 texColor = texDiffuse.Sample(samLinear, vOut.tex)*finalColor;
	//texColor.a = 1.0f;
	//
	//return texColor;
	return texDiffuse.Sample(samLinear, vOut.tex);
}

RasterizerState SolidframeRS
{
	FillMode = Solid;
	CullMode = Back;
	FrontCounterClockwise = false;
};

technique11 TechSolid
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));

		SetRasterizerState(SolidframeRS);
	}
}
//RasterizerState SolidFrameRS
//{
//	FillMode = Solidframe;
//	CullMode = Back;
//	FrontCounterClockwise = false;
//};
//
//technique11 TechSolid
//{
//	pass P0
//	{
//		SetVertexShader(CompileShader(vs_5_0, VS()));
//		SetPixelShader(CompileShader(ps_5_0, PS()));
//
//		SetRasterizerState(SolidFrameRS);
//	}
//}
////Semantic : 일종의 Tag로 개발자에 따라 변수명이 다를수 있으므로
//변수의 사용 용도에 대해 명시하는 키워드

/*
struct VertexIn
{
	float3 pos:POSITION;
	float4 color:COLOR;
};

struct VertexOut
{
	float4 pos:SV_POSITION;
	float4 color:COLOR;
};

VertexOut VS(VertexIn vIn)
{
	VertexOut vOut;
	vOut.pos = float4(vIn.pos, 1.f);
	vOut.color = vIn.color;

	return vOut;
}

float4 PS(VertexOut vOut) : SV_TARGET
{
	return vOut.color;
}
*/

/*
float4 VS(float4 pos:POSITION):SV_POSITION
{
	return pos;
}

float4 PS(float4 pos:SV_POSITION):SV_TARGET
{
	float x = 1.f,y = 1.f;
	x = (pos.x/ 800.f)*0.5f;
	y = (pos.y/ 600.f)*0.5f;
	return float4(x,y,0.f,1.0f);
}
*/