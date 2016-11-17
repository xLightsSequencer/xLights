#ifndef SEQUENCECHECK_H
#define SEQUENCECHECK_H

#define FORMATTIME(ms) (const char *)wxString::Format("%d:%02d.%03d", (ms) / 60000, ((ms) % 60000) / 1000, (ms) % 1000).c_str()

#endif
