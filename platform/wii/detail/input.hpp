#ifndef _WII_DETAIL_INPUT_HPP
#define _WII_DETAIL_INPUT_HPP

#include <gccore.h>
#include <wiiuse/wpad.h>

class CInput {
public:
    CInput(u32 padIndex) {
        mPadIndex = padIndex;

        mButtonsDown = 0;
        mButtonsUp = 0;
        mButtonsHeld = 0;
    }
    virtual ~CInput(void) {}

    virtual void calculate() = 0;
    virtual void submit() = 0;

protected:
    void submitKey(u32 button, u32 key);

protected:
    u32 mPadIndex;
    u32 mButtonsDown;
	u32 mButtonsUp;
	u32 mButtonsHeld;
};

class CInputAxis : public CInput {
public:
    CInputAxis(u32 padIndex) :
        CInput(padIndex)
    {
        mAxisX = 0;
        mAxisY = 0;
        mAxisButton = 0;
    }
    virtual ~CInputAxis() {}

    virtual void calculate() = 0;
    virtual void submit() = 0;

protected:
    void submitAxisKey(s8 axis, u32 negativeButton, u32 positiveButton, s32 negativeKey, s32 positiveKey);

protected:
    s8 mAxisX;
    s8 mAxisY;
    u32 mAxisButton;
};

class CInputWiimote : public CInput {
public:
    CInputWiimote(u32 padIndex) :
        CInput(padIndex)
    {}
    virtual ~CInputWiimote() {}

    virtual void calculate();
    virtual void submit();

    bool isHomeButtonPressed() {
        return (mButtonsDown & WPAD_BUTTON_HOME) != 0;
    }
};

class CInputGC : public CInputAxis {
public:
    CInputGC(u32 padIndex) :
        CInputAxis(padIndex)
    {}
    virtual ~CInputGC() {}

    virtual void calculate();
    virtual void submit();
};

class CInputClassic : public CInputAxis {
public:
    CInputClassic(u32 padIndex) :
        CInputAxis(padIndex)
    {
        mButtonsLast = 0;
    }
    virtual ~CInputClassic() {}

    virtual void calculate();
    virtual void submit();

    bool isHomeButtonPressed() {
        return (mButtonsDown & CLASSIC_CTRL_BUTTON_HOME) != 0;
    }

private:
    u32 mButtonsLast;
};

#endif // _WII_DETAIL_INPUT_HPP
