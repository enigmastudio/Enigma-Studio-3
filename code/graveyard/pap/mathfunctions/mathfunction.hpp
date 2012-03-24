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

#ifndef MATH_FUNCTION_HPP
#define MATH_FUNCTION_HPP

class eMathFunction
{
private:

public:

    eMathFunction();
    virtual ~eMathFunction();
    virtual eBool evaluateAt(const eF32* inValue, eF32* outValue, eU32 multiplicity, eU32 dimInput, eU32 stride) const = 0;
    const eU32&    dimOutput;    
protected:
    eU32    m_dimOutput;    // default = 1 by parent constructor
};

// --------------------------------------------------------------------
// ---------------------------MAKROS-----------------------------------
// --------------------------------------------------------------------
#define eMATHFUNCTION_CONSTRUCT(eClass)                                                               \
    class eClass : public eMathFunction\
    {\
    public:\
        eClass \

#define eMATHFUNCTION_DESTRUCT(eClass)                                                               \
    ~eClass()

#define eMATHFUNCTION_RUN(eClass)                                                               \
    virtual eBool evaluateAt(const eF32* inValue, eF32* outValue, eU32 multiplicity , eU32 dimInput, eU32 stride) const\
    {\
        for(eU32 _i = 0; _i < multiplicity; _i++, inValue += (dimInput+stride), outValue += dimOutput)


#define eMATHFUNCTION_RUN_FOR_EACH_DIMENSION(eClass)                                                               \
    virtual eBool evaluateAt(const eF32* inValue, eF32* outValue, eU32 multiplicity , eU32 dimInput, eU32 stride) const\
    {\
        for(eU32 _i = 0; _i < multiplicity; _i++, inValue += stride)\
            for(eU32 _dim = 0; _dim < dimOutput; _dim++, inValue++, outValue++)

#define eMATHFUNCTION_END                                                               \
        return eTRUE;\
    }\
};
// --------------------------------------------------------------------
// --------------------------------------------------------------------

// --------------------------------------------------------------------
// Add function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Add)     (const eMathFunction *fnc1, const eMathFunction *fnc2)  : m_fnc1(fnc1), m_fnc2(fnc2) {this->m_dimOutput = fnc1->dimOutput; this->m_buffer = new eF32[fnc2->dimOutput];}
    const eMathFunction* m_fnc1,*m_fnc2;
    eF32*    m_buffer;
eMATHFUNCTION_DESTRUCT(eMathFunction_Add)        {delete [] this->m_buffer;};

eMATHFUNCTION_RUN(eMathFunction_Add)
    {    
        this->m_fnc1->evaluateAt(inValue,outValue,1,dimInput,stride);
        this->m_fnc2->evaluateAt(inValue,this->m_buffer,1,dimInput,stride);
        for(eU32 dim = 0; dim < this->dimOutput; dim++)
            *(outValue+dim) += this->m_buffer[dim % m_fnc2->dimOutput];
    }
eMATHFUNCTION_END

// --------------------------------------------------------------------
// Subtract function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Sub)    (const eMathFunction *fnc1, const eMathFunction *fnc2)  : m_fnc1(fnc1), m_fnc2(fnc2) {this->m_dimOutput = fnc1->dimOutput; this->m_buffer = new eF32[fnc2->dimOutput];}
    const eMathFunction* m_fnc1,*m_fnc2;
    eF32*    m_buffer;
eMATHFUNCTION_DESTRUCT(eMathFunction_Sub)        {delete [] this->m_buffer;};

eMATHFUNCTION_RUN(eMathFunction_Sub)
    {    
        this->m_fnc1->evaluateAt(inValue,outValue,1,dimInput,stride);
        this->m_fnc2->evaluateAt(inValue,this->m_buffer,1,dimInput,stride);
        for(eU32 dim = 0; dim < this->dimOutput; dim++)
            *(outValue+dim) -= this->m_buffer[dim % m_fnc2->dimOutput];
    }
eMATHFUNCTION_END

// --------------------------------------------------------------------
// Multiply function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Multiply)     (const eMathFunction *fnc1, const eMathFunction *fnc2) : m_fnc1(fnc1), m_fnc2(fnc2) { this->m_dimOutput = fnc1->dimOutput; this->m_buffer = new eF32[fnc2->dimOutput];}
    const eMathFunction *m_fnc1, *m_fnc2;
    eF32*    m_buffer;
eMATHFUNCTION_DESTRUCT(eMathFunction_Multiply)        {delete [] this->m_buffer;};

eMATHFUNCTION_RUN(eMathFunction_Multiply)
    {    
        this->m_fnc1->evaluateAt(inValue,outValue,1,dimInput,stride);
        this->m_fnc2->evaluateAt(inValue,this->m_buffer,1,dimInput,stride);
        for(eU32 dim = 0; dim < this->dimOutput; dim++)
            *(outValue+dim) *= this->m_buffer[dim % m_fnc2->dimOutput];
    }
eMATHFUNCTION_END

// --------------------------------------------------------------------
// Divide function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Div)    (const eMathFunction *fnc1, const eMathFunction *fnc2)  : m_fnc1(fnc1), m_fnc2(fnc2) { this->m_dimOutput = fnc1->dimOutput; this->m_buffer = new eF32[fnc2->dimOutput];}
    const eMathFunction* m_fnc1,*m_fnc2;
    eF32*    m_buffer;
eMATHFUNCTION_DESTRUCT(eMathFunction_Div)        {delete [] this->m_buffer;};

eMATHFUNCTION_RUN(eMathFunction_Div)
    {    
        this->m_fnc1->evaluateAt(inValue,outValue,1,dimInput,stride);
        this->m_fnc2->evaluateAt(inValue,this->m_buffer,1,dimInput,stride);
        for(eU32 dim = 0; dim < this->dimOutput; dim++)
            *(outValue+dim) /= this->m_buffer[dim % m_fnc2->dimOutput];
    }
eMATHFUNCTION_END

// --------------------------------------------------------------------
// Variable Function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Var)     (eU32 dimensions) { m_dimOutput = dimensions;}
eMATHFUNCTION_RUN_FOR_EACH_DIMENSION(eMathFunction_Var)
    *outValue = *inValue;
eMATHFUNCTION_END

// --------------------------------------------------------------------
// Const Function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Const) (const eF32* value, eU32 count) 
    {    
        m_dimOutput = count; 
        this->m_value = new eF32[count];
        eMemCopy(this->m_value, value, count * sizeof(eF32));        
    }
    eF32*    m_value;

eMATHFUNCTION_DESTRUCT(eMathFunction_Const)
    {    delete [] this->m_value;    };

eMATHFUNCTION_RUN_FOR_EACH_DIMENSION(eMathFunction_Const)
    *outValue = this->m_value[_dim];

eMATHFUNCTION_END

// --------------------------------------------------------------------
// Vectorize function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Vectorize)        (const eMathFunction *fnc1, const eMathFunction *fnc2) : m_fnc1(fnc1), m_fnc2(fnc2)
    { 
        m_dimOutput = fnc1->dimOutput + fnc2->dimOutput; 
    }
    const eMathFunction* m_fnc1, *m_fnc2;

eMATHFUNCTION_RUN(eMathFunction_Vectorize)
{
    this->m_fnc1->evaluateAt(inValue,outValue,1,dimInput,stride);
    this->m_fnc2->evaluateAt(inValue,outValue + m_fnc1->dimOutput,1,dimInput,stride);
}

eMATHFUNCTION_END

// --------------------------------------------------------------------
// Classic Sinus function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Sin)     (const eMathFunction *fnc) : m_fnc(fnc)    { m_dimOutput = fnc->dimOutput; }
    const eMathFunction* m_fnc;

eMATHFUNCTION_RUN_FOR_EACH_DIMENSION(eMathFunction_Sin)
    {    
        eF32 v;
        this->m_fnc->evaluateAt(inValue,&v,1,dimInput,stride);
        *outValue = eSin(v * ePI);
    }
eMATHFUNCTION_END


// --------------------------------------------------------------------
// Classic Cosinus function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Cos)     (const eMathFunction *fnc) : m_fnc(fnc) {m_dimOutput = fnc->dimOutput;}
    const eMathFunction* m_fnc;

eMATHFUNCTION_RUN_FOR_EACH_DIMENSION(eMathFunction_Cos)
    {    
        eF32 v;
        this->m_fnc->evaluateAt(inValue,&v,1,dimInput,stride);
        *outValue = eCos(v * ePI);
    }
eMATHFUNCTION_END

// --------------------------------------------------------------------
// Random Function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Random)    (eU32 dimensions) { m_dimOutput = dimensions;}
eMATHFUNCTION_RUN_FOR_EACH_DIMENSION(eMathFunction_Var)
    *outValue = eRandomF();    
eMATHFUNCTION_END

// --------------------------------------------------------------------
// DownSample function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_DownSample)
    (const eMathFunction *fnc, eF32 from, eF32 to, eU32 granularity) 
    { 
        m_from = from;
        m_to = to;
        m_granularity = granularity;
        m_granularityInverse = 1.0f / (eF32)m_granularity;
        m_dimOutput = fnc->dimOutput; 
        this->m_samples = new eF32[this->dimOutput * granularity];
        eF32 pos = from;
        m_step = 1.0;
        if(granularity > 1)
            m_step = (to - from) / (eF32)(granularity - 1);
        for(eU32 i = 0; i < granularity; i++)
        {
            fnc->evaluateAt(&pos,this->m_samples + (i * dimOutput),1,1,0);
            pos += m_step;
        }
    }
    eF32* m_samples;
    eF32 m_from;
    eF32 m_to;
    eU32 m_granularity;
    eF32 m_granularityInverse;
    eF32 m_step;

eMATHFUNCTION_DESTRUCT(eMathFunction_DownSample)
    {    delete [] this->m_samples;    }

eMATHFUNCTION_RUN(eMathFunction_DownSample)
    {    
/*
		eU32 leftPos =1;
		if((*inValue) > 3.0f)
			leftPos = 2;
*/
        eU32 leftPos = eFtoL(((*inValue) - this->m_from) / m_step);
        if(leftPos < 0)
            leftPos = 0;
        if(leftPos >= this->m_granularity)
            leftPos = this->m_granularity - 1;
        eF32 mix = (((*inValue) - (eF32)leftPos * m_step)) / m_step;
		if(mix < 0.0f)
			mix = 0.0f;
		if(mix > 1.0f)
			mix = 1.0f;
        eU32 rightPos = leftPos + 1;
        if(rightPos >= this->m_granularity)
            rightPos = this->m_granularity - 1;
        for(eU32 dim = 0; dim < dimOutput; dim++)
            *(outValue + dim) = this->m_samples[leftPos * dimOutput + dim] * (1.0f - mix) + this->m_samples[rightPos * dimOutput + dim] * mix;
    }
eMATHFUNCTION_END

// --------------------------------------------------------------------
// Link function
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_Link)        (const eMathFunction *fnc) : m_fnc(fnc)
    { 
        m_dimOutput = fnc->dimOutput; 
    }
    const eMathFunction* m_fnc;

eMATHFUNCTION_RUN(eMathFunction_Link)
    return this->m_fnc->evaluateAt(inValue,outValue,multiplicity,dimInput,stride);

eMATHFUNCTION_END

// --------------------------------------------------------------------
// MeshToSurface
// --------------------------------------------------------------------
eMATHFUNCTION_CONSTRUCT(eMathFunction_MeshToSurface)        (const eMesh *mesh) : m_mesh(mesh)
    { 
        m_dimOutput = 6; 
        this->m_sumArea = 0.0f;
        this->m_areas = new double[this->m_mesh->getTriangleCount()];
        for(eU32 i = 0; i < this->m_mesh->getTriangleCount(); i++)
        {
            // calculate Area

            const eVector3& P = this->m_mesh->getVertex(this->m_mesh->getTriangle(i).indices[0]).position;
            const eVector3& Q = this->m_mesh->getVertex(this->m_mesh->getTriangle(i).indices[1]).position;
            const eVector3& X = this->m_mesh->getVertex(this->m_mesh->getTriangle(i).indices[2]).position;
            eVector3 u = X - P;
            eF32 ul = u.length();
            eF32 t = ((Q - P) * u) / (ul * ul);
            eVector3 S = P + u * t;
            eF32 h = (S - Q).length();

            eF32 area = h * ul * 0.5f;

/*
            const eVector3& a = this->m_mesh->getVertex(this->m_mesh->getTriangle(i).indices[0]).position;
            const eVector3& b = this->m_mesh->getVertex(this->m_mesh->getTriangle(i).indices[1]).position;
            const eVector3& c = this->m_mesh->getVertex(this->m_mesh->getTriangle(i).indices[2]).position;
            eVector3    p = (a+b+c) * 0.5;
            double    area  = eSqrt(p*(p-a)*(p-b)*(p-c));
*/
            this->m_areas[i] = area;
            this->m_sumArea += area;
        }
    }
    const eMesh*    m_mesh;
    double*            m_areas;
    double            m_sumArea;

eMATHFUNCTION_DESTRUCT(eMathFunction_MeshToSurface)
    {    delete [] this->m_areas;    }

eMATHFUNCTION_RUN(eMathFunction_MeshToSurface)
{
    double x = *(inValue);
//    eF32 y = *(invalue+1);
//    eU32 triangle = (eU32)(x * (eF32)this->m_mesh->getTriangleCount());    
    double dstTriangleArea = (x * (double)this->m_sumArea);    
    if(dstTriangleArea < 0)
        dstTriangleArea = 0;
    if(dstTriangleArea > this->m_sumArea)
        dstTriangleArea = this->m_sumArea;
    eU32 triangle = 0;
    while((triangle < this->m_mesh->getTriangleCount() - 1) && (dstTriangleArea >= this->m_areas[triangle]))
    {
        dstTriangleArea -= this->m_areas[triangle];
        triangle++;
    }

    eF32 w0 = eRandomF();
    eF32 w1 = eRandomF();
    if(w0 + w1 > 1.0f)
    {
        w0 = 1.0f - w0;
        w1 = 1.0f - w1;
    }

    eF32 w2 = 1.0f - w0 - w1;

    const eMesh::Triangle& tri = this->m_mesh->getTriangle(triangle);
    eVector3 pos0 = this->m_mesh->getVertex(tri.indices[0]).position;
    eVector3 pos1 = this->m_mesh->getVertex(tri.indices[1]).position;
    eVector3 pos2 = this->m_mesh->getVertex(tri.indices[2]).position;
    eVector3 norm0 = this->m_mesh->getVertex(tri.indices[0]).normal;
    eVector3 norm1 = this->m_mesh->getVertex(tri.indices[1]).normal;
    eVector3 norm2 = this->m_mesh->getVertex(tri.indices[2]).normal;

    eVector3 res = ((pos0 * w0) + (pos1 * w1) + (pos2 * w2));
    eVector3 norm = ((norm0 * w0) + (norm1 * w1) + (norm2 * w2)) ;
    norm.normalize();
    *(outValue) = res.x;
    *(outValue+1) = res.y;
    *(outValue+2) = res.z;
    *(outValue+3) = norm.x;
    *(outValue+4) = norm.y;
    *(outValue+5) = norm.z;
}

eMATHFUNCTION_END


#endif // MATH_FUNCTION_HPP