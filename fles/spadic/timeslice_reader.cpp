//! \author Michael Krieger

#include <iostream>
#include "TimesliceInputArchive.hpp"
#include "TimesliceReader.hpp"

void print_message(const spadic::Message& m);

void read_timeslice_archive(const std::string& filename)
{
    fles::TimesliceInputArchive ar {filename};
    spadic::TimesliceReader r;

    while (auto p = ar.get()) {
        r.add_timeslice(*p);

        for (auto addr : r.sources()) {
            std::cout << "---- reader " << addr << " ----" << std::endl;
            while (auto mp = r.get_message(addr)) {
                print_message(*mp);
            }
        }
    }
}

void print_message(const spadic::Message& m)
{
    std::cout << "v: " << (m.is_valid() ? "o" : "x");
    std::cout << " / ts: " << m.timestamp();
    std::cout << " / samples (" << m.samples().size() << "):";
    for (auto x : m.samples()) {
        std::cout << " " << x;
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    if (!(argc > 1)) {
        std::cout << "Please specify path to .tsa file." << std::endl;
        return 1;
    }
    std::string filename;
    try {
        filename = argv[1];
        read_timeslice_archive(filename);
        return 0;
    }
    catch (boost::archive::archive_exception& e) {
        std::cout << "Could not read \"" << filename <<
                     "\" as Timeslice archive." << std::endl;
        return 1;
    }
}
