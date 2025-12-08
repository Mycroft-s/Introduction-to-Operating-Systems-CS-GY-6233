#include <stdio.h>

#define NUM_PAGES 12

int pages[NUM_PAGES] = {2, 5, 1, 3, 4, 1, 2, 1, 3, 5, 1, 4};

int simulate_lru(int num_frames, int final_frames[])
{
    int frames[10];
    int last_used[10];
    int time = 0, faults = 0;

    for (int i = 0; i < num_frames; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    for (int i = 0; i < NUM_PAGES; i++) {
        int page = pages[i];
        time++;

        int hit_index = -1;
        for (int j = 0; j < num_frames; j++) {
            if (frames[j] == page) {
                hit_index = j;
                break;
            }
        }

        if (hit_index != -1) {
            last_used[hit_index] = time;
        } else {
            faults++;
            int empty_index = -1;
            for (int j = 0; j < num_frames; j++) {
                if (frames[j] == -1) {
                    empty_index = j;
                    break;
                }
            }

            int replace_index;
            if (empty_index != -1) {
                replace_index = empty_index;
            } else {
                replace_index = 0;
                for (int j = 1; j < num_frames; j++) {
                    if (last_used[j] < last_used[replace_index]) {
                        replace_index = j;
                    }
                }
            }

            frames[replace_index] = page;
            last_used[replace_index] = time;
        }
    }

    for (int i = 0; i < num_frames; i++) {
        final_frames[i] = frames[i];
    }

    return faults;
}

int main(void)
{
    int final_frames_3[3];
    int final_frames_4[4];

    int faults_3 = simulate_lru(3, final_frames_3);
    int faults_4 = simulate_lru(4, final_frames_4);

    printf("Page trace: ");
    for (int i = 0; i < NUM_PAGES; i++) printf("%d ", pages[i]);
    printf("\n\n");

    printf("=== LRU with 3 frames ===\n");
    printf("Total page faults: %d\n", faults_3);
    printf("Final frames: ");
    for (int i = 0; i < 3; i++) printf("%d ", final_frames_3[i]);
    printf("\n\n");

    printf("=== LRU with 4 frames ===\n");
    printf("Total page faults: %d\n", faults_4);
    printf("Final frames: ");
    for (int i = 0; i < 4; i++) printf("%d ", final_frames_4[i]);
    printf("\n");

    return 0;
}
