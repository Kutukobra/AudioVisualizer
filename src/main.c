#include <raylib.h>
#include <raudio.h>

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#define MAX_BUFFER_SIZE 400

float g_samples[MAX_BUFFER_SIZE] = {0.0f};
uint32_t g_frameSize;

void processingCallback(void *buffer, uint32_t frames) {
    float *data = buffer;
    g_frameSize = frames / 2;

    float average = 0;
    for (uint32_t i = 0; i < g_frameSize; i++) {
        average += fabs(data[i * 2]) + fabs(data[i * 2 + 1]);
    }
    average /= g_frameSize * 2;

    for (uint32_t i = 0; i < MAX_BUFFER_SIZE - 1; i++) {
        g_samples[i] = g_samples[i + 1];
    }

    g_samples[MAX_BUFFER_SIZE - 1] = average;
}

int32_t main() {
    InitWindow(960, 720, "AudioVisualizer");
    SetTargetFPS(24);

    InitAudioDevice();
    AttachAudioMixedProcessor(processingCallback);
    Music music = LoadMusicStream("../audio.mp3");
    music.looping = true;
    PlayMusicStream(music);

    int32_t width = GetScreenWidth();
    int32_t height = GetScreenHeight();
    float barWidth = (float)width / MAX_BUFFER_SIZE;
    
    while (!WindowShouldClose()) {
        UpdateMusicStream(music);

        BeginDrawing();
        ClearBackground(BLACK);

        float barPosition = 0.0f;
        for (uint32_t i = 0; i < MAX_BUFFER_SIZE; i++) {
            int32_t barHeight = abs(g_samples[i] * height);

            DrawRectangle(barPosition, (height - barHeight) / 2, (int32_t)barWidth, barHeight, GREEN);
            barPosition += barWidth;
        }
        EndDrawing();
    }

    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
}