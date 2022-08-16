#ifndef securyt_hpp
#define securyt_hpp

class Rect {
  public:
    int x, y, width, height;
    void Paint(int border_sz);
    bool Inside(int x, int y);
};

class Button {
  public:
    char *text;
    int row = 0, col = 0, font_sz = 4;
    Rect rect;
    void Paint(void);
    void Layout(Rect &bounds);
};

typedef enum { Alarm_Unknown, Alarm_Arming, Alarm_Armed, Alarm_Home, Alarm_Disarmed } AlarmState;

#endif
