#include "wii/detail/input.hpp"

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
        if (axis <= -STICK_DEADZONE && (mAxisButton & negativeButton) == 0) {
            framework_EventHandle(FW_KEYDOWN, negativeKey);
            mAxisButton |= negativeButton;
        }
        if (axis >= STICK_DEADZONE && (mAxisButton & positiveButton) == 0) {
            framework_EventHandle(FW_KEYDOWN, positiveKey);
            mAxisButton |= positiveButton;
        }
    }
    else {
        if ((mAxisButton & negativeButton) == negativeButton) {
            framework_EventHandle(FW_KEYUP, negativeKey);
        }
        else if ((mAxisButton & positiveButton) == positiveButton) {
            framework_EventHandle(FW_KEYUP, positiveKey);
        }
        mAxisButton &= ~(negativeButton | positiveButton);
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
    expansion_t wpadExtension;
	WPAD_Expansion(mPadIndex, &wpadExtension);

	switch (wpadExtension.type) {
        case EXP_CLASSIC: {
            const classic_ctrl_t *classic = &wpadExtension.classic;
        
            if (classic->ljs.mag != 0.0f) {
                f32 axisX = calcClassicStick(&classic->ljs, false);
                f32 axisY = calcClassicStick(&classic->ljs, true);

                axisX *= 128.0f;
                axisY *= 128.0f;
                
                axisX += 0.5f;
                axisY += 0.5f;

                if (axisX < -128.0f) axisX = -128.0f;
                if (axisX >  127.0f) axisX =  127.0f;

                if (axisY < -128.0f) axisY = -128.0f;
                if (axisY >  127.0f) axisY =  127.0f;

                mAxisX = static_cast<s8>(axisX);
                mAxisY = static_cast<s8>(axisY);
            }
            else {
                mAxisX = 0;
                mAxisY = 0;
            }

            mButtonsHeld = classic->btns;
            mButtonsDown = mButtonsHeld & ~mButtonsLast;
            mButtonsUp = ~mButtonsHeld & mButtonsLast;

            mButtonsLast = mButtonsHeld;
        } break;

        default:
        case EXP_NONE: {
            mAxisX = 0;
            mAxisY = 0;
            mButtonsDown = 0;
            mButtonsUp = 0;
            mButtonsHeld = 0;
            mButtonsLast = 0;
        } break;
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
