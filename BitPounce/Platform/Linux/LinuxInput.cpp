#include "bp_pch.h"
#include "LinuxInput.h"
#include <GLFW/glfw3.h>
#include "BitPounce/Core/Application.h"

namespace BitPounce
{
	Input* Input::s_Instance = new LinuxInput;

	int CustomKeyToGLFW(Key key) {
        switch (key) {
            case Key::Esc: return GLFW_KEY_ESCAPE;
            case Key::F1: return GLFW_KEY_F1; case Key::F2: return GLFW_KEY_F2;
            case Key::F3: return GLFW_KEY_F3; case Key::F4: return GLFW_KEY_F4;
            case Key::F5: return GLFW_KEY_F5; case Key::F6: return GLFW_KEY_F6;
            case Key::F7: return GLFW_KEY_F7; case Key::F8: return GLFW_KEY_F8;
            case Key::F9: return GLFW_KEY_F9; case Key::F10: return GLFW_KEY_F10;
            case Key::F11: return GLFW_KEY_F11; case Key::F12: return GLFW_KEY_F12;

            case Key::Key1: return GLFW_KEY_1; case Key::Key2: return GLFW_KEY_2;
            case Key::Key3: return GLFW_KEY_3; case Key::Key4: return GLFW_KEY_4;
            case Key::Key5: return GLFW_KEY_5; case Key::Key6: return GLFW_KEY_6;
            case Key::Key7: return GLFW_KEY_7; case Key::Key8: return GLFW_KEY_8;
            case Key::Key9: return GLFW_KEY_9; case Key::Key0: return GLFW_KEY_0;
            case Key::Minus: return GLFW_KEY_MINUS; case Key::Equal: return GLFW_KEY_EQUAL;
            case Key::Backspace: return GLFW_KEY_BACKSPACE;

            case Key::Tab: return GLFW_KEY_TAB; case Key::Q: return GLFW_KEY_Q;
            case Key::W: return GLFW_KEY_W; case Key::E: return GLFW_KEY_E; case Key::R: return GLFW_KEY_R;
            case Key::T: return GLFW_KEY_T; case Key::Y: return GLFW_KEY_Y; case Key::U: return GLFW_KEY_U;
            case Key::I: return GLFW_KEY_I; case Key::O: return GLFW_KEY_O; case Key::P: return GLFW_KEY_P;
            case Key::LeftBracket: return GLFW_KEY_LEFT_BRACKET; case Key::RightBracket: return GLFW_KEY_RIGHT_BRACKET;
            case Key::Backslash: return GLFW_KEY_BACKSLASH;

            case Key::CapsLock: return GLFW_KEY_CAPS_LOCK; case Key::A: return GLFW_KEY_A; case Key::S: return GLFW_KEY_S;
            case Key::D: return GLFW_KEY_D; case Key::F: return GLFW_KEY_F; case Key::G: return GLFW_KEY_G;
            case Key::H: return GLFW_KEY_H; case Key::J: return GLFW_KEY_J; case Key::K: return GLFW_KEY_K;
            case Key::L: return GLFW_KEY_L; case Key::Semicolon: return GLFW_KEY_SEMICOLON; case Key::Apostrophe: return GLFW_KEY_APOSTROPHE;
            case Key::Enter: return GLFW_KEY_ENTER;

            case Key::LeftShift: return GLFW_KEY_LEFT_SHIFT; case Key::RightShift: return GLFW_KEY_RIGHT_SHIFT;
            case Key::Z: return GLFW_KEY_Z; case Key::X: return GLFW_KEY_X; case Key::C: return GLFW_KEY_C;
            case Key::V: return GLFW_KEY_V; case Key::B: return GLFW_KEY_B; case Key::N: return GLFW_KEY_N;
            case Key::M: return GLFW_KEY_M; case Key::Comma: return GLFW_KEY_COMMA; case Key::Period: return GLFW_KEY_PERIOD;
            case Key::Slash: return GLFW_KEY_SLASH;

            case Key::LeftCtrl: return GLFW_KEY_LEFT_CONTROL; case Key::RightCtrl: return GLFW_KEY_RIGHT_CONTROL;
            case Key::LeftAlt: return GLFW_KEY_LEFT_ALT; case Key::RightAlt: return GLFW_KEY_RIGHT_ALT;
            case Key::LeftWin: return GLFW_KEY_LEFT_SUPER; case Key::RightWin: return GLFW_KEY_RIGHT_SUPER;
            case Key::Menu: return GLFW_KEY_MENU; case Key::Space: return GLFW_KEY_SPACE;

            case Key::UpArrow: return GLFW_KEY_UP; case Key::DownArrow: return GLFW_KEY_DOWN;
            case Key::LeftArrow: return GLFW_KEY_LEFT; case Key::RightArrow: return GLFW_KEY_RIGHT;
            case Key::Insert: return GLFW_KEY_INSERT; case Key::Home: return GLFW_KEY_HOME;
            case Key::PageUp: return GLFW_KEY_PAGE_UP; case Key::Delete: return GLFW_KEY_DELETE;
            case Key::End: return GLFW_KEY_END; case Key::PageDown: return GLFW_KEY_PAGE_DOWN;

            case Key::NumLock: return GLFW_KEY_NUM_LOCK;
            case Key::NumpadDiv: return GLFW_KEY_KP_DIVIDE; case Key::NumpadMul: return GLFW_KEY_KP_MULTIPLY;
            case Key::NumpadSub: return GLFW_KEY_KP_SUBTRACT; case Key::NumpadAdd: return GLFW_KEY_KP_ADD;
            case Key::Numpad0: return GLFW_KEY_KP_0; case Key::Numpad1: return GLFW_KEY_KP_1;
            case Key::Numpad2: return GLFW_KEY_KP_2; case Key::Numpad3: return GLFW_KEY_KP_3;
            case Key::Numpad4: return GLFW_KEY_KP_4; case Key::Numpad5: return GLFW_KEY_KP_5;
            case Key::Numpad6: return GLFW_KEY_KP_6; case Key::Numpad7: return GLFW_KEY_KP_7;
            case Key::Numpad8: return GLFW_KEY_KP_8; case Key::Numpad9: return GLFW_KEY_KP_9;
            case Key::NumpadDot: return GLFW_KEY_KP_DECIMAL; case Key::NumpadEnter: return GLFW_KEY_KP_ENTER;

            default: return GLFW_KEY_UNKNOWN;
        }
    }

	// Convert GLFW key code → custom Key
    Key GLFWKeyToCustom(int glfwKey) {
        switch (glfwKey) {
            case GLFW_KEY_ESCAPE: return Key::Esc;
            case GLFW_KEY_F1: return Key::F1; case GLFW_KEY_F2: return Key::F2;
            case GLFW_KEY_F3: return Key::F3; case GLFW_KEY_F4: return Key::F4;
            case GLFW_KEY_F5: return Key::F5; case GLFW_KEY_F6: return Key::F6;
            case GLFW_KEY_F7: return Key::F7; case GLFW_KEY_F8: return Key::F8;
            case GLFW_KEY_F9: return Key::F9; case GLFW_KEY_F10: return Key::F10;
            case GLFW_KEY_F11: return Key::F11; case GLFW_KEY_F12: return Key::F12;

            case GLFW_KEY_1: return Key::Key1; case GLFW_KEY_2: return Key::Key2;
            case GLFW_KEY_3: return Key::Key3; case GLFW_KEY_4: return Key::Key4;
            case GLFW_KEY_5: return Key::Key5; case GLFW_KEY_6: return Key::Key6;
            case GLFW_KEY_7: return Key::Key7; case GLFW_KEY_8: return Key::Key8;
            case GLFW_KEY_9: return Key::Key9; case GLFW_KEY_0: return Key::Key0;
            case GLFW_KEY_MINUS: return Key::Minus; case GLFW_KEY_EQUAL: return Key::Equal;
            case GLFW_KEY_BACKSPACE: return Key::Backspace;

            case GLFW_KEY_TAB: return Key::Tab; case GLFW_KEY_Q: return Key::Q;
            case GLFW_KEY_W: return Key::W; case GLFW_KEY_E: return Key::E;
            case GLFW_KEY_R: return Key::R; case GLFW_KEY_T: return Key::T;
            case GLFW_KEY_Y: return Key::Y; case GLFW_KEY_U: return Key::U;
            case GLFW_KEY_I: return Key::I; case GLFW_KEY_O: return Key::O;
            case GLFW_KEY_P: return Key::P; case GLFW_KEY_LEFT_BRACKET: return Key::LeftBracket;
            case GLFW_KEY_RIGHT_BRACKET: return Key::RightBracket; case GLFW_KEY_BACKSLASH: return Key::Backslash;

            case GLFW_KEY_CAPS_LOCK: return Key::CapsLock; case GLFW_KEY_A: return Key::A;
            case GLFW_KEY_S: return Key::S; case GLFW_KEY_D: return Key::D; case GLFW_KEY_F: return Key::F;
            case GLFW_KEY_G: return Key::G; case GLFW_KEY_H: return Key::H; case GLFW_KEY_J: return Key::J;
            case GLFW_KEY_K: return Key::K; case GLFW_KEY_L: return Key::L;
            case GLFW_KEY_SEMICOLON: return Key::Semicolon; case GLFW_KEY_APOSTROPHE: return Key::Apostrophe;
            case GLFW_KEY_ENTER: return Key::Enter;

            case GLFW_KEY_LEFT_SHIFT: return Key::LeftShift; case GLFW_KEY_RIGHT_SHIFT: return Key::RightShift;
            case GLFW_KEY_Z: return Key::Z; case GLFW_KEY_X: return Key::X; case GLFW_KEY_C: return Key::C;
            case GLFW_KEY_V: return Key::V; case GLFW_KEY_B: return Key::B; case GLFW_KEY_N: return Key::N;
            case GLFW_KEY_M: return Key::M; case GLFW_KEY_COMMA: return Key::Comma; case GLFW_KEY_PERIOD: return Key::Period;
            case GLFW_KEY_SLASH: return Key::Slash;

            case GLFW_KEY_LEFT_CONTROL: return Key::LeftCtrl; case GLFW_KEY_RIGHT_CONTROL: return Key::RightCtrl;
            case GLFW_KEY_LEFT_ALT: return Key::LeftAlt; case GLFW_KEY_RIGHT_ALT: return Key::RightAlt;
            case GLFW_KEY_LEFT_SUPER: return Key::LeftWin; case GLFW_KEY_RIGHT_SUPER: return Key::RightWin;
            case GLFW_KEY_MENU: return Key::Menu;
            case GLFW_KEY_SPACE: return Key::Space;

            case GLFW_KEY_UP: return Key::UpArrow; case GLFW_KEY_DOWN: return Key::DownArrow;
            case GLFW_KEY_LEFT: return Key::LeftArrow; case GLFW_KEY_RIGHT: return Key::RightArrow;
            case GLFW_KEY_INSERT: return Key::Insert; case GLFW_KEY_HOME: return Key::Home;
            case GLFW_KEY_PAGE_UP: return Key::PageUp; case GLFW_KEY_DELETE: return Key::Delete;
            case GLFW_KEY_END: return Key::End; case GLFW_KEY_PAGE_DOWN: return Key::PageDown;

            case GLFW_KEY_NUM_LOCK: return Key::NumLock;
            case GLFW_KEY_KP_DIVIDE: return Key::NumpadDiv; case GLFW_KEY_KP_MULTIPLY: return Key::NumpadMul;
            case GLFW_KEY_KP_SUBTRACT: return Key::NumpadSub; case GLFW_KEY_KP_ADD: return Key::NumpadAdd;
            case GLFW_KEY_KP_0: return Key::Numpad0; case GLFW_KEY_KP_1: return Key::Numpad1;
            case GLFW_KEY_KP_2: return Key::Numpad2; case GLFW_KEY_KP_3: return Key::Numpad3;
            case GLFW_KEY_KP_4: return Key::Numpad4; case GLFW_KEY_KP_5: return Key::Numpad5;
            case GLFW_KEY_KP_6: return Key::Numpad6; case GLFW_KEY_KP_7: return Key::Numpad7;
            case GLFW_KEY_KP_8: return Key::Numpad8; case GLFW_KEY_KP_9: return Key::Numpad9;
            case GLFW_KEY_KP_DECIMAL: return Key::NumpadDot; case GLFW_KEY_KP_ENTER: return Key::NumpadEnter;

            default: return Key::Esc;
        }
    }

	bool LinuxInput::IsKeyPressedImpl(Key key)
	{
		int glfwKey = CustomKeyToGLFW(key);
		return glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), glfwKey);
	}

    bool LinuxInput::IsMouseButtonPressedImpl(int button)
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
    }

    glm::vec2 LinuxInput::GetMousePositionImpl()
    {
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
    }

    float LinuxInput::GetMouseXImpl()
    {
        auto pos = GetMousePositionImpl();
		return pos.x;
    }

    float LinuxInput::GetMouseYImpl()
    {
        auto pos = GetMousePositionImpl();
		return pos.y;
    }

    Key PlatformKeyToKey(uint32_t key)
    {
        return GLFWKeyToCustom(key);
    }
    uint32_t KeyToPlatformKey(Key key)
    {
        return CustomKeyToGLFW(key);
    }

}