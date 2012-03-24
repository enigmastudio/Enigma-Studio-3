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

eIMathFunctionOp::eIMathFunctionOp() :
    m_mathFunc(eNULL),
    m_result(m_mathFunc)
{
#ifdef eEDITOR
    setCategory("mathFunc");
    setColor(eColor(63, 183, 35));
#endif
}

eIMathFunctionOp::~eIMathFunctionOp()
{
    eSAFE_DELETE(m_mathFunc);
}

const eIMathFunctionOp::Result & eIMathFunctionOp::getResult() const
{
    return m_result;
}

eIMathFunctionOp::Result & eIMathFunctionOp::getResult()
{
    return m_result;
}

eIOperator::Category eIMathFunctionOp::getCategory() const
{
    return CATEGORY_MATHFUNCTIONOP;
}

// Sinus Operator
eOPERATOR_DECLARE(eSinOp, eIMathFunctionOp, eIMathFunctionOp::MATHFUNCTION_SIN, "Sin", 3, 1, 1, 's')
{
}
eOPERATOR_DECLARE_END();
eOPERATOR_IMPL_BEGIN(eSinOp) 
{
    const eMathFunction *input0 = ((eIMathFunctionOp *)getInputOperator(0))->getResult().mathFunc;
    eASSERT(input0 != eNULL);
    this->m_mathFunc = new eMathFunction_Sin(input0);
}
eOPERATOR_IMPL_END();

// Cosinus Operator
eOPERATOR_DECLARE(eCosOp, eIMathFunctionOp, eIMathFunctionOp::MATHFUNCTION_COS, "Cos", 3, 1, 1, 'c')
{
}
eOPERATOR_DECLARE_END();
eOPERATOR_IMPL_BEGIN(eCosOp) 
{
    const eMathFunction *input0 = ((eIMathFunctionOp *)getInputOperator(0))->getResult().mathFunc;
    eASSERT(input0 != eNULL);
    this->m_mathFunc = new eMathFunction_Cos(input0);
}
eOPERATOR_IMPL_END();


// Constant Operator
eOPERATOR_DECLARE(eConstOp, eIMathFunctionOp, eIMathFunctionOp::MATHFUNCTION_CONST, "Constant", 3, 0, 0, 'c')
{
    eOP_PARAM_ADD_FLOAT("Value", eF32_MIN, eF32_MAX, 1.0f);
}
eOPERATOR_DECLARE_END();
eOPERATOR_IMPL_BEGIN(eConstOp) 
{
    const eF32 &val1 = getParameter(0).getValue().flt;
    this->m_mathFunc = new eMathFunction_Const(&val1, 1);
}
eOPERATOR_IMPL_END();

// Multiply Operator
eOPERATOR_DECLARE(eMathMultiplyOp, eIMathFunctionOp, eIMathFunctionOp::MATH_FUNCTION_MULTIPLY, "Mul", 4, 2, 2, 'm')
{
}
eOPERATOR_DECLARE_END();
eOPERATOR_IMPL_BEGIN(eMathMultiplyOp) 
{
    const eMathFunction *input0 = ((eIMathFunctionOp *)getInputOperator(0))->getResult().mathFunc;
    const eMathFunction *input1 = ((eIMathFunctionOp *)getInputOperator(1))->getResult().mathFunc;
    eASSERT(input0 != eNULL);
    eASSERT(input1 != eNULL);
    this->m_mathFunc = new eMathFunction_Multiply(input0, input1);
}
eOPERATOR_IMPL_END();

// Add
eOPERATOR_DECLARE(eMathAddOp, eIMathFunctionOp, eIMathFunctionOp::MATH_FUNCTION_ADD, "Add", 6, 2, 2, 'a')
{
}
eOPERATOR_DECLARE_END();
eOPERATOR_IMPL_BEGIN(eMathAddOp) 
{
    const eMathFunction *input0 = ((eIMathFunctionOp *)getInputOperator(0))->getResult().mathFunc;
    const eMathFunction *input1 = ((eIMathFunctionOp *)getInputOperator(1))->getResult().mathFunc;
    eASSERT(input0 != eNULL);
    eASSERT(input1 != eNULL);
    this->m_mathFunc = new eMathFunction_Add(input0, input1);
}
eOPERATOR_IMPL_END();


// Var
eOPERATOR_DECLARE(eMathVarOp, eIMathFunctionOp, eIMathFunctionOp::MATH_FUNCTION_VAR, "X", 3, 0, 0, 'x')
{
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eMathVarOp) 
{
    this->m_mathFunc = new eMathFunction_Var(1);
}
eOPERATOR_IMPL_END();

// Random
eOPERATOR_DECLARE(eMathRandomOp, eIMathFunctionOp, eIMathFunctionOp::MATH_FUNCTION_RANDOM, "Random01", 3, 0, 0, 'r')
{
}
eOPERATOR_DECLARE_END();
eOPERATOR_IMPL_BEGIN(eMathRandomOp) 
{
    this->m_mathFunc = new eMathFunction_Random(1);
}
eOPERATOR_IMPL_END();


// Downsample Operator
eOPERATOR_DECLARE(eDownSampleOp, eIMathFunctionOp, eIMathFunctionOp::MATH_FUNCTION_DOWNSAMPLE, "DownSample", 6, 1, 1, 'd')
{
    eOP_PARAM_ADD_FLOAT("From", eF32_MIN, eF32_MAX, 0.0f);
    eOP_PARAM_ADD_FLOAT("To", eF32_MIN, eF32_MAX, 1.0f);
    eOP_PARAM_ADD_FLOAT("Granularity", 2.0f, eF32_MAX, 100.0f);
}
eOPERATOR_DECLARE_END();
eOPERATOR_IMPL_BEGIN(eDownSampleOp) 
{
    const eMathFunction *input0 = ((eIMathFunctionOp *)getInputOperator(0))->getResult().mathFunc;
    const eF32 &from = getParameter(0).getValue().flt;
    const eF32 &to = getParameter(1).getValue().flt;
    const eF32 &granularity = getParameter(2).getValue().flt;
    eASSERT(input0 != eNULL);
    this->m_mathFunc = new eMathFunction_DownSample(input0,from,to,eFtoL(granularity));
}
eOPERATOR_IMPL_END();

// Link
eOPERATOR_DECLARE(eMathLinkOp, eIMathFunctionOp, eIMathFunctionOp::MATH_FUNCTION_LINK, "Link", 3, 0, 0, 'l')
{
    eOP_PARAM_ADD_LINK("Link To", CATEGORY_MATHFUNCTIONOP);
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eMathLinkOp) 
{
    const eIMathFunctionOp *linkedOp = (eIMathFunctionOp *)eDemo::findOperator(getParameter(0).getValue().linkedOpId);
    const eMathFunction* linkedFunction = eNULL;
    if(linkedOp)
    {
        linkedFunction = linkedOp->getResult().mathFunc;
        this->m_mathFunc = new eMathFunction_Link(linkedFunction);
    }
}
eOPERATOR_IMPL_END();

// Sub
eOPERATOR_DECLARE(eMathSubOp, eIMathFunctionOp, eIMathFunctionOp::MATH_FUNCTION_SUB, "Sub", 6, 2, 2, 's')
{
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eMathSubOp) 
{
    const eMathFunction *input0 = ((eIMathFunctionOp *)getInputOperator(0))->getResult().mathFunc;
    const eMathFunction *input1 = ((eIMathFunctionOp *)getInputOperator(1))->getResult().mathFunc;
    eASSERT(input0 != eNULL);
    eASSERT(input1 != eNULL);
    this->m_mathFunc = new eMathFunction_Sub(input0, input1);
}
eOPERATOR_IMPL_END();

// Div
eOPERATOR_DECLARE(eMathDivOp, eIMathFunctionOp, eIMathFunctionOp::MATH_FUNCTION_DIV, "Div", 6, 2, 2, 'd')
{
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eMathDivOp) 
{
    const eMathFunction *input0 = ((eIMathFunctionOp *)getInputOperator(0))->getResult().mathFunc;
    const eMathFunction *input1 = ((eIMathFunctionOp *)getInputOperator(1))->getResult().mathFunc;
    eASSERT(input0 != eNULL);
    eASSERT(input1 != eNULL);
    this->m_mathFunc = new eMathFunction_Div(input0, input1);
}
eOPERATOR_IMPL_END();

// Attractor Operator
eOPERATOR_DECLARE(eAttractorOp, eIMathFunctionOp, eIMathFunctionOp::MATHFUNCTION_ATTRACTOR, "Attractor", 3, 1, 1, 'a')
{
//    eOP_PARAM_ADD_LINK("Position", CATEGORY_MATHFUNCTIONOP);
    eOP_PARAM_ADD_FLOAT("Mass", eF32_MIN, eF32_MAX, 1.0f);
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eAttractorOp) 
{
    const eMathFunction *source = ((eIMathFunctionOp *)getInputOperator(0))->getResult().mathFunc;
    eF32 mass = getParameter(0).getValue().flt * 1.0e10f;
    
    eMathFunction* constant = new eMathFunction_Const(&mass,1);
    this->m_mathFunc = new eMathFunction_Vectorize(source,constant);
}
eOPERATOR_IMPL_END();


// Constant3 Operator
eOPERATOR_DECLARE(eConst3Op, eIMathFunctionOp, eIMathFunctionOp::MATHFUNCTION_CONST3, "Constant3", 3, 0, 0, 'c')
{
    eOP_PARAM_ADD_FLOAT("X", eF32_MIN, eF32_MAX, 1.0f);
    eOP_PARAM_ADD_FLOAT("Y", eF32_MIN, eF32_MAX, 1.0f);
    eOP_PARAM_ADD_FLOAT("Z", eF32_MIN, eF32_MAX, 1.0f);
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eConst3Op) 
{
    eF32 val[3];
    val[0] = getParameter(0).getValue().flt;
    val[1] = getParameter(1).getValue().flt;
    val[2] = getParameter(2).getValue().flt;
    this->m_mathFunc = new eMathFunction_Const((eF32*)&val, 3);
}
eOPERATOR_IMPL_END();

// Vectorize
eOPERATOR_DECLARE(eMathVectorizeOp, eIMathFunctionOp, eIMathFunctionOp::MATHFUNCTION_VECTORIZE, "vectorize", 6, 1, 3, 'v')
{
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eMathVectorizeOp) 
{
    this->m_mathFunc = eNULL;
    for(eU32 i = 0; i < 3; i++)
    {
        const eIMathFunctionOp *linkedOp = ((eIMathFunctionOp *)getInputOperator(i));
        if(linkedOp != eNULL)
        {
            const eMathFunction *input = linkedOp->getResult().mathFunc;
            if(this->m_mathFunc == eNULL)
                this->m_mathFunc = new eMathFunction_Link(input);
            else            
                this->m_mathFunc = new eMathFunction_Vectorize(this->m_mathFunc, input);
        }
    };
}
eOPERATOR_IMPL_END();

// Mesh to Surface
eOPERATOR_DECLARE(eMathMeshToSurfaceOp, eIMathFunctionOp, eIMathFunctionOp::MATHFUNCTION_MESHTOSURFACE, "mesh2surf", 3, 0, 0, 'm')
{
    eOP_PARAM_ADD_LINK("Mesh", CATEGORY_MESHOP);
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eMathMeshToSurfaceOp) 
{
    const eID meshID = getParameter(0).getValue().linkedOpId;
    const eIMeshOp *meshOp = (eIMeshOp *)eDemo::findOperator(meshID);

    m_mathFunc = eNULL;
    if(meshOp != eNULL)
    {
        eMesh *mesh = new eMesh(gfx, eMesh::TYPE_DYNAMIC, meshOp->getResult().mesh);
        eASSERT(mesh != eNULL);
        m_mathFunc = new eMathFunction_MeshToSurface(mesh);
        eASSERT(m_mathFunc != eNULL);
    };
}
eOPERATOR_IMPL_END();

// Constant4 Operator
eOPERATOR_DECLARE(eConst4Op, eIMathFunctionOp, eIMathFunctionOp::MATHFUNCTION_CONST4, "Constant4", 4, 0, 0, 'c')
{
    eOP_PARAM_ADD_FLOAT("X", eF32_MIN, eF32_MAX, 1.0f);
    eOP_PARAM_ADD_FLOAT("Y", eF32_MIN, eF32_MAX, 1.0f);
    eOP_PARAM_ADD_FLOAT("Z", eF32_MIN, eF32_MAX, 1.0f);
    eOP_PARAM_ADD_FLOAT("A", eF32_MIN, eF32_MAX, 1.0f);
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eConst4Op) 
{
    eF32 val[4];
    val[0] = getParameter(0).getValue().flt;
    val[1] = getParameter(1).getValue().flt;
    val[2] = getParameter(2).getValue().flt;
    val[3] = getParameter(3).getValue().flt;
    this->m_mathFunc = new eMathFunction_Const((eF32*)&val, 4);
}
eOPERATOR_IMPL_END();


// Interpolate
/*
eOPERATOR_DECLARE(eMathInterpolateOp, eIMathFunctionOp, eIMathFunctionOp::MATHFUNCTION_INTERPOLATE, "interpolate", 6, 1, 3, 'v')
{
}
eOPERATOR_DECLARE_END();

eOPERATOR_IMPL_BEGIN(eMathInterpolateOp) 
{
    eArray<const eMathFunction*> functions = new eArray<const eMathFunction*>();
    for(eU32 i = 0; i < 6; i++)
    {
        const eIMathFunctionOp *linkedOp = ((eIMathFunctionOp *)getInputOperator(i));
        if(linkedOp != eNULL)
        {
            const eMathFunction *input = linkedOp->getResult().mathFunc;
            functions.append(input);
        }
    };
    this->m_mathFunc = new eMathFunction_Link(functions[0]);
    if(functions.size() > 1)
    {
        for(eU32 i = 1; i < functions.size(); i++)
            this->m_mathFunc = new eMathFunction_Add(this->m_mathFunc, functions[1]);
        eF32 mulBy = 1.0f / (eF32)functions.size();
        this->m_mathFunc = new eMathFunction_Multiply(this->m_mathFunc, new eMathFunction_Const(&mulBy,1));
    };    
}
eOPERATOR_IMPL_END();
*/