#ifndef TEXTUREH
#define TEXTUREH

#include "perlin.h"

class texture
{
public:
	virtual vec3 value(float u, float v, const vec3& p) const = 0;
};

class constant_texture : public texture
{
public:
	constant_texture() {}
	constant_texture(vec3 c) : color(c) {}
	virtual vec3 value(float u, float v, const vec3& p) const
	{
		return color;
	}

	vec3 color;
};

class checker_texture : public texture
{
public:
	checker_texture() {}
	checker_texture(texture* t0, texture* t1) : even(t0), odd(t1) {}
	virtual vec3 value(float u, float v, const vec3& p) const
	{
		float s = sin(10.0f*p.x())*sin(10.0f*p.y())*sin(10.0f*p.z());
		if (s < 0.0f)
			return odd->value(u, v, p);
		else
			return even->value(u, v, p);
	}

	texture* even;
	texture* odd;
};


class noise_texture : public texture
{
public:
	noise_texture() {}
	noise_texture(float sc) :scale(sc) {}
	virtual vec3 value(float u, float v, const vec3& p) const
	{
		//return vec3(1.0f, 1.0f, 1.0f)*noise.turb(scale*p);
		return vec3(1.0f, 1.0f, 1.0f)*0.5f*(1.0f + sin(scale*p.z() + 10.0f*noise.turb(p)));
	}
	perlin noise;
	float scale;
};

class image_texture : public texture
{
public:
	image_texture() {}
	image_texture(unsigned char* pixels, int A, int B) : data(pixels), nx(A), ny(B) {}
	virtual vec3 value(float u, float v, const vec3& p) const
	{
		int i = int(u*float(nx));
		int j = int((1.0f - v)*float(ny) - 0.001f);
		if (i < 0) i = 0;
		if (j < 0) j = 0;
		if (i > nx - 1) i = nx - 1;
		if (j > ny - 1) j = ny - 1;
		float _r = int(data[3 * i + 3 * nx*j]) / 255.0f;
		float _g = int(data[3 * i + 3 * nx*j+1]) / 255.0f;
		float _b = int(data[3 * i + 3 * nx*j+2]) / 255.0f;
		return vec3(_r, _g, _b);
	}

	unsigned char* data;
	int nx, ny;
};

#endif//TEXTUREH
