#include <bits/stdc++.h>
using namespace std;

#include "point.cxx"

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