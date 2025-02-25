// omult7.c

extern void write_image(char *filename);

// specify range of input values
static const uint64_t INPUT_START = 0UL;
static const uint64_t INPUT_END   = 65536UL;

static int result[65536UL];
__thread int test_input = 0;

const int close_enough = 6;
const int array_length = 1 + 2*close_enough;
int hist[array_length] = { 0,0,0,0,0,0, 0, 0,0,0,0,0,0 };

// **************************************************************************************
void test_pre(thread_context_t* threadContext, uint64_t input) {
    zuint8* memory = threadContext->machine.context;

    threadContext->machine.state.a = input & 255UL;
    memory[2] = (input / 256UL) & 255UL;

    test_input = input;
}

// **************************************************************************************
uint64_t test_post(thread_context_t* threadContext) {
    zuint8* memory = threadContext->machine.context;

    uint64_t high = threadContext->machine.state.a;

    result[test_input] = high;
    return high;
}

// **************************************************************************************
int is_correct(thread_context_t* threadContext, uint64_t input, uint64_t actual_result, uint64_t* expected) {
    uint64_t a = input & 255UL;
    uint64_t b = (input / 256UL) & 255UL;

    uint64_t e = (a*b) / 256;
    *expected = e;

    // log is 'close enough' to expected result
    int err = (int) actual_result- (int) e;
    int index = err + close_enough;

    // lock mutex, update hist, unlock mutex
    pthread_mutex_lock(&mutex);
    hist[index] += 1;
    pthread_mutex_unlock(&mutex);

    return abs(err) <= close_enough;
}


// **************************************************************************************
void test_cleanup()
{
    int from = -1;
    int to = 0;

    for(int index = 0; index < array_length; index++) {
        int err = index - close_enough;
        if (hist[index] > 0) {
            to = index;
            if (from < 0) {
                from = index;
            }
        }
    }

    if (from >= 0) {
        double deviation = 0.0;
        for(int index = from; index <= to; index++) {
            int err = index - close_enough;
            deviation += err*err*hist[index];
            printf("Error %d: %d\n", err, hist[index]);
        }
        deviation = sqrt(deviation);
        printf("Root-mean-square deviation: %.2f (smaller is better)\n", (float) deviation);
    }

    write_image("results/omult7.png");

/*
    FILE* out_file = fopen("in_outs.csv", "w");
    if (out_file) {
        fprintf(out_file, "a,b,a*b\n");
        for(uint64_t input = INPUT_START; input < INPUT_END; input++)
        {
            uint64_t a = input & 255UL;
            uint64_t b = (input / 256UL) & 255UL;

            uint64_t e = (a*b) / 256;

            fprintf(out_file, "%llu,%llu,%d,%d\n",
                    a,
                    b,
                    result[input],
                    (e-result[input])>1);
            if ((e-result[input])>5) {
                printf("%d\n", (e-result[input]));
            }
        }

        fclose(out_file);
        out_file = NULL;
    }
*/
}
