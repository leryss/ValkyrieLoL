#pragma once

/* Took from https://www.millisecond.com/support/docs/v6/html/language/scancodes.htm */
enum HKey {
	NO_KEY = 0, ESC, N_1, N_2, N_3, N_4, N_5, N_6, N_7, N_8, N_9, N_0, MINUS, EQUAL, BS, Tab, Q, W, E, R, T, Y, U, I, O, P, LBRACKET, RBRACKET, ENTER, CTRL, A, S, D, F, G,
	H, J, K, L, SEMICOLON, SINGLE_QUOTE, TILDE, LSHIFT, BACKSLASH, Z, X, C, V, B, N, M, COMMA, DOT, FRONTSLASH, RSHIFT, PRINT_SCREEN, ALT, SPACE, CAPS, F1, F2,
	F3, F4, F5, F6, F7, F8, F9, F10, NUM, SCROLL, HOME, UP, PAGE_UP, NUM_MINUS, LEFT, CENTER, RIGHT, PLUS, END, DOWN, PAGE_DOWN, INSERT, DEL
};

static const char* HKeyNames[] = {
	"None", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace", "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "Ctrl", "A", "S", "D", "F", "G",
	"H", "J", "K", "L", ";", "'", "`", "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "RShift", "PrtScrn", "Alt", "Space", "Caps", "F1", "F2",
	"F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num", "Scrl", "Home", "Num8", "PgUp", "NumMinus", "Num4", "Num5", "Num6", "NumPlus", "End", "NumDown", "PgDown", "Insert", "Del"
};