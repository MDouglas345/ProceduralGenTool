#include <iostream>
#include "FastNoiseLite.h"
#include <random>

float RandomFloatBetween(float LO, float HI){
    return LO + static_cast<float> (std::rand()) / (static_cast<float>(RAND_MAX / (HI - LO)));
}

float Clip(float n, float lower, float upper){
    return std::max(lower, std::min(n, upper));
}

class IWorldGenerator{
public:
    virtual void GenWorldShape(int seed, int width, int height, float* raw, FastNoiseLite* N) = 0;
    virtual void GenCaves(int seed, int width, int height, float* raw, FastNoiseLite* N) = 0;
private:
};


class SpikyWorldGen : public IWorldGenerator{
public:
    void GenWorldShape(int seed, int width, int height, float* raw, FastNoiseLite* N)
    {
        N->SetFrequency(0.02f);
        float overworldHeight = RandomFloatBetween(height / 3.f, height);

        float* heightmap = new float[width];
        for (int x = 0; x < width; x++){
            heightmap[x] = Clip(N->GetNoise((float)x, overworldHeight), 0, 1) * overworldHeight; 
        }

        for (int x = 0; x < width; x++){
            for (int y = 0; y < height; y++){
                if (y < (int)heightmap[x]) continue;
                raw[x + y * width] = 1;
            }
        }
        delete[] heightmap;
    }


    void GenCaves(int seed, int width, int height, float* raw, FastNoiseLite* N)
    {
        N->SetFrequency(0.2f);
        GenCaveHelper(width, height, 0.002, 1.0f, raw, N);
    }

private:
    void GenCaveHelper(int width, int height, float threshold, float height_threshold, float* raw, FastNoiseLite* N){
        for (int y = 0; y < height; y++){
            for (int x = 0; x < width; x++){
                float i = N->GetNoise((float)x, (float)y);
                float h = 1.0f - (float)y/height;
                if (raw[x + y * width] < 0.01f || h > height_threshold || i > threshold) continue;
                raw[x + y * width] = 0;
            }
        }
    }

};

class WorldGenerator{
public:
    WorldGenerator(int s, int w, int h){
        seed = s;
        width = w;
        height = h;
        N = new FastNoiseLite();
        output = new char[w * h];
        raw = new float[w * h];
    }

    ~WorldGenerator(){
        delete N;
        delete[] raw;
        delete[] output;
    }
    void GenWorldShape(){
        Mode->GenWorldShape(seed, width, height, raw, N);
    }

    void GenCaves(){
        Mode->GenCaves(seed, width, height, raw, N);
    }

    void GenerateOutput(){
        for (int y = 0; y < height; y++){
            for (int x = 0; x < width; x++){
                char t;
                float i = raw[x + y * width];

                if (i > 0.2){
                    t = '*';
                }
                else{
                    t = ' ';
                }
                output[x + y * width] = t;
            }
        }
    }

    void DisplayOutput(){
        char* display = new char[width * height + height];
        for (int y = 0; y < height; y++){
            for (int x = 0; x < width; x++){
                display[x + y * width] = output[x + y * width];
            }
            display[(width-1) + y * width] = '\n';
        }
        std::cout << display;
        delete[] display;
    }

    void Gen(){
        GenWorldShape();
        GenCaves();
    }

    void SetSeed(int s){
        N->SetSeed(s);
    }

    void SetMode(IWorldGenerator* m){
        Mode = m;
    }

    void ClearMapData(){
        memset(raw, 0, sizeof(float) * width * height);
    }

    void clear(){
        std::cout << "\x1B[2J\x1B[H";
    }
private:
    int seed,width,height;
    float* raw;
    char* output;
    IWorldGenerator* Mode;
    FastNoiseLite* N;
};

int main(){
    int seed = std::rand();
    int width = 200;
    int height = 50;
    WorldGenerator* Gen = new WorldGenerator(seed, width, height);
    Gen->SetMode(new SpikyWorldGen());

    char i;

    while (true){
        Gen->SetSeed(seed);
        Gen->ClearMapData();

        Gen->GenWorldShape();
        Gen->GenCaves();

        Gen->GenerateOutput();

        Gen->DisplayOutput();

        i = std::cin.get();
        if (i == 'x') break;

        switch(i){
        }   
        seed = std::rand();
        Gen->ClearMapData();
        Gen->clear();

    }
}