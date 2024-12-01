#include "timestamp.h"
#include <time.h>
Timestamp::Timestamp():microSecondsSinceEpoch_(0) {

}
Timestamp::Timestamp(int64_t microSecondsSinceEpoch):microSecondsSinceEpoch_(microSecondsSinceEpoch) {

}

Timestamp Timestamp::now() {
    return Timestamp(time(NULL));
}

std::string Timestamp::toString() const {
    char buf[128] = {0};
    tm* t = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, 
        "%4d/%02d/%02d %02d:%02d:%02d",
        t->tm_year + 1900,
        t->tm_mon,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec);
    return buf;
}

// int main() {
//     std::cout << Timestamp::now().toString() << std::endl;
//     return 0;
// }