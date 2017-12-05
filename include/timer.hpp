#ifndef OCTOPUS_INCLUDE_TIMER_H_
#define OCTOPUS_INCLUDE_TIMER_H_

#define NUM_SEEK_THREADS 4
#define MAX_TIMER_RECORD 200

//namespace octopus {

static uint64_t NowMicros() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

enum TimerStep {
    BEGIN,
    READDIR,
    GETATTR,
    END
};

class Timer {

private:
    uint64_t timer_micros[MAX_TIMER_RECORD];
    uint64_t timer_count[MAX_TIMER_RECORD];
    std::string message[MAX_TIMER_RECORD];

public:
    Timer() {
        init();
    }

    ~Timer() {
    }

    void init() {
        message[READDIR] = "file system readdir";
        message[GETATTR] = "file system getattr";
        clear();
    }

    void Start(TimerStep step) {
        timer_micros[step] = NowMicros();
    }

    void Record(TimerStep step) {
        assert(timer_count[step] != 0);
        timer_micros[step] = NowMicros() - timer_micros[step];
        timer_count[step]++;
    }

    void clear() {
        for (int i = BEGIN; i < END; i++) {
            timer_micros[i] = timer_count[i] = 0;
        }
    }

    void AppendNumberTo(std::string* str, uint64_t num) {
        char buf[30];
        snprintf(buf, sizeof(buf), "%llu", (unsigned long long) num);
        str->append(buf);
    }

    void Show(TimerStep step) {
        std::string result;
        if (timer_count[step] > 0) {
            result.append(message[step]).append(" time interval: ");
            AppendNumberTo(&result, timer_micros[step]);
            cout<<result<<endl;
        }
    }
};

//}
//end of namespace octopus

#endif //OCTOPUS_INCLUDE_TIMER_H_