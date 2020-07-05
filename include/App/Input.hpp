#pragma once

struct InputState {
    int left;
    int right;
    int up;
    int down;
    int lookup;
    int lookdown;

    int x;
    int y;
    int mouseClick;
};

extern InputState input;

