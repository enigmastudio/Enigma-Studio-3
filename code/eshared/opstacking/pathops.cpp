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

#include "../eshared.hpp"

// Full waypoint (path) operator
// -----------------------------
// Adds a full waypoint (translation, rotation, scaling)
// to the path.

#if defined(HAVE_OP_PATH_FULL_WP) || defined(eEDITOR)
OP_DEFINE_PATH(eFullWaypointOp, eFullWaypointOp_ID, "Full WP", 'f', 0, 0, "")
    OP_INIT()
{
    eOP_PARAM_ADD_FLOAT("Time", 0.0f, eF32_MAX, 0.0f);
    eOP_PARAM_ADD_ENUM("Interpolation", "Linear|Smooth|Step", 0);
    eOP_PARAM_ADD_FXYZ("Translate", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.0f);
    eOP_PARAM_ADD_FXYZ("Rotation", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.0f);
    eOP_PARAM_ADD_FXYZ("Scale", eF32_MIN, eF32_MAX, 1.0f, 1.0f, 1.0f)
}

OP_EXEC(eGraphicsApiDx9 *gfx, eF32 time, eInt interpol, const eVector3 &trans, const eVector3 &rot, const eVector3 &scale) 
{
    //m_path.addWaypoint(time, trans, rot, scale, (ePath::InterpolationType)interpol);
}
OP_END(eFullWaypointOp);
#endif


// Vector waypoint (path) operator
// -------------------------------
// Adds a vector waypoint to the path.

#if defined(HAVE_OP_PATH_VECTOR_WP) || defined(eEDITOR)
OP_DEFINE_PATH(eVectorWaypointOp, eVectorWaypointOp_ID, "Vector WP", 'v', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Time", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_ENUM("Interpolation", "Linear|Smooth|Step", 0);
        eOP_PARAM_ADD_FXYZ("Vector", eF32_MIN, eF32_MAX, 0.0f, 0.0f, 0.0f)
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 time, eInt interpol, const eVector3 &vec) 
    {
        m_path.addWaypointVec(time, eVector4(vec, 1.0f), (ePath::InterpolationType)interpol);
    }
OP_END(eVectorWaypointOp);
#endif

// Color waypoint (path) operator
// ------------------------------
// Adds a color waypoint to the path.

#if defined(HAVE_OP_PATH_COLOR_WP) || defined(eEDITOR)
OP_DEFINE_PATH(eColorWaypointOp, eColorWaypointOp_ID, "Color WP", 'c', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Time", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_ENUM("Interpolation", "Linear|Smooth|Step", 0);
        eOP_PARAM_ADD_RGBA("Color", 1.0f, 1.0f, 1.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 time, eInt interpol, const eFloatColor &color) 
    {
        m_path.addWaypointVec(time, eVector4(color.r, color.g, color.b, color.a), (ePath::InterpolationType)interpol);
    }
OP_END(eColorWaypointOp);
#endif

// Scalar waypoint (path) operator
// -------------------------------
// Adds a scalar waypoint to the path.

#if defined(HAVE_OP_PATH_SCALAR_WP) || defined(eEDITOR)
OP_DEFINE_PATH(eScalarWaypointOp, eScalarWaypointOp_ID, "Scalar WP", 'c', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Time", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_ENUM("Interpolation", "Linear|Smooth|Step", 0);
        eOP_PARAM_ADD_FLOAT("Scalar", eF32_MIN, eF32_MAX, 0.0f)
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 time, eInt interpol, eF32 scalar) 
    {
        m_path.addWaypointScalar(time, scalar, (ePath::InterpolationType)interpol);
    }
OP_END(eScalarWaypointOp);
#endif

// TShader waypoint (path) operator
// --------------------------------
// Adds a transform shader waypoint to the path.

#if defined(HAVE_OP_PATH_TSHADER_WP) || defined(eEDITOR)
OP_DEFINE_PATH(eWaypointTShaderOp, eWaypointTShaderOp_ID, "TShader WP", 't', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_FLOAT("Time", 0.0f, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_TSHADERCODE("Code", "");
    }

#ifdef eEDITOR
    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 time, const eChar *source)
    {
        eTShaderCompiler tsc;

        if (!tsc.compile(source))
        {
            eShowError(tsc.getErrors());
        }

        eByteArray code;
        tsc.getCode(code);
        m_tshader.load(code);
        m_path.addWaypointTShader(time, &m_tshader);
    }
#else
    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 time, const eByteArray &byteCode)
    {
        m_tshader.load(byteCode);
        m_path.addWaypointTShader(time, &m_tshader);
    }
#endif

    OP_VAR(eTShader m_tshader);
OP_END(eWaypointTShaderOp);
#endif

// Simple Path (path) operator
// ---------------------------
// Creates a path consisting of 4 scalar waypoints.

#if defined(HAVE_OP_PATH_SIMPLE_PATH) || defined(eEDITOR)
OP_DEFINE_PATH(eSimplePathOp, eSimplePathOp_ID, "Simple path", ' ', 0, 0, "")
    OP_INIT()
    {
        eOP_PARAM_ADD_LABEL("1. Waypoint", "1. Waypoint");
        eOP_PARAM_ADD_FLOAT("time1", 0, eF32_MAX, 0.0f);
        eOP_PARAM_ADD_FXYZW("value1", -eF32_MAX, eF32_MAX, 0.0f, 0.0f, 0.0f, 1.0f);

        eOP_PARAM_ADD_LABEL("2. Waypoint", "2. Waypoint");
        eOP_PARAM_ADD_FLOAT("time2", 0, eF32_MAX, 0.3333f);
        eOP_PARAM_ADD_FXYZW("value2 ", -eF32_MAX, eF32_MAX, 0.5f, 0.0f, 0.0f, 1.0f);

        eOP_PARAM_ADD_LABEL("3. Waypoint", "3. Waypoint");
        eOP_PARAM_ADD_FLOAT("time3", 0, eF32_MAX, 0.6666f);
        eOP_PARAM_ADD_FXYZW("value3 ", -eF32_MAX, eF32_MAX, 0.5f, 0.0f, 0.0f, 1.0f);

        eOP_PARAM_ADD_LABEL("4. Waypoint", "4. Waypoint");
        eOP_PARAM_ADD_FLOAT("time4", 0, eF32_MAX, 1.0f);
        eOP_PARAM_ADD_FXYZW("value4 ", -eF32_MAX, eF32_MAX, 1.0f, 0.0f, 0.0f, 1.0f);
    }

    OP_EXEC(eGraphicsApiDx9 *gfx, eF32 time0, const eVector4 &value0, eF32 time1,
            const eVector4 &value1, eF32 time2, const eVector4 &value2, eF32 time3, const eVector4 &value3) 
    {

        //m_path.addWaypoint(time0, eVector3(value0.x, value0.y, value0.z), eVector3(value0.w, 0.0f, 0.0f), eVector3(1.0f), ePath::INTERPOLATE_LINEAR);
        //m_path.addWaypoint(time1, eVector3(value1.x, value1.y, value1.z), eVector3(value1.w, 0.0f, 0.0f), eVector3(1.0f), ePath::INTERPOLATE_LINEAR);
        //m_path.addWaypoint(time2, eVector3(value2.x, value2.y, value2.z), eVector3(value2.w, 0.0f, 0.0f), eVector3(1.0f), ePath::INTERPOLATE_LINEAR);
        //m_path.addWaypoint(time3, eVector3(value3.x, value3.y, value3.z), eVector3(value3.w, 0.0f, 0.0f), eVector3(1.0f), ePath::INTERPOLATE_LINEAR);
    }
OP_END(eSimplePathOp);
#endif

// Path (path) operator
// --------------------
// Creates an empty path.

#if defined(HAVE_OP_PATH_PATH) || defined(eEDITOR)
OP_DEFINE_PATH(ePathOp, ePathOp_ID, "Path", 'p', 1, 256, "-1,Path")
    OP_EXEC(eGraphicsApiDx9 *gfx)
    {
        m_path.clear();

        for (eU32 i=0; i<getInputCount(); i++)
        {
            const ePath &path = ((eIPathOp *)getInputOperator(i))->getResult().path;
            m_path.merge(path);
        }
    }
OP_END(ePathOp);
#endif