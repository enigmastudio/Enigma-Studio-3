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
    float4 pos:         POSITION;
    float2 texCoord:    TEXCOORD0;
    float4 color:       COLOR0;
};

struct v2f
{
	float4 pos:         POSITION;
	float2 texCoord:    TEXCOORD0;
	float4 color:       COLOR0;
};

v2f main(const a2v input)
{
	v2f output = (v2f)0;
	
	output.pos = mul(input.pos, c_mvp);
	output.texCoord = input.texCoord;
	output.color = input.color;

	return output;
}