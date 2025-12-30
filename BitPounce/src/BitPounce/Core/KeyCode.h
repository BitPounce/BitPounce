#include "BitPounce/Core/Base.h"


enum class Key : uint16_t {
    // Row 1
    Esc = 1, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    // Row 2: number row + symbols + Backspace
    Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9, Key0,
    Minus, Equal, Backspace,
    
    // Row 3: Tab + Q-P + brackets + backslash
    Tab, Q, W, E, R, T, Y, U, I, O, P, LeftBracket, RightBracket, Backslash,
    
    // Row 4: CapsLock + A-L + semicolon + apostrophe + Enter
    CapsLock, A, S, D, F, G, H, J, K, L, Semicolon, Apostrophe, Enter,
    
    // Row 5: Shift + Z-M + comma + period + slash + Shift
    LeftShift, Z, X, C, V, B, N, M, Comma, Period, Slash, RightShift,
    
    // Row 6: Ctrl + Win + Alt + Space + Alt + Win + Menu + Ctrl
    LeftCtrl, LeftWin, LeftAlt, Space, RightAlt, RightWin, Menu, RightCtrl,
    
    // Arrow / navigation cluster
    Insert, Home, PageUp, Delete, End, PageDown, UpArrow, LeftArrow, DownArrow, RightArrow,
    
    // Numpad
    NumLock, NumpadDiv, NumpadMul, NumpadSub,
    Numpad7, Numpad8, Numpad9, NumpadAdd,
    Numpad4, Numpad5, Numpad6,
    Numpad1, Numpad2, Numpad3,
    Numpad0, NumpadDot, NumpadEnter
};
