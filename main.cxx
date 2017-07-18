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


int cp_count;

enum UniType {
    CP,
    POD
};

class Point;
class Unit;
class Pod;
class Collision;
class Checkpoint;

class Collision {
public:
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
    UniType type;

    Unit(int id, float r, UniType type):
        id(id), r(r), type(type)
    {
    }

    Collision* collision(Unit &u) {
        // Square of the distance
        float dist = distance2(u);

        // Sum of the radii squared
        float sr = (r + u.r)*(r + u.r);

        // We take everything squared to avoid calling sqrt uselessly. It is better for performances

        if (dist < sr) {
            // Objects are already touching each other. We have an immediate collision.
            return new Collision(this, &u, 0.0);
        }

        // Optimisation. Objects with the same speed will never collide
        if (vx == u.vx && vy == u.vy) {
            return NULL;
        }

        // We place ourselves in the reference frame of u. u is therefore stationary and is at (0,0)
        float x = x - u.x;
        float y = y - u.y;
        Point myp = Point(x, y);
        float vx = vx - u.vx;
        float vy = vy - u.vy;
        Point up = Point(0, 0);

        // We look for the closest point to u (which is in (0,0)) on the line described by our speed vector
        Point p = up.closest(myp, Point(x + vx, y + vy));

        // Square of the distance between u and the closest point to u on the line described by our speed vector
        float pdist = up.distance2(p);

        // Square of the distance between us and that point
        float mypdist = myp.distance2(p);

        // If the distance between u and this line is less than the sum of the radii, there might be a collision
        if (pdist < sr) {
         // Our speed on the line
            float length = sqrt(vx*vx + vy*vy);

            // We move along the line to find the point of impact
            float backdist = sqrt(sr - pdist);
            p.x = p.x - backdist * (vx / length);
            p.y = p.y - backdist * (vy / length);

            // If the point is now further away it means we are not going the right way, therefore the collision won't happen
            if (myp.distance2(p) > mypdist) {
                return NULL;
            }

            pdist = p.distance(myp);

            // The point of impact is further than what we can travel in one turn
            if (pdist > length) {
                return NULL;
            }

            // Time needed to reach the impact point
            float t = pdist / length;

            return new Collision(this, &u, t);
        }

        return NULL;
    }
};

class Checkpoint: public Unit {
public:
    Checkpoint(int id, float x, float y):
        Unit(id, 600, CP)
    {
        this->x = x;
        this->y = y;

        vx = 0;
        vy = 0;
    }

    void bounce(Unit*) {}
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
        Unit(id, POD_R, POD)
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

    void bounce(Unit *u) {
        if (u->type == CP) {
            bounce_with_cp();
        } else {
            bounce_with_pod(static_cast<Pod*>(u));
        }
    }

    void bounce_with_cp() {
        checked += 1;
        timeout = partner->timeout = 100;
        nextCPID = (nextCPID + 1) % cp_count;
    }

    void bounce_with_pod(Pod *pod) {
        // If a pod has its shield active its mass is 10 otherwise it's 1
        float m1 = ifthen(shield, 10, 1);
        float m2 = ifthen(pod->shield, 10, 1);
        float mcoeff = (m1 + m2) / (m1 * m2);

        float nx = x - pod->x;
        float ny = y - pod->y;

        // Square of the distance between the 2 pods. This value could be hardcoded because it is always 800²
        float nxnysquare = nx*nx + ny*ny;

        float dvx = vx - pod->vx;
        float dvy = vy - pod->vy;

        // fx and fy are the components of the impact vector. product is just there for optimisation purposes
        float product = nx*dvx + ny*dvy;
        float fx = (nx * product) / (nxnysquare * mcoeff);
        float fy = (ny * product) / (nxnysquare * mcoeff);

        // We apply the impact vector once
        vx -= fx / m1;
        vy -= fy / m1;
        pod->vx += fx / m2;
        pod->vy += fy / m2;

        // If the norm of the impact vector is less than 120, we normalize it to 120
        float impulse = sqrt(fx*fx + fy*fy);
        if (impulse < 120.0) {
            fx = fx * 120.0 / impulse;
            fy = fy * 120.0 / impulse;
        }

        // We apply the impact vector a second time
        vx -= fx / m1;
        vy -= fy / m1;
        pod->vx += fx / m2;
        pod->vy += fy / m2;

        // This is one of the rare places where a Vector class would have made the code more readable.
        // But this place is called so often that I can't pay a performance price to make it more readable.
    }
};

int main() {
    int laps;
    cin >> laps;
    cin >> cp_count;
    vector<pair<int, int>> checkpoints(cp_count);
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
