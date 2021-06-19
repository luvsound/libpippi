#include "pippi.h"

int main() {
    lpbuffer_t * snd;

    snd = LPSoundFile.read("examples/linus.wav");
    LPSoundFile.write("renders/readsoundfile-out.wav", snd);
    LPBuffer.destroy(snd);

    return 0;
}
