#define HAVE_OP_MISC_DEMO
#define eDemoOp_ID 0
#define Misc_CID 0
#define HAVE_OP_SEQUENCER_SCENE
#define eSeqSceneOp_ID 1
#define Sequencer_CID 1
#define HAVE_OP_EFFECT_MERGE
#define eFxMergeOp_ID 2
#define Effect_CID 2
#define HAVE_OP_EFFECT_ADJUST
#define eFxAdjustOp_ID 3
#define HAVE_OP_EFFECT_SSAO
#define eFxSsaoOp_ID 4
#define HAVE_OP_EFFECT_CAMERA
#define eFxCameraOp_ID 5
#define HAVE_OP_MISC_SCENE
#define eSceneOp_ID 6
#define HAVE_OP_MODEL_LIGHT
#define eLightOp_ID 7
#define Model_CID 3
#define HAVE_OP_MODEL_MERGE
#define eModelMergeOp_ID 8
#define HAVE_OP_MODEL_TRANSFORM
#define eModelTransformOp_ID 9
#define HAVE_OP_MODEL_MODEL
#define eModelModelOp_ID 10
#define HAVE_OP_MESH_TRANSFORM
#define eMeshTransformOp_ID 11
#define Mesh_CID 4
#define HAVE_OP_MESH_MULTIPLY
#define eMeshMultiplyOp_ID 12
#define HAVE_OP_MESH_MERGE
#define eMeshMergeOp_ID 13
#define HAVE_OP_MESH_SET_MATERIAL
#define eSetMaterialOp_ID 14
#define HAVE_OP_MESH_PLANE
#define ePlaneOp_ID 15
#define HAVE_OP_MISC_MATERIAL
#define eMaterialOp_ID 16
#define HAVE_OP_BITMAP_FILL
#define eFillOp_ID 17
#define Bitmap_CID 5
#define HAVE_OP_MODEL_MULTIPLY
#define eModelMultiplyOp_ID 18
#define HAVE_OP_MESH_SELECT_CUBE
#define eSelectCubeOp_ID 19
#define HAVE_OP_MESH_SPHERE
#define eSphereOp_ID 20
#define HAVE_OP_BITMAP_NORMALS
#define eNormalsOp_ID 21
#define HAVE_OP_BITMAP_BLUR
#define eBlurOp_ID 22
#define HAVE_OP_BITMAP_RECT
#define eRectOp_ID 23
#define HAVE_OP_BITMAP_COLOR
#define eColorOp_ID 24
#define HAVE_OP_BITMAP_ADJUST
#define eAdjustOp_ID 25
#define HAVE_OP_BITMAP_MERGE
#define eBitmapMergeOp_ID 26
#define HAVE_OP_BITMAP_PERLIN
#define ePerlinOp_ID 27
#define HAVE_OP_BITMAP_ROTOZOOM
#define eRotoZoomOp_ID 28
#define HAVE_OP_MESH_OPTIONS_UV
#define eOptionsUvOp_ID 29
#define HAVE_OP_MODEL_LSYSTEM
#define eLSystemOp2_ID 30
#define HAVE_OP_MISC_LSYMBOL
#define eLSymbolOp_ID 31
#define HAVE_OP_MESH_DUP__CIRCULAR
#define eMeshDuplicateCircularOp_ID 32
#define HAVE_OP_BITMAP_ALPHA
#define eAlphaOp_ID 33
#define HAVE_OP_BITMAP_LINE
#define eLineOp_ID 34
#define HAVE_OP_PATH_PATH
#define ePathOp_ID 35
#define Path_CID 6
#define HAVE_OP_PATH_VECTOR_WP
#define eVectorWaypointOp_ID 36
#define HAVE_OP_MODEL_DUP__CIRCULAR
#define eModelDuplicateCircularOp_ID 37
#define HAVE_OP_MESH_CYLINDER
#define eCylinderOp_ID 38
#define HAVE_OP_PATH_TSHADER_WP
#define eWaypointTShaderOp_ID 39
#define HAVE_OP_PATH_SCALAR_WP
#define eScalarWaypointOp_ID 40
#define HAVE_OP_EFFECT_BLUR
#define eFxBlurOp_ID 41
#define HAVE_OP_EFFECT_DOWNSAMPLE
#define eFxDownsampleOp_ID 42
#define HAVE_OP_MESH_RING
#define eRingOp_ID 43
#define HAVE_OP_MESH_TEXT_3D
#define eText3dOp_ID 44
#define HAVE_OP_EFFECT_FOG
#define eFxFogOp_ID 45
#define HAVE_OP_MESH_VERTEX_NOISE
#define eVertexNoiseOp_ID 46
#define HAVE_OP_MESH_DISPLACE
#define eDisplaceOp_ID 47
#define HAVE_OP_MESH_INSIDEOUT
#define eInsideOutOp_ID 48
#define HAVE_OP_MODEL_POPULATE_SURFACE
#define eModelPopulateSurfaceOp_ID 49
#define HAVE_OP_MESH_BICUBICPATCH
#define eBicubicPatchOp_ID 50
#define HAVE_OP_MESH_DELETE
#define eMeshDeleteOp_ID 51
#define HAVE_OP_MISC_LATTRACTOR
#define eLAttractorOp_ID 52
#define HAVE_OP_MESH_CUBE
#define eCubeOp_ID 53
#define HAVE_OP_EFFECT_RIPPLE
#define eFxRippleOp_ID 54
#define HAVE_OP_BITMAP_PIXELS
#define ePixelsOp_ID 55
#define HAVE_OP_SEQUENCER_OVERLAY
#define eSeqOverlayOp_ID 56
#define HAVE_OP_PATH_COLOR_WP
#define eColorWaypointOp_ID 57

#define TOTAL_OP_TYPE_COUNT 58
#ifndef SCRIPT_OPS_HPP
#define SCRIPT_OPS_HPP

class eIOperator;
class SCRIPT_OP_CREATOR {
public:
    static eIOperator* createOp(unsigned int nr);
};

#endif
