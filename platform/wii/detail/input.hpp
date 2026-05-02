#ifndef _INCLUDE_HPP
#define _INCLUDE_HPP

#include <gccore.h>
#include <wiiuse/wpad.h>

class CInput {
public:
    CInput(u32 padIndex) {
        mPadIndex = padIndex;
    }

    void calculate() {}
    void submit() {}
protected:
    u32 mPadIndex;
    u32 mButtonsDown;
	u32 mButtonsUp;
	u32 mButtonsHeld;

    void submitKey(u32 button, u32 key);
};

class CInputAxis: CInput {
public:
    using CInput::CInput;

    void calculate();
    void submit();
protected:
    using CInput::mPadIndex;
    using CInput::mButtonsDown;
    using CInput::mButtonsUp;
    using CInput::mButtonsHeld;
    s8 mAxisX;
    s8 mAxisY;
    u32 mAxisFlags;

    using CInput::submitKey;
    void submitAxisKey(s8 axis, u32 negativeButton, u32 positiveButton, s32 negativeKey, s32 positiveKey);
};

class CInputWiimote: CInput {
public:
    using CInput::CInput;

    void calculate();
    void submit();

    bool isHomeButtonPressed() { return mButtonsDown & WPAD_BUTTON_HOME; }
};

class CInputGC: CInputAxis {
public:
    using CInputAxis::CInputAxis;

    void calculate();
    void submit();
};

class CInputClassic: CInputAxis {
public:
    using CInputAxis::CInputAxis;

    void calculate();
    void submit();

    bool isHomeButtonPressed() { return mButtonsDown & CLASSIC_CTRL_BUTTON_HOME; }
private:
    u32 mButtonsLast;
};

#endif