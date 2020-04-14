float4 VSVertexShader(float4 Pos : POSITION) : SV_POSITION
{
    return Pos;
}

float4 PSPixelShader(float4 Pos : SV_POSITION) : SV_Target
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

technique10 Triangle
{
    pass P0
    {
        SetGeometryShader(NULL);
        SetVertexShader(CompileShader(vs_4_0, VSVertexShader()));
        SetPixelShader(CompileShader(ps_4_0, PSPixelShader()));
    }
}
