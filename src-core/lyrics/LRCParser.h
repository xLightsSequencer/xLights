#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <utility>
#include <vector>

namespace lrc {

// Parse an `.lrc`-format synced-lyrics body (one or more lines
// shaped like `[mm:ss.xx] text…`) into a list of (millisecond,
// text) pairs in source order.
//
// Lines that don't open with `[` are skipped. Metadata tags like
// `[ti:Song Title]` (non-numeric prefix before the colon) are
// ignored. Timestamp formats accepted: `mm:ss`, `mm:ss.x`,
// `mm:ss.xx` (centiseconds), `mm:ss.xxx` (milliseconds). Empty
// trailing text is preserved as `("", time)` so callers can
// decide whether to drop it.
//
// Wx-free; called by both the desktop wx dialog
// (`src-ui-wx/sequencer/LRCLIBSearchDialog.cpp`) and the iPad
// bridge (`src-iPad/Bridge/XLLyricsImport.mm`).
std::vector<std::pair<int, std::string>> ParseLRC(const std::string& syncedLyrics);

// Apply the small text-cleanup pass the desktop runs before
// adding a phrase to a timing layer:
//   - Replace common Unicode quotes with ASCII (' " ").
//   - Strip remaining double-quotes and `<`/`>` (illegal in
//     phrase labels — they trip the XML save path).
//   - Run `PhonemeDictionary::InsertSpacesAfterPunctuation` so
//     adjacent words that are stuck to commas / periods break
//     into separate words on the breakdown pass.
//
// Mutates `text` in place. Returns true if the resulting string
// has any non-whitespace content (i.e. is worth writing).
bool SanitizePhraseText(std::string& text);

} // namespace lrc
