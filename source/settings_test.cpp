#define main FarmerApplicationMain
#include "main.cpp"
#undef main

static int checks = 0;
static void Check(bool valid, const char* message) {
    ++checks;
    if (!valid) throw std::runtime_error(message);
}
static void CheckSame(const ThongTinTool& a, const ThongTinTool& b, const char* message) {
    Check(FarmEncodePreferences(FarmGetPreferences(a)) == FarmEncodePreferences(FarmGetPreferences(b)), message);
}
int main(int argc, char** argv) {
    try {
        Check(argc == 2, "Pass an isolated scratch directory");
        std::filesystem::path scratch = std::filesystem::absolute(argv[1]);
        Check(SetEnvironmentVariableW(L"LOCALAPPDATA", scratch.c_str()), "Set process-only scratch path");
        ThongTinTool first;
        first.tenTab = "settings-test-primary";
        Check(FarmEncodePreferences(FarmGetPreferences(first)) == FarmEncodePreferences(FarmPreferences{}), "First use must have no preset selections");
        FarmLoadPreferences(first);
        Check(first.thongBaoLuu == "Chua co lua chon da luu", "Missing profile is first use");
        Check(!std::filesystem::exists(FarmPreferencesPath(first)), "First use must not create or replace a profile");
        first.kichHoatThuHoachNhanh = true;
        first.thuHoachBangTenTim = true;
        first.kichHoatMuaHat = true;
        first.kichHoatMuaCongCu = true;
        first.kichHoatTrongCay = true;
        first.cacHatCanTrong[0] = first.cacHatCanTrong[33] = true;
        first.cacHatDuocChon[0] = first.cacHatDuocChon[30] = true;
        first.cacHatCanMua[0] = first.cacHatCanMua[28] = true;
        first.cacCongCuCanMua[0] = first.cacCongCuCanMua[7] = true;
        first.batAutoThoiTiet = true;
        first.luaChonMapGoc = 4;
        first.dangChay = true;
        first.h_game = reinterpret_cast<HWND>(123);
        first.hatTrongDaNho.valid=true;
        first.hatTrongDaNho.snapshot.seeds[0].present=true;
        Check(FarmSavePreferences(first), "Write mixed selections atomically");
        ThongTinTool reopened;
        reopened.tenTab = first.tenTab;
        FarmLoadPreferences(reopened);
        CheckSame(first, reopened, "Reopen restores every user preference");
        Check(!reopened.dangChay && reopened.h_game == nullptr, "Loading must never restore running state or handles");
        Check(!reopened.hatTrongDaNho.valid&&reopened.hatTrongDaNho.scans==0,"A new process must scan current stock instead of loading an old inventory cache");
        Check(reopened.mapMuonSan == 4 && reopened.luaChonMapGoc == 4, "Restore the original map choice");
        ThongTinTool second;
        second.tenTab = "settings-test-secondary";
        second.kichHoatBan = true;
        second.modeThuHoachTenTim = 2;
        second.cacHatCanMua[14] = true;
        Check(FarmSavePreferences(second), "Save an independent second instance");
        FarmLoadPreferences(reopened);
        CheckSame(first, reopened, "Second instance must not overwrite the first");
        ThongTinTool secondReopened;
        secondReopened.tenTab = second.tenTab;
        FarmLoadPreferences(secondReopened);
        CheckSame(second, secondReopened, "Second instance restores its own choices");
        auto valid = FarmEncodePreferences(FarmGetPreferences(second));
        auto legacy = FarmEncodePreferences(FarmGetPreferences(first));
        legacy.replace(legacy.find("version=2"),9,"version=1");
        for(const std::string key:{"plant=", "planting_seeds="}) {
            auto at=legacy.find(key);
            legacy.erase(at,legacy.find('\n',at)-at+1);
        }
        FarmPreferences migrated;
        Check(FarmDecodePreferences(legacy,migrated),"Read previous v0.1.1 schema");
        Check(migrated.harvest && migrated.buySeeds && !migrated.plant,"Keep old choices without enabling new planting");
        Check(std::none_of(migrated.plantingSeeds.begin(),migrated.plantingSeeds.end(),[](bool x){return x;}),"Legacy planting selection stays empty");
        auto badPlant=valid;
        auto plantAt=badPlant.find("planting_seeds=");
        badPlant.erase(plantAt+std::string("planting_seeds=").size(),1);
        Check(!FarmDecodePreferences(badPlant,migrated),"Reject incomplete planting choices");
        for (int mode : {0, 1, 2}) {
            FarmPreferences expected;
            expected.harvestMode = mode;
            FarmPreferences actual;
            Check(FarmDecodePreferences(FarmEncodePreferences(expected), actual) && actual.harvestMode == mode, "Round-trip each harvest mode");
        }
        auto sentinel = FarmGetPreferences(first);
        for (const auto& bad : {valid.substr(0, valid.size()/2), valid + "sell=1\n", std::string(5000, 'x'), std::string("version=99\n")}) {
            auto unchanged = sentinel;
            Check(!FarmDecodePreferences(bad, unchanged), "Reject malformed, duplicate, oversized or unsupported data");
            Check(FarmEncodePreferences(unchanged) == FarmEncodePreferences(sentinel), "Malformed data must not partially enable any choice");
        }
        auto badMode = valid;
        badMode.replace(badMode.find("harvest_mode=2"), std::string("harvest_mode=2").size(), "harvest_mode=9");
        Check(!FarmDecodePreferences(badMode, sentinel), "Reject unknown harvest mode");
        auto badBoolean = valid;
        badBoolean.replace(badBoolean.find("sell=1"), 6, "sell=2");
        Check(!FarmDecodePreferences(badBoolean, sentinel), "Reject non-boolean toggle");
        {
            std::ofstream broken(FarmPreferencesPath(second), std::ios::binary | std::ios::trunc);
            broken << "version=1\nsell=1\n";
        }
        ThongTinTool brokenReopened;
        brokenReopened.tenTab = second.tenTab;
        FarmLoadPreferences(brokenReopened);
        Check(FarmEncodePreferences(FarmGetPreferences(brokenReopened)) == FarmEncodePreferences(FarmPreferences{}), "Corrupt stored profile leaves empty first-use settings");
        Check(brokenReopened.thongBaoLuu.find("Loi doc") == 0, "Corrupt profile is reported in the menu");
        FarmApplyPreferences(first, FarmPreferences{});
        Check(FarmSavePreferences(first), "Clearing all selections is a real saved preference");
        FarmLoadPreferences(reopened);
        CheckSame(first, reopened, "Cleared selections stay cleared after reopening");
        Check(reopened.thongBaoLuu == "Da khoi phuc lua chon da luu", "Saved empty profile is distinct from missing profile");
        const auto longA = FarmProfileKey(std::string(240, 'a') + "1");
        const auto longB = FarmProfileKey(std::string(240, 'a') + "2");
        Check(longA != longB && longA.size() < 64, "Long instance titles remain distinct with bounded filenames");
        auto hostile = FarmProfileKey("../a/b\\c: names");
        Check(hostile.find_first_not_of("0123456789abcdef-") == std::string::npos, "Tab title cannot change the settings directory");
        // A blocked write reports an error without modifying an unrelated file.
        auto blocked = scratch / "blocked-local";
        { std::ofstream file(blocked); file << "preserve"; }
        Check(SetEnvironmentVariableW(L"LOCALAPPDATA", blocked.c_str()), "Select test-only blocked path");
        Check(!FarmSavePreferences(first) && first.thongBaoLuu.find("Loi luu") == 0, "Report save failures instead of claiming success");
        { std::ifstream file(blocked); std::string value; file >> value; Check(value == "preserve", "Write failure must preserve the existing file"); }
        std::cout << checks << " settings checks passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Settings check failed: " << e.what() << "\n";
        return 1;
    }
}
