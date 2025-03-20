#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define BUFF_SIZE 2048

int width = 800;
int height = 400;
int halfh;
int quarterh;

int channels = 1;
float sensetivity = 2.0;
float buff[BUFF_SIZE];
float buff2[BUFF_SIZE];
int ptrCall = 0;

void CheckErr(ma_result result) {
    if (result != MA_SUCCESS) {
        printf("ERROR: %d", result);
        exit(result);
    }
}

void callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    float *in = pInput;
    for (int i = 0; i < frameCount * channels; i++) {
        if (ptrCall >= BUFF_SIZE) {
            break;
        }
        buff[ptrCall] = in[i] * sensetivity;
        ptrCall++;
    }
}

int main(int argc, char *argv[]) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_TRANSPARENT);
    InitWindow(width, height, "Audio Visualizer");
    SetWindowMinSize(200, 200);
    SetWindowMaxSize(BUFF_SIZE - 16, 1080);
    SetTargetFPS(20);

    ma_result result;
    ma_context context;
    ma_device_info* pPlaybackDeviceInfos;
    ma_device_info* pCaptureDeviceInfos;
    ma_uint32 playbackDeviceCount;
    ma_uint32 captureDeviceCount;
    ma_uint32 iDevice;

    result = ma_context_init(NULL, 0, NULL, &context);
    CheckErr(result);

    result = ma_context_get_devices(&context,
                                    &pPlaybackDeviceInfos, &playbackDeviceCount,
                                    &pCaptureDeviceInfos, &captureDeviceCount);
    CheckErr(result);

    printf("Playback devices:\n");
    for (iDevice = 0; iDevice < playbackDeviceCount; ++iDevice) {
        printf("  %d %s\n", iDevice, pPlaybackDeviceInfos[iDevice].name);
    }

    printf("Capture devices:\n");
    for (iDevice = 0; iDevice < captureDeviceCount; ++iDevice) {
        printf("  %d %s\n", iDevice, pCaptureDeviceInfos[iDevice].name);
    }

    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.format = ma_format_f32;
    config.capture.pDeviceID = &pCaptureDeviceInfos[0].id;
    config.dataCallback = callback;

    ma_device device;
    result = ma_device_init(&context, &config, &device);
    CheckErr(result);

    channels = device.capture.channels;

    ma_device_start(&device);

    Color colorMain = VIOLET;
    if (argc > 4) {
        colorMain = (Color){atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])};
    }
    Color colorBG = {0, 0, 0, 63};
    Color colorSec = colorMain;
    colorSec.a = 127;
    Color colorBar = {0, 0, 0, 127};
    Color colorBarFull = colorSec;
    Color colorLine = colorSec;
    Color colorDot = colorMain;
    Color colorDot2 = colorDot;
    colorDot2.r /= 2;
    colorDot2.g /= 2;
    colorDot2.b /= 2;
    int colmod = 0;

    // Mainloop
    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_UP)) {
            sensetivity += 0.2;
        } else if (IsKeyPressed(KEY_DOWN)) {
            sensetivity -= 0.2;
            if (sensetivity < 0.4) {
                sensetivity = 0.4;
            }
        }
        
        // Draw everything
        if (IsWindowResized()) {
            width = GetScreenWidth();
            height = GetScreenHeight();
            halfh = height / 2;
            quarterh = halfh / 2;
        }

        BeginDrawing();

        ClearBackground(colorBG);

        for (int i = 0; i < width; i++) {
            colorLine.a = colorSec.a;
            if (i % 16 < 2) {
                colorLine.a = 0;
            } else if (i % 2 == 0) {
                colorLine.a /= 2;
            }
            DrawLine(i, height, i,
                     height - abs(buff[i] * quarterh * 3) +
                         buff[i + 16] * quarterh,
                     colorLine);

            DrawPixel(i, quarterh + buff2[i] * quarterh + 5, GRAY);
            DrawPixel(i, quarterh + buff2[i] * quarterh + 5 + 1, colorDot2);
            DrawPixel(i, quarterh + buff2[i] * quarterh + 5 + 3, colorDot2);
            DrawPixel(i, quarterh + buff2[i] * quarterh + 5 + 5, colorDot2);

            DrawPixel(i, quarterh + buff[i] * quarterh, WHITE);
            DrawPixel(i, quarterh + buff[i] * quarterh + 1, colorDot);
            DrawPixel(i, quarterh + buff[i] * quarterh + 3, colorDot);
            DrawPixel(i, quarterh + buff[i] * quarterh + 5, colorDot);
        }

        memcpy(buff2, buff, BUFF_SIZE * sizeof(buff[0]));
        ptrCall = 0;
        EndDrawing();
    }

    ma_device_uninit(&device);
    ma_context_uninit(&context);
    
    return 0;
}
