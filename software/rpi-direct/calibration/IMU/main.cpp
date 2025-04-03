#include "comms.hpp"
#include "debug.hpp"
#include "types.hpp"

int main();
struct RawIMUData {
    types::i16 accel_1[3], gyro_1[3], accel_2[3], gyro_2[3];
};

static RawIMUData current_raw_data = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
};

void IMU_processor(const types::u8 *data, types::u16 data_len);

int main() {}
