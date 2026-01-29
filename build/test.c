#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265359

void dft(float in[], float complex out[], size_t n)
{
        for (int i = 0; i < n; i++) {
                out[i] = 0.0f;
                for (int j = 0; j < n; j++) {
                        float complex x = 2 * PI * I * i * j / n;
                        float complex w = in[j] * cexpf(x);
                        out[i] += w;
                }
                out[i] /= (float)n / 2;
        }
}

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

int main()
{
        size_t n = 16;

        float* samples = calloc(n, sizeof(float));

        for (int i = 0; i < n; i++) {
                float x = i * 2 * PI / n;
                samples[i] = cosf(x);
                // samples[i] = cosf(x);
        }

        float complex* frequencies = malloc(n * sizeof(float complex));

        fft(samples, frequencies, n);

        // dft(samples, frequencies, n);

        printf("%8s | %10s  %10s %10s %10s\n", "Index", "Sample", "Cosine",
            "Sine", "Magnitude");
        for (int i = 0; i < n; i++) {
                printf("%8d | %10.4f  %10.4f %10.4f %10.4f\n", i, samples[i],
                    crealf(frequencies[i]), cimagf(frequencies[i]),
                    cabsf(frequencies[i]));
        }

        free(samples);
}