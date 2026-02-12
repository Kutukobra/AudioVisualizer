#include <raudio.h>
#include <raylib.h>

#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 4096

float g_samples[BUFFER_SIZE] = { 0.0f };
float complex g_frequencies[BUFFER_SIZE];

#define WAVE_WINDOW_SIZE 1024
float g_wave_window[WAVE_WINDOW_SIZE] = { 0.0f };
float g_wave_window2[WAVE_WINDOW_SIZE] = { 0.0f };

void _fft(float in[], float complex out[], size_t n, int stride) 
{
        if (n == 1) {
                out[0] = in[0];
                return;
        }
        _fft(in, out, n / 2, stride * 2);
        _fft(in + stride, out + n / 2, n / 2, stride * 2);

        for (int i = 0; i < n / 2; i++) {
                float complex v = cexpf(2 * PI * I * i / n) * out[i + n / 2];
                float complex e = out[i];
                out[i] = e + v;
                out[i + n /2] = e - v;
        }
}

void fft(float in[], float complex out[], size_t n) 
{
        _fft(in, out, n, 1);
}

void processingCallback(void* buffer, uint32_t frames)
{
        float *data = buffer;
        float average = 0.0f;
        for (int i = 0; i < frames; i++) {
                average += data[i];
        }
        average /= frames;

        memmove(g_wave_window, g_wave_window + 1, (WAVE_WINDOW_SIZE - 1) * sizeof(float));
        memmove(g_wave_window2, g_wave_window2 + 2, (WAVE_WINDOW_SIZE - 2) * sizeof(float));

        g_wave_window[WAVE_WINDOW_SIZE - 1] = average;
        g_wave_window2[WAVE_WINDOW_SIZE - 1] = g_wave_window2[WAVE_WINDOW_SIZE - 2] = average;

        int data_size = frames / 2;

        for (int i = 0; i < BUFFER_SIZE - data_size; i++) {
                g_samples[i] = g_samples[i + data_size];
        }

        for (int i = 0; i < data_size; i++) {
                g_samples[BUFFER_SIZE - data_size + i] = (data[i * 2] + data[i * 2 + 1]) / 2;
        }
        
        fft(g_samples, g_frequencies, BUFFER_SIZE);
}

int main(int argc, char **argv)
{
        InitWindow(1920, 960, "AudioVisualizer");
        SetTargetFPS(60);

        InitAudioDevice();
        AttachAudioMixedProcessor(processingCallback);
        Music music = LoadMusicStream(argv[1]);
        music.looping = true;
        PlayMusicStream(music);

        int32_t width = GetScreenWidth();
        int32_t height = GetScreenHeight();
        float barWidth = roundf((float)width / WAVE_WINDOW_SIZE);

        while (!WindowShouldClose()) {
                UpdateMusicStream(music);

                BeginDrawing();
                ClearBackground((Color){ 25, 25, 25, 255 });
                
                float prominent_magnitude = 0.0f;
                int prominent_index = 0;


                for (int i = 0; i < WAVE_WINDOW_SIZE / 2; i++) {
                        int top_parallax = (fabs(g_wave_window[i * 2]) + 0.1) * height;
                        DrawRectangle(barWidth * i * 2, 0, barWidth * 2, top_parallax, (Color){ 100, 100, 100, 100 });                        
                        DrawRectangle(barWidth * i * 2, height - top_parallax, barWidth * 2, top_parallax, (Color){ 100, 100, 100, 100 });
                }

                for (int i = 0; i < WAVE_WINDOW_SIZE; i++) {
                        int frontHeight = (fabs(g_wave_window2[i]) + 0.04) * height;
                        DrawRectangle(barWidth * i, 0, barWidth, frontHeight, BLACK);
                        DrawRectangle(barWidth * i, height - frontHeight, barWidth, frontHeight, BLACK);
                }

                // Frequencies
                for (uint32_t i = 1; i < BUFFER_SIZE / 4; i++) {
                        float magnitude = cabsf(g_frequencies[i]);
                        if (magnitude > prominent_magnitude) {
                                prominent_magnitude = magnitude;
                                prominent_index = i;
                        }
                        int barHeight = magnitude * height / (BUFFER_SIZE / 13);
                        if (barHeight <= 0) {
                                barHeight = 1;
                        }
                        float drawWidth = barWidth < 1 ? 1 : barWidth;
                        DrawRectangle(drawWidth * i * 4, (height - barHeight) / 2, barWidth, barHeight, GREEN);
                }
                EndDrawing();
        }

        UnloadMusicStream(music);
        CloseAudioDevice();
        CloseWindow();
}