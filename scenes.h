#ifndef SCENESH
#define SCENESH

#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include <iostream>

vec3 color_emit_light(const ray& r, hitable* world, int depth)
{
	hit_record rec;
	if (world->hit(r, 0.001f, FLT_MAX, rec))
	{
		ray scattered;
		vec3 attenuation;
		vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return emitted + attenuation*color_emit_light(scattered, world, depth + 1);
		//else if (depth == 0 && (emitted.r() || emitted.g() || emitted.b()))
		//{
		//	emitted.make_unit_vector();
		//	return emitted;
		//}
		else
			return emitted;
    }
	else
	{
		return vec3(0, 0, 0);
	}
}

vec3 color_ambient(const ray& r, hitable* world, int depth)
{
    hit_record rec;
	if (world->hit(r, 0.001f, FLT_MAX, rec))
	{
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation*color_ambient(scattered, world, depth + 1);
		else
			return vec3(0, 0, 0);
	}
	else
	{
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5f * (unit_direction.y() + 1.0f);
		return ((1.0f - t) * vec3(1, 1, 1)) + (t*vec3(0.5f, 0.7f, 1.0f));
	}
}

void random_scene(hitable** ret)
{
	int n = 500;
	hitable** list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new lambertian(new checker_texture(new constant_texture(vec3(0.2f, 0.3f, 0.1f)), new constant_texture(vec3(0.9f, 0.9f, 0.9f)))));
	int i = 1;

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			float choose_mat = rand48();
			vec3 center(float(a) + 0.9f*rand48(), 0.2f, float(b) + 0.9f*rand48());

			if ((center - vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f)
			{
				if (choose_mat < 0.8f) // diffuse
				{
					list[i++] = new moving_sphere(center, center+vec3(0.0f, 0.5f*rand48(), 0.0f),0.0f, 1.0f, 0.2f, new lambertian(new constant_texture(vec3(rand48()*rand48(), rand48()*rand48(), rand48()*rand48()))));
				}
				else if (choose_mat < 0.95f) // metal
				{
					list[i++] = new sphere(center, 0.2f, new metal(vec3(0.5f*(1.0f + rand48()), 0.5f*(1.0f + rand48()), 0.5f*(1.0f + rand48()))));
				}
				else // glass
				{
					list[i++] = new sphere(center, 0.2f, new dielectric(1.5f));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0.0f, 1.0f, 0.0f), 1.0f, new dielectric(1.5f));
	list[i++] = new sphere(vec3(-4.0f, 1.0f, 0.0f), 1.0f, new lambertian(new constant_texture(vec3(0.4f, 0.2f, 0.1f))));
	list[i++] = new sphere(vec3(4.0f, 1.0f, 0.0f), 1.0f, new metal(vec3(0.7f, 0.6f, 0.5f)));

	//*ret = new hitable_list(list, i);
	*ret = new bvh_node(list, i, 0.0f, 1.0f);
}

void two_spheres(hitable** ret)
{
	texture* checker = new checker_texture(new constant_texture(vec3(0.2f, 0.3f, 0.1f)), new constant_texture(vec3(0.9f, 0.9f, 0.9f)));
	//int n = 50;
	hitable** list = new hitable*[2];
	list[0] = new sphere(vec3(0.0f, -10.0f, 0.0f), 10.0f, new lambertian(checker));
	list[1] = new sphere(vec3(0.0f, 10.0f, 0.0f), 10.0f, new lambertian(checker));

	*ret = new hitable_list(list, 2);
}

void two_perlin_spheres(hitable** ret)
{
	texture* pertext = new noise_texture(4.0f);
	hitable** list = new hitable*[2];
	list[0] = new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new lambertian(pertext));
	list[1] = new sphere(vec3(0.0f, 2.0f, 0.0f), 2.0f, new lambertian(pertext));

	*ret = new hitable_list(list, 2);
}

void earth(hitable** ret)
{
	int nx, ny, nn;
	unsigned char* tex_data = stbi_load("earth_m.jpg", &nx, &ny, &nn, 0);
	material* mat = new lambertian(new image_texture(tex_data, nx, ny));

	*ret = new sphere(vec3(0.0f, 1.0f, 0.0f), 2.0f, mat);
}

void simple_light(hitable** ret)
{
	texture* pertext = new noise_texture(4.0f);
	hitable** list = new hitable*[4];
	list[0] = new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new lambertian(pertext));
	list[1] = new sphere(vec3(0.0f, 2.0f, 0.0f), 2.0f, new lambertian(pertext));
	list[2] = new sphere(vec3(0.0f, 7.0f, 0.0f), 2.0f, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	list[3] = new xy_rect(3, 5, 1, 3, -2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));

	*ret = new hitable_list(list, 4);
}

void cornell_box(hitable** ret)
{
	hitable** list = new hitable*[8];
	int i = 0;
	material* red = new diffuse_light(new constant_texture(vec3(2, 0.1f, 0.1f)));
	material* white = new lambertian(new constant_texture(vec3(0.73f, 0.73f, 0.73f)));
	material* green = new diffuse_light(new constant_texture(vec3(.1, 2, .1)));
	material* blue = new diffuse_light(new constant_texture(vec3(1, 1, 34)));
	//material* grey = new lambertian(new constant_texture(vec3(0.12f, 0.12f, 0.15f)));
	//material* light = new diffuse_light(new constant_texture(vec3(11, 11, 11)));
	//int nx, ny, nn;
	//unsigned char* tex_data = stbi_load("gun.jpg", &nx, &ny, &nn, 0);
	//material* tex = new lambertian(new image_texture(tex_data, nx, ny));
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(213, 343, 227, 332, 554, blue);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130,0,65));
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265,0,295));

	*ret = new hitable_list(list, i);
	//*ret = new bvh_node(list, i, 0, 1);
}

void cornell_smoke(hitable** ret)
{
    hitable** list = new hitable*[8];
    int i = 0;
    material* red = new lambertian(new constant_texture(vec3(0.65f, 0.05f, 0.05f)));
    material* white = new lambertian(new constant_texture(vec3(0.73f, 0.73f, 0.73f)));
    material* green = new lambertian(new constant_texture(vec3(0.12f, 0.45f, 0.15f)));
    material* light = new diffuse_light(new constant_texture(vec3(7,7,7)));

    list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
    list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
    list[i++] = new xz_rect(113, 443, 127, 432, 554, light);
    list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
    list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
    list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));

    hitable* b1 = new translate(new rotate_y(new box(vec3(0,0,0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
    hitable* b2 = new translate(new rotate_y(new box(vec3(0,0,0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

    list[i++] = new constant_medium(b1, 0.01f, new constant_texture(vec3(1,1,1)));
    list[i++] = new constant_medium(b2, 0.01f, new constant_texture(vec3(0,0,0)));

    *ret = new hitable_list(list, i);
}


void final_scene(hitable** ret)
{
    int nb = 20;
    hitable **list = new hitable*[30];
    hitable **boxlist = new hitable*[400];
    hitable **boxlist2 = new hitable*[1000];
    material *white = new lambertian( new constant_texture(vec3(0.73, 0.73, 0.73)) );
    material *ground = new lambertian( new constant_texture(vec3(0.48, 0.83, 0.53)) );
    int l = 0;
    int b = 0;
    for (int i = 0; i < nb; i++)
    {
        for (int j = 0; j < nb; j++)
        {
            float w = 100;
            float x0 = -1000 + i*w;
            float z0 = -1000 + j*w;
            float y0 = 0;
            float x1 = x0 + w;
            float y1 = 100*(rand48()+0.01);
            float z1 = z0 + w;
            boxlist[b++] = new box(vec3(x0,y0,z0), vec3(x1,y1,z1), ground);
        }
    }
    list[l++] = new bvh_node(boxlist, b, 0, 1);
    material *light = new diffuse_light( new constant_texture(vec3(12, 12, 12)) );
    list[l++] = new xz_rect(123, 423, 147, 412, 554, light);
    vec3 center(400, 400, 200);
    list[l++] = new moving_sphere(center, center+vec3(30, 0, 0), 0, 1, 50, new lambertian(new constant_texture(vec3(0.7, 0.3, 0.1))));
    list[l++] = new sphere(vec3(260, 150, 45), 50, new dielectric(1.5));
    list[l++] = new sphere(vec3(0, 150, 145), 50, new metal(vec3(0.8, 0.8, 0.9), 10.0));
    hitable *boundary = new sphere(vec3(360, 150, 145), 70, new dielectric(1.5));
    list[l++] = boundary;
    list[l++] = new constant_medium(boundary, 0.2, new constant_texture(vec3(0.2, 0.4, 0.9)));
    boundary = new sphere(vec3(0, 0, 0), 5000, new dielectric(1.5));
    list[l++] = new constant_medium(boundary, 0.0001, new constant_texture(vec3(1.0, 1.0, 1.0)));
    int nx, ny, nn;
    unsigned char *tex_data = stbi_load("earth_m.jpg", &nx, &ny, &nn, 0);
    material *emat =  new lambertian(new image_texture(tex_data, nx, ny));
    list[l++] = new sphere(vec3(400,200, 400), 100, emat);
    texture *pertext = new noise_texture(0.1);
    list[l++] =  new sphere(vec3(220,280, 300), 80, new lambertian( pertext ));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxlist2[j] = new sphere(vec3(165*rand48(), 165*rand48(), 165*rand48()), 10, white);
    }
    list[l++] =   new translate(new rotate_y(new bvh_node(boxlist2,ns, 0.0, 1.0), 15), vec3(-100,270,395));
    *ret = new hitable_list(list,l);
}

#endif // SCENESH
