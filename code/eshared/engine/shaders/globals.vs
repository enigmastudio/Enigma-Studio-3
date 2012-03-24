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
 
float3      c_lightViewPos:		register(c0);	// light position in view-space
float3      c_lightWorldPos:	register(c1); 	// light position in world-space
float       c_lightInvRange:	register(c2);	// 1 / light range

float3      c_camWorldPos:      register(c3);   // camera position in world-space

float4x4    c_viewMtx:			register(c4);	// view matrix
float4x4    c_projMtx:			register(c8);	// projection matrix
float4x4    c_mvp:				register(c12);	// model-view-projection (c_mvp) matrix