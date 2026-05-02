#include "input.hpp"

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogcsys.h>
#include <math.h>

#include "framewrk/frm_int.hpp"
#include "moonchild/mc.hpp"
#include "moonchild/globals.hpp"
#include "moonchild/prefs.hpp"

static constexpr s32 STICK_DEADZONE = 32;

void CInput::submitKey(u32 button, u32 key) {
    if ((mButtonsDown & button) == button) {
        framework_EventHandle(FW_KEYDOWN, key);
    }
    else if ((mButtonsUp & button) == button) {
        framework_EventHandle(FW_KEYUP, key);
    }
}

void CInputAxis::submitAxisKey(s8 axis, u32 negativeButton, u32 positiveButton, s32 negativeKey, s32 positiveKey) {
    if (axis <= -STICK_DEADZONE || axis >= STICK_DEADZONE) {
        if (axis <= -STICK_DEADZONE && !(mAxisFlags & negativeButton)) {
            framework_EventHandle(FW_KEYDOWN, negativeKey);
            mAxisFlags |= negativeButton;
        }
        if (axis >= STICK_DEADZONE && !(mAxisFlags & positiveButton)) {
            framework_EventHandle(FW_KEYDOWN, positiveKey);
            mAxisFlags |= positiveButton;
        }
    }
    else {
        if ((mAxisFlags & negativeButton) == negativeButton) {
            framework_EventHandle(FW_KEYUP, negativeKey);
        }
        else if ((mAxisFlags & positiveButton) == positiveButton) {
            framework_EventHandle(FW_KEYUP, positiveKey);
        }
        mAxisFlags &= ~(negativeButton | positiveButton);
    }
}

void CInputWiimote::calculate() {
    mButtonsDown = WPAD_ButtonsDown(mPadIndex);
    mButtonsUp = WPAD_ButtonsUp(mPadIndex);
    mButtonsHeld = WPAD_ButtonsHeld(mPadIndex);
}

void CInputWiimote::submit() {
    submitKey(WPAD_BUTTON_LEFT, KEY_DOWN);
	submitKey(WPAD_BUTTON_RIGHT, KEY_UP);

	submitKey(WPAD_BUTTON_DOWN, KEY_RIGHT);
	submitKey(WPAD_BUTTON_UP, KEY_LEFT);

	submitKey(WPAD_BUTTON_2, KEY_SHOOT);
	submitKey(WPAD_BUTTON_1, KEY_USE);

	submitKey(WPAD_BUTTON_PLUS, KEY_QUIT);
}

void CInputGC::calculate() {
    mButtonsDown = PAD_ButtonsDown(mPadIndex);
	mButtonsUp = PAD_ButtonsUp(mPadIndex);
	mButtonsHeld = PAD_ButtonsHeld(mPadIndex);
	mAxisX = PAD_StickX(mPadIndex);
	mAxisY = PAD_StickY(mPadIndex);
}

void CInputGC::submit() {
    submitAxisKey(mAxisX, PAD_BUTTON_LEFT, PAD_BUTTON_RIGHT, KEY_LEFT, KEY_RIGHT);
	submitAxisKey(mAxisY, PAD_BUTTON_DOWN, PAD_BUTTON_UP, KEY_DOWN, KEY_UP);

	submitKey(PAD_BUTTON_DOWN, KEY_DOWN);
	submitKey(PAD_BUTTON_UP, KEY_UP);

	submitKey(PAD_BUTTON_RIGHT, KEY_RIGHT);
	submitKey(PAD_BUTTON_LEFT, KEY_LEFT);

	submitKey(PAD_BUTTON_A, KEY_SHOOT);
	submitKey(PAD_BUTTON_B, KEY_USE);

	submitKey(PAD_BUTTON_START, KEY_QUIT);
}

static f32 calcClassicStick(const joystick_t *joystick, bool axisY) {
	f32 angle = 3.1415926535f * joystick->ang / 180.0f;

	f32 magnitude = joystick->mag;
	if (magnitude < -1.0f) {
		magnitude = -1.0f;
	}
	else if (magnitude > 1.0f) {
		magnitude = 1.0f;
	}

	return magnitude * (axisY ? cosf(angle) : sinf(angle));
}

void CInputClassic::calculate() {
    mButtonsDown = 0;
    mButtonsUp = 0;
    mButtonsHeld = 0;
    mAxisX = 0;
    mAxisY = 0;

    expansion_t wpadExtension;
	WPAD_Expansion(mPadIndex, &wpadExtension);

	switch (wpadExtension.type) {
        case EXP_CLASSIC: {
            classic_ctrl_t classic = wpadExtension.classic;
        
            if (classic.ljs.mag != 0.0f) {
                f32 axisX = calcClassicStick(&classic.ljs, false);
                f32 axisY = calcClassicStick(&classic.ljs, true);

                axisX *= 128.0f;
                axisY *= 128.0f;

                if (axisX > 127.0f) {
                    axisX = 127.0f;
                }
                if (axisY > 127.0f) {
                    axisY = 127.0f;
                }

                mAxisX = (u8)axisX;
                mAxisY = (u8)axisY;
            }

            static const u32 buttonMapping[] = {
                CLASSIC_CTRL_BUTTON_A,
                CLASSIC_CTRL_BUTTON_B,
                CLASSIC_CTRL_BUTTON_DOWN,
                CLASSIC_CTRL_BUTTON_UP,
                CLASSIC_CTRL_BUTTON_RIGHT,
                CLASSIC_CTRL_BUTTON_LEFT,
                CLASSIC_CTRL_BUTTON_PLUS,
                CLASSIC_CTRL_BUTTON_HOME
            };

            /*
            * LIBOGC is bugged; the classic controller button state (besides btns) is extremely
            * unreliable, so we need to maintain some external state (mButtonsLast) to
            * keep things from going haywire
            */

            for (u32 i = 0; i < (sizeof(buttonMapping) / sizeof(buttonMapping[0])); i++) {
                u32 button = buttonMapping[i];

                if ((classic.btns & button) != 0) {
                    if ((mButtonsLast & button) == 0) {
                        mButtonsDown |= button;
                    }
                    else {
                        mButtonsHeld |= button;
                    }
                }
                else if (((mButtonsLast & button) != 0) && ((classic.btns & button) == 0)) {
                    mButtonsUp |= button;
                }
            }

            mButtonsLast = classic.btns;
        } break;

        case EXP_NONE:
        default:
            return;
	}
}

void CInputClassic::submit() {
    submitAxisKey(mAxisX, CLASSIC_CTRL_BUTTON_LEFT, CLASSIC_CTRL_BUTTON_RIGHT, KEY_LEFT, KEY_RIGHT);
	submitAxisKey(mAxisY, CLASSIC_CTRL_BUTTON_DOWN, CLASSIC_CTRL_BUTTON_UP, KEY_DOWN, KEY_UP);

    submitKey(CLASSIC_CTRL_BUTTON_DOWN, KEY_DOWN);
	submitKey(CLASSIC_CTRL_BUTTON_UP, KEY_UP);

	submitKey(CLASSIC_CTRL_BUTTON_RIGHT, KEY_RIGHT);
	submitKey(CLASSIC_CTRL_BUTTON_LEFT, KEY_LEFT);

	submitKey(CLASSIC_CTRL_BUTTON_A, KEY_SHOOT);
	submitKey(CLASSIC_CTRL_BUTTON_B, KEY_USE);

	submitKey(CLASSIC_CTRL_BUTTON_PLUS, KEY_QUIT);
}