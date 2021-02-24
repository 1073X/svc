
#include "sample.hpp"

namespace miu::svc {
furnace* create() {
    return new sample::furnace();
}
}    // namespace miu::svc
