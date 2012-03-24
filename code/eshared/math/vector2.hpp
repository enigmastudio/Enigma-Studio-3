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

#ifndef VECTOR2_HPP
#define VECTOR2_HPP

class eFXY
{
public:
    union
    {
        struct
        {
            eF32    x;
            eF32    y;
        };

        struct
        {
            eF32    u;
            eF32    v;
        };
    };
};

// Two-dimensional vector class.
class eVector2 : public eFXY
{
public:
    eVector2()
    {
        null();
    }

    eVector2(const eFXY &fxy)
    {
        set(fxy.x, fxy.y);
    }

    // Sets x and y to value.
    eVector2(eF32 value)
    {
        set(value, value);
    }

    eVector2(eF32 nx, eF32 ny)
    {
        set(nx, ny);
    }

    void set(eF32 nx, eF32 ny)
    {
        x = nx;
        y = ny;
    }

    void null()
    {
        set(0.0f, 0.0f);
    }

	  ///////////////////////////////
	 //// Complex Operations ///////
	///////////////////////////////

	inline eF32 c_realPart() const {
		return x;
	}

	inline eF32 c_imaginaryPart() const {
		return y;
	}

	inline eVector2 c_conjugated() const {
		return eVector2(x,-y);
	}

	inline eVector2 c_conjugate() {
		y = -y;
	}

	inline eVector2 c_mul(const eVector2 &other) const {
		return eVector2(x*other.x  - y*other.y, y*other.x + x*other.y);
	}

	inline eVector2 c_div(const eVector2 &other) const {
		const eF32 div = other.x * other.x + other.y * other.y;
		return eVector2(x*other.x + y*other.y / div,
			            y*other.x - x*other.y / div);
	}

	inline eF32 c_abs() const {
		return eSqrt(x * x + y * y);
	}

	inline eF32 c_arg() const {
		return eATan2(y,x);
	}

	inline eVector2 c_ln() const {
		return eVector2(eLn(this->c_abs()), this->c_arg());
	}

	  ///////////////////////////////
	 //// Vector Operations ///////
	///////////////////////////////

    void negate()
    {
        set(-x, -y);
    }

    eF32 length() const
    {
        return eSqrt(sqrLength());
    }

    eF32 sqrLength() const
    {
        return x*x+y*y;
    }

    eF32 distance(const eVector2 &v) const
    {
        return ((*this)-v).length();
    }

    // Returns the distance between this vector and
    // the line defined by the two given vectors.
    eF32 distanceToLine(const eVector2 &v0, const eVector2 &v1) const
    {
        const eF32 u = ((x-v0.x)*(v1.x-v0.x)+(y-v0.y)*(v1.y-v0.y))/(v1-v0).sqrLength();
        const eVector2 pl = v0+u*(v1-v0);

        return distance(pl);
    }

    void normalize()
    {
        // Prevent danger of division by 0.
        if(eIsFloatZero(length()))
        {
            return;
        }

        const eF32 invLen = 1.0f/length();
        x *= invLen;
        y *= invLen;
    }

    eVector2 normalized() const
    {
        eVector2 n = *this;

        n.normalize();
        return n;
    }

    eVector2 random() const
    {
        return eVector2(x*eRandomF(-1.0f, 1.0f), y*eRandomF(-1.0f, 1.0f));
    }

    void absolute()
    {
        x = eAbs(x);
        y = eAbs(y);
    }

    void minimum(const eVector2 &v)
    {
        x = eMin(x, v.x);
        y = eMin(y, v.y);
    }

    void maximum(const eVector2 &v)
    {
        x = eMax(x, v.x);
        y = eMax(y, v.y);
    }

    void clamp(eF32 min, eF32 max)
    {
        x = eClamp(min, x, max);
        y = eClamp(min, y, max);
    }

    void scale(const eVector2 &v)
    {
        x *= v.x;
        y *= v.y;
    }

    // The angle is expected to be in radians.
    void rotate(eF32 angle)
    {
        eF32 s, c;

        eSinCos(angle, s, c);
        
        const eF32 nx = x*c-y*s;
        const eF32 ny = x*s+y*c;

        x = nx;
        y = ny;
    }

    // Rotates this vector around the given origin.
    // The angle is expected to be in radians.
    void rotate(const eVector2 &origin, eF32 angle)
    {
        *this -= origin;
        rotate(angle);
        *this += origin;
    }

    void translate(const eVector2 &v)
    {
        *this += v;
    }

    eBool equals(const eVector2 &v) const
    {
		return (v.x == this->x) && (v.y == this->y);
    }

    eVector2 midpoint(const eVector2 &v) const
    {
        return (*this+v)*0.5f;
    }

    // Linear interpolation (0 <= t <= 1).
    eVector2 lerp(const eVector2 &to, eF32 t) const
    {
        eASSERT(t >= 0.0f && t <= 1.0f);
        return eVector2(*this+(to-*this)*t);
    }

    // Returns wether or not this vector lies inside
    // the triangle specified by the points given points.
    eBool isInsideTriangle(const eVector2 &a, const eVector2 &b, const eVector2 &c) const
    {
        const eVector2 &p = *this;
        const eVector2 &v0 = p-c;
        const eVector2 &v1 = a-c;
        const eVector2 &v2 = b-c;

        const eF32 det = v1.x*v2.y-v1.y*v2.x;
        //eASSERT(eIsFloatZero(det) == eFALSE);
        const eF32 invDet = 1.0f/det;

        const eF32 u = (v0.x*v2.y-v0.y*v2.x)*invDet;
        const eF32 v = (v1.x*v0.y-v1.y*v0.x)*invDet;

        return (u >= 0.0f && v >= 0.0f && u+v <= 1.0f);
    }

	// returns the 2*area of then given 2d triangle, 
	// signum of returned value tells about the orientation
	static eF32 area2(const eVector2& v0, const eVector2& v1, const eVector2& v2) {
		return
			(v1.x - v0.x) * (v2.y - v0.y) -
			(v2.x - v0.x) * (v1.y - v0.y);
	}

    eVector2 operator + (const eVector2 &v) const
    {
        return eVector2(x+v.x, y+v.y);
    }

    eVector2 operator - (const eVector2 &v) const
    {
        return eVector2(x-v.x, y-v.y);
    }

    // Returns dot product.
    eF32 operator * (const eVector2 &v) const
    {
        return x*v.x+y*v.y;
    }

    // Scalar multiplication (scale).
    eVector2 operator * (eF32 s) const
    {
        return eVector2(x*s, y*s);
    }

    friend eVector2 operator * (eF32 s, const eVector2 &v)
    {
        return v*s;
    }

    friend eVector2 operator / (eF32 s, const eVector2 &v)
    {
        return eVector2(s/v.x,s/v.y);
    }

    eVector2 operator / (eF32 s) const
    {
        eASSERT(!eIsFloatZero(s));
        return *this*(1.0f/s);
    }

    eVector2 & operator += (const eVector2 &v)
    {
        *this = *this+v;
        return *this;
    }

    eVector2 & operator -= (const eVector2 &v)
    {
        *this = *this-v;
        return *this;
    }

    eVector2 & operator *= (eF32 s)
    {
        *this = *this*s;
        return *this;
    }

    eVector2 & operator /= (eF32 s)
    {
        *this = *this/s;
        return *this;
    }

    eVector2 operator - () const
    {
        return eVector2(-x, -y);
    }

    operator const eF32 * () const
    {
        return (eF32 *)this;
    }

    const eF32 & operator [] (eInt index) const
    {
        eASSERT(index < 2);
        return ((eF32 *)this)[index];
    }

    eF32 & operator [] (eInt index)
    {
        eASSERT(index < 2);
        return ((eF32 *)this)[index];
    }

public:
    static const eVector2 XAXIS;
    static const eVector2 YAXIS;
    static const eVector2 ORIGIN;

};

typedef eArray<eVector2> eVector2Array;

#endif // VECTOR2_HPP