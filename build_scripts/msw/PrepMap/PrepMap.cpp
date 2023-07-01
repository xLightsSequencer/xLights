// PrepMap.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>

void trim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));

    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(),
            s.end());
}

inline bool Contains(const std::string& in, const std::string& what)
{
    return in.find(what) != std::string::npos;
}

inline bool StartsWith(const std::string& in, const std::string& what)
{
    return in.size() >= what.size() && in.substr(0, what.size()) == what;
}

inline bool EndsWith(const std::string& in, const std::string& what)
{
    return in.size() >= what.size() && in.substr(in.size() - what.size(), what.size()) == what;
}

bool Replace(std::string& in, const std::string& what, const std::string& with, bool all)
{
    bool res = false;
    if (all) {
        while (Replace(in, what, with, false)) {
            res = true;
        }
    } else {
        auto pos = in.find(what);
        if (pos != std::string::npos) {
            in = in.substr(0, pos) + with + in.substr(pos + what.size());
            res = true;
        }
    }

    return res;
}

std::vector<std::string> Split(const std::string& in, char token)
{
    std::vector<std::string> res;
    std::istringstream sh(in);
    std::string s;
    while (std::getline(sh, s, token)) {
        res.push_back(s);
    }
    return res;
}

int main(int argc, char* argv[])
{
    int rc = 0;

    if (argc != 3) {
        printf("Usage: PrepMap <inputfile> <outputfile>\n");
        rc = 1;
    } else {
        std::string infile = std::string(argv[1]);
        std::string outfile = std::string(argv[2]);

        printf("Processing map file %s ==> %s.\n", infile.c_str(), outfile.c_str());

        std::ofstream ofh(outfile, std::ofstream::out);
        if (ofh.is_open()) {
            uint64_t preferedLoadAddress = 0;
            std::list<std::string> lines;
            char buffer[4096] = { 0 };
            uint64_t readCount = 0;
            std::ifstream ifh(infile, std::ifstream::in);
            if (ifh.is_open()) {
                printf("Reading...\n");
                while (rc == 0 && !ifh.eof()) {
                    std::string inl;
                    std::getline(ifh, inl);
                    ++readCount;

                    trim(inl);

                    if (Contains(inl, "Preferred load address is")) {
                        sscanf_s(inl.substr(28).c_str(), "%llx", &preferedLoadAddress);
                    }
                    else if (((StartsWith(inl, "0001:") || StartsWith(inl, "0002:")) && (EndsWith(inl, ".obj") || EndsWith(inl, ".o")))) {

                        if (preferedLoadAddress == 0) {
                            printf("Preferred load address not found.\n");
                            rc = 1;
                        }

                        // this is a line we want to keep
                        Replace(inl, "  ", " ", true);

                        auto comp = Split(inl, ' ');
                        if (comp.size() > 3) {
                            size_t ln = 0;
                            for (size_t i = 3; i < comp.size(); i++) {
                                if (comp[i].size() > 2) {
                                    ln = i;
                                    break;
                                }
                            }

                            uint64_t addr = 0;
                            sscanf_s(comp[2].c_str(), "%llx", &addr);
                            uint64_t actual_addr = addr - preferedLoadAddress;
                            if (actual_addr != 0 && addr != 0 && comp[1] != ".text" && !StartsWith(comp[1], "??__E")) {
                                sprintf_s(buffer, sizeof(buffer), "%016llx\t%s\t%s\n", actual_addr, comp[ln].c_str(), comp[1].c_str());
                                lines.push_back(std::string(buffer));
                            }
                        }
                    }
                }
            } else {
                printf("Failed to open file: %s.\n", infile.c_str());
                rc = 1;
            }

            printf("Read %llu. Wrote %llu.\n", readCount, lines.size());

            printf("Sorting...\n");
            lines.sort();

            printf("Writing...\n");
            for (const auto& it : lines) {
                ofh.write(it.c_str(), it.size());
            }
            ofh.close();

            printf("Done.\n");
        } else {
            printf("Failed to create file: %s.\n", outfile.c_str());
            rc = 1;
        }
    }
    return rc;
}