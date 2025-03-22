#pragma once

class Pos {
  public:
    Pos(int x, int y) : x(x), y(y), heading(0) {}

    // the shorter part of the field
    int x;

    // the longer part of the field
    int y;

    // the heading of the robot, where 0 is facing the blue goal
    int heading;

    bool operator==(const Pos &other) const {
        return (x == other.x && y == other.y);
    }

    bool operator!=(const Pos &other) const { return !(*this == other); }
};