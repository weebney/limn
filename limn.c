#include <math.h>
#include <stdio.h>

#include "lib/raylib/src/raylib.h"

int main(void) {
	struct Layer {
		bool visible;
		Image image;
	};

	struct State {
		struct Layer layers[256];
		int layerCount;
		Color palette[16];
		Image clipboard;

		int width;
		int height;
	};

	// struct State UndoableHistory[256];
	// struct State RedoableHistory[256];

	// Initialization
	int screenWidth = 1280;
	int screenHeight = 720;
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(screenWidth, screenHeight, "limn");
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);

	int pixelSize = 1;
	int panX = 0;
	int panY = 0;

	struct State state;

	state.clipboard = GenImageColor(0, 0, BLANK);
	state.layerCount = 0;

	state.palette[0] = RED;
	state.palette[1] = GREEN;
	state.palette[2] = BLUE;
	state.palette[3] = MAGENTA;
	state.palette[4] = PURPLE;
	state.palette[5] = PINK;
	state.palette[6] = BEIGE;
	state.palette[7] = BROWN;
	state.palette[8] = RED;
	state.palette[9] = RED;
	state.palette[10] = RED;
	state.palette[11] = RED;
	state.palette[12] = RED;
	state.palette[13] = RED;
	state.palette[14] = RED;
	state.palette[15] = RED;
	state.palette[15] = RED;

	// create initial layer
	state.layers[0].image = LoadImage("mikured.png");
	state.layers[0].visible = true;
	state.layerCount += 1;
	state.width = state.layers[0].image.width;
	state.height = state.layers[0].image.height;

	// second test layer
	state.layers[1].image = GenImageColor(state.width, state.height, (Color){40, 40, 0, 123});
	state.layers[1].visible = true;
	state.layerCount += 1;

	// third test layer
	state.layers[2].image = GenImageChecked(state.width, state.height, 6, 6, PINK, PURPLE);
	state.layers[2].visible = false;
	state.layerCount += 1;

	Texture2D compositeTexture;
	bool compositeDirty = true;
	while (!WindowShouldClose()) {
		if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()})) {
			HideCursor();
		};

		screenWidth = GetScreenWidth();
		screenHeight = GetScreenHeight();

		// cmd/ctrl
		if (IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_LEFT_CONTROL)) {
			// zoom
			if (IsKeyPressed(KEY_EQUAL) && pixelSize < 4) {
				compositeDirty = true;
				pixelSize += 1;
			} else if (IsKeyPressed(KEY_MINUS) && pixelSize > 1) {
				compositeDirty = true;
				pixelSize -= 1;
			}

			// pan
			if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
				Vector2 mouseDelta = GetMouseDelta();
				panX += (int) mouseDelta.x;
				panY += (int) mouseDelta.y;
			}

			// TODO: FIX!
			// float wheelMove = GetMouseWheelMove() * 2;
			// if (wheelMove > 0.25 && pixelSize > 1) {
			// pixelSize /= 2;
			//} else if (wheelMove < -0.25 && pixelSize < 16) {
			// pixelSize *= 2;
			//}
		}

		// TODO: only redraw on changes

		// Calculate dimensions
		int stageWidth = state.width * pixelSize;
		int stageHeight = state.height * pixelSize;
		int stageX = (screenWidth - stageWidth) / 2 + panX;
		int stageY = (screenHeight - stageHeight) / 2 + panY;

		// composite layers into texture
		if (compositeDirty) {
			UnloadTexture(compositeTexture);
			Rectangle compositeBounds = (Rectangle){0, 0, (float) stageWidth, (float) stageHeight};
			Image composite = GenImageColor(stageWidth, stageHeight, BLANK);
			for (int i = 0; i < state.layerCount; i++) {
				if (state.layers[i].visible) {
					ImageResizeNN(&state.layers[i].image, stageWidth, stageHeight);

					// TODO: this is aboslutely fucking murdering performance
					ImageDraw(&composite, state.layers[i].image, compositeBounds, compositeBounds, WHITE);
				};
			};
			compositeDirty = false;
			compositeTexture = LoadTextureFromImage(composite);
			UnloadImage(composite);
		};

		// TODO: don't redraw canvas if no update
		BeginDrawing();

		ClearBackground(RAYWHITE);
		DrawTexture(compositeTexture, stageX, stageY, WHITE);

		char debugText[1024];
		sprintf(debugText, "fps:%d\n\npixel size:%d\n\nwidth:%d\n\nheight:%d\n\naspect:%0.2f\n\nlayers:%d", GetFPS(),
			pixelSize, stageWidth, stageHeight, (float) stageWidth / (float) stageHeight, state.layerCount);
		DrawText(debugText, 24, 24, 24, BLACK);

		// cursor
		SetMouseCursor(MOUSE_CURSOR_NOT_ALLOWED);
		DrawCircle(GetMouseX(), GetMouseY(), 2, BLACK);

		EndDrawing();
	}

	// UnloadImage(state.layers[i].image); // Unload image before closing
	UnloadTexture(compositeTexture);
	CloseWindow();
	return 0;
}

// TODO:
// palette
// drawing
// layers & layer browser
// merge layer down
// layer reordering
// canvas cropping
// selection and clipboard
// resize image
// file dialog import
// paint bucket fill
