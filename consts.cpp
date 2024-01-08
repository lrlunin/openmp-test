#include <array>

class consts{
public:
    constexpr static int FRAME_WIDTH = 400;
    constexpr static int FRAME_HEIGHT = 400;
    constexpr static int LENGTH = FRAME_HEIGHT * FRAME_WIDTH;
    constexpr static float PEDESTAL_BUFFER_SIZE = 1000;
    constexpr static std::array<std::array<unsigned int, FRAME_HEIGHT>, FRAME_WIDTH> reorder_map{[]() constexpr {
        std::array<std::array<unsigned int, FRAME_HEIGHT>, FRAME_WIDTH> result;
        int nadc = 32;
        int adc_nr[32] = {300, 325, 350, 375, 300, 325, 350, 375,
        200, 225, 250, 275, 200, 225, 250, 275,
        100, 125, 150, 175, 100, 125, 150, 175,
        0, 25, 50, 75, 0, 25, 50, 75};
        int npackets = 40;
        int sc_width = 25;
        int sc_height = 200;
        int row;
        for (int ip = 0; ip < npackets; ip++) {
            for (int is = 0; is < 128; is++) {
                for (auto iadc = 0; iadc < nadc; iadc++) {
                    int i = 128 * ip + is;
                    int adc4 = (int)iadc / 4;
                    if (i < sc_width * sc_height) {
                        //  for (int i=0; i<sc_width*sc_height; i++) {
                        int col = adc_nr[iadc] + (i % sc_width);
                        if (adc4 % 2 == 0) {
                            row = 199 - i / sc_width;
                        } else {
                            row = 200 + i / sc_width;
                        }
                        result[row][col] = (nadc * i + iadc);
                    }
                }
            }
        }
        return result;
    }()};
};