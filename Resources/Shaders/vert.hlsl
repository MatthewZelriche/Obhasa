float4 main(in uint vertexID : SV_VertexID) : SV_POSITION
{
    float4 pos[3];
    pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    pos[1] = float4( 0.0, 0.5, 0.0, 1.0);
    pos[2] = float4(0.5, -0.5, 0.0, 1.0);

    return pos[vertexID];
}