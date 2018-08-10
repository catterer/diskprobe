#include <include/Probe.hh>

namespace dprobe {
namespace probe {

auto factory(const std::string& probename, const ptree& pt, Queue& queue)
    -> std::unique_ptr<AbstractProbe>
{
    // TODO
    return std::make_unique<FileWriter>(probename, pt, queue);
}

void FileWriter::loop(Channel& chan, const ptree& options) {
    for (;;) {
        ::sleep(1);
        chan.send<message::Heartbeat>();
    }
}

}
}
