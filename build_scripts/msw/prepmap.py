#!/usr/bin/env python3
"""Condense an MSVC linker .map into the symbol table xLights reads at crash time.

xlStackWalker (common/xlStackWalker.h) loads <exe-name>.map next to the running
executable and turns stack addresses into function names, so users get a
symbolicated trace without shipping PDBs. The raw linker map is tens of
megabytes and in link order; this keeps only the code symbols, converts each
address to an RVA so it survives ASLR, and sorts them.

Replaces PrepMap.exe, which was a committed binary nobody could check against
its source. Output must stay byte-identical: the format is a parsing contract
with the crash handler.

Usage: prepmap.py <input.map> <output.map>
"""
import re
import sys

LEADING_HEX = re.compile(r'[0-9a-fA-F]+')


def main(argv):
    if len(argv) != 3:
        print("Usage: prepmap.py <inputfile> <outputfile>")
        return 1
    infile, outfile = argv[1], argv[2]
    print("Processing map file %s ==> %s." % (infile, outfile))

    load_address = 0
    out = []
    read_count = 0

    print("Reading...")
    # latin-1 so any stray byte in a mangled name round-trips untouched.
    with open(infile, 'r', encoding='latin-1', errors='replace') as fh:
        for raw in fh:
            read_count += 1
            line = raw.strip()

            if "Preferred load address is" in line:
                # Offset 28 into the trimmed line, matching the original. The
                # value is zero-padded to 16 digits, so starting two digits in
                # yields the same number for any real image base.
                m = LEADING_HEX.match(line[28:].strip())
                if m:
                    load_address = int(m.group(0), 16)
                continue

            if not (line.startswith("0001:") or line.startswith("0002:")):
                continue
            if not (line.endswith(".obj") or line.endswith(".o")):
                continue

            if load_address == 0:
                print("Preferred load address not found.")
                return 1

            # Collapse runs of spaces, then split on single spaces, so the
            # column indices below line up the way the original's did.
            comp = re.sub(r' {2,}', ' ', line).split(' ')
            if len(comp) <= 3:
                continue

            # The object column is the first field from index 3 on that is more
            # than two characters; index 0 if there is none.
            ln = 0
            for i in range(3, len(comp)):
                if len(comp[i]) > 2:
                    ln = i
                    break

            m = LEADING_HEX.match(comp[2])
            addr = int(m.group(0), 16) if m else 0
            rva = (addr - load_address) & 0xFFFFFFFFFFFFFFFF

            # Skip the section symbol and dynamic-initialiser thunks; neither
            # names anything a stack trace can use.
            if rva and addr and comp[1] != ".text" and not comp[1].startswith("??__E"):
                out.append("%016x\t%s\t%s\n" % (rva, comp[ln], comp[1]))

    print("Read %d. Wrote %d." % (read_count, len(out)))
    print("Sorting...")
    out.sort()
    print("Writing...")
    # The original wrote through a Windows text-mode stream, so its newlines are
    # CRLF; keep that byte-for-byte.
    with open(outfile, 'w', encoding='latin-1', newline='\r\n') as fh:
        fh.writelines(out)
    print("Done.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
