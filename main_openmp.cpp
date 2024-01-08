#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <cmath>
#include <omp.h>
#include "consts.cpp"

int main(){
    auto map = consts::reorder_map;
    float *input = new float[400*400];
    // fill input with the normal distrubtion of mean 5400 and std 19
    srand(time(NULL));
    for (int i = 0; i < 400*400; i++)
    {
        input[i] = 5400 + 19 * sqrt(-2 * log((float)rand() / RAND_MAX)) * cos(2 * M_PI * (float)rand() / RAND_MAX);
    }
    float *output = new float[400*400];
    float *class_mask = new float[400*400];
    auto start = std::chrono::high_resolution_clock::now();
    #pragma omp parallel for num_threads(2) collapse(2)
    for (int y = 0; y<400; y++){
        for (int x = 0; x<400; x++){
            output[map[x][y]] = input[x+y*400];
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    // for (int i = 0; i < 2<<40; i++)
    // {
    //     std::cout << x[i];
    // }
    std::cout << std::endl;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout <<  "Time taken by function: " << duration.count() << " microseconds" << std::endl;
    constexpr int nsigma = 3;
    constexpr int cluster_size = 3;
    constexpr int c2 = (cluster_size + 1 ) / 2;
    constexpr int c3 = cluster_size;
    start = std::chrono::high_resolution_clock::now();
    for (int iy = 0; iy < consts::FRAME_HEIGHT; iy++){
        for (int ix = 0; ix < consts::FRAME_WIDTH; ix++){
            float max_value, tl, tr, bl, br, tot;
            max_value = tl = tr = bl = br = tot = 0;
            char pixel_class = 0;
            float rms = 4.0;
            for (int ir = -cluster_size / 2; ir < cluster_size / 2 + 1; ir++){
                for (int ic = -cluster_size / 2; ic < cluster_size / 2 + 1; ic++){
                    const int y_sub = iy + ir;
                    const int x_sub = ix + ix;
                    if (y_sub >= 0 && y_sub < consts::FRAME_HEIGHT && x_sub >= 0 && x_sub < consts::FRAME_WIDTH){
                        const int value = input[y_sub*400+x_sub];
                        tot += value;
                        if (ir <= 0 && ic <=0) bl+= value;
                        if (ir <= 0 && ic >=0) br+= value;
                        if (ir >= 0 && ic <=0) tl+= value;
                        if (ir >= 0 && ic >=0) tr+= value;
                        // or
                        // max_value = std::max(max_value, value);
                        if (value > max_value) max_value = value;
                    }
                }
            }
            if (input[ix+iy*400] < -nsigma * rms) {
                pixel_class = 3;
                class_mask[consts::reorder_map[iy][ix]] = 3;
                continue;
            }
            if (max_value > nsigma*rms){
                pixel_class = 1;
                class_mask[ix+iy*400] = 1;
                if (input[ix+iy*400] < max_value) continue;
            }
            else if (tot > c3*nsigma*rms) {
                pixel_class = 1;
                class_mask[ix+iy*400] = 1;
            }
            else if (std::max({bl, br, tl, tr}) > c2 * nsigma * rms){
                pixel_class = 1;
                class_mask[ix+iy*400] = 1;
            }
            if (pixel_class == 1 && input[ix+iy*400] == max_value){
                pixel_class = 2;
                class_mask[ix+iy*400] = 2;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout <<  "Time taken by function: " << duration.count() << " microseconds" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    float max_value, tl, tr, bl, br, tot, value;
    max_value = tl = tr = bl = br = tot = 0;
    char pixel_class = 0;
    float rms = 18.0;
    int iy, ix, ir, ic, y_sub, x_sub;
    #pragma omp parallel for private(iy, ix, ir, ic, max_value, tl, tr, bl, br, tot, y_sub, x_sub, value, pixel_class) num_threads(8) collapse(2)
    for (iy = 0; iy < consts::FRAME_HEIGHT; iy++){
        for (ix = 0; ix < consts::FRAME_WIDTH; ix++){
            for (ir = -cluster_size / 2; ir < cluster_size / 2 + 1; ir++){
                for (ic = -cluster_size / 2; ic < cluster_size / 2 + 1; ic++){
                    y_sub = iy + ir;
                    x_sub = ix + ix;
                    if (y_sub >= 0 && y_sub < consts::FRAME_HEIGHT && x_sub >= 0 && x_sub < consts::FRAME_WIDTH){
                        value = input[y_sub*400 + x_sub];
                        tot += value;
                        if (ir <= 0 && ic <=0) bl+= value;
                        if (ir <= 0 && ic >=0) br+= value;
                        if (ir >= 0 && ic <=0) tl+= value;
                        if (ir >= 0 && ic >=0) tr+= value;
                        // or
                        // max_value = std::max(max_value, value);
                        max_value = std::max(max_value, value);
                    }
                }
            }
            if (max_value > nsigma*rms || tot > c3*nsigma*rms || std::max({bl, br, tl, tr}) > c2 * nsigma * rms){
                class_mask[ix+iy*400] = 1;
                if (input[ix+iy*400] == max_value) class_mask[ix+iy*400] = 2;;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout <<  "Time taken by function: " << duration.count() << " microseconds" << std::endl;
    delete input;
    delete output;
    return 0;
}