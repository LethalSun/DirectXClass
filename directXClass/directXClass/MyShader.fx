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
	float4 pos:SV_POSITION; //:SV_POSITION �� �� �ٿ��� �Ѵ�. 
	float4 color:COLOR0;//�������� ���� ������ �ڿ� ���ڸ� ���δ� �Ⱥ��̸� 0�� �ִ°Ͱ� ����

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

float4 PS(VertexOut vOut) : SV_TARGET//: SV_TARGET �� ���ٿ��� �Ѵ�.
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
////Semantic : ������ Tag�� �����ڿ� ���� �������� �ٸ��� �����Ƿ�
//������ ��� �뵵�� ���� ����ϴ� Ű����

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