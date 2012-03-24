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
    float4 pos:         POSITION;	// position (object space)
    float3 normal:      NORMAL;		// normal (object space)
    float2 texCoord:    TEXCOORD0;	// texture coordinates
    float3 color:       COLOR0;		// color

	float4	modelMtx0:	TEXCOORD1;	// per instance model matrix
	float4	modelMtx1:  TEXCOORD2;
	float4	modelMtx2:  TEXCOORD3;
	float4	modelMtx3:  TEXCOORD4;

	float4	normalMtx0:	TEXCOORD5;	// per instance inverse transpose MVP matrix
	float4	normalMtx1:	TEXCOORD6;
	float4	normalMtx2:	TEXCOORD7;
	float4	normalMtx3:	TEXCOORD8;
};

struct v2f
{
    float4 hpos:        POSITION;	// position (clip-space)
    float2 texCoord:    TEXCOORD0;	// texture coordinate
    float3 vpos:        TEXCOORD1;	// position (view-space)
    float3 normal:      TEXCOORD2;	// normal (view space)
    float4 clipPos:     TEXCOORD3;  // position (clip-space), needed twice because POSITION semantic is inaccessible in PS
    float3 color:       COLOR0;		// color

    float3 viewDirWs:   TEXCOORD5;
    float3 normalWs:    TEXCOORD6;
};

v2f main(a2v input)
{
	const float4x4 modelMtx = float4x4(input.modelMtx0, input.modelMtx1, input.modelMtx2, input.modelMtx3);
	const float4x4 normalMtx = float4x4(input.normalMtx0, input.normalMtx1, input.normalMtx2, input.normalMtx3);

    const float4 wpos = mul(input.pos, modelMtx);
    const float4 vpos = mul(wpos, c_viewMtx);
    const float4 hpos = mul(vpos, c_projMtx);

	v2f output = (v2f)0;

	output.hpos		= hpos;
	output.vpos		= vpos.xyz;
	output.normal	= mul(float4(input.normal, 1.0f), normalMtx).xyz;
	output.texCoord = input.texCoord;
    output.clipPos  = hpos;
    output.color	= input.color;

    output.viewDirWs = c_camWorldPos-wpos.xyz;
    output.normalWs = mul(float4(input.normal, 1.0f), modelMtx).xyz;

   	return output;
}