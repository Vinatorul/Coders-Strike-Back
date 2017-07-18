#pragma GCC optimize("Ofast")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

#include <bits/stdc++.h>
using namespace std;

#define ifthen(x, y, z) (x ? y : z)

const float POD_R = 400;
const int POD_TIMEOUT = 100;
// Can't turn by more than 18° in one turn
const float MAX_ANGLE = 18.0;


class Point;
class Unit;
class Pod;
class Collision;

class Collision {
    Unit* a;
    Unit* b;
    float t;

    Collision() {}

    Collision(Unit* a, Unit* b, float t):
        a(a), b(b), t(t)
    {
    }
};

class Point {
public:
    float x, y;

    Point() {};

    Point(float x, float y):
        x(x), y(y)
    {
    }

    inline float distance2(const Point &p) {
        return (x - p.x)*(x - p.x) + (y - p.y)*(y - p.y);
    }

    inline float distance(const Point &p) {
        return sqrt(distance2(p));
    }

    Point closest(const Point &a, const Point &b) {
        float da = b.y - a.y;
        float db = a.x - b.x;
        float c1 = da*a.x + db*a.y;
        float c2 = -db*x + da*y;
        float det = da*da + db*db;
        float cx, cy;

        if (det != 0) {
            cx = (da*c1 - db*c2) / det;
            cy = (da*c2 + db*c1) / det;
        } else {
            cx = x;
            cy = y;
        }

        return Point(cx, cy);
    }
};

class Unit: public Point {
public:
    int id;
    float r;
    float vx, vy;

    Unit(int id, float r):
        id(id), r(r)
    {
    }

    Collision collision(const Unit &u) {

    }
};

class Pod: public Unit {
public:
    float angle;
    int nextCPID;
    int timeout;
    Pod* partner;
    int checked;
    bool shield;

    Pod(int id):
        angle(-1),
        nextCPID(-1),
        timeout(POD_TIMEOUT),
        partner(NULL),
        checked(0),
        shield(0),
        Unit(id, POD_R)
    {
    }

    float getAngle(const Point &p) {
        float d = distance(p);
        float dx = (p.x - x) / d;
        float dy = (p.y - y) / d;

        // Simple trigonometry. We multiply by 180.0 / PI to convert radiants to degrees.
        float a = acos(dx) * 180.0 / M_PI;

        // If the point I want is below me, I have to shift the angle for it to be correct
        if (dy < 0) {
            a = 360.0 - a;
        }

        return a;
    }

    float diffAngle(const Point &p) {
        float a = getAngle(p);

        // To know whether we should turn clockwise or not we look at the two ways and keep the smallest
        // The ternary operators replace the use of a modulo operator which would be slower
        float right = ifthen(angle <= a, a - angle, 360.0 - angle + a);
        float left = ifthen(angle >= a, angle - a, angle + 360.0 - a);

        if (right < left) {
            return right;
        } else {
            // We return a negative angle if we must rotate to left
            return -left;
        }
    }

    void rotate(const Point &p) {
        float a = diffAngle(p);

        if (a > MAX_ANGLE) {
            a = MAX_ANGLE;
        } else if (a < -MAX_ANGLE) {
            a = -MAX_ANGLE;
        }

        angle += a;

        // The % operator is slow. If we can avoid it, it's better.
        if (angle >= 360.0) {
            angle = angle - 360.0;
        } else if (angle < 0.0) {
            angle += 360.0;
        }
    }

    void boost(int thrust) {
      // Don't forget that a pod which has activated its shield cannot accelerate for 3 turns
        if (shield) {
            return;
        }

        // Conversion of the angle to radiants
        float ra = angle * M_PI / 180.0;

        // Trigonometry
        vx += cos(ra) * thrust;
        vy += sin(ra) * thrust;
    }

    void move(float t) {
        x += vx * t;
        y += vy * t;
    }

    void end() {
        x = round(x);
        y = round(y);
        vx = trunc(vx * 0.85);
        vy = trunc(vy * 0.85);

        // Don't forget that the timeout goes down by 1 each turn. It is reset to 100 when you pass a checkpoint
        timeout -= 1;
    }

    void play(Point &p, int thrust) {
        rotate(p);
        boost(thrust);
        move(1.0);
        end();
    }
};

int main() {
    int laps;
    cin >> laps;
    int checkpoint_count;
    cin >> checkpoint_count;
    vector<pair<int, int>> checkpoints(checkpoint_count);
    for (auto &i : checkpoints)
        cin >> i.first >> i.second; 
    while (1) {
        int fp_x;
        int fp_y;
        int fp_vx; // x position of the next check point
        int fp_vy; // y position of the next check point
        int fp_angle; // distance to the next checkpoint
        int fp_next_cp_id; // angle between your pod orientation and the direction of the next checkpoint
        cin >> fp_x >> fp_y >> fp_vx >> fp_vy >> fp_angle >> fp_next_cp_id;
        int sp_x;
        int sp_y;
        int sp_vx; // x position of the next check point
        int sp_vy; // y position of the next check point
        int sp_angle; // distance to the next checkpoint
        int sp_next_cp_id; // angle between your pod orientation and the direction of the next checkpoint
        cin >> sp_x >> sp_y >> sp_vx >> sp_vy >> sp_angle >> sp_next_cp_id;
        
        cout << checkpoints[fp_next_cp_id].first - 3*fp_vx << " " << checkpoints[fp_next_cp_id].second - 3*fp_vy << " BOOST" << endl;
        cout << checkpoints[sp_next_cp_id].first - 3*sp_vx << " " << checkpoints[sp_next_cp_id].second - 3*sp_vy << " BOOST" << endl;
    
        cin >> sp_x >> sp_y >> sp_vx >> sp_vy >> sp_angle >> sp_next_cp_id;
        cin >> sp_x >> sp_y >> sp_vx >> sp_vy >> sp_angle >> sp_next_cp_id;
    }
}
