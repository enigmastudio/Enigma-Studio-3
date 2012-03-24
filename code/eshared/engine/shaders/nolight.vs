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

struct a2f
{
    float4 pos:         POSITION;
    float4 color:       COLOR;
    float2 texCoord:    TEXCOORD0;
};

struct v2f 
{
	float4 pos:         POSITION;
	float4 color:       COLOR;
    float2 texCoord:    TEXCOORD0;
};

v2f main(const a2f input)
{
	v2f output = (v2f)0;
	
	output.pos      = mul(input.pos, c_mvp);
	output.color    = input.color;
    output.texCoord = input.texCoord;

	return output;
}