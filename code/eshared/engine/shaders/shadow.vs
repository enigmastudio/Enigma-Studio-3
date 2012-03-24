/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 * 
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "globals.vs"

struct a2v
{
    float4 pos:         POSITION;   // position (object space)

	float4 modelMtx0:	TEXCOORD1;	// per instance model matrix
	float4 modelMtx1:	TEXCOORD2;
	float4 modelMtx2:	TEXCOORD3;
	float4 modelMtx3:	TEXCOORD4;
};

struct v2f 
{
	float4 hpos:        POSITION;   // position (clip space)
	float3 lightVec:    TEXCOORD0;  // vector from vertex to light (world spcae)
};

v2f main(const a2v input)
{
    const float4x4 modelMtx = float4x4(input.modelMtx0, input.modelMtx1, input.modelMtx2, input.modelMtx3);
    const float4 wpos = mul(input.pos, modelMtx);
	
    v2f output = (v2f)0;

    output.hpos = mul(mul(wpos, c_viewMtx), c_projMtx);
	output.lightVec = wpos.xyz-c_lightWorldPos;

	return output;
}