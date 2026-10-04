#pragma once
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Storage.Streams.h>
#include <cctype>
#include "farm_crop_match.h"

struct FarmOcrWord { std::string text; cv::Rect box; std::wstring raw; };

inline std::string FarmNormalize(std::wstring text) {
    for (auto& ch : text) if (ch == L'đ' || ch == L'Đ') ch = L'd';
    int size = NormalizeString(NormalizationD, text.c_str(), (int)text.size(), nullptr, 0);
    if (size <= 0) return {};
    std::wstring normalized(size, L' ');
    size = NormalizeString(NormalizationD, text.c_str(), (int)text.size(), normalized.data(), size);
    std::string out;
    for (int i = 0; i < size; ++i) {
        wchar_t ch = normalized[i];
        if (ch >= L'A' && ch <= L'Z') out += char(ch - L'A' + 'a');
        else if (ch >= L'a' && ch <= L'z') out += char(ch);
    }
    return out;
}

inline std::vector<FarmOcrWord> FarmReadText(const cv::Mat& input, cv::Rect roi) {
    std::vector<FarmOcrWord> words;
    if (input.empty()) return words;
    roi &= cv::Rect(0, 0, input.cols, input.rows);
    if (roi.empty()) return words;
    try {
        static thread_local bool initialized = false;
        if (!initialized) { winrt::init_apartment(winrt::apartment_type::multi_threaded); initialized = true; }
        auto engine = winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromUserProfileLanguages();
        if (!engine) engine = winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromLanguage(winrt::Windows::Globalization::Language(L"en-US"));
        if (!engine) return words;
        cv::Mat scaled, bgra;
        cv::resize(input(roi), scaled, {}, 2.0, 2.0, cv::INTER_CUBIC);
        cv::cvtColor(scaled, bgra, cv::COLOR_BGR2BGRA);
        winrt::Windows::Storage::Streams::DataWriter writer;
        writer.WriteBytes(winrt::array_view<const uint8_t>(bgra.data, bgra.data + bgra.total() * bgra.elemSize()));
        auto bitmap = winrt::Windows::Graphics::Imaging::SoftwareBitmap::CreateCopyFromBuffer(
            writer.DetachBuffer(), winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8,
            bgra.cols, bgra.rows, winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Ignore);
        auto result = engine.RecognizeAsync(bitmap).get();
        for (auto const& line : result.Lines()) for (auto const& word : line.Words()) {
            auto b = word.BoundingRect();
            words.push_back({FarmNormalize(std::wstring(word.Text())), cv::Rect(
                roi.x + int(b.X / 2), roi.y + int(b.Y / 2),
                int(b.Width / 2), int(b.Height / 2)), std::wstring(word.Text())});
        }
    } catch (...) { /* Existing image recognition remains available if OCR is unavailable. */ }
    return words;
}

inline cv::Point FarmFindCrop(const std::vector<FarmOcrWord>& words, const char* name) {
    std::wstring wide;
    for (const char* p = name; *p; ++p) wide += wchar_t(*p);
    const std::string target = FarmNormalize(wide);
    for (int column = 0; column < 5; ++column) for (int row = 0; row < 7; ++row) {
        cv::Rect tile(32 + 175 * column, 120 + 48 * row, 171, 48);
        std::string text;
        for (auto const& word : words) {
            cv::Point center(word.box.x + word.box.width / 2, word.box.y + word.box.height / 2);
            if (tile.contains(center)) text += word.text;
        }
        if (!target.empty() && (text == target || text == "hat" + target))
            return cv::Point(tile.x + 85, tile.y + 24);
    }
    return {-1, -1};
}

struct FarmCropChoice {cv::Rect tile;int id=-1;std::string text;};
inline FarmCropChoice FarmReadCropChoice(const cv::Mat& frame,cv::Rect tile) {
    FarmCropChoice choice{tile};
    auto label=cv::Rect(tile.x+34,tile.y+1,tile.width-59,tile.height-2)&cv::Rect(0,0,frame.cols,frame.rows);
    if(label.empty())return choice;
    auto read=[](const cv::Mat& image) {
        std::string text;
        for(const auto& word:FarmReadText(image,{0,0,image.cols,image.rows}))text+=word.text;
        return text;
    };
    // Isolated labels prevent OCR from grouping neighbouring cards together
    // or omitting short names while reading the entire filter panel.
    cv::Mat padded;cv::copyMakeBorder(frame(label),padded,10,10,10,10,cv::BORDER_CONSTANT,cv::Scalar(255,255,255));
    choice.text=read(padded);
    auto primary=FarmMatchCrop(choice.text);
    if(primary.id>=0&&primary.edits==0){choice.id=primary.id;return choice;}
    cv::Mat gray,contrast;
    cv::cvtColor(frame(label),gray,cv::COLOR_BGR2GRAY);
    cv::threshold(gray,gray,160,255,cv::THRESH_BINARY);
    cv::resize(gray,gray,{},2,2,cv::INTER_NEAREST);
    cv::copyMakeBorder(gray,gray,16,16,16,16,cv::BORDER_CONSTANT,cv::Scalar(255));
    cv::cvtColor(gray,contrast,cv::COLOR_GRAY2BGR);
    auto retryText=read(contrast);
    auto retry=FarmMatchCrop(retryText);
    if(primary.id>=0&&retry.id>=0&&primary.id!=retry.id)return choice;
    auto match=retry.id>=0?retry:primary;
    if(match.id>=0){choice.id=match.id;if(retry.id>=0)choice.text=retryText;}
    return choice;
}
inline std::vector<FarmCropChoice> FarmReadCropChoices(const cv::Mat& frame) {
    std::vector<FarmCropChoice> choices;
    for(auto tile:FarmCropTiles(frame))choices.push_back(FarmReadCropChoice(frame,tile));
    return choices;
}
