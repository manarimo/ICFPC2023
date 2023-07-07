#ifndef ICFPC2023_SIMULATED_ANNEALING_H
#define ICFPC2023_SIMULATED_ANNEALING_H

#include <cstdio>
#include <cmath>

namespace sa {
    class timer {
        public:
        void start() {
            origin = rdtsc();
        }
        
        inline double get_time() {
            return (rdtsc() - origin) * SECONDS_PER_CLOCK;
        }
        
        private:
        constexpr static double SECONDS_PER_CLOCK = 1 / 3.0e9;
        unsigned long long origin;
        
        inline static unsigned long long rdtsc() {
            unsigned long long lo, hi;
            __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
            return (hi << 32) | lo;
        }
    };

    class random {
        public:
        // [0, x)
        inline static unsigned get(unsigned x) {
            return ((unsigned long long)xorshift() * x) >> 32;
        }
        
        // [x, y]
        inline static unsigned get(unsigned x, unsigned y) {
            return get(y - x + 1) + x;
        }
        
        // [0, x] (x = 2^c - 1)
        inline static unsigned get_fast(unsigned x) {
            return xorshift() & x;
        }
        
        // [0.0, 1.0]
        inline static double probability() {
            return xorshift() * INV_MAX;
        }
        
        inline static bool toss() {
            return xorshift() & 1;
        }
        
        private:
        constexpr static double INV_MAX = 1.0 / 0xFFFFFFFF;
        
        inline static unsigned xorshift() {
            static unsigned x = 123456789, y = 362436039, z = 521288629, w = 88675123;
            unsigned t = x ^ (x << 11);
            x = y, y = z, z = w;
            return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
        }
    };

    class simulated_annealing {
        public:
        simulated_annealing();
        inline bool end();
        inline bool accept(double current_score, double next_score);
        void print() const;
        
        private:
        constexpr static bool MAXIMIZE = true;
        constexpr static int LOG_SIZE = 0xFFFF;
        constexpr static int UPDATE_INTERVAL = 0xFF;
        constexpr static double TIME_LIMIT = 10;
        constexpr static double START_TEMP = 1000;
        constexpr static double END_TEMP = 1e-9;
        constexpr static double TEMP_RATIO = (END_TEMP - START_TEMP) / TIME_LIMIT;
        double log_probability[LOG_SIZE + 1];
        long long iteration = 0;
        long long accepted = 0;
        long long rejected = 0;
        double time = 0;
        double temp = START_TEMP;
        timer sa_timer;
    };

    simulated_annealing::simulated_annealing() {
        sa_timer.start();
        for (int i = 0; i <= LOG_SIZE; i++) log_probability[i] = log(random::probability());
    }

    inline bool simulated_annealing::end() {
        iteration++;
        if ((iteration & UPDATE_INTERVAL) == 0) {
            time = sa_timer.get_time();
            temp = START_TEMP + TEMP_RATIO * time;
            return time >= TIME_LIMIT;
        } else {
            return false;
        }
    }

    inline bool simulated_annealing::accept(double current_score, double next_score) {
        double diff = (MAXIMIZE ? next_score - current_score : current_score - next_score);
        if (diff >= 0 || diff > log_probability[random::get_fast(LOG_SIZE)] * temp) {
            accepted++;
            return true;
        } else {
            rejected++;
            return false;
        }
    }

    void simulated_annealing::print() const {
        fprintf(stderr, "iteration: %lld\n", iteration);
        fprintf(stderr, "accepted: %lld\n", accepted);
        fprintf(stderr, "rejected: %lld\n", rejected);
    }
};

/*
int main() {
    simulated_annealing sa;
    while (!sa.end()) {
        double current_score = 100;
        double next_score = 100;
        if (sa.accept(current_score, next_score)) {
            // use
        } else {
            // not use
        }
    }
    
    return 0;
}*/

#endif //ICFPC2023_SIMULATED_ANNEALING_H