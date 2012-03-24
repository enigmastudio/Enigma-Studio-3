#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctime>

#include <arm_neon.h>

const int SIG_LEN = 88200;
const int FACTOR = 40;

static clock_t time_start;
static clock_t time_end;

inline float32x2_t loadf2(float32_t v1, float32_t v2)
{
    const float32_t a[] = { v1, v2 };
    return vld1_f32(a);
}

inline float32x2_t loadf2s(float32_t v)
{
    const float32_t a[] = { v, v };
    return vld1_f32(a);
}

void start()
{
    time_start = clock();
}

void end(const char *msg)
{
    time_end = clock();
    float secs = (float)(time_end - time_start) / CLOCKS_PER_SEC;
    float secs_real = secs / FACTOR;
    printf("%s\tSeconds: %f, Passes per sec: %f\n", msg, secs_real, 1.0f / secs_real);
}

void bm_mul_f(float *sig)
{
    int len = SIG_LEN;
    
    while (len--)
    {
	*sig++ *= 2.45f;
    }
}

void bm_mul_i(long *sig)
{
    int len = SIG_LEN;
    
    while (len--)
    {
	*sig++ *= 56;
    }
}

void bm_mul_neon2_f(float *sig)
{
    int len = SIG_LEN/2;
    
    float32x2_t f = loadf2(2.45f, 2.45f);
        
    while (len--)
    {
	float32x2_t v = vld1_f32((const float32_t*)sig);
	v = vmul_f32(v, f);
	vst1_f32((float32_t*)sig, v);
	sig+=2;
    }
}


void bm_iir_f(float *sig)
{
    int len = SIG_LEN;
    
    float y1 = 6.0f;
    float y2 = 7.0f;
    float y3 = 8.0f;
    float y4 = 9.0f;
    
    float old_x = 2.0f;
    float old_y1 = 3.0f;
    float old_y2 = 4.0f;
    float old_y3 = 5.0f;
    
    float r = 6.320f;
    float p = 4.240f;
    float k = 1.243f;
    
    while (len--)
    {
	float in = (float)*sig;
        float out;
    
        float x = in - r * y4;

        //Four cascaded onepole filters (bilinear transform)
        y1 = x  * p + old_x  * p - k * y1;
        y2 = y1 * p + old_y1 * p - k * y2;
        y3 = y2 * p + old_y2 * p - k * y3;
        y4 = y3 * p + old_y3 * p - k * y4;

        //Clipper band limited sigmoid
        out = y4 - (y4 * y4 * y4) / 6.0f;

        old_x = x;
        old_y1 = y1;
        old_y2 = y2;
        old_y3 = y3;	
        
        *sig++ = out;
    }
}

void bm_iir_neon2_f(float *sig)
{
    int len = SIG_LEN/2;
    
    float32x2_t y1 = loadf2s(6.0f);
    float32x2_t y2 = loadf2s(7.0f);
    float32x2_t y3 = loadf2s(8.0f);
    float32x2_t y4 = loadf2s(9.0f);
    
    float32x2_t old_x = loadf2s(2.0f);
    float32x2_t old_y1 = loadf2s(3.0f);
    float32x2_t old_y2 = loadf2s(4.0f);
    float32x2_t old_y3 = loadf2s(5.0f);
    
    float32x2_t r = loadf2s(6.320f);
    float32x2_t p = loadf2s(4.240f);
    float32x2_t k = loadf2s(1.243f);
    
    float32x2_t const_6 = loadf2s(1.0f / 6.0f);
    
    while (len--)
    {
	float32x2_t in = vld1_f32((const float32_t*)sig);
	    
        float32x2_t x = vmul_f32(vsub_f32(in, r), y4);

        //Four cascaded onepole filters (bilinear transform)
        y1 = vsub_f32(vadd_f32(vmul_f32(x, p), vmul_f32(old_x, p)), vmul_f32(k, y1));
        y2 = vsub_f32(vadd_f32(vmul_f32(y1, p), vmul_f32(old_y1, p)), vmul_f32(k, y2));
        y3 = vsub_f32(vadd_f32(vmul_f32(y2, p), vmul_f32(old_y2, p)), vmul_f32(k, y3));
        y4 = vsub_f32(vadd_f32(vmul_f32(y3, p), vmul_f32(old_y3, p)), vmul_f32(k, y4));

        //Clipper band limited sigmoid
        float32x2_t out = vsub_f32(y4, vmul_f32(vmul_f32(vmul_f32(y4, y4), y4), const_6));

        old_x = x;
        old_y1 = y1;
        old_y2 = y2;
        old_y3 = y3;	
        
        vst1_f32((float32_t*)sig, out);
	sig+=2;
    }
}

void bm_iir_i(long *sig)
{
    int len = SIG_LEN;
    
    long y1 = 6234;
    long y2 = 3576;
    long y3 = 1245;
    long y4 = 3467;
    
    long old_x = 2356;
    long old_y1 = 2356;
    long old_y2 = 1245;
    long old_y3 = 3477;
    
    long r = 12545;
    long p = 12;
    long k = 235;
    
    while (len--)
    {
	long in = *sig;
        long out;
    
        long x = in - r * y4;

        //Four cascaded onepole filters (bilinear transform)
        y1 = x  * p + old_x  * p - k * y1;
        y2 = y1 * p + old_y1 * p - k * y2;
        y3 = y2 * p + old_y2 * p - k * y3;
        y4 = y3 * p + old_y3 * p - k * y4;

        //Clipper band limited sigmoid
        out = y4 - (y4 * y4 * y4) / 6000;

        old_x = x;
        old_y1 = y1;
        old_y2 = y2;
        old_y3 = y3;	
        
        *sig++ = out;
    }
}


int main(int argc, char **argv)
{
    float *sigf = (float*)malloc(sizeof(float) * SIG_LEN);
    long *sigi = (long*)malloc(sizeof(long) * SIG_LEN);

    clock_t time_start;
    clock_t time_end;
    
    // #######################################################
    start();
    for(int i=0; i<FACTOR; i++)
    {
	bm_mul_f(sigf);
    }
    end("Single float mul :");
    
    start();    
    for(int i=0; i<FACTOR; i++)
    {
	bm_mul_i(sigi);
    }
    end("Single int mul   :");
    
    start();
    for(int i=0; i<FACTOR; i++)
    {
	bm_mul_neon2_f(sigf);
    }
    end("SIMDx2 float mul :");
    
    start();
    for(int i=0; i<FACTOR; i++)
    {
	bm_iir_f(sigf);
    }
    end("float IIR        :");
    
    start();
    for(int i=0; i<FACTOR; i++)
    {
	bm_iir_i(sigi);
    }
    end("int IIR          :");
    
    start();
    for(int i=0; i<FACTOR; i++)
    {
	bm_iir_neon2_f(sigf);
    }
    end("SIMDx2 float IIR :");

    free(sigf);
    free(sigi);
    
    return 0;
}