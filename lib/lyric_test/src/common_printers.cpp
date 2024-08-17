
#include <lyric_test/common_printers.h>

namespace lyric_common {

    void PrintTo(const ModuleLocation &moduleLocation, std::ostream* os) {
        *os << moduleLocation.toString();
    }

    void PrintTo(const SymbolUrl &symbolUrl, std::ostream* os) {
        *os << symbolUrl.toString();
    }

    void PrintTo(const SymbolPath &symbolPath, std::ostream* os) {
        *os << symbolPath.toString();
    }
}
