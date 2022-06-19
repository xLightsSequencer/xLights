#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>

int main(int argc, const char* argv[])
{
    std::filesystem::path dir;

    if (argc <= 1) {
        // just use the current folder
        dir = std::filesystem::path(".");
    } else {
        dir = std::filesystem::path(argv[1]);
    }

    auto outFilename = dir.string() + "/tod.xml";

    std::cout << outFilename << "\n";

    std::ofstream out(outFilename);

    out << "<!-- Caution this file is generated ... do not edit-->\n";
    out << "<tipofday>\n";

    for (auto const& it : std::filesystem::directory_iterator{ dir }) {
        if (it.is_regular_file()) {

            // we only process html files
            std::filesystem::path p(it);
            if (p.extension() == ".html") {

                // allocate a buffer to read in the whole file
                std::ifstream html(p);
                html.seekg(0, std::ios::end);
                size_t size = html.tellg();
                std::string buffer(size, ' ');
                html.seekg(0);
                html.read(&buffer[0], size); 

                // now find the tip tag
                std::regex tip_regex("xlights>\\s*(<tip.*/>)\\s*</xlights");
                auto tip_begin = std::sregex_iterator(buffer.begin(), buffer.end(), tip_regex);
                auto tip_end = std::sregex_iterator();

                if (tip_begin == tip_end) {
                    std::cout << p << " Did not contain tip tag. Ignored.\n";
                } else {
                    auto sub = tip_begin->begin();
                    ++sub;
                    out << "   " << sub->str() << "\n";
                    std::cout << p << " " << sub->str() << " OK\n";
                }
            } else {
                std::cout << p << " Ignored due to extension " << p.extension() << ".\n";
            }
        }
    }

    out << "</tipofday>";
    out.close();
}