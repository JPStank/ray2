#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif

#include <SDL2/SDL.h>

#include <thread>
#include <chrono>
#include <ctime>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#include "scenes.h"

struct global_state
{
    int width, height, samples;
    unsigned char* data;
    unsigned int data_size;
    int thread_count;
    hitable* world;
    vec3 (*colorFunc)(const ray& r, hitable* world, int depth);
    camera* cam;
};

global_state g;

void worker(int begin, int end, int index_start)
{
    int index = index_start;
    for (int j = begin-1; j >= end; --j)
    {
        for (int i = 0; i < g.width; ++i)
        {
            //float _r = float(i) / float(g.width);
            //float _g = float(j) / float(g.height);
            //float _b = 0.2f;

            vec3 col(0,0,0);
            for (int s = 0; s < g.samples; ++s)
            {
                float u = float(i+rand48()) / float(g.width);
                float v = float(j+rand48()) / float(g.height);
                ray r = g.cam->get_ray(u, v);
                col += g.colorFunc(r, g.world, 0);
            }

            col /= float(g.samples);
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
            g.data[index++] = (unsigned char)(255.99*col[0]);
            g.data[index++] = (unsigned char)(255.99*col[1]);
            g.data[index++] = (unsigned char)(255.99*col[2]);
            g.data[index++] = 255;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

/* Moving Rectangle */
int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));

        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *texture;
        SDL_Event event;
        SDL_Surface* surf;
        //int width, height;

        if (argc == 3)
        {
            g.width = atoi(argv[1]);
            g.height = atoi(argv[2]);
        }
        else
        {
            g.width = g.height = 256;
        }

        vec3 origin(278,278,-800);
        //vec3 origin(478, 278, -600);
        vec3 look(278, 278, 0);

        //vec3 origin(13, 2, 3);
        //vec3 look(0,0,0);
        float focus = 10;
        float aperature = 0;
        float vfov = 40;
        g.cam = new camera(origin, look, vec3(0,1,0), vfov, float(g.width)/float(g.height), aperature, focus, 0, 1);

        g.samples = 100;
        g.colorFunc = color_emit_light;

        cornell_box(&g.world);

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
                return 3;
        }

        window = SDL_CreateWindow("Stank's Ray Trace",
                        SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED,
                        g.width, g.height,
                        SDL_WINDOW_SHOWN);


        renderer = SDL_CreateRenderer(window, -1, 0);


        g.data_size = g.width*g.height*4;
        g.data = new unsigned char[g.data_size];

        g.thread_count = 16;

        int slice = g.height/g.thread_count;
        int data_slice = g.data_size/g.thread_count;

        for (int i = 0; i < g.thread_count; i++)
        {
            int start = g.height - i*slice;
            int end = start - slice;
            int index = i*data_slice;
            std::thread t(worker, start, end, index);
            t.detach();
            //worker(start, end, index);
        }

        surf = SDL_CreateRGBSurfaceFrom(g.data, g.width, g.height, 32, 4*g.width, 0xff, 0xff00, 0xff0000, 0xff000000);
        texture = SDL_CreateTextureFromSurface(renderer, surf);

        while (1)
        {
            SDL_PollEvent(&event);
            if(event.type == SDL_QUIT)
                break;

            SDL_FreeSurface(surf);
            surf = SDL_CreateRGBSurfaceFrom(g.data, g.width, g.height, 32, 4*g.width, 0xff, 0xff00, 0xff0000, 0xff000000);

            SDL_DestroyTexture(texture);
            texture = SDL_CreateTextureFromSurface(renderer, surf);


            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        stbi_write_png("test.png", g.width, g.height, 4, g.data, g.width*4);
        delete[] g.data;

        SDL_FreeSurface(surf);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 0;
}
