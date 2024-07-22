#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib/raylib/src/raylib.h"
#include "lib/tfd/tinyfiledialogs.h"

int main(void) {
	struct Layer {
		bool visible;
		Texture2D texture;
	};
	typedef struct Layer Layer;

	struct State {
		Layer layers[256];
		int layerCount;
		int layerCurrent;

		Texture2D clipboard;
		RenderTexture paintBuffer;

		Color palette[36];
		int paletteCurrent;
		int brushSize;

		int width;
		int height;
	};
	typedef struct State State;

	enum Cursor { CUR_BRUSH, CUR_ERASER, CUR_POINTER };

	// struct State UndoableHistory[256];
	// struct State RedoableHistory[256];

	// Initialization
	int screenWidth = 1280;
	int screenHeight = 720;
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	SetTargetFPS(200);
	InitWindow(screenWidth, screenHeight, "limn");
	SetExitKey(KEY_NULL);

	int pixelSize = 1;
	int panX = 0;
	int panY = 0;

	State state;
	state.layerCurrent = 1;
	state.layerCount = 0;
	state.paletteCurrent = 1;
	state.brushSize = 2;
	state.clipboard = LoadTextureFromImage(GenImageColor(state.width, state.height, BLANK));

	state.palette[0] = WHITE;
	state.palette[1] = BLACK;
	state.palette[2] = RED;
	state.palette[3] = MAGENTA;
	state.palette[4] = PURPLE;
	state.palette[5] = PINK;
	state.palette[6] = BEIGE;
	state.palette[7] = BROWN;
	state.palette[8] = GREEN;
	state.palette[9] = RED;
	state.palette[10] = YELLOW;
	state.palette[11] = RED;
	state.palette[12] = RED;
	state.palette[13] = RED;
	state.palette[14] = RED;
	state.palette[15] = RED;
	state.palette[16] = RED;
	state.palette[17] = RED;
	state.palette[18] = RED;
	state.palette[19] = RED;
	state.palette[20] = RED;
	state.palette[21] = RED;
	state.palette[22] = RED;
	state.palette[23] = RED;
	state.palette[24] = RED;
	state.palette[25] = RED;
	state.palette[26] = RED;
	state.palette[27] = RED;
	state.palette[28] = RED;
	state.palette[29] = RED;
	state.palette[30] = RED;
	state.palette[31] = RED;
	state.palette[32] = RED;
	state.palette[33] = RED;
	state.palette[34] = RED;
	state.palette[35] = RED;

	// DEBUG create initial layer
	// state.layers[2].texture = LoadTexture("/Users/weeb/Developer/limn/mikured_miku.png");
	state.layers[2].texture = LoadTextureFromImage(GenImageColor(80, 80, BLANK));
	state.layers[2].visible = true;
	state.layerCount += 1;
	state.width = state.layers[2].texture.width;
	state.height = state.layers[2].texture.height;

	// DEBUG create initial layer
	state.layers[0].texture = LoadTextureFromImage(GenImageColor(state.width, state.height, (Color){125, 123, 155, 130}));
	state.layers[0].visible = true;
	state.layerCount += 1;

	// DEBUG create initial layer
	state.layers[1].texture = LoadTextureFromImage(GenImageColor(state.width, state.height, BLANK));
	state.layers[1].visible = true;
	state.layerCount += 1;

	// Texture2D compositeTexture;
	RenderTexture2D composite = LoadRenderTexture(state.width, state.height);
	bool compositeDirty = true;
	bool isPainting = false;
	double scrollZoomCooldown = GetTime();
	Texture2D substageTexture = LoadTextureFromImage(GenImageChecked(state.width, state.height, 4, 4, WHITE, GRAY));

	while (!WindowShouldClose()) {
		if (IsWindowState(FLAG_WINDOW_UNFOCUSED | FLAG_WINDOW_HIDDEN | FLAG_WINDOW_MINIMIZED)) {
			WaitTime(0.01);
			PollInputEvents();
			continue;
		}
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

			// TODO: Fix scroll zoom
			float wheelMove = GetMouseWheelMove();
			if ((GetTime() - scrollZoomCooldown) > 0.08 && wheelMove != 0) {
				if (wheelMove > 0 && pixelSize > 1) {
					pixelSize /= 2;
					scrollZoomCooldown = GetTime();
				} else if (wheelMove < 0 && pixelSize < 16) {
					pixelSize *= 2;
					scrollZoomCooldown = GetTime();
				}
			}
		}

		if (IsKeyPressed(KEY_LEFT_BRACKET) && state.brushSize > 1) {
			state.brushSize--;
		}
		if (IsKeyPressed(KEY_RIGHT_BRACKET) && state.brushSize < 20) { // Set a maximum brush size
			state.brushSize++;
		}

		// Calculate dimensions
		int stageWidth = state.width * pixelSize;
		int stageHeight = state.height * pixelSize;
		int stageX = (screenWidth - stageWidth) / 2 + panX;
		int stageY = (screenHeight - stageHeight) / 2 + panY;

		// painting
		// TODO: fix corners
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !(IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_LEFT_CONTROL))) {
			compositeDirty = true;

			Vector2 mouseDelta = GetMouseDelta();
			Vector2 mousePos = GetMousePosition();

			// adjustments for offsets
			mousePos.x = (mousePos.x - stageX) / pixelSize;
			mousePos.y = state.height - (mousePos.y - stageY) / pixelSize; // Flip Y-coordinate

			// paint into render texture
			if (!isPainting) {
				state.paintBuffer = LoadRenderTexture(state.width, state.height);
				BeginTextureMode(state.paintBuffer);
				DrawRectangle((int) mousePos.x - state.brushSize / 2, (int) mousePos.y - state.brushSize / 2,
					      state.brushSize, state.brushSize, state.palette[state.paletteCurrent]);
				EndTextureMode();
				isPainting = true;
			} else {
				BeginTextureMode(state.paintBuffer);
				// draw a line from previous position to current position
				Vector2 startPos = {
				    mousePos.x - mouseDelta.x / pixelSize,
				    mousePos.y + mouseDelta.y / pixelSize // Note the '+' here to flip the delta
				};
				DrawLineEx(startPos, mousePos, state.brushSize, state.palette[state.paletteCurrent]);
				EndTextureMode();
			}
		} else if (isPainting) {
			// flush paintBuffer to selected layer
			Image layerImage = LoadImageFromTexture(state.layers[state.layerCurrent].texture);
			Image bufferImage = LoadImageFromTexture(state.paintBuffer.texture);

			ImageDraw(&layerImage, bufferImage, (Rectangle){0, 0, state.width, state.height},
				  (Rectangle){0, 0, state.width, state.height}, WHITE);

			UnloadTexture(state.layers[state.layerCurrent].texture);
			state.layers[state.layerCurrent].texture = LoadTextureFromImage(layerImage);

			isPainting = false;
			UnloadRenderTexture(state.paintBuffer);
			UnloadImage(bufferImage);
			UnloadImage(layerImage);
		}

		// compositing
		if (compositeDirty) {
			BeginTextureMode(composite);
			DrawTextureRec(substageTexture, (Rectangle){0, 0, state.width, state.height}, (Vector2){0, 0}, WHITE);
			for (int i = 0; i < state.layerCount; i++) {
				if (state.layers[i].visible) {
					Texture2D* tex = &state.layers[i].texture;
					DrawTextureRec(*tex, (Rectangle){0, 0, state.width, state.height}, (Vector2){0, 0},
						       WHITE);
					// inject paint buffer into current layer
					if (i == state.layerCurrent) {
						DrawTextureRec(state.paintBuffer.texture,
							       (Rectangle){0, 0, state.width, state.height}, (Vector2){0, 0},
							       WHITE);
					}
				}
			}

			EndTextureMode();
			compositeDirty = false;
		}

		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			// 0,0 is in the bottom left so we must render the composite with negative height
			DrawTexturePro(composite.texture, (Rectangle){0, 0, state.width, -state.height},
				       (Rectangle){stageX, stageY, stageWidth, stageHeight}, (Vector2){0, 0}, 0.0f, WHITE);

			char debugText[1024];
			sprintf(
			    debugText,
			    "fps:%d\n\npixel size:%d\n\nwidth:%d\n\nheight:%d\n\naspect:%0.2f\n\nlayers:%d\n\nbrush size:%d",
			    GetFPS(), pixelSize, stageWidth, stageHeight, (float) stageWidth / (float) stageHeight,
			    state.layerCount, state.brushSize);
			DrawText(debugText, 24, 24, 24, BLACK);

			// palette
			int paletteX = screenWidth / 2 - (432 / 2);
			int paletteY = screenHeight - 48;
			DrawRectangle(paletteX - 2, paletteY - 2, 436, 52, GRAY);
			for (int i = 0; i < 36; i++) {
				int paletteOffset = (i % 2 != 0) ? 12 : 0;
				Rectangle paletteSelectionRect = {paletteX + (i * 12) - paletteOffset,
								  paletteY + (2 * paletteOffset), 24, 24};
				if (CheckCollisionPointRec(GetMousePosition(), paletteSelectionRect) &&
				    IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !GetKeyPressed()) {
					state.paletteCurrent = i;
				}
				DrawRectangleRec(paletteSelectionRect, state.palette[i]);
			}

			// cursor
			if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){0, 0, screenWidth, screenHeight})) {
				HideCursor();
				int curSize = state.brushSize * pixelSize;
				DrawRectangle(GetMouseX() - curSize / 2, GetMouseY() - curSize / 2, curSize, curSize, BLACK);
			} else {
				ShowCursor();
				SetMouseCursor(MOUSE_CURSOR_ARROW);
			}
		}
		EndDrawing();
	};

	// todo: unload all layers

	UnloadTexture(state.clipboard);
	UnloadRenderTexture(composite);
	CloseWindow();
	return 0;
}

// TODO:
//
// UI
// new/open/save/etc
// cute cursors
// palette loading and exporting
// palette last used colors and current selection
//
// layerswitching & layer browser
// merge layer down
// layer reordering
//
// canvas cropping / selection / clipboard
// resize image
//
// file dialog import
// paint bucket fill
