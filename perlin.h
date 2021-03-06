#ifndef PERLINH
#define PERLINH

#include "vec3.h"

inline float trilinear_interp(float c[2][2][2], float u, float v, float w)
{
	float accum = 0;
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 0; k < 2; ++k)
				accum += (float(i)*u + float(1 - i)*(1.0f - u))*
						 (float(j)*v + float(1 - j)*(1.0f - v))*
						 (float(k)*w + float(1 - k)*(1.0f - w))*c[i][j][k];
	return accum;
}

inline float perlin_interp(vec3 c[2][2][2], float u, float v, float w)
{
	float uu = u*u*(3.0f - 2.0f*u);
	float vv = v*v*(3.0f - 2.0f*v);
	float ww = w*w*(3.0f - 2.0f*w);
	float accum = 0.0f;
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 0; k < 2; ++k)
			{
				vec3 weight_v(u - i, v - j, w - k);
				accum += (i*uu + (1 - i)*(1 - uu))*
						 (j*vv + (1 - j)*(1 - vv))*
						 (k*ww + (1 - k)*(1 - ww))*dot(c[i][j][k], weight_v);
			}
	return accum;
}

class perlin
{
public:
    ~perlin()
    {
        if (ranvec)
        {
            delete[] ranvec;
            ranvec = nullptr;
        }
        if (perm_x)
        {
            delete[] perm_x;
            ranvec = nullptr;
        }
        if (perm_y)
        {
            delete[] perm_y;
            perm_y = nullptr;
        }
        if (perm_z)
        {
            delete[] perm_z;
            perm_z = nullptr;
        }
    }
	float noise(const vec3& p) const
	{
		float u = p.x() - floor(p.x());
		float v = p.y() - floor(p.y());
		float w = p.z() - floor(p.z());
		u = u*u*(3.0f - 2.0f*u);
		v = v*v*(3.0f - 2.0f*v);
		w = w*w*(3.0f - 2.0f*w);
		int i = (int)floor(p.x());
		int j = (int)floor(p.y());
		int k = (int)floor(p.z());
		vec3 c[2][2][2];
		for (int di = 0; di < 2; ++di)
			for (int dj = 0; dj < 2; ++dj)
				for (int dk = 0; dk < 2; ++dk)
					c[di][dj][dk] = ranvec[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];
		return perlin_interp(c, u, v, w);
	}
	float turb(const vec3& p, int depth = 7) const
	{
		float accum = 0;
		vec3 temp_p = p;
		float weight = 1.0f;
		for (int i = 0; i < depth; i++)
		{
			accum += weight*noise(temp_p);
			weight *= 0.5f;
			temp_p *= 2;
		}
		return fabs(accum);
	}
	static vec3* ranvec;
	static int* perm_x;
	static int* perm_y;
	static int* perm_z;
};

static vec3* perlin_generate()
{
	vec3* p = new vec3[256];
	for (int i = 0; i < 256; i++)
		p[i] = unit_vector(vec3(-1.0f + 2.0f*rand48(), -1.0f + 2.0f*rand48(), -1.0f + 2.0f*rand48()));
	return p;
}

void permute(int* p, int n)
{
	for (int i = n - 1; i > 0; i--)
	{
		int target = int(rand48()*float(i + 1));
		int tmp = p[i];
		p[i] = p[target];
		p[target] = tmp;
	}
}

static int* perlin_generate_perm()
{
	int* p = new int[256];
	for (int i = 0; i < 256; ++i)
		p[i] = i;
	permute(p, 256);
	return p;
}

vec3* perlin::ranvec = perlin_generate();
int* perlin::perm_x = perlin_generate_perm();
int* perlin::perm_y = perlin_generate_perm();
int* perlin::perm_z = perlin_generate_perm();

#endif//PERLINH
