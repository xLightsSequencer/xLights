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

#include <wx/filename.h>
#include <wx/string.h>

#include <functional>
#include <list>
#include <string>

class wxWindow;

// Helpers shared by the Matrix (image) face editors - ModelFaceDialog's Matrix
// tab and the Sequence Settings Faces tab. Matrix face definitions store one
// image path per phoneme x eye state under keys of the form
// Mouth-<PHONEME>-EyesOpen / Mouth-<PHONEME>-EyesClosed.
namespace FaceMatrixHelpers
{
    // Canonical phoneme order - also the row order of the 10x2 editor grids
    const std::list<std::string>& Phonemes();

    const wxString& SupportedImageTypesFilter();

    // col 0 = EyesOpen, col 1 = EyesClosed
    std::string GenerateKey(int col, const std::string& phoneme);
    // "Mouth-AI-EyesOpen" -> "AI"
    std::string ExtractKey(const wxString& key);
    // Case/spelling variants a phoneme appears as in image file names
    std::list<std::string> GetPhonemes(const std::string& key);
    // replace the count'th occurrence of key with phoneme and return the new name
    wxFileName GetFileNamePhoneme(const wxFileName& fn, const std::string& key, int count, const std::string& phoneme);

    std::string FixPhonemeCase(const std::string& p);
    bool IsValidPhoneme(const std::string& phoneme);
    int GetRowForPhoneme(const std::string& phoneme);

    // Given a just-picked image for one phoneme cell, try to infer the images
    // for all other phonemes from the file naming pattern. `fullKey` is the
    // picked cell's key (e.g. "Mouth-AI-EyesOpen"), `col` its column. For each
    // inferred image, `setter(key, row, col, path)` is called unless
    // `getter(key)` already returns an existing file.
    void AutoFillMatrixPhonemes(const std::string& pickedFile, const std::string& fullKey, int col,
                                const std::function<std::string(const std::string& key)>& getter,
                                const std::function<void(const std::string& key, int row, int col, const std::string& value)>& setter);

    // Run the MatrixFaceDownloadDialog, extract the chosen face zip into
    // <showDir>/DownloadedFaces and map the extracted images to phoneme keys
    // via `setter(key, row, col, path)`. Returns true if images were applied.
    bool DownloadFaceImages(wxWindow* parent, const std::string& showDir, int bufferWi, int bufferHt,
                            const std::function<void(const std::string& key, int row, int col, const std::string& value)>& setter);
}
