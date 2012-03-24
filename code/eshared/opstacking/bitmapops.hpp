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

#ifndef BITMAP_OPS_HPP
#define BITMAP_OPS_HPP

// Base class for bitmap operators.
class eIBitmapOp : public eIOperator
{
public:
    struct Result : public eIOperator::Result
    {
        Result(eU32 &w, eU32 &h, eU32 &s, eColor *&bmp) :
            width(w),
            height(h),
            size(s),
            bitmap(bmp)
        {
        }

        eU32 &              width;
        eU32 &              height;
        eU32 &              size;
        eColor *&           bitmap;
    };

public:
    eIBitmapOp() :
        m_result(m_bmpDimSize[0], m_bmpDimSize[1], m_bmpSize, m_bitmap),
        m_bitmap(eNULL),
        m_bmpSize(0)
    {
	    m_bmpDimSize[0] = 0;
	    m_bmpDimSize[1] = 0;
    }

#ifdef eEDITOR
    virtual ~eIBitmapOp()
    {
        eSAFE_DELETE_ARRAY(m_bitmap);
    }
#endif

    virtual const Result &  getResult() const
    {
        return m_result;
    }

    virtual Result & getResult()
    {
        return m_result;
    }

private:
    virtual void _preExecute(eGraphicsApiDx9 *gfx)
    {
        // If there's at least one input operator, set
        // our self to the size of this input operator
        // (all input operators have the same bitmap
        // size, so it doesn't matter which one).
        // Further more, bitmap operators only allow
        // bitmap operators as input, so it's impossible
        // to get an operator of another type as input.
        
        if (getInputCount() > 0)
        {
            eIBitmapOp *op = (eIBitmapOp *)getInputOperator(0);
            eASSERT(op != eNULL);
            eASSERT(TEST_CATEGORY(op, "Bitmap", Bitmap_CID));

            _reallocate(op->getResult().width, op->getResult().height);
        }
    }

protected:
    void _reallocate(eU32 newWidth, eU32 newHeight)
    {
        eASSERT(eIsPowerOf2(newWidth) != eFALSE);
        eASSERT(eIsPowerOf2(newHeight) != eFALSE);

        if (m_bmpDimSize[0] != newWidth || m_bmpDimSize[1] != newHeight)
        {
            eSAFE_DELETE_ARRAY(m_bitmap);
            m_bitmap = new eColor[newWidth*newHeight];
            eASSERT(m_bitmap != eNULL);

            m_bmpDimSize[0] = newWidth;
            m_bmpDimSize[1] = newHeight;
            m_bmpSize = newWidth*newHeight;
        }
    }

    void _copyFirstInputBitmap()
    {
        if (getInputCount() != 0)
        {
            const Result &res = ((eIBitmapOp *)getInputOperator(0))->getResult();
            eMemCopy(m_bitmap, res.bitmap, res.size*sizeof(eColor));
        }
    }

#ifdef eEDITOR
    virtual eBool checkValidity() const
    {
        // Get all input operators of type bitmap.
        eArray<eIBitmapOp *> bmpInputs;

        for (eU32 i=0; i<getInputCount(); i++)
        {
            const eIOperator *op = getInputOperator(i);
            eASSERT(op != eNULL);

            if (op->getCategory() == "Bitmap")
            {
                bmpInputs.append((eIBitmapOp *)op);
            }
        }

        // All input operators must have same size.
        // Check if all input operators of type
        // bitmap have pair-wise same bitmap size.
        for (eU32 i=1; i<bmpInputs.size(); i++)
        {
            if (bmpInputs[i-1]->getResult().width != bmpInputs[i]->getResult().width ||
                bmpInputs[i-1]->getResult().height != bmpInputs[i]->getResult().height)
            {
                return eFALSE;
            }
        }

        return eIOperator::checkValidity();
    }
#endif

protected:


protected:
    static const eU32   DEFAULT_SIZE = 256;
    static const eU32   DEFSIZE_SEL = 8;
	static const eU32	COLOR_GRADING_SIZE = 16;

protected:
    eU32        m_bmpDimSize[2];
    eU32        m_bmpSize;
    eColor *    m_bitmap;

    Result      m_result;
};

#endif // BITMAP_OPS_HPP