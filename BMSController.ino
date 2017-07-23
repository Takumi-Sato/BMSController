#include "HID-Project.h"

#define NoTurn 3
#define CWStart 4
#define CWContinue 5
#define CWEnd 6
#define CCWStart 7
#define CCWContinue 8
#define CCWEnd 9
#define INT_MAX 32767

int button1 = 2;
int button2 = 4;
int button3 = 6;
int button4 = 8;
int button5 = 10;
int button6 = 12;
int button7 = 13;
int scrA = A0;
int scrB = A2;
int start = 1;
int select = A4;

// 0, 1, 2, 3 のいずれか
//scrR : 0 -> 1 -> 3 -> 2 -> 0
//scrL : 0 -> 2 -> 3 -> 1 -> 0
int scrPre = 0;
int scrNow = 0;

int clockState = NoTurn;
int preClockState = NoTurn;

bool CWScratchedFlg = false;
bool CCWScratchedFlg = false;

bool PreScratchedFlg = false;
int t = 0;
const int T = 128;

void setup() {
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
  pinMode(button6, INPUT_PULLUP);
  pinMode(button7, INPUT_PULLUP);
  pinMode(scrA, INPUT_PULLUP);
  pinMode(scrB, INPUT_PULLUP);
  pinMode(start, INPUT_PULLUP);
  pinMode(select, INPUT_PULLUP);
  NKROKeyboard.begin();
}

void loop() {
  if (!digitalRead(button1)) NKROKeyboard.add('0');
  else                      NKROKeyboard.remove('0');
  if (!digitalRead(button2)) NKROKeyboard.add('s');
  else                      NKROKeyboard.remove('s');
  if (!digitalRead(button3)) NKROKeyboard.add('x');
  else                      NKROKeyboard.remove('x');
  if (!digitalRead(button4)) NKROKeyboard.add('d');
  else                      NKROKeyboard.remove('d');
  if (!digitalRead(button5)) NKROKeyboard.add('c');
  else                      NKROKeyboard.remove('c');
  if (!digitalRead(button6)) NKROKeyboard.add('f');
  else                      NKROKeyboard.remove('f');
  if (!digitalRead(button7)) NKROKeyboard.add('v');
  else                      NKROKeyboard.remove('v');
  if (!digitalRead(start))   NKROKeyboard.add('q');
  else                      NKROKeyboard.remove('q');
  if (!digitalRead(select))  NKROKeyboard.add('w');
  else                      NKROKeyboard.remove('w');

  // ロータリーエンコーダーの値を変換
  EncodeRotaryEncoder();

  // 現在のロータリーエンコーダーの値からわかる実際の状態
  GetRealState();

  if (clockState != NoTurn && clockState != CWEnd && clockState != CCWEnd)
  {
    // CCW中にCW入力
    if (CWScratchedFlg && ((preClockState == CCWStart) || (preClockState == CCWContinue)))
    {
      GetClockState();
      PushOrReleaseScr();
      //ResetClockCnt();
    }
    // CW中にCCW入力
    else if (CCWScratchedFlg && ((preClockState == CWStart) || (preClockState == CWContinue)))
    {
      GetClockState();
      PushOrReleaseScr();
      //ResetClockCnt();
    }
    else if (t > T)
    {
      // ScratchedFlg と preClockState から clockState を求める
      // ただしこれは一定の周期(clock)を満たしたときのみ行われる処理
      GetClockState();

      // clockState からわかる、行うべき処理を行う
      PushOrReleaseScr();

      t = 0;
      CWScratchedFlg = false;
      CCWScratchedFlg = false;
    }
    else
    {
      ++t;

      //CWScratchedFlg = false;
      //CCWScratchedFlg = false;

      scrPre = scrNow;

      NKROKeyboard.send();

      return;
    }
  }
  else {
    GetClockState();
    PushOrReleaseScr();
  }

  CWScratchedFlg = false;
  CCWScratchedFlg = false;

  scrPre = scrNow;

  NKROKeyboard.send();
}

void ResetClockCnt()
{
  t = 0;
}


void EncodeRotaryEncoder()
{

  if (!digitalRead(scrA))
  {
    if (!digitalRead(scrB))   scrNow = 3;
    else                      scrNow = 1;
  }
  else
  {
    if (!digitalRead(scrB))   scrNow = 2;
    else                      scrNow = 0;
  }
}

void GetRealState()
{
  switch (scrPre)
  {
    case 0:
      if (scrNow == 1)        CWScratchedFlg = true;
      else if (scrNow == 2)   CCWScratchedFlg = true;
      break;
    case 1:
      if (scrNow == 3)        CWScratchedFlg = true;
      else if (scrNow == 0)   CCWScratchedFlg = true;
      break;
    case 2:
      if (scrNow == 0)        CWScratchedFlg = true;
      else if (scrNow == 3)   CCWScratchedFlg = true;
      break;
    case 3:
      if (scrNow == 2)        CWScratchedFlg = true;
      else if (scrNow == 1)   CCWScratchedFlg = true;
      break;
    default:
      break;
  }
}

void GetClockState()
{
  switch (preClockState)
  {
    case NoTurn:
      if (CWScratchedFlg)         clockState = CWStart;
      else if (CCWScratchedFlg)   clockState = CCWStart;
      else                        clockState = NoTurn;
      break;

    case CWStart:
      if (CWScratchedFlg)         clockState = CWContinue;
      else if (CCWScratchedFlg)   clockState = CCWStart;
      else                        clockState = CWEnd;
      break;
    case CWContinue:
      if (CWScratchedFlg)         clockState = CWContinue;
      else if (CCWScratchedFlg)   clockState = CCWStart;
      else                        clockState = CWEnd;
      break;
    case CWEnd:
      if (CWScratchedFlg)         clockState = CWStart;
      else if (CCWScratchedFlg)   clockState = CCWStart;
      else                        clockState = NoTurn;
      break;

    case CCWStart:
      if (CWScratchedFlg)         clockState = CWStart;
      else if (CCWScratchedFlg)   clockState = CCWContinue;
      else                        clockState = CCWEnd;
      break;
    case CCWContinue:
      if (CWScratchedFlg)         clockState = CWStart;
      else if (CCWScratchedFlg)   clockState = CCWContinue;
      else                        clockState = CCWEnd;
      break;
    case CCWEnd:
      if (CWScratchedFlg)         clockState = CWStart;
      else if (CCWScratchedFlg)   clockState = CCWStart;
      else                        clockState = NoTurn;
      break;

    default:
      clockState = NoTurn;
      break;
  }

  preClockState = clockState;
}

void PushOrReleaseScr()
{
  switch (clockState)
  {
    case NoTurn:
      NKROKeyboard.remove(KEY_LEFT_SHIFT);
      NKROKeyboard.remove(KEY_LEFT_CTRL);
      break;

    case CWStart:
      NKROKeyboard.remove(KEY_LEFT_CTRL);
      NKROKeyboard.add(KEY_LEFT_SHIFT);
      break;
    case CWContinue:
      NKROKeyboard.add(KEY_LEFT_SHIFT);
      break;
    case CWEnd:
      NKROKeyboard.remove(KEY_LEFT_SHIFT);
      break;

    case CCWStart:
      NKROKeyboard.remove(KEY_LEFT_SHIFT);
      NKROKeyboard.add(KEY_LEFT_CTRL);
      break;
    case CCWContinue:
      NKROKeyboard.add(KEY_LEFT_CTRL);
      break;
    case CCWEnd:
      NKROKeyboard.remove(KEY_LEFT_CTRL);
      break;
    default:
      NKROKeyboard.remove(KEY_LEFT_SHIFT);
      NKROKeyboard.remove(KEY_LEFT_CTRL);
      break;
  }
}

