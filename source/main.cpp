#include <opencv2/core/utils/logger.hpp>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>


#include <atomic>
#include <mutex>
#include "farm_crop_match.h"
#include "farm_ocr.h"
#include "farm_settings.h"
#include "farm_sell_vision.h"
#include "farm_plant_vision.h"
#include "farm_plant_inventory.h"
#include "farm_garden_plan.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <stdexcept>


using namespace std;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// --- ĐỊNH NGHĨA TRẠNG THÁI ---
enum BotState {
    STATE_IDLE = 0,         // Trạng thái chờ
    STATE_FARMING = 1,      // Đang farm
    STATE_SELLING = 2,      // Đang đi bán
    STATE_BUYING_SEEDS = 3, // Đang mua hạt
    STATE_BUYING_TOOLS = 4,  // Đang mua công cụ
};

constexpr int SO_HAT_MUA = 29;
string ds_anh_chu_qua[] = {
    "txt_crop_carot.png", "txt_crop_cucai.png", "txt_dautay.png", "txt_crop_vietquat.png", "txt_crop_khoaitay.png", "txt_crop_nam.png", "txt_hatbap.png", "txt_crop_cachua.png", "txt_hatsung.png", "txt_crop_anhdao.png", "txt_crop_khoailang.png", "txt_crop_saguaro.png", "txt_crop_gaivang.png", "txt_hattao.png", "txt_crop_hatde.png", "txt_hatnho.png", "txt_crop_cholla.png", "txt_crop_mangcau.png", "txt_crop_legai.png", "txt_crop_bingo.png", "txt_crop_lua.png", "txt_crop_duahau.png", "txt_hatdua.png", "txt_hatxoai.png", "txt_hatdudu.png", "txt_crop_cayphong.png", "txt_crop_caydau.png", "txt_crop_khe.png", "txt_hattaoduong.png", "txt_trangkhuyet.png", "txt_nhansam.png"
};
const char* ds_hat_giongmua[] = {
    "Ca rot", "Cu cai", "Dau tay", "Viet quat", "Khoai tay", "Nam", "Bap", "Ca chua", "Sung", "Anh dao", "Khoai lang", "Xuong rong Saguaro", "Xuong rong gai vang", "Tao", "Hat de", "Nho", "Xuong rong Cholla", "Mang cau", "Xuong rong le gai", "Bi ngo", "Lua", "Dua hau", "Dua", "Xoai", "Du du", "Cay phong", "Cay dau", "Khe", "Tao duong"
};

string ds_anh_cong_cu[] = { 
    "voi_tuoi_thuong.png", "voi_tuoi_cao_cap.png", "voi_tuoi_sieu_cao_cap.png", "ong_bom_sach.png",
    "bao_ve_trai.png", "xeng_bung_cay.png", "xeng.png", "ve_tang_qua.png"

};

string ds_anh_hat_hai[] = {
    "seed_carot.png", "seed_cucai.png", "seed_dautay.png", "seed_vietquat.png", "seed_khoaitay.png", "seed_nam.png", "seed_bap.png", "seed_cachua.png", "seed_sung.png", "seed_anhdao.png", "seed_khoailang.png", "seed_saguaro.png", "seed_gaivang.png", "seed_tao.png", "seed_hatde.png", "seed_nho.png", "seed_cholla.png", "seed_mangcau.png", "seed_legai.png", "seed_bingo.png", "seed_lua.png", "seed_duahau.png", "seed_dua.png", "seed_xoai.png", "seed_dudu.png", "seed_cayphong.png", "seed_caydau.png", "seed_khe.png", "seed_taoduong.png"
};



string ds_anh_hat[] = {
    "seed_carot.png", "seed_cucai.png", "seed_dautay.png", "seed_vietquat.png", "seed_khoaitay.png", "seed_nam.png", "seed_bap.png", "seed_cachua.png", "seed_sung.png", "seed_anhdao.png", "seed_khoailang.png", "seed_saguaro.png", "seed_gaivang.png", "seed_tao.png", "seed_hatde.png", "seed_nho.png", "seed_cholla.png", "seed_mangcau.png", "seed_legai.png", "seed_bingo.png", "seed_lua.png", "seed_duahau.png", "seed_dua.png", "seed_xoai.png", "seed_dudu.png", "seed_cayphong.png", "seed_caydau.png", "seed_khe.png", "seed_taoduong.png"
};

const char* ds_cong_cu[] = { "Voi Tuoi Thuong", "Voi Tuoi Cao Cap", "Voi Tuoi Sieu Cao Cap",
"Ong Bom Sach", "Bao Ve Trai", "Xeng Bung Cay"
, "Xeng", "Ve Tang Qua"};















enum MapID {
    MAP_UNKNOWN = -1,
    MAP_PLAZA = 0,
    MAP_KND = 1
};



enum TrangThaiBot {
    TT_CHECK_THOI_TIET = 0,
    TT_DI_CHUYEN_MAP = 1,
    TT_SETUP_VITRI = 2,
    TT_BAT_CAN = 3,
    TT_CAU_CA_PRO = 4
};

// --- CHẾ ĐỘ THU HOẠCH TEN TIM ---
enum CheDoThuHoachTenTim {
    CHE_DO_CHON_HAT = 0,          // Chọn hạt như cũ (tick từng loại)
    CHE_DO_HAI_ALL = 1,           // Hái ALL, bỏ qua bước tìm & chọn trái
    CHE_DO_HAI_LOC_BIEN_THE = 2   // Hái nhưng loại trừ biến thể (vẫn cần chọn hạt)
};

// 2. Struct ThongTinTool phải chứa đầy đủ cho cả 2 luồng:
struct ThongTinTool {
    HWND h_cha = NULL;
    HWND h_game = NULL;
    std::string tenTab = "";

    std::atomic<bool> dangChay{false};
    bool hienMatBot = false;

    bool kichHoatBan = false;
    bool kichHoatMuaHat = false;
    bool kichHoatMuaCongCu = false;
    bool kichHoatThuHoachNhanh = false;
    bool thuHoachBangTenTim = false;
    bool kichHoatTrongCay = false;
    bool cacHatCanTrong[SO_HAT_TRONG] = {};
    unsigned long long henTrongCay = 0;
    int soCayVuaTrong = 0;
    std::string ketQuaTrong = "Chua chay luot trong";
    PlantInventoryCache hatTrongDaNho;
    PlantBagCursor conTroBalo;
    GardenPlan soDoVuon;
    std::string ketQuaBalo="Chua quet hat";
    std::mutex trongThongTinMutex;
    std::atomic<int> vuonSoLuong{0},vuonSoDiem{0},vuonDaKiemTra{0},vuonDaTrong{0};
    std::string ketQuaLocTrai;

    long long time_cho_hoi_qua = 0;
    int buocHienTai = 0;

    bool cacHatDuocChon[SO_NONG_SAN] = { false };
    bool cacHatCanMua[SO_HAT_MUA] = { false };
    bool cacCongCuCanMua[8] = { false };

    int loaiCongCuChon = 0;

    // Chế độ thu hoạch Ten Tim: 0 = Chon hat, 1 = Hai ALL, 2 = Hai loc bo bien the
    int modeThuHoachTenTim = CHE_DO_CHON_HAT;

    // 🔴 GIỮ NGUYÊN BIẾN NÀY CHO HÀM FARM CŨ KHÔNG BỊ ĐỎ
    BotState trangThaiHienTai = STATE_FARMING;

    // 🔵 THÊM BIẾN NÀY CHO HÀM CÂU CÁ MỚI (Đúng theo lỗi đỏ trong ảnh của ông)
    TrangThaiBot trangThaiCauCa = TT_CHECK_THOI_TIET;

    int soLuotBan = 0;
    std::string thongBaoStatus = "San sang";
    std::string thongBaoLuu = "Chua co lua chon da luu";

    std::chrono::steady_clock::time_point thoiGianMuaHatGanNhat;
    std::chrono::steady_clock::time_point thoiGianMuaCongCuGanNhat;

    // =========================================================================
    // ⚙️ CẤU HÌNH TỪ GIAO DIỆN IMGUI
    // =========================================================================
    bool batAutoThoiTiet = false;
    int mapMuonSan = -1;
    int thoiTietMuonSan = -1;

    int mapHienTai = -1;
    int mapSanDuoc = -1;
    int thoiTietSannDuoc = -1;

    int thoiTietTungHang[5] = { -1, -1, -1, -1, -1 };
    int khoangCachTungHang[5] = { 99999, 99999, 99999, 99999, 99999 };
    cv::Rect roiBangThoiTiet = cv::Rect(176, 102, 707, 393);

    // Biến đa luồng điều khiển câu cá
    std::atomic<bool> yeuCauNgatCau{ false };
    std::atomic<int> giayDemNguoc{ 0 };

    int luaChonMapGoc;

    std::atomic<bool> canResetLuongNgam{ false };



    ThongTinTool() {
        // Lan dau mo: khong tu tick trai, hat, cong cu hoac bat auto.
        thoiGianMuaHatGanNhat = std::chrono::steady_clock::now();
        thoiGianMuaCongCuGanNhat = std::chrono::steady_clock::now();
        batAutoThoiTiet = false;
        thoiTietMuonSan = -1;
        mapMuonSan = -1;
        mapHienTai = -1;
        luaChonMapGoc = -1;
    }
};





// Preferences live outside the extracted EXE directory so updates retain them.
FarmPreferences FarmGetPreferences(const ThongTinTool& tool) {
    static_assert(SO_NONG_SAN == 31 && SO_HAT_MUA == 29);
    FarmPreferences p;
    p.sell = tool.kichHoatBan;
    p.harvest = tool.kichHoatThuHoachNhanh;
    p.purpleName = tool.thuHoachBangTenTim;
    p.buySeeds = tool.kichHoatMuaHat;
    p.buyTools = tool.kichHoatMuaCongCu;
    p.showVision = tool.hienMatBot;
    p.plant = tool.kichHoatTrongCay;
    std::copy(std::begin(tool.cacHatCanTrong), std::end(tool.cacHatCanTrong), p.plantingSeeds.begin());
    p.harvestMode = tool.modeThuHoachTenTim;
    std::copy(std::begin(tool.cacHatDuocChon), std::end(tool.cacHatDuocChon), p.crops.begin());
    std::copy(std::begin(tool.cacHatCanMua), std::end(tool.cacHatCanMua), p.seeds.begin());
    std::copy(std::begin(tool.cacCongCuCanMua), std::end(tool.cacCongCuCanMua), p.tools.begin());
    p.weather = tool.batAutoThoiTiet;
    p.weatherMap = tool.luaChonMapGoc;
    return p;
}

void FarmApplyPreferences(ThongTinTool& tool, const FarmPreferences& p) {
    tool.kichHoatBan = p.sell;
    tool.kichHoatThuHoachNhanh = p.harvest;
    tool.thuHoachBangTenTim = p.purpleName;
    tool.kichHoatMuaHat = p.buySeeds;
    tool.kichHoatMuaCongCu = p.buyTools;
    tool.hienMatBot = p.showVision;
    tool.kichHoatTrongCay = p.plant;
    std::copy(p.plantingSeeds.begin(), p.plantingSeeds.end(), std::begin(tool.cacHatCanTrong));
    tool.modeThuHoachTenTim = p.harvestMode;
    std::copy(p.crops.begin(), p.crops.end(), std::begin(tool.cacHatDuocChon));
    std::copy(p.seeds.begin(), p.seeds.end(), std::begin(tool.cacHatCanMua));
    std::copy(p.tools.begin(), p.tools.end(), std::begin(tool.cacCongCuCanMua));
    tool.batAutoThoiTiet = p.weather;
    tool.luaChonMapGoc = p.weatherMap;
    tool.mapMuonSan = p.weatherMap;
}

std::filesystem::path FarmPreferencesPath(const ThongTinTool& tool) {
    wchar_t local[32768] = {};
    DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", local, 32768);
    if (!length || length >= 32768) throw std::runtime_error("LocalAppData is unavailable");
    return std::filesystem::path(local) / L"BOT FARMER X DLee" / L"settings"
        / (FarmProfileKey(tool.tenTab) + ".ini");
}

bool FarmSavePreferences(ThongTinTool& tool) {
    try {
        auto path = FarmPreferencesPath(tool);
        std::filesystem::create_directories(path.parent_path());
        auto temporary = path;
        temporary += ".tmp." + std::to_string(GetCurrentProcessId());
        {
            std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
            if (!file) throw std::runtime_error("Cannot write preferences");
            file << FarmEncodePreferences(FarmGetPreferences(tool));
            file.flush();
            if (!file) throw std::runtime_error("Cannot finish preferences");
        }
        if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("Cannot replace preferences");
        tool.thongBaoLuu = "Da luu lua chon (tu dong luu)";
        return true;
    } catch (...) {
        tool.thongBaoLuu = "Loi luu lua chon - hay bam LUu LUA CHON lai";
        return false;
    }
}

void FarmLoadPreferences(ThongTinTool& tool) {
    try {
        auto path = FarmPreferencesPath(tool);
        if (!std::filesystem::exists(path)) return;
        if (std::filesystem::file_size(path) > 4096) throw std::runtime_error("Invalid preferences");
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot read preferences");
        std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        FarmPreferences p;
        if (!FarmDecodePreferences(data, p)) throw std::runtime_error("Invalid preferences");
        FarmApplyPreferences(tool, p);
        tool.thongBaoLuu = "Da khoi phuc lua chon da luu";
    } catch (...) {
        tool.thongBaoLuu = "Loi doc lua chon - hay chon va luu lai";
    }
}

// Quản lý danh sách
std::vector<ThongTinTool*> danhSachTabs;
int tabDangChon = 0;



void QuetTatCaLDPlayer() {
    // Xóa danh sách cũ nếu có (nhớ delete để tránh leak memory)
    for (auto t : danhSachTabs) t->dangChay = false;
    danhSachTabs.clear();

    HWND hwndLD = NULL;
    while ((hwndLD = FindWindowExA(NULL, hwndLD, "LDPlayerMainFrame", NULL)) != NULL) {
        ThongTinTool* tabMoi = new ThongTinTool();

        tabMoi->h_cha = hwndLD;
        tabMoi->h_game = FindWindowExA(hwndLD, NULL, "RenderWindow", NULL);

        // Lấy tiêu đề để phân biệt các cửa sổ (Ví dụ: LDPlayer, LDPlayer-1...)
        char title[256];
        GetWindowTextA(hwndLD, title, sizeof(title));
        tabMoi->tenTab = std::string(title);
        FarmLoadPreferences(*tabMoi);

        // Khởi tạo thời gian ban đầu để không bị mua đồ ngay lập tức
        tabMoi->thoiGianMuaHatGanNhat = std::chrono::steady_clock::now();
        tabMoi->thoiGianMuaCongCuGanNhat = std::chrono::steady_clock::now();

        danhSachTabs.push_back(tabMoi);
    }
}





static ID3D11Device* thietBiD3D = NULL;
static ID3D11DeviceContext* boDieuKhienD3D = NULL;
static IDXGISwapChain* chuoiTraoDoi = NULL;
static ID3D11RenderTargetView* hinhAnhDich = NULL;

bool KhoiTaoThietBiD3D(HWND hWnd);
void DonDepThietBiD3D();
void TaoHinhAnhDich();
void DonDepHinhAnhDich();
LRESULT WINAPI XuLyTinHieuCuaSo(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class BoDieuKhien {
public:
    static cv::Mat ChupManHinh(HWND hWnd) {
        if (!hWnd) return cv::Mat();

        // 1. Lấy kích thước thực tế của vùng làm việc (Client Area)
        RECT rc; GetClientRect(hWnd, &rc);
        int w = rc.right - rc.left, h = rc.bottom - rc.top;
        if (w <= 0 || h <= 0) return cv::Mat();

        // 2. Thiết lập DC và Bitmap để chứa dữ liệu ảnh
        HDC hdcW = GetDC(hWnd);
        HDC hdcM = CreateCompatibleDC(hdcW);
        HBITMAP hbm = CreateCompatibleBitmap(hdcW, w, h);
        SelectObject(hdcM, hbm);

        // --- ĐOẠN FIX QUAN TRỌNG: Thay BitBlt bằng PrintWindow ---
        // Cờ số 2 (PW_RENDERFULLCONTENT) giúp chụp được nội dung DirectX/OpenGL của LDPlayer
        BOOL bRet = PrintWindow(hWnd, hdcM, 2);

        // Nếu PrintWindow (cờ 2) thất bại, dùng BitBlt làm phương án dự phòng
        if (!bRet) {
            BitBlt(hdcM, 0, 0, w, h, hdcW, 0, 0, SRCCOPY);
        }

        // 3. Đổ dữ liệu từ Bitmap vào cv::Mat
        BITMAPINFOHEADER bi = { sizeof(bi), w, -h, 1, 32, BI_RGB };
        cv::Mat bmp(h, w, CV_8UC4);
        GetDIBits(hdcM, hbm, 0, h, bmp.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        // 4. Giải phóng tài nguyên ngay để tránh tràn bộ nhớ (Memory Leak)
        DeleteObject(hbm);
        DeleteDC(hdcM);
        ReleaseDC(hWnd, hdcW);

        // 5. Chuyển đổi hệ màu sang BGR để OpenCV xử lý chuẩn
        cv::Mat res;
        cv::cvtColor(bmp, res, cv::COLOR_BGRA2BGR);

        return res;
    }

    static void BamChuot(HWND cua_so, int x, int y) {
        if (!cua_so) return;
        LPARAM pos = MAKELPARAM(x, y);
        PostMessage(cua_so, WM_MOUSEMOVE, 0, pos);
        this_thread::sleep_for(chrono::milliseconds(20));
        PostMessage(cua_so, WM_LBUTTONDOWN, MK_LBUTTON, pos);
        this_thread::sleep_for(chrono::milliseconds(100));
        PostMessage(cua_so, WM_LBUTTONUP, 0, pos);
    }

    static cv::Point TimAnhTrongVung(cv::Mat screenshot, string pathAnh, cv::Rect roi, double score = 0.7) {

        string pathThucTe = "images/" + pathAnh;

        cv::Mat temp = cv::imread(pathThucTe);
        if (screenshot.empty() || temp.empty()) return cv::Point(-1, -1);
        cv::Rect safeRoi = roi & cv::Rect(0, 0, screenshot.cols, screenshot.rows);
        if (safeRoi.width <= 0 || safeRoi.height <= 0) return cv::Point(-1, -1);
        if (safeRoi.width < temp.cols || safeRoi.height < temp.rows) return cv::Point(-1, -1);

        // Cắt vùng soi theo ROI đã định nghĩa
        cv::Mat vungCrop = screenshot(safeRoi);
        cv::Mat res;
        cv::matchTemplate(vungCrop, temp, res, cv::TM_CCOEFF_NORMED);

        double maxV; cv::Point maxL;
        cv::minMaxLoc(res, NULL, &maxV, NULL, &maxL);

        if (maxV >= score) {
            // --- LOGIC TRẢ VỀ TÂM ẢNH ---
            // 1. Tọa độ góc trên-trái trong ảnh lớn: (maxL.x + roi.x, maxL.y + roi.y)
            // 2. Cộng thêm một nửa chiều rộng (temp.cols / 2) và nửa chiều cao (temp.rows / 2) của ảnh mẫu
            int tamX = maxL.x + safeRoi.x + (temp.cols / 2);
            int tamY = maxL.y + safeRoi.y + (temp.rows / 2);

            return cv::Point(tamX, tamY);
        }
        return cv::Point(-1, -1);
    }
};

void BamChuotRandomVung(HWND h_game, cv::Rect vung) {
    if (vung.width <= 0 || vung.height <= 0) return;

    // Lấy tọa độ ngẫu nhiên trong khung
    int x_rd = vung.x + (rand() % vung.width);
    int y_rd = vung.y + (rand() % vung.height);

    // Thực hiện click
    BoDieuKhien::BamChuot(h_game, x_rd, y_rd);
}

// Thêm hàm bổ trợ này ở trên đầu file
bool FileTonTai(string path) {
    cv::Mat check = cv::imread(path);
    if (!check.empty()) return true;

    check = cv::imread("images/" + path);
    return !check.empty();
}

// 2. Hàm mua vét (Áp dụng tại Bước 3 trong logic mua hạt)
void thao_tac_mua_allhat(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "Dang thuc hien mua Max...";

    // --- BƯỚC 1: Ấn vào nút vật phẩm (loại 10 đồng) để hiện bảng chọn số lượng --
    BoDieuKhien::BamChuot(thongTin->h_game, 667, 473);
    this_thread::sleep_for(chrono::milliseconds(1500)); // Đợi bảng chọn số lượng hiện lên

    if (!thongTin->dangChay) return;
    auto purchaseScreen = BoDieuKhien::ChupManHinh(thongTin->h_game);
    if (BoDieuKhien::TimAnhTrongVung(purchaseScreen, "khong_du_xu_moi.png", {270, 70, 440, 130}, 0.80).x != -1) {
        thongTin->thongBaoStatus = "Khong du xu nong trai; bo qua mua";
        return;
    }
    // The quantity dialog hides the store's top-left header. Do not drag or
    // confirm if clicking Buy did not open it (no currency, no stock, delay).
    bool storeStillUndimmed = !purchaseScreen.empty() &&
        BoDieuKhien::TimAnhTrongVung(purchaseScreen, "shop_header.png", {80, 25, 230, 70}, 0.78).x != -1 &&
        cv::mean(purchaseScreen(cv::Rect(115, 42, 35, 20)))[2] > 190;
    if (purchaseScreen.empty() || storeStillUndimmed) {
        thongTin->thongBaoStatus = "Khong mo duoc chon so luong; bo qua mua";
        return;
    }

    // --- BƯỚC 2: Kéo thanh trượt từ Min sang Max ---
    // Giả sử thanh trượt nằm từ X=400 đến X=600 tại độ cao Y=500
    // dùng logic PostMessage (giống CuonTrang) để kéo cho chuẩn
    int x_start = 348; // Tọa độ đầu thanh trượt (Min)
    int x_end = 630;   // Tọa độ cuối thanh trượt (Max)
    int y_slider = 341; // Độ cao của thanh trượt
    int steps = 20;    // Chia làm 10 bước kéo cho mượt

    // Nhấn giữ chuột tại điểm Min
    PostMessage(thongTin->h_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x_start, y_slider));
    this_thread::sleep_for(chrono::milliseconds(100));

    // Kéo dần sang điểm Max
    for (int i = 1; i <= steps && thongTin->dangChay; i++) {
        int x_current = x_start + (x_end - x_start) * i / steps;
        PostMessage(thongTin->h_game, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x_current, y_slider));
        this_thread::sleep_for(chrono::milliseconds(20));
    }

    // Thả chuột tại điểm Max
    PostMessage(thongTin->h_game, WM_LBUTTONUP, 0, MAKELPARAM(x_end, y_slider));
    this_thread::sleep_for(chrono::milliseconds(500));

    // --- BƯỚC 3: Xác nhận mua (Lệnh click vào nút Mua/Xác nhận trên bảng) ---
    // Ông thay tọa độ nút "Xác nhận mua" (ví dụ 540, 600) vào đây nhé
    if (thongTin->dangChay) BoDieuKhien::BamChuot(thongTin->h_game, 521, 396);
    this_thread::sleep_for(chrono::milliseconds(800));
}

// 3. Hàm vuốt mượt mà (Phong cách Mode 4 - Kiểm soát quán tính)
void CuonTrang_ThuanCode(HWND h_game, string huong) {
    int x_fix = 144;
    int y_dau, y_cuoi;
    int so_buoc = 20;    // Chia nhỏ bước để vuốt mượt
    int thoi_gian_keo = 15;

    if (huong == "xuong") { y_dau = 399; y_cuoi = 129; }
    else { y_dau = 129; y_cuoi = 399; }

    PostMessage(h_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x_fix, y_dau));
    this_thread::sleep_for(chrono::milliseconds(50));

    for (int i = 1; i <= so_buoc; i++) {
        int y_hien_tai = y_dau + (y_cuoi - y_dau) * i / so_buoc;
        PostMessage(h_game, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x_fix, y_hien_tai));
        this_thread::sleep_for(chrono::milliseconds(thoi_gian_keo));
    }

    PostMessage(h_game, WM_LBUTTONUP, 0, MAKELPARAM(x_fix, y_cuoi));
    this_thread::sleep_for(chrono::milliseconds(800)); // Đợi danh sách dừng hẳn
}
    

void DiBo_ThuanCode(HWND h_game, string huong) {
    int x_fix = 144;
    int y_dau, y_cuoi;
    int so_buoc = 50;    // Chia nhỏ bước để vuốt mượt
    int thoi_gian_keo = 15;

    if (huong == "xuong") { y_dau = 399; y_cuoi = 129; }
    else { y_dau = 399; y_cuoi = 538; }

    PostMessage(h_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x_fix, y_dau));
    this_thread::sleep_for(chrono::milliseconds(50));

    for (int i = 1; i <= so_buoc; i++) {
        int y_hien_tai = y_dau + (y_cuoi - y_dau) * i / so_buoc;
        PostMessage(h_game, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x_fix, y_hien_tai));
        this_thread::sleep_for(chrono::milliseconds(thoi_gian_keo));
    }

    PostMessage(h_game, WM_LBUTTONUP, 0, MAKELPARAM(x_fix, y_cuoi));
    this_thread::sleep_for(chrono::milliseconds(800)); // Đợi danh sách dừng hẳn
}

bool KiemTraTuiDay(cv::Mat anhChup) {
    // Vẽ vùng ROI nơi bảng "Balo đầy" thường xuất hiện
    // Tọa độ ví dụ: x=312, y=180, rộng=340, cao=120
    cv::Rect roiTuiDay(340, 93, 287, 71);

    // Đảm bảo vùng ROI không bị tràn ra ngoài ảnh
    roiTuiDay &= cv::Rect(0, 0, anhChup.cols, anhChup.rows);

    cv::Mat vungSoi = anhChup(roiTuiDay);
    // Quét ảnh "tui_day.png" trong vùng ROI này
    cv::Point p = BoDieuKhien::TimAnhTrongVung(vungSoi, "tui_day.png", cv::Rect(0, 0, vungSoi.cols, vungSoi.rows), 0.75);

    return (p.x != -1);
}

void ThucHienDiBan(ThongTinTool* tool);

bool FarmHasImage(const cv::Mat& screen, const char* file, cv::Rect roi, double score = 0.78) {
    return BoDieuKhien::TimAnhTrongVung(screen, file, roi, score).x != -1;
}

bool FarmIsHarvest(const cv::Mat& screen) {
    if (screen.empty() || screen.cols < 420 || screen.rows < 65) return false;
    return FarmHasImage(screen, "tieu_de_thu_hoach.png", {15, 5, 400, 60}) &&
        cv::mean(screen(cv::Rect(260, 20, 100, 20)))[2] > 190;
}

bool FarmIsFilter(const cv::Mat& screen) {
    return FarmHasImage(screen, "Loc_Nong_San.png", {300, 15, 350, 80});
}

void FarmCloseHarvest(ThongTinTool* tool) {
    for (int attempt = 0; attempt < 6 && tool->dangChay; ++attempt) {
        auto screen = BoDieuKhien::ChupManHinh(tool->h_game);
        if (!FarmIsHarvest(screen) && !FarmIsFilter(screen)) return;
        BoDieuKhien::BamChuot(tool->h_game, 921, FarmIsFilter(screen) ? 50 : 35);
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void FarmExitStore(ThongTinTool* tool);

bool FarmOpenHarvest(ThongTinTool* tool) {
    FarmExitStore(tool);
    tool->thongBaoStatus = "Mo Thu hoach truc tiep (khong ve nha)...";
    for (int attempt = 0; attempt < 14 && tool->dangChay; ++attempt) {
        auto screen = BoDieuKhien::ChupManHinh(tool->h_game);
        if (FarmIsHarvest(screen) || FarmIsFilter(screen)) return true;
        auto p = BoDieuKhien::TimAnhTrongVung(screen, "nut_thu_hoach_moi.png", {550, 40, 400, 210}, 0.78);
        if (p.x != -1) BoDieuKhien::BamChuot(tool->h_game, p.x, p.y);
        else if (attempt % 3 == 0) {
            if (FarmHasImage(screen, "nut_tien_ich_dong.png", {895, 0, 50, 50}, 0.78)) BoDieuKhien::BamChuot(tool->h_game, 925, 25);
            else BoDieuKhien::BamChuot(tool->h_game, 925, 75);
        }
        this_thread::sleep_for(chrono::milliseconds(650));
    }
    tool->thongBaoStatus = "Khong mo duoc Thu hoach; kiem tra giao dien LDPlayer 960x540";
    return false;
}

bool FarmChecked(const cv::Mat& screen, cv::Point center) {
    cv::Rect r(center.x - 7, center.y - 7, 14, 14);
    r &= cv::Rect(0, 0, screen.cols, screen.rows);
    if (r.empty()) return false;
    cv::Mat hsv;
    cv::cvtColor(screen(r), hsv, cv::COLOR_BGR2HSV);
    cv::Mat saturated;
    cv::inRange(hsv, cv::Scalar(0, 75, 65), cv::Scalar(179, 255, 255), saturated);
    return cv::countNonZero(saturated) > 35;
}

cv::Point FarmCompletedSaleOk(const cv::Mat& frame) {
    static const auto title=cv::imread("images/tieu_de_ban_xong_moi.png");
    static const auto ok=cv::imread("images/ok_ban_xong_moi.png");
    return FarmSaleResultOk(frame,title,ok);
}

bool FarmSellScreenReady(const cv::Mat& frame) {
    static const auto header=cv::imread("images/tieu_de_ban_moi.png");
    return FarmSaleReady(frame,header);
}

bool FarmDismissCompletedSale(ThongTinTool* tool) {
    bool clicked=false;
    for(int attempt=0;attempt<16&&tool->dangChay;++attempt) {
        auto frame=BoDieuKhien::ChupManHinh(tool->h_game);
        auto ok=FarmCompletedSaleOk(frame);
        if(ok.x>=0) {
            tool->thongBaoStatus="Ban xong: bam OK va cho dong hop thoai...";
            BoDieuKhien::BamChuot(tool->h_game,ok.x,ok.y);
            clicked=true;
        } else if(clicked&&FarmSellScreenReady(frame)) {
            tool->thongBaoStatus="Da dong OK sau ban";
            return true;
        } else if(!clicked)break;
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    tool->thongBaoStatus=clicked?"Da bam OK, chua xac minh dong hop thoai":"Khong thay hop thoai Hoan tat ban hang";
    return false;
}

void FarmExitStore(ThongTinTool* tool) {
    for (int attempt = 0; attempt < 8 && tool->dangChay; ++attempt) {
        auto frame = BoDieuKhien::ChupManHinh(tool->h_game);
        if (FarmCompletedSaleOk(frame).x>=0) {
            if(!FarmDismissCompletedSale(tool))return;
        } else if (FarmHasImage(frame, "shop_header.png", {80, 25, 230, 70})) {
            BoDieuKhien::BamChuot(tool->h_game, 844, 55);
        } else if (FarmHasImage(frame, "tieu_de_ban_moi.png", {15, 5, 400, 70})) {
            BoDieuKhien::BamChuot(tool->h_game, 915, 38);
        } else if (FarmHasImage(frame, "gio_hang_mua.png", {610, 180, 335, 190}, 0.80) ||
                   FarmHasImage(frame, "npc_mua_cong_cu_moi.png", {610, 180, 335, 190}, 0.80) ||
                   FarmHasImage(frame, "npc_ban_nongsan_moi.png", {610, 180, 335, 190}, 0.80)) {
            BoDieuKhien::BamChuot(tool->h_game, 780, 348);
        } else if (FarmHasImage(frame, "npc_dialog.png", {195, 390, 65, 100}, 0.82)) {
            BoDieuKhien::BamChuot(tool->h_game, 500, 450);
        } else return;
        this_thread::sleep_for(chrono::milliseconds(600));
    }
}

bool FarmOpenStore(ThongTinTool* tool, int kind) {
    if (kind < 0 || kind > 2 || !tool->dangChay) return false;
    FarmCloseHarvest(tool);
    FarmExitStore(tool);
    if (!tool->dangChay) return false;
    const char* shortcuts[] = {"nut_cua_hang_hat.png", "nut_cua_hang_cong_cu.png", "nut_ban_moi.png"};
    const char* doors[] = {"vao_hat.png", "vao_cong_cu_moi.png", "vao_ban_moi.png"};
    const char* choices[] = {"gio_hang_mua.png", "npc_mua_cong_cu_moi.png", "npc_ban_nongsan_moi.png"};
    const string storeName = kind == 0 ? "hat giong" : kind == 1 ? "cong cu" : "ban nong san";

    // Travel to the requested store first. A nearby NPC may belong to the
    // previous selling/tool workflow, so never click its door before travel.
    bool travelRequested = false;
    for (int attempt = 0; attempt < 20 && tool->dangChay; ++attempt) {
        tool->thongBaoStatus = "Mo " + storeName + ": tim nut den cua hang";
        auto frame = BoDieuKhien::ChupManHinh(tool->h_game);
        if (frame.empty()) { this_thread::sleep_for(chrono::milliseconds(250)); continue; }
        auto shortcut = BoDieuKhien::TimAnhTrongVung(frame, shortcuts[kind], {550, 40, 400, 210}, 0.88);
        if (shortcut.x != -1) {
            BoDieuKhien::BamChuot(tool->h_game, shortcut.x, shortcut.y);
            travelRequested = true;
            this_thread::sleep_for(chrono::milliseconds(900));
            break;
        }
        if (FarmHasImage(frame, "npc_dialog.png", {195, 390, 65, 100}, 0.82)) FarmExitStore(tool);
        else if (attempt % 4 == 0) {
            if (FarmHasImage(frame, "nut_tien_ich_dong.png", {895, 0, 50, 50}, 0.78)) BoDieuKhien::BamChuot(tool->h_game, 925, 25);
            else BoDieuKhien::BamChuot(tool->h_game, 925, 75);
        }
        this_thread::sleep_for(chrono::milliseconds(650));
    }
    if (!travelRequested || !tool->dangChay) {
        if (tool->dangChay) tool->thongBaoStatus = "Khong tim thay nut den cua hang " + storeName;
        return false;
    }

    // Let the arrival/NPC speech finish. Re-teleporting here restarts the
    // animation and can keep the entry icon hidden indefinitely.
    bool choiceClicked = false;
    for (int attempt = 0; attempt < 35 && tool->dangChay; ++attempt) {
        auto frame = BoDieuKhien::ChupManHinh(tool->h_game);
        if (frame.empty()) { this_thread::sleep_for(chrono::milliseconds(250)); continue; }
        bool opened = kind == 2 ? FarmHasImage(frame, "tieu_de_ban_moi.png", {15, 5, 400, 70})
                                : FarmHasImage(frame, "shop_header.png", {80, 25, 230, 70});
        if (choiceClicked && opened) {
            tool->thongBaoStatus = "Da mo cua hang " + storeName;
            return true;
        }
        auto choice = BoDieuKhien::TimAnhTrongVung(frame, choices[kind], {610, 180, 335, 190}, 0.80);
        if (choice.x != -1) {
            tool->thongBaoStatus = "Mo " + storeName + ": bam lua chon NPC";
            BoDieuKhien::BamChuot(tool->h_game, choice.x, choice.y);
            choiceClicked = true;
        } else if (FarmHasImage(frame, "npc_dialog.png", {195, 390, 65, 100}, 0.82)) {
            tool->thongBaoStatus = "Mo " + storeName + ": tiep tuc hoi thoai NPC";
            BoDieuKhien::BamChuot(tool->h_game, 500, 450);
        } else {
            auto door = BoDieuKhien::TimAnhTrongVung(frame, doors[kind], {350, 80, 260, 230}, 0.78);
            if (door.x != -1) {
                tool->thongBaoStatus = "Mo " + storeName + ": bam bieu tuong NPC";
                BoDieuKhien::BamChuot(tool->h_game, door.x, door.y);
            } else tool->thongBaoStatus = "Mo " + storeName + ": cho tai canh / bieu tuong NPC";
        }
        this_thread::sleep_for(chrono::milliseconds(650));
    }
    if (tool->dangChay) tool->thongBaoStatus = "Het thoi gian mo cua hang " + storeName + ": kiem tra NPC";
    return false;
}

void ThucHienDiBan(ThongTinTool* tool) {
    if (!FarmOpenStore(tool, 2)) return;
    bool completed = false;
    for (int batch = 0; batch < 10 && tool->dangChay; ++batch) {
        auto frame = BoDieuKhien::ChupManHinh(tool->h_game);
        auto select = BoDieuKhien::TimAnhTrongVung(frame, "chon_tu_dong_moi.png", {545, 460, 150, 65}, 0.82);
        if (select.x == -1) break;
        BoDieuKhien::BamChuot(tool->h_game, select.x, select.y);
        this_thread::sleep_for(chrono::milliseconds(600));
        frame = BoDieuKhien::ChupManHinh(tool->h_game);
        if (!FarmHasImage(frame, "bo_chon_tat_ca_moi.png", {545, 460, 150, 65}, 0.82)) break;
        BoDieuKhien::BamChuot(tool->h_game, 791, 480);
        bool finished = false;
        for (int attempt = 0; attempt < 48 && tool->dangChay; ++attempt) {
            this_thread::sleep_for(chrono::milliseconds(500));
            frame = BoDieuKhien::ChupManHinh(tool->h_game);
            auto resultOk=FarmCompletedSaleOk(frame);
            if(resultOk.x>=0) {
                if(!FarmDismissCompletedSale(tool))return;
                finished=true;
                break;
            } else if (FarmHasImage(frame, "xac_nhan_ban_moi.png", {300, 60, 350, 100}, 0.82)) {
                BoDieuKhien::BamChuot(tool->h_game, 480, 422);
            } else if (FarmHasImage(frame, "xac_nhan_ban_cao_cap_moi.png", {300, 60, 350, 100}, 0.82)) {
                auto confirm = BoDieuKhien::TimAnhTrongVung(frame, "nut_xac_nhan_ban_moi.png", {495, 350, 225, 125}, 0.82);
                if (confirm.x != -1) BoDieuKhien::BamChuot(tool->h_game, confirm.x, confirm.y);
            } else {
                if (FarmSellScreenReady(frame)&&FarmHasImage(frame, "chon_tu_dong_moi.png", {545, 460, 150, 65}, 0.82)) { finished = true; break; }
            }
        }
        if (!finished) { tool->thongBaoStatus = "Chua xac minh ban xong; dung luong ban"; return; }
        completed = true;
        this_thread::sleep_for(chrono::milliseconds(700));
    }
    if (tool->dangChay) FarmExitStore(tool);
    if (completed) ++tool->soLuotBan;
    tool->thongBaoStatus = completed ? "Da xu ly luong ban" : "Khong co nong san duoc chon de ban";
}

bool FarmPrepareHarvestFilter(ThongTinTool* tool,bool apply=true) {
    tool->ketQuaLocTrai.clear();
    if (!FarmOpenHarvest(tool)) { tool->time_cho_hoi_qua = GetTickCount64() + 15000; return false; }
    bool filterOpened = false;
    for (int attempt = 0; attempt < 10 && tool->dangChay; ++attempt) {
        auto screen = BoDieuKhien::ChupManHinh(tool->h_game);
        if (FarmIsFilter(screen)) { filterOpened = true; break; }
        auto p = BoDieuKhien::TimAnhTrongVung(screen, "Loc.png", {750, 0, 150, 70}, 0.78);
        if (p.x != -1) BoDieuKhien::BamChuot(tool->h_game, p.x, p.y);
        this_thread::sleep_for(chrono::milliseconds(450));
    }
    if (!filterOpened) { tool->thongBaoStatus = "Khong mo duoc bo loc"; FarmCloseHarvest(tool); return false; }

    // Reset both tabs so selections from an earlier mode cannot leak into this one.
    BoDieuKhien::BamChuot(tool->h_game, 60, 93);
    this_thread::sleep_for(chrono::milliseconds(250));
    BoDieuKhien::BamChuot(tool->h_game, 397, 472);
    this_thread::sleep_for(chrono::milliseconds(250));
    auto variants = BoDieuKhien::ChupManHinh(tool->h_game);
    bool exclude = tool->modeThuHoachTenTim == CHE_DO_HAI_LOC_BIEN_THE;
    if (FarmChecked(variants, {57, 402}) != exclude) {
        BoDieuKhien::BamChuot(tool->h_game, 57, 402);
        this_thread::sleep_for(chrono::milliseconds(250));
    }
    BoDieuKhien::BamChuot(tool->h_game, 230, 93);
    this_thread::sleep_for(chrono::milliseconds(250));
    BoDieuKhien::BamChuot(tool->h_game, 397, 472);
    this_thread::sleep_for(chrono::milliseconds(300));

    int selected = 0;
    if (tool->modeThuHoachTenTim == CHE_DO_CHON_HAT) {
        auto screen = BoDieuKhien::ChupManHinh(tool->h_game);
        auto choices=FarmReadCropChoices(screen);
        int requested=0;std::string missing;
        for (int i = 0; i < SO_NONG_SAN && tool->dangChay; ++i) {
            if (!tool->cacHatDuocChon[i]) continue;
            ++requested;
            auto found=std::find_if(choices.begin(),choices.end(),[&](const auto& item){return item.id==i;});
            if(found==choices.end()&&i!=13&&i!=22&&FileTonTai(ds_anh_chu_qua[i])) {
                // Keep legacy images only for an unreadable label; never
                // override a different readable crop or prefix-match Dua/Tao.
                found=std::find_if(choices.begin(),choices.end(),[&](const auto& item){
                    return item.id<0&&item.text.empty()&&FarmHasImage(screen,ds_anh_chu_qua[i].c_str(),item.tile,.90);
                });
            }
            if (found==choices.end()) {
                if(!missing.empty())missing+=", ";missing+=ds_hat_gionghai[i];
                continue;
            }
            auto crop=FarmCropClick(found->tile);
            auto checkbox=FarmCropCheckbox(found->tile);
            bool checked = false;
            for (int retry = 0; retry < 2 && tool->dangChay; ++retry) {
                auto before = BoDieuKhien::ChupManHinh(tool->h_game);
                if (FarmChecked(before, checkbox)) { checked = true; break; }
                BoDieuKhien::BamChuot(tool->h_game, crop.x, crop.y);
                this_thread::sleep_for(chrono::milliseconds(300));
                auto after = BoDieuKhien::ChupManHinh(tool->h_game);
                if (FarmChecked(after, checkbox)) { checked = true; break; }
            }
            if (checked) ++selected;
            else {if(!missing.empty())missing+=", ";missing+=ds_hat_gionghai[i];}
        }
        tool->ketQuaLocTrai="Bo loc: "+std::to_string(selected)+"/"+std::to_string(requested)+" loai";
        if(!missing.empty())tool->ketQuaLocTrai+="; chua chon duoc: "+missing;
        tool->thongBaoStatus=tool->ketQuaLocTrai;
        if (selected == 0) {
            if(apply)FarmCloseHarvest(tool);
            bool unread=std::any_of(choices.begin(),choices.end(),[](const auto& item){return item.id<0&&item.text.empty();});
            tool->time_cho_hoi_qua = GetTickCount64() + (unread?15000:300000);
            return false;
        }
    }
    else tool->ketQuaLocTrai="Bo loc: "+std::string(exclude?"hai loc bo bien the":"hai ALL");
    if (!tool->dangChay) return false;
    if(!apply){tool->thongBaoStatus=tool->ketQuaLocTrai+"; dang hien bo loc de kiem tra";return true;}
    BoDieuKhien::BamChuot(tool->h_game, 564, 472);
    bool applied = false;
    for (int retry = 0; retry < 10 && tool->dangChay; ++retry) {
        this_thread::sleep_for(chrono::milliseconds(350));
        if (FarmIsHarvest(BoDieuKhien::ChupManHinh(tool->h_game))) { applied = true; break; }
    }
    if (!applied) { tool->thongBaoStatus = "Khong ap dung duoc bo loc"; FarmCloseHarvest(tool); return false; }
    return true;
}

void ThuHoachTenTim(ThongTinTool* tool) {
    if (GetTickCount64() < tool->time_cho_hoi_qua) {
        tool->thongBaoStatus = "Cho trai chin: " + to_string((tool->time_cho_hoi_qua - GetTickCount64()) / 1000) + "s";
        return;
    }
    if(!FarmPrepareHarvestFilter(tool))return;

    auto screen = BoDieuKhien::ChupManHinh(tool->h_game);
    bool locked = FarmHasImage(screen, "khoa_thu_hoach.png", {895, 450, 50, 65}, 0.75);
    if (tool->thuHoachBangTenTim) {
        if (locked) {
            tool->thongBaoStatus = "Ten Tim: nut dong loat dang bi khoa; khong tu chuyen thu tung trai";
            FarmCloseHarvest(tool);
            tool->time_cho_hoi_qua = GetTickCount64() + 30000;
            return;
        }
        tool->thongBaoStatus = "Thu hoach bang TEN TIM (dong loat)...";
        BoDieuKhien::BamChuot(tool->h_game, 855, 499);
        bool confirmed = false;
        for (int retry = 0; retry < 15 && tool->dangChay; ++retry) {
            this_thread::sleep_for(chrono::milliseconds(400));
            auto frame = BoDieuKhien::ChupManHinh(tool->h_game);
            if (!FarmHasImage(frame, "tieu_de_ten_tim_moi.png", {300, 60, 360, 100}, 0.82)) continue;
            auto p = BoDieuKhien::TimAnhTrongVung(frame, "xac_nhan_ten_tim_moi.png", {485, 350, 235, 120}, 0.82);
            if (p.x != -1) { BoDieuKhien::BamChuot(tool->h_game, p.x, p.y); confirmed = true; break; }
        }
        if (!confirmed) {
            tool->thongBaoStatus = "Ten Tim: khong thay hop xac nhan thu hoach; dung luot nay";
            FarmCloseHarvest(tool);
            tool->time_cho_hoi_qua = GetTickCount64() + 15000;
            return;
        }
        bool completed = false;
        int confirmRetries = 0;
        for (int retry = 0; retry < 20 && tool->dangChay; ++retry) {
            this_thread::sleep_for(chrono::milliseconds(400));
            auto frame = BoDieuKhien::ChupManHinh(tool->h_game);
            if (FarmHasImage(frame, "tieu_de_ten_tim_moi.png", {300, 60, 360, 100}, 0.82)) {
                // LDPlayer can lose a click as the modal finishes animating.
                // Retry only while this exact confirmation remains visible.
                if (retry % 3 == 2 && confirmRetries < 3) {
                    auto p = BoDieuKhien::TimAnhTrongVung(frame, "xac_nhan_ten_tim_moi.png", {485, 350, 235, 120}, 0.82);
                    if (p.x != -1) { BoDieuKhien::BamChuot(tool->h_game, p.x, p.y); ++confirmRetries; }
                }
                continue;
            }
            auto ok = BoDieuKhien::TimAnhTrongVung(frame, "ok.png", {250, 250, 500, 250}, 0.80);
            if (ok.x != -1) { BoDieuKhien::BamChuot(tool->h_game, ok.x, ok.y); continue; }
            if (FarmIsHarvest(frame)) { completed = true; break; }
        }
        if (!completed) {
            tool->thongBaoStatus = "Ten Tim: da bam xac nhan, chua thay man hinh thu hoach tro lai";
            tool->time_cho_hoi_qua = GetTickCount64() + 30000;
            return;
        }
        tool->thongBaoStatus = "Da thu hoach bang TEN TIM (dong loat)";
    } else {
        // Ordinary harvesting is used only when the user explicitly selects it.
        int harvested = 0, missingFrames = 0;
        const auto deadline = GetTickCount64() + 90000;
        while (tool->dangChay && harvested < 120 && GetTickCount64() < deadline) {
            auto frame = BoDieuKhien::ChupManHinh(tool->h_game);
            if (KiemTraTuiDay(frame)) break;
            auto p = BoDieuKhien::TimAnhTrongVung(frame, "thu_hoach_don.png", {25, 75, 895, 390}, 0.84);
            if (p.x == -1) {
                if (++missingFrames >= 3) break;
                this_thread::sleep_for(chrono::milliseconds(500));
                continue;
            }
            missingFrames = 0;
            tool->thongBaoStatus = "Thu tung trai (da chon cach thu thuong): " + to_string(harvested + 1);
            BoDieuKhien::BamChuot(tool->h_game, p.x, p.y);
            this_thread::sleep_for(chrono::milliseconds(1000));
            auto after = BoDieuKhien::ChupManHinh(tool->h_game);
            auto confirmation = BoDieuKhien::TimAnhTrongVung(after, "xac_nhan.png", {250, 210, 470, 280}, 0.82);
            if (confirmation.x != -1) {
                BoDieuKhien::BamChuot(tool->h_game, confirmation.x, confirmation.y);
                this_thread::sleep_for(chrono::milliseconds(1000));
            }
            ++harvested;
        }
        tool->thongBaoStatus = "Da bam " + to_string(harvested) + " luot thu hoach thuong";
    }
    FarmCloseHarvest(tool);
    if(!tool->ketQuaLocTrai.empty())tool->thongBaoStatus+="; "+tool->ketQuaLocTrai;
    tool->time_cho_hoi_qua = GetTickCount64() + 30000;
    if (tool->kichHoatBan && tool->dangChay) ThucHienDiBan(tool);
}

void ThuHoachTenTimall(ThongTinTool* tool) { ThuHoachTenTim(tool); }


bool FarmOpenSeedShop(ThongTinTool* tool) {
    return FarmOpenStore(tool, 0);
}

void ThucHienMuaHat(ThongTinTool* tool) {
    bool anySelected = false;
    for (int i = 0; i < SO_HAT_MUA; ++i) anySelected |= tool->cacHatCanMua[i];
    if (!anySelected || !FarmOpenSeedShop(tool)) return;
    bool processed[SO_HAT_MUA] = {};
    int bought = 0;
    // A fresh shop opens at the top. One forward pass serves all selected seeds,
    // instead of repeatedly scrolling down and back up for every seed.
    for (int page = 0; page < 24 && tool->dangChay; ++page) {
        auto frame = BoDieuKhien::ChupManHinh(tool->h_game);
        for (int i = 0; i < SO_HAT_MUA && tool->dangChay; ++i) {
            if (!tool->cacHatCanMua[i] || processed[i]) continue;
            auto p = BoDieuKhien::TimAnhTrongVung(frame, ds_anh_hat[i], {110, 82, 70, 431}, 0.94);
            if (p.x == -1) continue;
            processed[i] = true;
            tool->thongBaoStatus = "Kiem tra kho: " + string(ds_hat_giongmua[i]);
            BoDieuKhien::BamChuot(tool->h_game, p.x, p.y);
            this_thread::sleep_for(chrono::milliseconds(450));
            auto selected = BoDieuKhien::ChupManHinh(tool->h_game);
            if (FarmHasImage(selected, "chu_trong_kho_moi.png", {525, 390, 200, 50}, 0.78)) {
                thao_tac_mua_allhat(tool);
                ++bought;
            }
            frame = BoDieuKhien::ChupManHinh(tool->h_game);
        }
        bool done = true;
        for (int i = 0; i < SO_HAT_MUA; ++i) if (tool->cacHatCanMua[i] && !processed[i]) done = false;
        if (done) break;
        auto before = BoDieuKhien::ChupManHinh(tool->h_game);
        CuonTrang_ThuanCode(tool->h_game, "xuong");
        auto after = BoDieuKhien::ChupManHinh(tool->h_game);
        if (!before.empty() && before.size() == after.size()) {
            cv::Rect strip(110, 110, 65, 350);
            double difference = cv::norm(before(strip), after(strip), cv::NORM_L1) / (strip.area() * 3.0);
            if (difference < 1.5) break;
        }
    }
    if (tool->dangChay) FarmExitStore(tool);
    tool->thongBaoStatus = "Xong kiem tra cua hang: " + to_string(bought) + " loai co hang";
    tool->hatTrongDaNho.Purchased(bought);
}


void ThucHienMuaCongCu(ThongTinTool* tool) {
    bool any = false;
    for (int i = 0; i < 8; ++i) any |= tool->cacCongCuCanMua[i];
    if (!any || !FarmOpenStore(tool, 1)) return;
    bool processed[8] = {};
    for (int page = 0; page < 10 && tool->dangChay; ++page) {
        auto frame = BoDieuKhien::ChupManHinh(tool->h_game);
        for (int i = 0; i < 8 && tool->dangChay; ++i) {
            if (!tool->cacCongCuCanMua[i] || processed[i]) continue;
            auto p = BoDieuKhien::TimAnhTrongVung(frame, ds_anh_cong_cu[i], {105, 82, 80, 431}, 0.86);
            if (p.x == -1) continue;
            processed[i] = true;
            tool->thongBaoStatus = "Kiem tra cong cu: " + string(ds_cong_cu[i]);
            BoDieuKhien::BamChuot(tool->h_game, p.x, p.y);
            this_thread::sleep_for(chrono::milliseconds(450));
            auto selected = BoDieuKhien::ChupManHinh(tool->h_game);
            if (FarmHasImage(selected, "chu_trong_kho_moi.png", {525, 390, 200, 50})) thao_tac_mua_allhat(tool);
            frame = BoDieuKhien::ChupManHinh(tool->h_game);
        }
        bool done = true;
        for (int i = 0; i < 8; ++i) if (tool->cacCongCuCanMua[i] && !processed[i]) done = false;
        if (done) break;
        auto before = BoDieuKhien::ChupManHinh(tool->h_game);
        CuonTrang_ThuanCode(tool->h_game, "xuong");
        auto after = BoDieuKhien::ChupManHinh(tool->h_game);
        if (!before.empty() && before.size() == after.size()) {
            cv::Rect strip(110, 110, 65, 350);
            if (cv::norm(before(strip), after(strip), cv::NORM_L1) / (strip.area() * 3.0) < 1.5) break;
        }
    }
    if (tool->dangChay) FarmExitStore(tool);
    tool->thongBaoStatus = "Da kiem tra cua hang cong cu";
}

#include "farm_plant_runtime.h"

// thu nhỏ màn hình 
void ZoomNhoManHinh_SieuMuot1(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "Dang ghim chuot phai & day chuot trai ra (Thu nho)...";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    // Tọa độ tâm màn hình (Mốc cố định cho ngón 1)
    int x_tam = 480;
    int y_tam = 270;
    LPARAM lParamTam = MAKELPARAM(x_tam, y_tam);

    // Tọa độ của ngón 2 (Bắt đầu từ gần tâm, đẩy dần ra rìa phải)
    int x_ngon2_bat_dau = x_tam + 40; // Bắt đầu sát tâm
    int x_ria_ket_thuc = 820;         // Đẩy ra tận rìa màn hình
    int y_ria = 270;

    // --- BƯỚC 1: GHIM CHẶT CHUỘT PHẢI TẠI TÂM (NGÓN TAY 1 CỐ ĐỊNH) ---
    PostMessage(h_gameplay, WM_RBUTTONDOWN, MK_RBUTTON, lParamTam);
    this_thread::sleep_for(chrono::milliseconds(50));

    // --- BƯỚC 2: DÙNG CHUỘT TRÁI ĐẨY RA XA TRONG 5 GIÂY (NGÓN TAY 2 DI CHUYỂN) ---
    int tong_thoi_gian_ms = 3000;
    int thoi_gian_moi_buoc_ms = 20; // 30ms một bước cho mượt căng đét 60fps
    int so_buoc = tong_thoi_gian_ms / thoi_gian_moi_buoc_ms;

    // Tính toán quãng đường cộng thêm vào X sau mỗi bước
    double buoc_dich_x = (double)(x_ria_ket_thuc - x_ngon2_bat_dau) / so_buoc;
    double x_hien_tai_ngon2 = x_ngon2_bat_dau;

    for (int i = 0; i < so_buoc; i++) {
        if (!thongTin->dangChay) break;

        // ĐỔI LOGIC: Cộng dồn để tọa độ X tiến ra xa rìa
        x_hien_tai_ngon2 += buoc_dich_x;
        if (x_hien_tai_ngon2 > x_ria_ket_thuc) x_hien_tai_ngon2 = x_ria_ket_thuc;

        LPARAM lParamNgon2 = MAKELPARAM(cvRound(x_hien_tai_ngon2), y_ria);

        // Nhấp giữ chuột trái và di chuyển ra ngoài
        PostMessage(h_gameplay, WM_LBUTTONDOWN, MK_LBUTTON | MK_RBUTTON, lParamNgon2);
        PostMessage(h_gameplay, WM_MOUSEMOVE, MK_LBUTTON | MK_RBUTTON, lParamNgon2);
        PostMessage(h_gameplay, WM_LBUTTONUP, MK_RBUTTON, lParamNgon2);

        // Cập nhật text ra UI mỗi giây
        if (i % 33 == 0) {
            thongTin->thongBaoStatus = "Dang thu nho camera... con " + to_string((tong_thoi_gian_ms - (i * thoi_gian_moi_buoc_ms)) / 1000) + "s";
        }

        this_thread::sleep_for(chrono::milliseconds(thoi_gian_moi_buoc_ms));
    }

    // --- BƯỚC 3: NHẢ CHUỘT PHẢI Ở TÂM RA ---
    PostMessage(h_gameplay, WM_RBUTTONUP, 0, lParamTam);

    thongTin->thongBaoStatus = "Da thu nho camera ";
}






void SetUpCauCaMapKnd(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "--- KND NEW: BAT DAU SET UP DI CAU ---";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    // =========================================================================
    // KHAI BÁO CÁC VÙNG ROI VÀ TÊN ẢNH ĐOÁN
    // =========================================================================
    cv::Rect roi_NutBanDo(885, 113, 60, 60);             // Vùng chứa nút mở Bản Đồ
    cv::Rect roi_TamGiacNgang(5, 235, 70, 70);       // Vùng chứa nút tam giác ngang  5, 235, 70, 70
    cv::Rect roi_ChutThongTinKhuVuc(34, 8, 80, 80);  // Vùng check chữ "Thông tin khu vực"
    cv::Rect roi_BayCaHoXanh(69, 71, 290, 468);         // Vùng cố định để quét tìm chữ "Bẫy cá Hố xanh"
    cv::Rect roi_BangChiDuong(414, 420, 133, 60);       // Vùng xuất hiện Bảng Chỉ Đường

    string anh_NutBanDo = "nut_ban_do.png";
    string anh_TamGiacNgang = "tam_giac_ngang.png";
    string anh_ChutThongTinKhuVuc = "thong_tin_khu_vuc.png";
    string anh_BayCaHoXanh = "bay_ca_ho_xanh.png";
    string anh_BangChiDuong = "bang_chi_duong.png";
    string anh_NutChiDuong = "nut_chi_duong.png";

    // =========================================================================
    // BƯỚC 1: ẤN NÚT BẢN ĐỒ CHO ĐẾN KHI BIẾN MẤT
    // =========================================================================
    thongTin->thongBaoStatus = "KND - Buoc 1: Dang mo ban do...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pBanDo = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NutBanDo, roi_NutBanDo, 0.8);
        if (pBanDo.x == -1) break; // Bản đồ mở, nút biến mất -> Thoát

        BoDieuKhien::BamChuot(h_gameplay, pBanDo.x, pBanDo.y);
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    // =========================================================================
    // BƯỚC 2: ẤN TAM GIÁC NGANG CHO ĐẾN KHI HIỆN "THÔNG TIN KHU VỰC"
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KND - Buoc 2: Dang mo thong tin khu vuc...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra chữ "Thông tin khu vực" đã hiện ra chưa
        thongTin->thongBaoStatus = "Kiểm tra chữ Thông tin khu vực đã hiện ra chưa";
        cv::Point pChuTTKV = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChutThongTinKhuVuc, roi_ChutThongTinKhuVuc, 0.7);
        if (pChuTTKV.x != -1) break; // Đã thấy chữ -> Danh sách đã mở -> Thoát

        // Nếu chưa thấy thì tiếp tục ấn vào tam giác ngang
        thongTin->thongBaoStatus = "Nếu chưa thấy thì tiếp tục ấn vào tam giác ngang";
        cv::Point pTamGiac = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_TamGiacNgang, roi_TamGiacNgang, 0.8);
        if (pTamGiac.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pTamGiac.x, pTamGiac.y);
        }
        this_thread::sleep_for(chrono::milliseconds(300));
    }

    // =========================================================================
    // BƯỚC 3: LƯỚT XUỐNG TÌM BẪY CÁ HỐ XANH CHO ĐẾN KHI HIỆN BẢNG CHỈ ĐƯỜNG
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KND - Buoc 3: Dang luot tim Bay ca Ho xanh...";

    int gioiHanCuon = 0; // Tránh kẹt loop nếu danh sách hết trang
    while (thongTin->dangChay && gioiHanCuon < 15) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // 1. Kiểm tra xem Bảng Chỉ Đường xuất hiện chưa
        cv::Point pBangChiDuong = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BangChiDuong, roi_BangChiDuong, 0.8);
        if (pBangChiDuong.x != -1) break; // Thấy bảng chỉ đường -> Sang bước cuối

        // 2. Tìm chữ "Bẫy cá Hố xanh" trong vùng danh sách
        cv::Point pBayCa = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BayCaHoXanh, roi_BayCaHoXanh, 0.8);
        if (pBayCa.x != -1) {
            // Thấy rồi thì click liên tục vào nó cho đến khi hiện bảng chỉ đường
            BoDieuKhien::BamChuot(h_gameplay, pBayCa.x, pBayCa.y);
            this_thread::sleep_for(chrono::milliseconds(2000));
            continue; // Quay lại vòng lặp check xem bảng chỉ đường lên chưa
        }

        // 3. Nếu không thấy chữ "Bẫy cá Hố xanh" ở trang hiện tại -> Cuộn trang xuống dưới
        // Hướng "xuong" theo hàm của ông: kéo từ dưới lên để đẩy danh sách đi xuống
        CuonTrang_ThuanCode(h_gameplay, "xuong");
        gioiHanCuon++;
    }

    // =========================================================================
    // BƯỚC 4: BẢNG CHỈ ĐƯỜNG HIỆN RA -> ẤN NÚT CHỈ ĐƯỜNG
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KND - Buoc 4: Dang an nut chi duong...";

    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pNutChiDuong = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NutChiDuong, roi_BangChiDuong, 0.8);
        if (pNutChiDuong.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pNutChiDuong.x, pNutChiDuong.y);
            this_thread::sleep_for(chrono::milliseconds(200));
            break; // Hoàn thành
        }
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    thongTin->thongBaoStatus = "--- KND: UP MAP MOI THANH CONG! ---";
}

// Hàm điều phối chính: Di chuyển từ Plaza sang Khu Nghỉ Dưỡng (KND)
bool ChuyenSangMapKND(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "Bat dau luong chuyen sang map KND...";

    // --- CÁC ĐƯỜNG DẪN ẢNH MẪU ---
    string img_BanDo = "nut_ban_do.png";
    string img_TamGiacNgang = "tam_giac_ngang.png";
    string img_ThongTinKhuVuc = "thong_tin_khu_vuc.png";
    string img_ChuBenCang = "chu_ben_cang.png";
    string img_BangTimDuong = "bang_chi_duong.png";
    string img_NutTimDuong = "nut_chi_duong.png";
    string img_BieuTuongNPC = "bieu_tuong_npc_ben_cang.png";
    string img_MatCuoiXanh = "mat_cuoi_xanh.png";

    // --- BỘ CÁC HỘP ROI KHÓA MỤC TIÊU KHÉP KÍN ---
    cv::Rect roi_NutBanDo(885, 113, 60, 60);            // Vùng chứa nút mở Bản Đồ
    cv::Rect roi_TamGiacNgang(5, 235, 70, 70);          // Vùng chứa nút tam giác ngang
    cv::Rect roi_ChutThongTinKhuVuc(34, 8, 80, 80);     // Vùng check tiêu đề Thông tin khu vực
    cv::Rect roi_BangChiDuong(414, 420, 133, 60);       // Vùng check Bảng tìm đường / Nút tìm đường
    cv::Rect roi_BayAo(69, 71, 290, 468);               // Vùng Danh sách để kéo cuộn tìm chữ Bến Cảng

    // Hai vùng tương tác với NPC ở bến tàu
    cv::Rect roi_NPC(215, 102, 542, 375);               // Vùng xuất hiện biểu tượng bong bóng thoại nói chuyện với NPC
    cv::Rect roi_MatCuoiXanh(630, 270, 57, 57);         // Vùng chứa nút Mặt cười màu xanh để đồng ý ra đảo

    cv::Mat anhGoc;
    cv::Point pFound;
    int maxThoatKet = 20; // Giới hạn tối đa 20 lần bấm thử cho mỗi bước để chống kẹt luồng khi lag

    // =========================================================================
    // BƯỚC 1: Tìm bản đồ trong ROI riêng rồi ấn vào ĐẾN KHI tam giác ngang xuất hiện
    // =========================================================================
    thongTin->thongBaoStatus = "KND: Dang mo ban do...";
    int demStep1 = 0;
    while (thongTin->dangChay) {
        anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);

        pFound = BoDieuKhien::TimAnhTrongVung(anhGoc, img_TamGiacNgang, roi_TamGiacNgang, 0.75);
        if (pFound.x != -1) break;

        cv::Point pBanDo = BoDieuKhien::TimAnhTrongVung(anhGoc, img_BanDo, roi_NutBanDo, 0.75);
        if (pBanDo.x != -1) {
            BoDieuKhien::BamChuot(thongTin->h_game, pBanDo.x, pBanDo.y);
        }

        demStep1++;
        if (demStep1 > maxThoatKet) return false;
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    // =========================================================================
    // BƯỚC 2: Khi tam giác ngang xuất hiện thì ấn vào CHO ĐẾN KHI thông tin khu vực xuất hiện
    // =========================================================================
    thongTin->thongBaoStatus = "KND: Dang mo thong tin khu vuc...";
    int demStep2 = 0;
    while (thongTin->dangChay) {
        anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);

        pFound = BoDieuKhien::TimAnhTrongVung(anhGoc, img_ThongTinKhuVuc, roi_ChutThongTinKhuVuc, 0.75);
        if (pFound.x != -1) break;

        cv::Point pTamGiac = BoDieuKhien::TimAnhTrongVung(anhGoc, img_TamGiacNgang, roi_TamGiacNgang, 0.75);
        if (pTamGiac.x != -1) {
            BoDieuKhien::BamChuot(thongTin->h_game, pTamGiac.x, pTamGiac.y);
        }

        demStep2++;
        if (demStep2 > maxThoatKet) return false;
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    // =========================================================================
    // BƯỚC 3: Thực hiện logic kéo tìm chữ bến cảng trong vùng Bãy Ảo
    // =========================================================================
    thongTin->thongBaoStatus = "KND: Dang vuot tim chu Ben Cang...";
    int demStep3 = 0;
    bool timThayBenCang = false;
    cv::Point pBenCang;

    while (thongTin->dangChay && demStep3 < 15) {
        anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);

        pBenCang = BoDieuKhien::TimAnhTrongVung(anhGoc, img_ChuBenCang, roi_BayAo, 0.75);
        if (pBenCang.x != -1) {
            timThayBenCang = true;
            break;
        }

        // --- ĐÃ ĐỔI SANG HÀM CUỐN TRANG CHUẨN CỦA ÔNG GIÁO ---
        CuonTrang_ThuanCode(thongTin->h_game, "xuong");

        demStep3++;
        // Không cần sleep nhiều nữa vì cuối hàm CuonTrang_ThuanCode ông đã cho nghỉ 800ms rồi
    }
    if (!timThayBenCang) return false;

    // =========================================================================
    // BƯỚC 4: Thấy bến cảng thì ấn vào CHO ĐẾN KHI bảng tìm đường xuất hiện
    // =========================================================================
    thongTin->thongBaoStatus = "KND: Chon Ben Cang, doi bang tim duong...";
    int demStep4 = 0;
    while (thongTin->dangChay) {
        anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);

        pFound = BoDieuKhien::TimAnhTrongVung(anhGoc, img_BangTimDuong, roi_BangChiDuong, 0.75);
        if (pFound.x != -1) break;

        BoDieuKhien::BamChuot(thongTin->h_game, pBenCang.x, pBenCang.y);

        demStep4++;
        if (demStep4 > maxThoatKet) return false;
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    // =========================================================================
    // BƯỚC 5: Bấm vào nút tìm đường CHO ĐẾN KHI nút tìm đường biến mất
    // =========================================================================
    thongTin->thongBaoStatus = "KND: Kich hoat Auto-Path chay ra ben cang...";
    int demStep5 = 0;
    while (thongTin->dangChay) {
        anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);

        pFound = BoDieuKhien::TimAnhTrongVung(anhGoc, img_NutTimDuong, roi_BangChiDuong, 0.75);
        if (pFound.x == -1) break;

        BoDieuKhien::BamChuot(thongTin->h_game, pFound.x, pFound.y);

        demStep5++;
        if (demStep5 > maxThoatKet) return false;
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    // =========================================================================
    // NHỊP CHỜ ĐẶC BIỆT: Chờ nhân vật tự chạy bộ đến chỗ NPC Bến Cảng
    // =========================================================================
    thongTin->thongBaoStatus = "KND: Nhan vat dang di bo, cho tiep can NPC...";
    bool daDenChoNPC = false;
    for (int i = 0; i < 60; i++) {
        if (!thongTin->dangChay) return false;
        anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);

        cv::Point pNPC = BoDieuKhien::TimAnhTrongVung(anhGoc, img_BieuTuongNPC, roi_NPC, 0.72);
        if (pNPC.x != -1) {
            daDenChoNPC = true;
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    if (!daDenChoNPC) return false;

    // =========================================================================
    // BƯỚC 6: Tìm biểu tượng tương tác rồi ấn vào CHO ĐẾN KHI nó hiện ra mặt cười xanh
    // =========================================================================
    thongTin->thongBaoStatus = "KND: Bat dau noi chuyen voi NPC...";
    int demStep6 = 0;
    cv::Point pMatCuoi;
    while (thongTin->dangChay) {
        anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);

        pMatCuoi = BoDieuKhien::TimAnhTrongVung(anhGoc, img_MatCuoiXanh, roi_MatCuoiXanh, 0.75);
        if (pMatCuoi.x != -1) break;

        cv::Point pNPC = BoDieuKhien::TimAnhTrongVung(anhGoc, img_BieuTuongNPC, roi_NPC, 0.72);
        if (pNPC.x != -1) {
            BoDieuKhien::BamChuot(thongTin->h_game, pNPC.x, pNPC.y);
        }

        demStep6++;
        if (demStep6 > maxThoatKet) return false;
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    // =========================================================================
    // BƯỚC 7 & 8: Ấn vào nút mặt cười xanh và Spam bấm 4 lần delay 2 giây
    // =========================================================================
    thongTin->thongBaoStatus = "KND: Spam xac nhan vao Đảo Nghỉ Dưỡng...";

    BoDieuKhien::BamChuot(thongTin->h_game, pMatCuoi.x, pMatCuoi.y);
    this_thread::sleep_for(chrono::seconds(2));

    for (int i = 0; i < 4; i++) {
        if (!thongTin->dangChay) break;
        BoDieuKhien::BamChuot(thongTin->h_game, pMatCuoi.x, pMatCuoi.y);
        this_thread::sleep_for(chrono::seconds(2));
    }

    thongTin->thongBaoStatus = "KND: Da hoan thanh chuoi logic sang KND, doi loading map!";
    return true;
}












// Hàm chuyên trách: Mở điện thoại, chọn tiệc trò chơi để chuyển sang Map Plaza
void ChuyenToiMapPlaza(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "--- PLAZA: BAT DAU DI CHUYEN SANG MAP PLAZA ---";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    // =========================================================================
    // SỬ DỤNG BỘ ROI VÀ TỌA ĐỘ CHUẨN ĐÃ LƯU CỦA ÔNG GIÁO (BƯỚC 1 - BƯỚC 3)
    // =========================================================================
    int x_DienThoai = 931, y_DienThoai = 217;            // Tọa độ cố định của nút Điện thoại

    cv::Rect roi_ChuChoiGame(647, 127, 80, 30);       // Vùng check chữ "Chơi game" khi mở ĐT
    cv::Rect roi_BangChonGame(379, 12, 190, 60);      // Vùng xuất hiện Bảng chọn game
    cv::Rect roi_ChuTiecTroChoi(479, 216, 111, 30);     // Vùng quét chữ "Tiệc trò chơi"

    string anh_ChuChoiGame = "chu_choi_game.png";
    string anh_BangChonGame = "bang_chon_game.png";
    string anh_ChuTiecTroChoi = "tiec_tro_choi.png";

    // =========================================================================
    // BƯỚC 1: CLICK TỌA ĐỘ ĐIỆN THOẠI CHO TỚI KHI HIỆN CHỮ "CHƠI GAME" (1s/lần)
    // =========================================================================
    thongTin->thongBaoStatus = "PLAZA - Buoc 1: Dang mo dien thoai...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra xem chữ "Chơi game" đã xuất hiện chưa
        cv::Point pChoiGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuChoiGame, roi_ChuChoiGame, 0.8);
        if (pChoiGame.x != -1) break; // Đã thấy chữ -> Điện thoại đã mở -> Thoát loop

        // Nếu chưa thấy thì click mồi vào vị trí Điện thoại cố định
        BoDieuKhien::BamChuot(h_gameplay, x_DienThoai, y_DienThoai);
        this_thread::sleep_for(chrono::milliseconds(2000));
    }

    // =========================================================================
    // BƯỚC 2: ẤN VÀO "CHƠI GAME" CHO TỚI KHI XUẤT HIỆN BẢNG CHỌN GAME
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "PLAZA - Buoc 2: Dang doi bang chon game...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra Bảng chọn game đã xuất hiện chưa
        cv::Point pBangGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BangChonGame, roi_BangChonGame, 0.8);
        if (pBangGame.x != -1) break; // Thấy bảng game xuất hiện -> Thoát loop

        // Nếu chưa thấy bảng, quét lại chữ "Chơi game" để ấn kích hoạt
        cv::Point pChoiGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuChoiGame, roi_ChuChoiGame, 0.8);
        if (pChoiGame.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pChoiGame.x, pChoiGame.y);
        }
        this_thread::sleep_for(chrono::milliseconds(4000));
    }

    // =========================================================================
    // BƯỚC 3: TÌM CHỮ "TIỆC TRÒ CHƠI" VÀ BẤM VÀO
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "PLAZA - Buoc 3: Dang chon Tiec tro choi...";
    int demThuLaiStep3 = 0;
    while (thongTin->dangChay && demThuLaiStep3 < 10) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pTiecTroChoi = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuTiecTroChoi, roi_ChuTiecTroChoi, 0.8);
        if (pTiecTroChoi.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pTiecTroChoi.x, pTiecTroChoi.y);
            this_thread::sleep_for(chrono::milliseconds(1000)); // Đợi game chuyển cảnh load sang Map Tiệc
            break;
        }
        demThuLaiStep3++;
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    thongTin->thongBaoStatus = "PLAZA: Da bam chon thap, dang doi load han sang map Plaza...";
}























// hàm setup vị trí câu cá plaza ở hải đăng 
void SetUpCauMapPlazaHaiDang(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "--- PLAZA: BAT DAU SET UP DI CAU HAI DANG ---";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    // =========================================================================
    // KHAI BÁO TỌA ĐỘ VÀ CÁC VÙNG ROI 
    // =========================================================================
    int x_DienThoai = 931, y_DienThoai = 217;            // Tọa độ cố định của nút Điện thoại

    cv::Rect roi_ChuChoiGame(647, 127, 80, 30);       // Vùng check chữ "Chơi game" khi mở ĐT
    cv::Rect roi_BangChonGame(379, 12, 190, 60);      // Vùng xuất hiện Bảng chọn game
    cv::Rect roi_ChuTiecTroChoi(479, 216, 111, 30);     // Vùng quét chữ "Tiệc trò chơi"
    cv::Rect roi_NutBanDo(885, 113, 60, 60);            // Vùng chứa nút mở Bản Đồ
    cv::Rect roi_TamGiacNgang(5, 235, 70, 70);      // Vùng chứa nút tam giác ngang
    cv::Rect roi_ChutThongTinKhuVuc(34, 8, 80, 80); // Vùng check chữ "Thông tin khu vực"
    cv::Rect roi_HaiDang(69, 71, 290, 468);            // Vùng cố định quét chữ "Hải đăng" trong danh sách
    cv::Rect roi_BangChiDuong(414, 420, 133, 60);      // Vùng xuất hiện Bảng Chỉ Đường

    string anh_ChuChoiGame = "chu_choi_game.png";
    string anh_BangChonGame = "bang_chon_game.png";
    string anh_ChuTiecTroChoi = "tiec_tro_choi.png";
    string anh_NutBanDo = "nut_ban_do.png";
    string anh_TamGiacNgang = "tam_giac_ngang.png";
    string anh_ChutThongTinKhuVuc = "thong_tin_khu_vuc.png";
    string anh_HaiDang = "hai_dang.png";                 // Ảnh chữ "Hải đăng" trong menu 2D
    string anh_BangChiDuong = "bang_chi_duong.png";
    string anh_NutChiDuong = "nut_chi_duong.png";

    // =========================================================================
    // BƯỚC 1: CLICK TỌA ĐỘ ĐIỆN THOẠI CHO TỚI KHI HIỆN CHỮ "CHƠI GAME" (1s/lần)
    // =========================================================================
    thongTin->thongBaoStatus = "PLAZA - Buoc 1: Dang mo dien thoai...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra xem chữ "Chơi game" đã xuất hiện chưa
        cv::Point pChoiGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuChoiGame, roi_ChuChoiGame, 0.8);
        if (pChoiGame.x != -1) break; // Đã thấy chữ -> Điện thoại đã mở -> Thoát loop

        // Nếu chưa thấy thì click mồi vào vị trí Điện thoại cố định
        BoDieuKhien::BamChuot(h_gameplay, x_DienThoai, y_DienThoai);
        this_thread::sleep_for(chrono::milliseconds(2000)); // Delay đúng 1 giây 
    }

    // =========================================================================
    // BƯỚC 2: ẤN VÀO "CHƠI GAME" CHO TỚI KHI XUẤT HIỆN BẢNG CHỌN GAME
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "PLAZA - Buoc 2: Dang doi bang chon game...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra Bảng chọn game đã xuất hiện chưa
        cv::Point pBangGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BangChonGame, roi_BangChonGame, 0.8);
        if (pBangGame.x != -1) break; // Thấy bảng game xuất hiện -> Thoát loop

        // Nếu chưa thấy bảng, quét lại chữ "Chơi game" để ấn kích hoạt
        cv::Point pChoiGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuChoiGame, roi_ChuChoiGame, 0.8);
        if (pChoiGame.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pChoiGame.x, pChoiGame.y);
        }
        this_thread::sleep_for(chrono::milliseconds(4000));
    }

    // =========================================================================
    // BƯỚC 3: TÌM CHỮ "TIỆC TRÒ CHƠI" VÀ BẤM VÀO
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "PLAZA - Buoc 3: Dang chon Tiec tro choi...";
    int demThuLaiStep3 = 0;
    while (thongTin->dangChay && demThuLaiStep3 < 10) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pTiecTroChoi = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuTiecTroChoi, roi_ChuTiecTroChoi, 0.8);
        if (pTiecTroChoi.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pTiecTroChoi.x, pTiecTroChoi.y);
            this_thread::sleep_for(chrono::milliseconds(1000)); // Đợi game chuyển cảnh load sang Map Tiệc
            break;
        }
        demThuLaiStep3++;
        this_thread::sleep_for(chrono::milliseconds(200));
    }
    this_thread::sleep_for(chrono::milliseconds(1000));
    // =========================================================================
    // BƯỚC 4: TÌM BẢN ĐỒ, ẤN CHO TỚI KHI TAM GIÁC NGANG XUẤT HIỆN
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "PLAZA - Buoc 4: Dang mo ban do map moi...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra xem nút tam giác ngang đã xuất hiện chưa (Bản đồ mở thành công)
        cv::Point pTamGiac = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_TamGiacNgang, roi_TamGiacNgang, 0.8);
        if (pTamGiac.x != -1) break; // Tam giác ngang xuất hiện -> Xong bước 4

        // Nếu chưa thấy tam giác ngang, tiếp tục tìm nút bản đồ để bấm
        cv::Point pBanDo = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NutBanDo, roi_NutBanDo, 0.8);
        if (pBanDo.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pBanDo.x, pBanDo.y);
        }
        this_thread::sleep_for(chrono::milliseconds(300));
    }

    // =========================================================================
    // BƯỚC 5: ẤN TAM GIÁC NGANG -> HIỆN "THÔNG TIN KHU VỰC"
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "PLAZA - Buoc 5: Dang mo danh sach khu vuc...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pChuTTKV = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChutThongTinKhuVuc, roi_ChutThongTinKhuVuc, 0.8);
        if (pChuTTKV.x != -1) break; // Đã mở bảng danh sách thành công

        cv::Point pTamGiac = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_TamGiacNgang, roi_TamGiacNgang, 0.8);
        if (pTamGiac.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pTamGiac.x, pTamGiac.y);
        }
        this_thread::sleep_for(chrono::milliseconds(300));
        this_thread::sleep_for(chrono::milliseconds(3000));
    }

    // =========================================================================
    // BƯỚC 6: LƯỚT XUỐNG TÌM HẢI ĐĂNG CHO ĐẾN KHI HIỆN BẢNG CHỈ ĐƯỜNG
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "PLAZA - Buoc 6: Dang cuon trang tim Hai dang...";
    int gioiHanCuon = 0;
    while (thongTin->dangChay && gioiHanCuon < 15) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // 1. Kiểm tra Bảng chỉ đường lên chưa
        cv::Point pBangChiDuong = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BangChiDuong, roi_BangChiDuong, 0.8);
        if (pBangChiDuong.x != -1) break;

        // 2. Tìm địa điểm "Hải đăng" trong vùng danh sách
        cv::Point pHaiDang = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_HaiDang, roi_HaiDang, 0.8);
        if (pHaiDang.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pHaiDang.x, pHaiDang.y);
            this_thread::sleep_for(chrono::milliseconds(300));
            this_thread::sleep_for(chrono::milliseconds(3000));
            continue;
        }

        // 3. Nếu không thấy "Hải đăng" ở màn hiện tại -> Gọi hàm của ông vuốt cuộn xuống
        CuonTrang_ThuanCode(h_gameplay, "xuong");
        gioiHanCuon++;
    }

    // =========================================================================
    // BƯỚC 7: BẢNG CHỈ ĐƯỜNG HIỆN RA -> ẤN NÚT CHỈ ĐƯỜNG
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "PLAZA - Buoc 7: Dang bam nut bat dau di chuyen...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pNutChiDuong = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NutChiDuong, roi_BangChiDuong, 0.8);
        if (pNutChiDuong.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pNutChiDuong.x, pNutChiDuong.y);
            this_thread::sleep_for(chrono::milliseconds(200));
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(200));
        this_thread::sleep_for(chrono::milliseconds(3000));
    }
    this_thread::sleep_for(chrono::milliseconds(3000));
    thongTin->thongBaoStatus = "--- PLAZA: NHAN VAT DANG CHAY RA HAI DANG CAU CA! ---";
}





// Hàm chuyên trách: Mở điện thoại, chọn game leo tháp để chuyển sang Map Khu Trung Tâm (KTT)
void ChuyenToiMapKtt(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "--- KTT: BAT DAU DI CHUYEN SANG MAP KTT ---";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    // =========================================================================
    // SỬ DỤNG BỘ ROI VÀ TỌA ĐỘ CHUẨN ĐÃ LƯU CỦA ÔNG GIÁO (BƯỚC 1 - BƯỚC 3)
    // =========================================================================
    int x_DienThoai = 931, y_DienThoai = 217;            // Tọa độ nút Điện thoại cố định
    cv::Rect roi_ChuChoiGame(647, 127, 80, 30);       // Vùng check chữ "Chơi game" khi mở ĐT
    cv::Rect roi_BangChonGame(379, 12, 190, 60);      // Vùng xuất hiện Bảng chọn game
    cv::Rect roi_ChuLeoThap(265, 219, 125, 30);        // Vùng quét chữ "Leo" 

    string anh_ChuChoiGame = "chu_choi_game.png";
    string anh_BangChonGame = "bang_chon_game.png";
    string anh_ChuLeoThap = "chu_leo_thap.png";          // Ảnh nhận diện chữ "Leo" 

    // =========================================================================
    // BƯỚC 1: CLICK ĐIỆN THOẠI CHO TỚI KHI HIỆN CHỮ "CHƠI GAME" (1s/lần)
    // =========================================================================
    thongTin->thongBaoStatus = "KTT - Buoc 1: Dang mo dien thoai...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pChoiGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuChoiGame, roi_ChuChoiGame, 0.8);
        if (pChoiGame.x != -1) break; // Thấy chữ "Chơi game" -> Thoát loop

        BoDieuKhien::BamChuot(h_gameplay, x_DienThoai, y_DienThoai);
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    // =========================================================================
    // BƯỚC 2: ẤN VÀO "CHƠI GAME" CHO TỚI KHI XUẤT HIỆN BẢNG CHỌN GAME
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KTT - Buoc 2: Dang doi bang chon game...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pBangGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BangChonGame, roi_BangChonGame, 0.8);
        if (pBangGame.x != -1) break;

        cv::Point pChoiGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuChoiGame, roi_ChuChoiGame, 0.8);
        if (pChoiGame.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pChoiGame.x, pChoiGame.y);
        }
        this_thread::sleep_for(chrono::milliseconds(4000));
    }

    // =========================================================================
    // BƯỚC 3: TÌM CHỮ "LEO" (THÁP) VÀ BẤM VÀO ĐỂ CHUYỂN MAP KHU TRUNG TÂM
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KTT - Buoc 3: Dang chon khu vuc Leo Thap...";
    int demThuLaiStep3 = 0;
    while (thongTin->dangChay && demThuLaiStep3 < 10) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pLeoThap = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuLeoThap, roi_ChuLeoThap, 0.8);
        if (pLeoThap.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pLeoThap.x, pLeoThap.y);
            this_thread::sleep_for(chrono::milliseconds(1200)); // Đợi game load cảnh sang map KTT công cộng
            break;
        }
        demThuLaiStep3++;
        this_thread::sleep_for(chrono::milliseconds(2000));
    }
    this_thread::sleep_for(chrono::milliseconds(3000));
    thongTin->thongBaoStatus = "KTT: Da bam len thap, dang trong qua trinh load sang map KTT...";
}




// Setup Câu cá Map Ktt
// cần thêm 1 vòng logic về tàu điện ngầm để định hình lại hướng nhân vật . đã có
void SetUpCauMapKtt(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "--- KTT: BAT DAU SET UP DI CAU BAY AO ---";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    // =========================================================================
    // SỬ DỤNG BỘ ROI VÀ TỌA ĐỘ CHUẨN ĐÃ LƯU CỦA ÔNG GIÁO
    // =========================================================================
    int x_DienThoai = 931, y_DienThoai = 217;            // Tọa độ nút Điện thoại cố định

    cv::Rect roi_ChuChoiGame(647, 127, 80, 30);       // Vùng check chữ "Chơi game" khi mở ĐT
    cv::Rect roi_BangChonGame(379, 12, 190, 60);      // Vùng xuất hiện Bảng chọn game
    cv::Rect roi_ChuLeoThap(265, 219, 125, 30);        // Vùng quét chữ "Leo" 
    cv::Rect roi_NutBanDo(885, 113, 60, 60);            // Vùng chứa nút mở Bản Đồ
    cv::Rect roi_TamGiacNgang(5, 235, 70, 70);          // Vùng chứa nút tam giác ngang
    cv::Rect roi_ChutThongTinKhuVuc(34, 8, 80, 80);     // Vùng check chữ "Thông tin khu vực"
    cv::Rect roi_BayAo(69, 71, 290, 468);              // Vùng cố định quét chữ "Bẫy ao" trong danh sách
    cv::Rect roi_BangChiDuong(414, 420, 133, 60);      // Vùng xuất hiện Bảng Chỉ Đường

    string anh_ChuChoiGame = "chu_choi_game.png";
    string anh_BangChonGame = "bang_chon_game.png";
    string anh_ChuLeoThap = "chu_leo_thap.png";          // Ảnh nhận diện chữ "Leo" 
    string anh_NutBanDo = "nut_ban_do.png";
    string anh_TamGiacNgang = "tam_giac_ngang.png";
    string anh_ChutThongTinKhuVuc = "thong_tin_khu_vuc.png";
    string anh_BayAo = "bay_ao.png";                    // Ảnh chữ "Bẫy ao" trong menu 2D
    string anh_BangChiDuong = "bang_chi_duong.png";
    string anh_NutChiDuong = "nut_chi_duong.png";

    // =========================================================================
    // BƯỚC 1: CLICK ĐIỆN THOẠI CHO TỚI KHI HIỆN CHỮ "CHƠI GAME" (1s/lần)
    // =========================================================================
    thongTin->thongBaoStatus = "KTT - Buoc 1: Dang mo dien thoai...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pChoiGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuChoiGame, roi_ChuChoiGame, 0.8);
        if (pChoiGame.x != -1) break; // Thấy chữ "Chơi game" -> Thoát loop

        BoDieuKhien::BamChuot(h_gameplay, x_DienThoai, y_DienThoai);
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    // =========================================================================
    // BƯỚC 2: ẤN VÀO "CHƠI GAME" CHO TỚI KHI XUẤT HIỆN BẢNG CHỌN GAME
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KTT - Buoc 2: Dang doi bang chon game...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pBangGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BangChonGame, roi_BangChonGame, 0.8);
        if (pBangGame.x != -1) break;

        cv::Point pChoiGame = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuChoiGame, roi_ChuChoiGame, 0.8);
        if (pChoiGame.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pChoiGame.x, pChoiGame.y);
        }
        this_thread::sleep_for(chrono::milliseconds(4000));
    }

    // =========================================================================
    // BƯỚC 3: TÌM CHỮ "LEO" (THÁP) VÀ BẤM VÀO ĐỂ CHUYỂN MAP KHU TRUNG TÂM
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KTT - Buoc 3: Dang chon khu vuc Leo Thap...";
    int demThuLaiStep3 = 0;
    while (thongTin->dangChay && demThuLaiStep3 < 10) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pLeoThap = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuLeoThap, roi_ChuLeoThap, 0.8);
        if (pLeoThap.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pLeoThap.x, pLeoThap.y);
            this_thread::sleep_for(chrono::milliseconds(1200)); // Đợi game load cảnh sang map KTT công cộng
            break;
        }
        demThuLaiStep3++;
    }
    
    this_thread::sleep_for(chrono::milliseconds(2000));
    DiBo_ThuanCode(thongTin->h_game, "xuong");
    this_thread::sleep_for(chrono::milliseconds(2000));
    DiBo_ThuanCode(thongTin->h_game, "xuong");
    this_thread::sleep_for(chrono::milliseconds(3000));
    // =========================================================================
    // BƯỚC 4: TÌM BẢN ĐỒ KTT, ẤN CHO TỚI KHI TAM GIÁC NGANG XUẤT HIỆN
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KTT - Buoc 4: Dang mo ban do KTT...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pTamGiac = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_TamGiacNgang, roi_TamGiacNgang, 0.8);
        if (pTamGiac.x != -1) break;

        cv::Point pBanDo = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NutBanDo, roi_NutBanDo, 0.8);
        if (pBanDo.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pBanDo.x, pBanDo.y);
        }
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    // =========================================================================
    // BƯỚC 5: ẤN TAM GIÁC NGANG -> HIỆN "THÔNG TIN KHU VỰC"
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KTT - Buoc 5: Dang bung danh sach thong tin...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pChuTTKV = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChutThongTinKhuVuc, roi_ChutThongTinKhuVuc, 0.8);
        if (pChuTTKV.x != -1) break;

        cv::Point pTamGiac = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_TamGiacNgang, roi_TamGiacNgang, 0.8);
        if (pTamGiac.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pTamGiac.x, pTamGiac.y);
        }
        this_thread::sleep_for(chrono::milliseconds(300));
    }

    // =========================================================================
    // BƯỚC 6: CUỘN DANH SÁCH TÌM CHỮ "BẪY AO"
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KTT - Buoc 6: Dang cuon tim vi tri Bay ao...";
    int gioiHanCuon = 0;
    while (thongTin->dangChay && gioiHanCuon < 15) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra Bảng chỉ đường lên chưa
        cv::Point pBangChiDuong = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BangChiDuong, roi_BangChiDuong, 0.8);
        if (pBangChiDuong.x != -1) break;

        // Tìm địa điểm "Bẫy ao"
        cv::Point pBayAo = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BayAo, roi_BayAo, 0.8);
        if (pBayAo.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pBayAo.x, pBayAo.y);
            this_thread::sleep_for(chrono::milliseconds(3000));
            continue;
        }

        // Nếu không thấy -> Cuộn trang tiếp
        CuonTrang_ThuanCode(h_gameplay, "xuong");
        gioiHanCuon++;
    }

    // =========================================================================
    // BƯỚC 7: BẢNG CHỈ ĐƯỜNG HIỆN RA -> ẤN NÚT CHỈ ĐƯỜNG ĐỂ DI CHUYỂN
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "KTT - Buoc 7: Dang an nut chi duong...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pNutChiDuong = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NutChiDuong, roi_BangChiDuong, 0.8);
        if (pNutChiDuong.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pNutChiDuong.x, pNutChiDuong.y);
            this_thread::sleep_for(chrono::milliseconds(200));
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(1000));
        this_thread::sleep_for(chrono::milliseconds(3000));
    }


    



    thongTin->thongBaoStatus = "--- KTT: DANH DAU THANH CONG! NHAN VAT DANG CHAY DI CAU ---";
}










// Hàm chuyên trách: Mở điện thoại, chọn biểu tượng Nhà Của Tôi để biến mất và chuyển về Home
void ChuyenVeMapHome(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "--- HOME: BAT DAU DI CHUYEN VE MAP HOME ---";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    // =========================================================================
    // KHAI BÁO TỌA ĐỘ VÀ VÙNG ROI THEO LOGIC MỚI CỦA ÔNG GIÁO
    // c
    int x_DienThoai = 930, y_DienThoai = 238;            // Tọa độ cố định của nút Điện thoại

    // Ông giáo tự căn chỉnh lại vùng ROI quét biểu tượng Nhà của tôi trên màn hình ĐT nhé
    cv::Rect roi_IconNhaCuaToi(843, 449, 74, 74);
    string anh_IconNhaCuaToi = "icon_nha_cua_toi.png";

    // =========================================================================
    // BƯỚC 1: CLICK TỌA ĐỘ ĐIỆN THOẠI CHO TỚI KHI HIỆN BIỂU TƯỢNG "NHÀ CỦA TÔI"
    // =========================================================================
    thongTin->thongBaoStatus = "HOME - Buoc 1: Dang mo dien thoai...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra xem biểu tượng "Nhà của tôi" đã xuất hiện chưa
        cv::Point pNhaCuaToi = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_IconNhaCuaToi, roi_IconNhaCuaToi, 0.8);
        if (pNhaCuaToi.x != -1) break; // Đã thấy biểu tượng -> Điện thoại đã mở -> Thoát loop

        // Nếu chưa thấy thì click mồi vào vị trí Điện thoại cố định
        BoDieuKhien::BamChuot(h_gameplay, x_DienThoai, y_DienThoai);
        this_thread::sleep_for(chrono::milliseconds(2000));
    }
    this_thread::sleep_for(chrono::milliseconds(3000));
    // =========================================================================
    // BƯỚC 2: ẤN VÀO BIỂU TƯỢNG "NHÀ CỦA TÔI" CHO TỚI KHI NÓ BIẾN MẤT
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "HOME - Buoc 2: Dang an chon Nha Cua Toi cho den khi bien mat...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra xem biểu tượng "Nhà của tôi" còn ở đó không
        cv::Point pNhaCuaToi = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_IconNhaCuaToi, roi_IconNhaCuaToi, 0.8);
        if (pNhaCuaToi.x == -1) {
            break; // Biểu tượng đã biến mất (Game đã nhận lệnh và đang chuyển cảnh) -> Thoát loop
        }

        // Nếu vẫn còn thấy biểu tượng thì tiếp tục click vào tọa độ của nó
        BoDieuKhien::BamChuot(h_gameplay, pNhaCuaToi.x, pNhaCuaToi.y);
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
    this_thread::sleep_for(chrono::milliseconds(3000));
    thongTin->thongBaoStatus = "HOME: Bieu tuong da bien mat, dang doi load ve nha...";
}




















void SetUpCauCaMapHome(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "--- HOME: BAT DAU SET UP DI CAU MAP NHA ---";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    // =========================================================================
    // TỌA ĐỘ VÀ VÙNG ROI CỐ ĐỊNH (Áp dụng bộ dùng chung và thêm vùng mới map Home)
    // =========================================================================
    int x_DoiTienIchNhanh = 925, y_DoiTienIchNhanh = 75; // Tọa độ nút đổi tiện ích nhanh (Chỉnh lại nhé ông)

    cv::Rect roi_NutBanDo(836, 118, 60, 60);            // Vùng chứa nút mở Bản Đồ
    cv::Rect roi_TamGiacNgang(5, 235, 70, 70);          // Vùng chứa nút tam giác ngang
    cv::Rect roi_ChuCuDan(144, 6, 108, 53);              // Vùng quét tab hoặc chữ "Cư dân"
    cv::Rect roi_NpcBipBip(77, 72, 103, 88);          // Vùng danh sách quét tên NPC Bíp Bíp
    cv::Rect roi_BangChiDuong(414, 420, 133, 60);       // Vùng xuất hiện Bảng Chỉ Đường

    string anh_NutBanDo = "nut_ban_do.png";
    string anh_TamGiacNgang = "tam_giac_ngang.png";
    string anh_ChuCuDan = "chu_cu_dan.png";
    string anh_NpcBipBip = "npc_bip_bip.png";           // Ảnh mẫu cắt chữ tên NPC Bíp Bíp
    string anh_BangChiDuong = "bang_chi_duong.png";
    string anh_NutChiDuong = "nut_chi_duong.png";

    // =========================================================================
    // BƯỚC 1: KIỂM TRA BẢN ĐỒ - NẾU CHƯA CÓ THÌ ẤN ĐỔI TIỆN ÍCH NHANH (2s/LẦN)
    // =========================================================================
    thongTin->thongBaoStatus = "HOME - Buoc 1: Dang tim nut ban do...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra xem nút bản đồ đã xuất hiện hiển thị trên màn hình chưa
        cv::Point pBanDo = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NutBanDo, roi_NutBanDo, 0.8);
        if (pBanDo.x != -1) {
            break; // Thấy nút bản đồ hiện ra rồi -> Thoát loop chuyển sang Bước 2 luôn
        }

        // Nếu chưa thấy bản đồ thì ấn nút Đổi tiện ích nhanh và đợi 2 giây
        BoDieuKhien::BamChuot(h_gameplay, x_DoiTienIchNhanh, y_DoiTienIchNhanh);
        this_thread::sleep_for(chrono::milliseconds(2000)); // Delay đúng 2 giây theo logic
    }

    // =========================================================================
    // BƯỚC 2: THẤY BẢN ĐỒ -> ẤN VÀO CHO ĐẾN KHI TAM GIÁC NGANG XUẤT HIỆN
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "HOME - Buoc 2: Dang mo ban do nha rieng...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pTamGiac = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_TamGiacNgang, roi_TamGiacNgang, 0.8);
        if (pTamGiac.x != -1) break; // Tam giác ngang xuất hiện -> Bản đồ đã bung mở xong

        cv::Point pBanDo = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NutBanDo, roi_NutBanDo, 0.8);
        if (pBanDo.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pBanDo.x, pBanDo.y);
        }
        this_thread::sleep_for(chrono::milliseconds(300));
        this_thread::sleep_for(chrono::milliseconds(3000));
    }

    // =========================================================================
    // BƯỚC 3: ẤN TAM GIÁC NGANG CHO ĐẾN KHI THẤY CHỮ "CƯ DÂN"
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "HOME - Buoc 3: Dang tim mục Cu dan...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pChuCuDan = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuCuDan, roi_ChuCuDan, 0.8);
        if (pChuCuDan.x != -1) break; // Đã thấy mục cư dân xuất hiện -> Thoát chuyển bước

        cv::Point pTamGiac = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_TamGiacNgang, roi_TamGiacNgang, 0.8);
        if (pTamGiac.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pTamGiac.x, pTamGiac.y);
        }
        this_thread::sleep_for(chrono::milliseconds(300));
    }

    // =========================================================================
    // BƯỚC 4: ẤN VÀO CHỮ "CƯ DÂN" CHO ĐẾN KHI THẤY TÊN NPC BÍP BÍP
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "HOME - Buoc 4: Dang mo danh sach NPC...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Check xem tên NPC Bíp Bíp đã lộ diện trong danh sách chưa
        cv::Point pNpc = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NpcBipBip, roi_NpcBipBip, 0.8);
        if (pNpc.x != -1) break; // Thấy Bíp Bíp rồi -> Ngừng click mục cư dân

        // Nếu chưa thấy danh sách xổ ra, tiếp tục click vào chữ Cư Dân
        cv::Point pChuCuDan = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ChuCuDan, roi_ChuCuDan, 0.8);
        if (pChuCuDan.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pChuCuDan.x, pChuCuDan.y);
        }
        this_thread::sleep_for(chrono::milliseconds(400));
    }

    // =========================================================================
    // BƯỚC 5: ẤN VÀO NPC BÍP BÍP CHO ĐẾN KHI BẢNG CHỈ ĐƯỜNG XUẤT HIỆN
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "HOME - Buoc 5: Dang chon dinh vi NPC Bip Bip...";
    int gioiHanCuon = 0; // Dự phòng trường hợp cần cuộn tìm NPC (nếu danh sách dài)
    while (thongTin->dangChay && gioiHanCuon < 10) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        // Kiểm tra bảng chỉ đường lên chưa
        cv::Point pBangChiDuong = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_BangChiDuong, roi_BangChiDuong, 0.7);
        if (pBangChiDuong.x != -1) break;

        // Tìm NPC Bíp Bíp để click kích hoạt bảng chỉ đường
        cv::Point pNpc = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NpcBipBip, roi_NpcBipBip, 0.8);
        if (pNpc.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pNpc.x, pNpc.y);
            this_thread::sleep_for(chrono::milliseconds(1500));
            this_thread::sleep_for(chrono::milliseconds(3000));
            continue;
        }

        // Nếu danh sách NPC dài quá che mất Bíp Bíp thì tự động vuốt cuộn trang tìm
        CuonTrang_ThuanCode(h_gameplay, "xuong");
        gioiHanCuon++;
    }

    // =========================================================================
    // BƯỚC 6: BẢNG CHỈ ĐƯỜNG XUẤT HIỆN -> ẤN NÚT CHỈ ĐƯỜNG
    // =========================================================================
    if (!thongTin->dangChay) return;
    thongTin->thongBaoStatus = "HOME - Buoc 6: Dang bam bat dau tim duong...";
    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) { this_thread::sleep_for(chrono::milliseconds(100)); continue; }

        cv::Point pNutChiDuong = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_NutChiDuong, roi_BangChiDuong, 0.7);
        if (pNutChiDuong.x != -1) {
            BoDieuKhien::BamChuot(h_gameplay, pNutChiDuong.x, pNutChiDuong.y);
            this_thread::sleep_for(chrono::milliseconds(200));
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(2000));
    }


    this_thread::sleep_for(chrono::milliseconds(5000));
    DiBo_ThuanCode(thongTin->h_game, "len");
    this_thread::sleep_for(chrono::milliseconds(3000));
    DiBo_ThuanCode(thongTin->h_game, "len");
    this_thread::sleep_for(chrono::milliseconds(3000));
    DiBo_ThuanCode(thongTin->h_game, "len");


    thongTin->thongBaoStatus = "--- HOME: SET UP THANH CONG! NHAN VAT DANG DI TIM NPC ---";
}






















// phóng to màn hình 
void ZoomToManHinh_ChuotPhai(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "Dang vuot chuot phai thu nho (5 giay)...";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    int x_tam = 480;
    int y_tam = 270;
    int x_ria = 800;
    int y_ria = 270;

    // 1. Nhấn giữ chuột phải ở rìa màn hình (WM_RBUTTONDOWN)
    PostMessage(h_gameplay, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(x_ria, y_ria));
    this_thread::sleep_for(chrono::milliseconds(100));

    int tong_thoi_gian_ms = 5000;
    int thoi_gian_buoc_ms = 50;
    int so_buoc = tong_thoi_gian_ms / thoi_gian_buoc_ms;

    double buoc_dich_x = (double)(x_ria - x_tam) / so_buoc;
    double x_hien_tai = x_ria;

    // 2. Kéo từ từ chuột phải về tâm game
    for (int i = 0; i < so_buoc; i++) {
        if (!thongTin->dangChay) break;

        x_hien_tai -= buoc_dich_x;
        if (x_hien_tai < x_tam) x_hien_tai = x_tam;

        // Gửi thông điệp di chuyển chuột trong khi vẫn đè chuột phải
        PostMessage(h_gameplay, WM_MOUSEMOVE, MK_RBUTTON, MAKELPARAM(cvRound(x_hien_tai), y_ria));

        if (i % 20 == 0) {
            thongTin->thongBaoStatus = "Dang keo camera... con " + to_string((tong_thoi_gian_ms - (i * thoi_gian_buoc_ms)) / 1000) + "s";
        }
        this_thread::sleep_for(chrono::milliseconds(thoi_gian_buoc_ms));
    }

    // 3. Nhả chuột phải ra
    PostMessage(h_gameplay, WM_RBUTTONUP, 0, MAKELPARAM(cvRound(x_hien_tai), y_ria));
    thongTin->thongBaoStatus = "Da thu nho camera ve chuan!";
}








void ZoomNhoManHinh_SieuMuot(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "Dang ghim chuot phai & giat chuot trai (5 giay)...";
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    // Tọa độ tâm màn hình (Mốc cố định)
    int x_tam = 480;
    int y_tam = 270;
    LPARAM lParamTam = MAKELPARAM(x_tam, y_tam);

    // Tọa độ bắt đầu của ngón kéo (Rìa phải)
    int x_ria_bat_dau = 800;
    int y_ria = 270;

    // --- BƯỚC 1: GHIM CHẶT CHUỘT PHẢI TẠI TÂM (TẠO NGÓN TAY 1 CỐ ĐỊNH) ---
    PostMessage(h_gameplay, WM_RBUTTONDOWN, MK_RBUTTON, lParamTam);
    this_thread::sleep_for(chrono::milliseconds(50));

    // --- BƯỚC 2: DÙNG CHUỘT TRÁI CO VÀO TRONG 5 GIÂY (NGÓN TAY 2 DI CHUYỂN) ---
    int tong_thoi_gian_ms = 5000;
    int thoi_gian_moi_buoc_ms = 30; // 30ms gửi 1 lần để tạo độ mượt tuyệt đối như tay vuốt
    int so_buoc = tong_thoi_gian_ms / thoi_gian_moi_buoc_ms;

    double buoc_dich_x = (double)(x_ria_bat_dau - x_tam) / so_buoc;
    double x_hien_tai_ngon2 = x_ria_bat_dau;

    for (int i = 0; i < so_buoc; i++) {
        if (!thongTin->dangChay) break;

        // Tính tọa độ X lùi dần về tâm
        x_hien_tai_ngon2 -= buoc_dich_x;
        if (x_hien_tai_ngon2 < x_tam) x_hien_tai_ngon2 = x_tam;
        LPARAM lParamNgon2 = MAKELPARAM(cvRound(x_hien_tai_ngon2), y_ria);

        // Nhấp chuột trái xuống -> Di chuyển -> Nhấc chuột trái lên cực nhanh
        // Đè thêm cờ MK_RBUTTON để báo cho Windows biết chuột phải vẫn đang được đè giữ ngầm ở tâm
        PostMessage(h_gameplay, WM_LBUTTONDOWN, MK_LBUTTON | MK_RBUTTON, lParamNgon2);
        PostMessage(h_gameplay, WM_MOUSEMOVE, MK_LBUTTON | MK_RBUTTON, lParamNgon2);
        PostMessage(h_gameplay, WM_LBUTTONUP, MK_RBUTTON, lParamNgon2);

        // Cập nhật text ra UI mỗi giây
        if (i % 33 == 0) {
            thongTin->thongBaoStatus = "Dang zoom muot... con " + to_string((tong_thoi_gian_ms - (i * thoi_gian_moi_buoc_ms)) / 1000) + "s";
        }

        this_thread::sleep_for(chrono::milliseconds(thoi_gian_moi_buoc_ms));
    }

    // --- BƯỚC 3: NHẢ CHUỘT PHẢI Ở TÂM RA ĐỂ KẾT THÚC THAO TÁC ĐA ĐIỂM ---
    PostMessage(h_gameplay, WM_RBUTTONUP, 0, lParamTam);

    thongTin->thongBaoStatus = "Da thu nho camera ve mốc chuan bang hàm Native!";
}











// Hàm kiểm tra trạng thái cần câu bằng cách check màu HSV (Chạy đơn lẻ 1 lần)
void CheckTrangThaiCanCau_HSV_TestLoop(ThongTinTool* thongTin) {
    int x_check = 318;
    int y_check = 498;
    int saiSo = 10;

    cv::Mat anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);
    if (anhGoc.empty()) return;

    // Chặn lỗi nếu tọa độ điền lọt ra ngoài biên ảnh gây crash
    if (x_check < 0 || x_check >= anhGoc.cols || y_check < 0 || y_check >= anhGoc.rows) {
        thongTin->thongBaoStatus = "Loi: Toa do diem check vuot qua bien man hinh!";
        return;
    }

    // Chuyển đổi sang hệ màu HSV
    cv::Mat anhHSV;
    cv::cvtColor(anhGoc, anhHSV, cv::COLOR_BGR2HSV);

    // Bốc giá trị HSV thực tế (Truyền y trước, x sau)
    cv::Vec3b hsvThucTe = anhHSV.at<cv::Vec3b>(y_check, x_check);
    int h = hsvThucTe[0];
    int s = hsvThucTe[1];
    int v = hsvThucTe[2];

    // Đối chiếu logic màu và cập nhật thẳng lên thongBaoStatus cho Dashboard ImGui
    if (abs(h - 0) <= saiSo && abs(s - 0) <= saiSo && abs(v - 200) <= saiSo) {
        thongTin->thongBaoStatus = "Cam can";
    }
    else if (abs(h - 150) <= saiSo && abs(s - 8) <= saiSo && abs(v - 166) <= saiSo) {
        thongTin->thongBaoStatus = "Khong cam can";
    }
    else {
        thongTin->thongBaoStatus = "Khong xac dinh (Sai mau)";
    }
}












void KiemTraLaiViTri(ThongTinTool* thongTin) {
    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    int x_Click = 123;  // Thay bằng tọa độ nút bấm mở bảng của ông
    int y_Click = 40;

    int x_Dong = 896;
    int y_Dong = 40;

    cv::Rect roi_ThoiTiet(174, 8, 124, 95); // Ví dụ vùng ROI chứa bảng thời tiết

    thongTin->thongBaoStatus = "Đang click mở cho đến khi hiện bảng thời tiết...";

    while (thongTin->dangChay) {
        // Bước 1: Chụp màn hình để kiểm tra trước
        cv::Mat screenshot = BoDieuKhien::ChupManHinh(thongTin->h_game);
        if (screenshot.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Bước 2: Tìm ảnh "chu_bang_du_bao_thoi_tiet.png" trong vùng ROI
        cv::Point viTriAnh = BoDieuKhien::TimAnhTrongVung(screenshot, "chu_bang_du_bao_thoi_tiet.png", roi_ThoiTiet, 0.7);

        // Bước 3: Nếu tìm thấy ảnh (tọa độ khác -1) -> Đạt mục đích -> Dừng vòng lặp
        if (viTriAnh.x != -1) {
            thongTin->thongBaoStatus = "🎉 Đã thấy chữ bảng dự báo thời tiết! Dừng click.";
            break;
        }

        // Bước 4: Nếu chưa thấy ảnh -> Thực hiện hành động click vào tọa độ
        BoDieuKhien::BamChuot(thongTin->h_game, x_Click, y_Click);

        // Nghỉ 300ms - 500ms để giả lập nhận lệnh và hoạt ảnh UI kịp hiển thị, tránh spam quá nhanh gây lag
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }


    // RESET MẶC ĐỊNH
    int tamMapHienTai = -1;
    bool timThay = false;

    // Quét 2 lần để đảm bảo không quét trúng lúc game đang chuyển cảnh (màn hình đen/mờ)
    for (int retry = 0; retry < 2; retry++) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);
        if (anhGoc.empty()) {
            this_thread::sleep_for(chrono::milliseconds(200));
            continue;
        }

        string anh_ViTriHienTai = "vi_tri_hien_tai.png";
        cv::Rect roi_TenMapTungHang[5] = {
            cv::Rect(4, 100, 173, 80), // Plaza
            cv::Rect(4, 180, 173, 80), // CAMP
            cv::Rect(4, 240, 173, 80), // KND
            cv::Rect(4, 320, 173, 80), // KTT
            cv::Rect(4, 400, 173, 80)  // HOME
        };

        for (int hang = 0; hang < 5; hang++) {
            cv::Rect roiViTri = roi_TenMapTungHang[hang];
            if (roiViTri.x + roiViTri.width > anhGoc.cols || roiViTri.y + roiViTri.height > anhGoc.rows) continue;

            cv::Point pViTri = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ViTriHienTai, roiViTri, 0.75);
            if (pViTri.x != -1) {
                tamMapHienTai = hang + 1;
                timThay = true;
                break;
            }
        }

        if (timThay) break; // Nếu tìm thấy rồi thì không cần retry nữa
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    // Cập nhật kết quả vào Struct
    thongTin->mapHienTai = tamMapHienTai;

    // Log kết quả
    string mapNames[] = { "Unknown", "Plaza", "CAMP", "KND", "KTT", "HOME" };
    string name = (tamMapHienTai >= 1 && tamMapHienTai <= 5) ? mapNames[tamMapHienTai] : "Khong xac dinh";
    thongTin->thongBaoStatus = "Vi tri hien tai: [" + name + "]";


    thongTin->thongBaoStatus = "Đang click đóng cho đến khi bảng thời tiết biến mất...";

    while (thongTin->dangChay) {
        // Bước 1: Chụp màn hình để kiểm tra trạng thái hiện tại
        cv::Mat screenshot = BoDieuKhien::ChupManHinh(thongTin->h_game);
        if (screenshot.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Bước 2: Quét xem ảnh "chu_bang_du_bao_thoi_tiet.png" còn tồn tại không
        cv::Point viTriAnh = BoDieuKhien::TimAnhTrongVung(screenshot, "chu_bang_du_bao_thoi_tiet.png", roi_ThoiTiet, 0.7);

        // Bước 3: Nếu KHÔNG tìm thấy ảnh (trả về -1) -> Bảng đã đóng thành công -> Dừng vòng lặp
        if (viTriAnh.x == -1) {
            thongTin->thongBaoStatus = "✅ Bảng thời tiết đã biến mất!";
            break;
        }

        // Bước 4: Nếu ảnh vẫn lù lù ở đó -> Click tiếp vào tọa độ đóng
        BoDieuKhien::BamChuot(thongTin->h_game, x_Dong, y_Dong);

        // Nghỉ một chút chờ hoạt ảnh đóng UI biến mất hoàn toàn
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}
































void CheckThoiTietVaViTri(ThongTinTool* thongTin) {


    HWND h_gameplay = thongTin->h_game;
    if (h_gameplay == NULL) return;

    int x_Click = 123;  // Thay bằng tọa độ nút bấm mở bảng của ông
    int y_Click = 40;

    int x_Dong = 896;
    int y_Dong = 40;

    cv::Rect roi_ThoiTiet(174, 8, 124, 95); // Ví dụ vùng ROI chứa bảng thời tiết

    thongTin->thongBaoStatus = "Đang click mở cho đến khi hiện bảng thời tiết...";

    while (thongTin->dangChay) {
        // Bước 1: Chụp màn hình để kiểm tra trước
        cv::Mat screenshot = BoDieuKhien::ChupManHinh(thongTin->h_game);
        if (screenshot.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Bước 2: Tìm ảnh "chu_bang_du_bao_thoi_tiet.png" trong vùng ROI
        cv::Point viTriAnh = BoDieuKhien::TimAnhTrongVung(screenshot, "chu_bang_du_bao_thoi_tiet.png", roi_ThoiTiet, 0.7);

        // Bước 3: Nếu tìm thấy ảnh (tọa độ khác -1) -> Đạt mục đích -> Dừng vòng lặp
        if (viTriAnh.x != -1) {
            thongTin->thongBaoStatus = "🎉 Đã thấy chữ bảng dự báo thời tiết! Dừng click.";
            break;
        }

        // Bước 4: Nếu chưa thấy ảnh -> Thực hiện hành động click vào tọa độ
        BoDieuKhien::BamChuot(thongTin->h_game, x_Click, y_Click);

        // Nghỉ 300ms - 500ms để giả lập nhận lệnh và hoạt ảnh UI kịp hiển thị, tránh spam quá nhanh gây lag
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }






    cv::Rect roi_Goc(176, 102, 707, 393);

    cv::Mat anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);
    if (anhGoc.empty()) return;

    // 1. Tìm vạch đỏ (Mốc thời gian)
    cv::Mat vungROI = anhGoc(roi_Goc);
    cv::Mat anhHSV, matNaDo1, matNaDo2, matNaDoTong;
    cv::cvtColor(vungROI, anhHSV, cv::COLOR_BGR2HSV);
    cv::inRange(anhHSV, cv::Scalar(0, 127, 225), cv::Scalar(32, 187, 255), matNaDo1);
    cv::inRange(anhHSV, cv::Scalar(152, 127, 225), cv::Scalar(180, 187, 255), matNaDo2);
    matNaDoTong = matNaDo1 | matNaDo2;

    cv::Mat cacDiemDo;
    cv::findNonZero(matNaDoTong, cacDiemDo);
    if (cacDiemDo.empty()) {
        thongTin->mapHienTai = -1;
        return;
    }

    // Tính tâm vạch đỏ
    long long tongX = 0;
    for (int i = 0; i < cacDiemDo.total(); i++) tongX += cacDiemDo.at<cv::Point>(i).x;
    int x_vach_do_tuyet_doi = roi_Goc.x + (int)(tongX / cacDiemDo.total());

    // 2. Quét vị trí & thời tiết
    string ds_icon_thoi_tiet[] = { "icon_mua.png", "icon_suongsom.png", "icon_bao.png", "icon_nang.png", "icon_baotuyet.png", "icon_tuyet.png", "icon_cucquang.png", "icon_gio.png", "icon_trang.png", "icon_suongmu.png", "icon_giocat.png" };
    int tamMapHienTai = -1;
    int khoangCachNganNhatToanBang = 99999;
    int hangDuocChonCuoiCung = -1;
    int thoiTietDuocChonCuoiCung = -1;

    double chieuCaoMoiHang = (double)roi_Goc.height / 5.0;
    cv::Rect roi_TenMapTungHang[5] = { cv::Rect(4, 100, 173, 80), cv::Rect(4, 180, 173, 80), cv::Rect(4, 240, 173, 80), cv::Rect(4, 320, 173, 80), cv::Rect(4, 400, 173, 80) };

    for (int hang = 0; hang < 5; hang++) {
        // Quét vị trí
        if (BoDieuKhien::TimAnhTrongVung(anhGoc, "vi_tri_hien_tai.png", roi_TenMapTungHang[hang], 0.75).x != -1) {
            tamMapHienTai = hang + 1;
        }

        // Quét thời tiết
        if (hang == 1) continue; // Bỏ hàng 2
        int y_start = roi_Goc.y + (int)(hang * chieuCaoMoiHang);
        int d_rong = (roi_Goc.x + roi_Goc.width) - x_vach_do_tuyet_doi;
        if (d_rong <= 10) continue;

        cv::Rect roiHang(x_vach_do_tuyet_doi, y_start, d_rong, (int)chieuCaoMoiHang);
        for (int i = 0; i < 11; i++) {
            cv::Point p = BoDieuKhien::TimAnhTrongVung(anhGoc, ds_icon_thoi_tiet[i], roiHang, 0.8);
            if (p.x != -1) {
                int dist = p.x - x_vach_do_tuyet_doi;
                if (dist < khoangCachNganNhatToanBang) {
                    khoangCachNganNhatToanBang = dist;
                    hangDuocChonCuoiCung = hang;
                    thoiTietDuocChonCuoiCung = i;
                }
            }
        }
    }
    thongTin->mapHienTai = tamMapHienTai;
    thongTin->mapSanDuoc = (hangDuocChonCuoiCung != -1) ? (hangDuocChonCuoiCung + 1) : -1;
    thongTin->thoiTietSannDuoc = thoiTietDuocChonCuoiCung;



    while (thongTin->dangChay) {
        // Bước 1: Chụp màn hình để kiểm tra trạng thái hiện tại
        cv::Mat screenshot = BoDieuKhien::ChupManHinh(thongTin->h_game);
        if (screenshot.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Bước 2: Quét xem ảnh "chu_bang_du_bao_thoi_tiet.png" còn tồn tại không
        cv::Point viTriAnh = BoDieuKhien::TimAnhTrongVung(screenshot, "chu_bang_du_bao_thoi_tiet.png", roi_ThoiTiet, 0.7);

        // Bước 3: Nếu KHÔNG tìm thấy ảnh (trả về -1) -> Bảng đã đóng thành công -> Dừng vòng lặp
        if (viTriAnh.x == -1) {
            thongTin->thongBaoStatus = "✅ Bảng thời tiết đã biến mất!";
            break;
        }

        // Bước 4: Nếu ảnh vẫn lù lù ở đó -> Click tiếp vào tọa độ đóng
        BoDieuKhien::BamChuot(thongTin->h_game, x_Dong, y_Dong);

        // Nghỉ một chút chờ hoạt ảnh đóng UI biến mất hoàn toàn
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}


























// hàm xác định thời tiết tiếp theo cần đến 
void CheckThoiTietVaViTri1(ThongTinTool* thongTin) {
    // --- BƯỚC 1: KHAI BÁO TỌA ĐỘ VÀ CHIA VÙNG 
    // Sử dụng chính xác ROI gốc do ông cung cấp
    cv::Rect roi_Goc(176, 102, 707, 393);

    string tenCuaSo = "Mat Bot Check Thoi Tiet Va Vi Tri - Tab " + to_string((uintptr_t)thongTin->h_game);
    thongTin->thongBaoStatus = "Dang chay quet thoi tiet va vi tri hien tai...";

    // Khai báo ảnh mẫu chữ "Vị trí hiện tại" 
    string anh_ViTriHienTai = "vi_tri_hien_tai.png";

    // Tính đều chiều cao mỗi hàng dựa trên roi_Goc (393 / 5 = ~78.6 pixel)
    double chieuCaoMoiHang = (double)roi_Goc.height / 5.0;

    // MẢNG 5 ROI BAO QUANH CÁC CHỮ TÊN MAP ĐỂ QUÉT VỊ TRÍ HIỆN TẠI (Cột dọc bên trái bảng)
    // Tọa độ X và Chiều rộng (Width) khóa chặt vùng chữ từ rìa trái đến sát viền bảng thời tiết
    cv::Rect roi_TenMapTungHang[5] = {
    cv::Rect(4, 100, 173, 80), // Hàng 1: Điền số chuẩn cho vùng chữ "Plaza"
    cv::Rect(4, 180, 173, 80), // Hàng 2: Điền số chuẩn cho vùng chữ "Công viên Khủng long"
    cv::Rect(4, 240, 173, 80), // Hàng 3: Điền số chuẩn cho vùng chữ "Khu nghỉ dưỡng"
    cv::Rect(4, 320, 173, 80), // Hàng 4: Điền số chuẩn cho vùng chữ "Khu trung tâm"
    cv::Rect(4, 400, 173, 80)  // Hàng 5: Điền số chuẩn cho vùng chữ "Thị trấn / HOME"
    };

    while (thongTin->dangChay) {
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);
        if (anhGoc.empty()) {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }

        cv::Mat anhDebug = anhGoc.clone();

        // Kiểm tra an toàn biên giới tấm ảnh lớn tránh crash tràn ROI
        if (roi_Goc.x + roi_Goc.width > anhGoc.cols || roi_Goc.y + roi_Goc.height > anhGoc.rows) {
            thongTin->thongBaoStatus = "Loi: Kich thuoc gia lap thay doi, ROI thiet lap bi tran!";
            this_thread::sleep_for(chrono::milliseconds(200));
            continue;
        }

        // Vẽ viền tổng của Bảng thời tiết (Màu xanh lá)
        cv::rectangle(anhDebug, roi_Goc, cv::Scalar(0, 255, 0), 2);

        // --- BƯỚC 2: QUÉT HSV TÌM MỐC THỜI GIAN (VẠCH ĐỎ) ---
        cv::Mat vungROI = anhGoc(roi_Goc);
        cv::Mat anhHSV, matNaDo1, matNaDo2, matNaDoTong;
        cv::cvtColor(vungROI, anhHSV, cv::COLOR_BGR2HSV);

        // Giữ dải màu lệch 30 đơn vị chuẩn [2, 157, 255] của ông
        cv::Scalar lower_red1(0, 127, 225);   cv::Scalar upper_red1(32, 187, 255);
        cv::Scalar lower_red2(152, 127, 225); cv::Scalar upper_red2(180, 187, 255);

        cv::inRange(anhHSV, lower_red1, upper_red1, matNaDo1);
        cv::inRange(anhHSV, lower_red2, upper_red2, matNaDo2);
        matNaDoTong = matNaDo1 | matNaDo2;

        int x_vach_do_trong_roi = -1;
        cv::Mat cacDiemDo;
        cv::findNonZero(matNaDoTong, cacDiemDo);
        if (!cacDiemDo.empty()) {
            long long tongX = 0;
            for (int i = 0; i < cacDiemDo.total(); i++) {
                tongX += cacDiemDo.at<cv::Point>(i).x;
            }
            x_vach_do_trong_roi = tongX / cacDiemDo.total();
        }

        // VÁ LỖI CRASH: Nếu tắt bảng đột ngột làm mất vạch đỏ, thoát an toàn lượt này chứ không chạy tiếp xuống dưới
        if (x_vach_do_trong_roi == -1) {
            thongTin->thongBaoStatus = "Loi: Khong tim thay vach do thoi gian! (Co the bang da dong)";
            thongTin->mapHienTai = -1; // Reset trạng thái vị trí về không xác định
            cv::putText(anhDebug, "ERROR: KHONG THAY VACH DO", cv::Point(roi_Goc.x, roi_Goc.y - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
            cv::imshow(tenCuaSo, anhDebug);
            cv::waitKey(30);
            continue;
        }

        // Tính tọa độ X tuyệt đối trên màn hình
        int x_vach_do_tuyet_doi = roi_Goc.x + x_vach_do_trong_roi;

        // Vẽ vạch dọc màu đỏ rực đè lên màn debug xuyên suốt 5 hàng
        cv::line(anhDebug, cv::Point(x_vach_do_tuyet_doi, roi_Goc.y),
            cv::Point(x_vach_do_tuyet_doi, roi_Goc.y + roi_Goc.height), cv::Scalar(0, 0, 255), 2);

        // --- BƯỚC 3: NẠP ĐẦY ĐỦ DANH SÁCH 11 LOẠI ICON THỜI TIẾT ĐUÔI .PNG ---
        string ds_icon_thoi_tiet[] = {
            "icon_mua.png", "icon_suongsom.png", "icon_bao.png", "icon_nang.png",
            "icon_baotuyet.png", "icon_tuyet.png", "icon_cucquang.png", "icon_gio.png",
            "icon_trang.png", "icon_suongmu.png", "icon_giocat.png"
        };
        int so_luong_icon = 11;

        int khoangCachNganNhatToanBang = 99999;
        int hangDuocChonCuoiCung = -1;
        int thoiTietDuocChonCuoiCung = -1;
        cv::Point toaDoIconChonCuoiCung(-1, -1);

        // Mặc định ban đầu chưa tìm thấy vị trí trong lượt quét này
        int tamMapHienTai = -1;

        // --- BƯỚC 4: VÒNG LẶP QUÉT TỪNG HÀNG (THỜI TIẾT + VỊ TRÍ) ---
        for (int hang = 0; hang < 5; hang++) {
            if (!thongTin->dangChay) break;

            int y_bat_dau_hang = roi_Goc.y + cvRound(hang * chieuCaoMoiHang);
            int y_ket_thuc_hang = roi_Goc.y + cvRound((hang + 1) * chieuCaoMoiHang);
            int chieuCaoThucCuaHang = y_ket_thuc_hang - y_bat_dau_hang;

            // =================================================================
            // 🎯 ĐOẠN ĐÃ SỬA: QUÉT CHỮ "VỊ TRÍ HIỆN TẠI" TRONG KHUNG ROI CỐ ĐỊNH CỦA TỪNG MAP
            // =================================================================
            cv::Rect roiViTriCuaHang = roi_TenMapTungHang[hang];

            // Vẽ khung màu xanh lơ mỏng hiển thị các phân vùng check vị trí bên trái để debug
            cv::rectangle(anhDebug, roiViTriCuaHang, cv::Scalar(255, 128, 0), 1);

            // Quét tìm xem chữ "Vị trí hiện tại" (màu cam) có nhảy vào khung ROI cố định của Map này không
            cv::Point pViTri = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_ViTriHienTai, roiViTriCuaHang, 0.75);
            if (pViTri.x != -1) {
                tamMapHienTai = hang + 1; // Hàng 0 -> Map 1 (Plaza), Hàng 2 -> Map 3 (KND)...

                // Vẽ khung màu cam đậm bọc chặt lấy chữ "Vị trí hiện tại" khi quét trúng
                cv::rectangle(anhDebug, roiViTriCuaHang, cv::Scalar(0, 165, 255), 2);
                cv::putText(anhDebug, "YOU HERE", cv::Point(roiViTriCuaHang.x + 5, roiViTriCuaHang.y + 25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 165, 255), 1, cv::LINE_AA);
            }

            // BỎ QUA HOÀN TOÀN DÒNG THỨ 2 KHI QUÉT ICON THỜI TIẾT (Theo code cũ của ông)
            if (hang == 1) {
                continue;
            }

            // Reset dữ liệu mảng kết quả từng hàng trong struct về mặc định trống
            thongTin->thoiTietTungHang[hang] = -1;
            thongTin->khoangCachTungHang[hang] = 99999;

            // Chiều rộng hợp lệ tính từ vị trí vạch đỏ hắt sang cạnh bên phải bảng để quét Icon
            int do_rong_quet_phai = (roi_Goc.x + roi_Goc.width) - x_vach_do_tuyet_doi;

            if (do_rong_quet_phai > 10) {
                // ĐỊNH NGHĨA VÙNG ROI TỪNG HÀNG: Khóa chặt Y của hàng, bắt đầu từ X vạch đỏ hắt sang phải
                cv::Rect roiCuaHang(x_vach_do_tuyet_doi, y_bat_dau_hang, do_rong_quet_phai, chieuCaoThucCuaHang);

                // Vẽ khung phân vùng ô chữ nhật màu vàng hiển thị trực quan vùng quét thời tiết
                cv::rectangle(anhDebug, roiCuaHang, cv::Scalar(255, 255, 0), 1, cv::LINE_AA);

                // Viết ký hiệu H1, H3, H4, H5 ngay đầu ô quét bám theo vạch đỏ
                string tenHang = "H" + to_string(hang + 1);
                cv::putText(anhDebug, tenHang, cv::Point(roiCuaHang.x + 5, roiCuaHang.y + 25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(255, 255, 0), 1, cv::LINE_AA);

                int khoangCachGanNhatTrongHang = 99999;
                int thoiTietGanNhatTrongHang = -1;
                cv::Point toaDoIconTrongHang(-1, -1);

                // Duyệt tìm từ icon thứ nhất đến cuối danh sách (11 icon)
                for (int i = 0; i < so_luong_icon; i++) {
                    cv::Point pIcon = BoDieuKhien::TimAnhTrongVung(anhGoc, ds_icon_thoi_tiet[i], roiCuaHang, 0.8);

                    if (pIcon.x != -1) {
                        int khoangCachX = pIcon.x - x_vach_do_tuyet_doi;
                        cv::circle(anhDebug, pIcon, 5, cv::Scalar(0, 255, 255), -1);

                        if (khoangCachX < khoangCachGanNhatTrongHang) {
                            khoangCachGanNhatTrongHang = khoangCachX;
                            thoiTietGanNhatTrongHang = i;
                            toaDoIconTrongHang = pIcon;
                        }
                    }
                }

                if (thoiTietGanNhatTrongHang != -1) {
                    thongTin->thoiTietTungHang[hang] = thoiTietGanNhatTrongHang;
                    thongTin->khoangCachTungHang[hang] = khoangCachGanNhatTrongHang;

                    string textDist = to_string(khoangCachGanNhatTrongHang) + "px";
                    cv::putText(anhDebug, textDist, cv::Point(roiCuaHang.x + 40, roiCuaHang.y + 25),
                        cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(255, 255, 0), 1, cv::LINE_AA);

                    if (khoangCachGanNhatTrongHang < khoangCachNganNhatToanBang) {
                        khoangCachNganNhatToanBang = khoangCachGanNhatTrongHang;
                        hangDuocChonCuoiCung = hang;
                        thoiTietDuocChonCuoiCung = thoiTietGanNhatTrongHang;
                        toaDoIconChonCuoiCung = toaDoIconTrongHang;
                    }
                }
            }
        }

        // Cập nhật kết quả vị trí nhân vật tìm được ra Struct tổng
        thongTin->mapHienTai = tamMapHienTai;

        // --- BƯỚC 5: XUẤT QUYẾT ĐỊNH CHO BOT ĐI SĂN ---
        string txtQuyetDinh = "Map hien tai: ";
        switch (thongTin->mapHienTai) {
        case 1: txtQuyetDinh += "[Plaza] "; break;
        case 2: txtQuyetDinh += "[CAMP] "; break;
        case 3: txtQuyetDinh += "[KND] "; break;
        case 4: txtQuyetDinh += "[KTT] "; break;
        case 5: txtQuyetDinh += "[HOME] "; break;
        default: txtQuyetDinh += "[Unknown/Sanh] "; break;
        }

        txtQuyetDinh += " | Thoi tiet tiep theo: ";
        if (hangDuocChonCuoiCung != -1) {
            thongTin->mapSanDuoc = hangDuocChonCuoiCung + 1;
            thongTin->thoiTietSannDuoc = thoiTietDuocChonCuoiCung;
            txtQuyetDinh += ds_icon_thoi_tiet[thoiTietDuocChonCuoiCung];

            cv::arrowedLine(anhDebug, cv::Point(x_vach_do_tuyet_doi, toaDoIconChonCuoiCung.y),
                toaDoIconChonCuoiCung, cv::Scalar(0, 0, 255), 2, 8, 0, 0.15);
        }
        else {
            thongTin->mapSanDuoc = -1;
            thongTin->thoiTietSannDuoc = -1;
            txtQuyetDinh += "Khong thay icon.";
        }

        thongTin->thongBaoStatus = txtQuyetDinh;

        // Vẽ banner kết quả
        cv::rectangle(anhDebug, cv::Rect(10, 10, 620, 35), cv::Scalar(0, 0, 0), -1);
        cv::putText(anhDebug, txtQuyetDinh, cv::Point(15, 32), cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
        cv::imshow(tenCuaSo, anhDebug);

        int key = cv::waitKey(30);
        if (key == 27 || key == 'q') {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }

    cv::destroyWindow(tenCuaSo);
    thongTin->thongBaoStatus = "Da dong mat quet thoi tiet va vi tri.";
}










void giulaicacanthiet(ThongTinTool* thongTin) {
    thongTin->thongBaoStatus = "--- HE THONG GIAI PHONG TAI NGUYEN: DA KICH HOAT ---";
    HWND h_gameplay = thongTin->h_game;

    // Vòng lặp chạy vô hạn cho đến khi người dùng tắt từ Dashboard
    while (thongTin->dangChay) {
        if (h_gameplay == NULL) {
            this_thread::sleep_for(chrono::milliseconds(1000));
            continue;
        }

        // --- 1. CHỤP ẢNH ---
        cv::Mat anhGoc = BoDieuKhien::ChupManHinh(h_gameplay);
        if (anhGoc.empty()) {
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }

        // --- 2. KHAI BÁO ROI & TÌM ẢNH ---
        // roiLogic dùng cho Bannhanh, Baoquan. roiCa dùng để check Khongcobienthe
        cv::Rect roiLogic(620, 430, 160, 70);
        cv::Rect roituithe(698, 427, 146, 60);
        cv::Rect roiCa(843, 304, 78, 62);

        cv::Point pBanNhanh = BoDieuKhien::TimAnhTrongVung(anhGoc, "bannhanh.png", roiLogic, 0.7);
        cv::Point pBaoQuan = BoDieuKhien::TimAnhTrongVung(anhGoc, "baoquan.png", roiLogic, 0.55);
        cv::Point pMoTuiThe = BoDieuKhien::TimAnhTrongVung(anhGoc, "motuithe.png", roituithe, 0.55);
        cv::Point pKhongCoBienThe = BoDieuKhien::TimAnhTrongVung(anhGoc, "khongcobienthe.png", roiCa, 0.7);

        // --- 3. THỰC THI LOGIC ---

        // Nhánh 1: Nếu thấy Bán Nhanh
        if (pBanNhanh.x != -1) {
            // Kiểm tra: Nếu thấy ảnh "khongcobienthe" THÌ BÁN, còn lại BẢO QUẢN
            if (pKhongCoBienThe.x != -1) {
                thongTin->thongBaoStatus = "Action: BAN CA (Phat hien khong co bien the)";
                BoDieuKhien::BamChuot(h_gameplay, 671, 438);
            }
            else {
                thongTin->thongBaoStatus = "Action: BAO QUAN (Co bien the hoac ko xac dinh)";
                BoDieuKhien::BamChuot(h_gameplay, 752, 459);
            }
            // Nghỉ sau khi thao tác để game kịp phản hồi
            this_thread::sleep_for(chrono::milliseconds(500));
        }
        // Nhánh 2: Nếu không thấy Bán Nhanh nhưng thấy ảnh Bảo Quản (Giữ nguyên logic cũ)
         if (pBaoQuan.x != -1) {
            thongTin->thongBaoStatus = "Action: CAT DO (Bao quan)";
            BoDieuKhien::BamChuot(h_gameplay, 629, 452);
            this_thread::sleep_for(chrono::milliseconds(500));
        }
         // nhánh 3 mở thẻ
         if (pMoTuiThe.x != -1) {
             thongTin->thongBaoStatus = "Action: mo tui the game";
             BoDieuKhien::BamChuot(h_gameplay, 722, 461);
             this_thread::sleep_for(chrono::milliseconds(500));
         }

        // --- 4. DỌN DẸP & NGHỈ ---
        anhGoc.release();

        // Nghỉ một chút giữa các vòng lặp để tránh chiếm CPU
        this_thread::sleep_for(chrono::milliseconds(300));
    }

    thongTin->thongBaoStatus = "--- HE THONG GIAI PHONG TAI NGUYEN: DA TAT ---";
}























void LuongTeleport(ThongTinTool* tab) {
    // 1. Khai báo vùng ROI (Ví dụ: Quét ở góc trên bên phải màn hình)
    // Ông thay x, y, width, height bằng tọa độ vùng ông muốn quét
    cv::Rect vungQuetROI(128, 67, 491, 350);

    cv::Mat nut_tele = cv::imread("btn_tele.png");
    if (nut_tele.empty()) {
        tab->thongBaoStatus = "Loi: Khong tim thay btn_tele.png!";
        tab->dangChay = false;
        return;
    }

    while (tab->dangChay) {
        // --- HANH DONG 1: XOAY MAN HINH (Giu nguyen logic keo chuot) ---
        tab->thongBaoStatus = "Dang xoay camera...";
        int midX = 131, midY = 155;
        int quang_duong = 300;
        int so_buoc = 50;

        ::PostMessage(tab->h_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(midX, midY));
        this_thread::sleep_for(chrono::milliseconds(100));

        for (int i = 1; i <= so_buoc; i++) {
            if (!tab->dangChay) break;
            int x_hien_tai = midX + (quang_duong * i / so_buoc);
            ::PostMessage(tab->h_game, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x_hien_tai, midY));
            this_thread::sleep_for(chrono::milliseconds(30));
        }
        ::PostMessage(tab->h_game, WM_LBUTTONUP, 0, MAKELPARAM(midX + quang_duong, midY));

        this_thread::sleep_for(chrono::seconds(2));

        // --- HANH DONG 2: QUET TIM NUT TRONG ROI ---
        cv::Mat screenFull = BoDieuKhien::ChupManHinh(tab->h_game);
        if (!screenFull.empty()) {

            // KIỂM TRA ROI CÓ NẰM TRONG MÀN HÌNH KHÔNG (Tránh crash)
            if (vungQuetROI.x + vungQuetROI.width <= screenFull.cols &&
                vungQuetROI.y + vungQuetROI.height <= screenFull.rows)
            {
                // Cắt lấy vùng ROI
                cv::Mat screenROI = screenFull(vungQuetROI);

                cv::Mat result;
                cv::matchTemplate(screenROI, nut_tele, result, cv::TM_CCOEFF_NORMED);
                double minVal, maxVal; cv::Point minLoc, maxLoc;
                cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

                if (maxVal > 0.75) {
                    // QUAN TRỌNG: Tọa độ bấm = tọa độ trong ROI + tọa độ gốc của ROI
                    int targetX = maxLoc.x + vungQuetROI.x + (nut_tele.cols / 2);
                    int targetY = maxLoc.y + vungQuetROI.y + (nut_tele.rows / 2);

                    tab->thongBaoStatus = "Thay Tele trong ROI (" + to_string((int)(maxVal * 100)) + "%)!";
                    BoDieuKhien::BamChuot(tab->h_game, targetX, targetY);
                }
                else {
                    tab->thongBaoStatus = "ROI khong co nut Tele.";
                }
            }
        }

        // Đợi 2 phút
        for (int i = 0; i < 120; i++) {
            if (!tab->dangChay) break;
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
}







//hàm chuẩn 
void LuongNgamCanhGioiThoiTiet1(ThongTinTool* thongTin) {
    cv::Rect roiDieuKienChuyenMap(94, 15, 65, 65);
    string anh_DieuKien_1 = "dieu_kien_chuyen_1.png";
    string anh_DieuKien_2 = "dieu_kien_chuyen_2.png";

    // Trạng thái nội bộ của luồng ngầm
    enum TrangThaiLuongNgam {
        LUONG_NGAM_DANG_QUET,       // GĐ 1: Đang chờ thấy ảnh điều kiện lần đầu
        LUONG_NGAM_CHO_120_GIAY,    // GĐ 1: Đã thấy ảnh, đang đếm ngược 120 giây (không quét)
        LUONG_NGAM_QUET_LAI_45_GIAY // GĐ 2: Quét lại tối đa 45 giây để xác nhận
    };

    TrangThaiLuongNgam trangThai = LUONG_NGAM_DANG_QUET;
    chrono::steady_clock::time_point thoiDiemBatDauGD;

    while (true) {
        this_thread::sleep_for(chrono::milliseconds(500));

        // Nếu bot dừng hoặc tắt auto thời tiết → reset toàn bộ về ban đầu
        if (!thongTin->dangChay || !thongTin->batAutoThoiTiet) {
            trangThai = LUONG_NGAM_DANG_QUET;
            thongTin->yeuCauNgatCau = false;
            thongTin->giayDemNguoc = 0;
            continue;
        }

        // Chỉ kích hoạt khi bot đang thực sự câu cá
        // SỬA BUG: Dùng trangThaiCauCa (TrangThaiBot mới) thay vì trangThaiHienTai (BotState cũ)
        if (thongTin->trangThaiCauCa != TT_CAU_CA_PRO) {
            trangThai = LUONG_NGAM_DANG_QUET;
            thongTin->yeuCauNgatCau = false;
            thongTin->giayDemNguoc = 0;
            continue;
        }

        // =====================================================================
        // GĐ 1A — ĐANG QUÉT: Chờ thấy ảnh điều kiện lần đầu
        // =====================================================================
        if (trangThai == LUONG_NGAM_DANG_QUET) {
            cv::Mat anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);
            if (anhGoc.empty()) continue;

            cv::Point p1 = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_DieuKien_1, roiDieuKienChuyenMap, 0.8);
            cv::Point p2 = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_DieuKien_2, roiDieuKienChuyenMap, 0.8);

            if (p1.x != -1 || p2.x != -1) {
                // Lần đầu thấy ảnh → chuyển sang giai đoạn chờ 120 giây
                trangThai = LUONG_NGAM_CHO_120_GIAY;
                thoiDiemBatDauGD = chrono::steady_clock::now();
                thongTin->thongBaoStatus = "[LUONG NGAM] Thay anh dieu kien! Dem nguoc 120 giay...";
            }
        }

        // =====================================================================
        // GĐ 1B — ĐẾM NGƯỢC 120 GIÂY: Không quét, chỉ cập nhật giayDemNguoc
        // =====================================================================
        else if (trangThai == LUONG_NGAM_CHO_120_GIAY) {
            auto bayGio = chrono::steady_clock::now();
            int giayTroiQua = (int)chrono::duration_cast<chrono::seconds>(bayGio - thoiDiemBatDauGD).count();
            int conLai = 120 - giayTroiQua;

            thongTin->giayDemNguoc = (conLai > 0) ? conLai : 0;

            if (conLai <= 0) {
                // Hết 120 giây → chuyển sang giai đoạn quét lại 45 giây
                trangThai = LUONG_NGAM_QUET_LAI_45_GIAY;
                thoiDiemBatDauGD = chrono::steady_clock::now(); // Reset mốc thời gian cho GĐ 2
                thongTin->giayDemNguoc = 0;
                thongTin->thongBaoStatus = "[LUONG NGAM] Het 120s. Bat dau quet lai xac nhan 45 giay...";
            }
        }

        // =====================================================================
        // GĐ 2 — QUÉT LẠI 45 GIÂY: Xác nhận thời tiết có còn xấu không
        // =====================================================================
        else if (trangThai == LUONG_NGAM_QUET_LAI_45_GIAY) {
            auto bayGio = chrono::steady_clock::now();
            int giayTroiQua = (int)chrono::duration_cast<chrono::seconds>(bayGio - thoiDiemBatDauGD).count();
            int conLai45 = 45 - giayTroiQua;

            // Hết 45 giây mà không thấy ảnh nào → thời tiết đã qua / nhiễu → reset về đầu
            if (conLai45 <= 0) {
                trangThai = LUONG_NGAM_DANG_QUET;
                thongTin->giayDemNguoc = 0;
                thongTin->thongBaoStatus = "[LUONG NGAM] Quet 45s khong thay lai. Tiep tuc cau.";
                continue;
            }

            // Vẫn còn trong 45 giây → quét thử 1 lần
            cv::Mat anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);
            if (anhGoc.empty()) continue;

            cv::Point p1 = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_DieuKien_1, roiDieuKienChuyenMap, 0.8);
            cv::Point p2 = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_DieuKien_2, roiDieuKienChuyenMap, 0.8);

            if (p1.x != -1 || p2.x != -1) {
                // Xác nhận vẫn thấy ảnh → PHÁT LỆNH NGẮT CẦN + ĐỔI MAP
                thongTin->yeuCauNgatCau = true;
                thongTin->giayDemNguoc = 0;
                trangThai = LUONG_NGAM_DANG_QUET; // Reset về đầu để vòng tiếp theo chờ lại từ đầu
                thongTin->thongBaoStatus = "[LUONG NGAM] XAC NHAN! Phat lenh cat can, chuan bi doi map!";
            }
            // Nếu lần quét này không thấy → tiếp tục vòng lặp cho đến hết 45 giây
        }
    }
}










void LuongNgamCanhGioiThoiTiet(ThongTinTool* thongTin) {
    cv::Rect roiDieuKienChuyenMap(94, 15, 65, 65);
    string anh_DieuKien_1 = "dieu_kien_chuyen_1.png";
    string anh_DieuKien_2 = "dieu_kien_chuyen_2.png";

    enum TrangThaiLuongNgam {
        LUONG_NGAM_DANG_QUET,
        LUONG_NGAM_CHO_120_GIAY,
        LUONG_NGAM_QUET_LAI_45_GIAY
    };

    TrangThaiLuongNgam trangThai = LUONG_NGAM_DANG_QUET;
    chrono::steady_clock::time_point thoiDiemBatDauGD;

    while (true) {
        this_thread::sleep_for(chrono::milliseconds(500));

        // CƠ CHẾ RESET THEO YÊU CẦU: Nếu dừng tool hoặc tắt tính năng, hoặc có tín hiệu ép reset
        if (!thongTin->dangChay || !thongTin->batAutoThoiTiet || thongTin->canResetLuongNgam) {
            trangThai = LUONG_NGAM_DANG_QUET;
            thongTin->yeuCauNgatCau = false;
            thongTin->giayDemNguoc = 0;
            thongTin->canResetLuongNgam = false; // Reset xong xóa cờ
            continue;
        }

        // CHỈ KÍCH HOẠT LỆNH NGẦM KHI ĐÃ BẬT CẦN VÀ ĐANG TRONG GUỒNG CÂU ĐỢI CÁ
        if (thongTin->trangThaiCauCa != TT_CAU_CA_PRO) {
            trangThai = LUONG_NGAM_DANG_QUET;
            thongTin->yeuCauNgatCau = false;
            thongTin->giayDemNguoc = 0;
            continue;
        }

        // =====================================================================
        // GĐ 1A — ĐANG QUÉT: Chờ thấy ảnh điều kiện lần đầu
        // =====================================================================
        if (trangThai == LUONG_NGAM_DANG_QUET) {
            cv::Mat anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);
            if (anhGoc.empty()) continue;

            cv::Point p1 = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_DieuKien_1, roiDieuKienChuyenMap, 0.8);
            cv::Point p2 = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_DieuKien_2, roiDieuKienChuyenMap, 0.8);

            if (p1.x != -1 || p2.x != -1) {
                trangThai = LUONG_NGAM_CHO_120_GIAY;
                thoiDiemBatDauGD = chrono::steady_clock::now();
                thongTin->thongBaoStatus = "[LUONG NGAM] Thay anh dieu kien! Dem nguoc 120 giay...";
            }
        }

        // =====================================================================
        // GĐ 1B — ĐẾM NGƯỢC 120 GIÂY: Đóng băng không quét hình
        // =====================================================================
        else if (trangThai == LUONG_NGAM_CHO_120_GIAY) {
            auto bayGio = chrono::steady_clock::now();
            int giayTroiQua = (int)chrono::duration_cast<chrono::seconds>(bayGio - thoiDiemBatDauGD).count();
            int conLai = 120 - giayTroiQua;

            thongTin->giayDemNguoc = (conLai > 0) ? conLai : 0;

            if (conLai <= 0) {
                trangThai = LUONG_NGAM_QUET_LAI_45_GIAY;
                thoiDiemBatDauGD = chrono::steady_clock::now();
                thongTin->giayDemNguoc = 0;
                thongTin->thongBaoStatus = "[LUONG NGAM] Het 120s. Bat dau quet lai xac nhan 45 giay...";
            }
        }

        // =====================================================================
        // GĐ 2 — QUÉT LẠI 45 GIÂY: Xác nhận đổi map tuyến tính
        // =====================================================================
        else if (trangThai == LUONG_NGAM_QUET_LAI_45_GIAY) {
            auto bayGio = chrono::steady_clock::now();
            int giayTroiQua = (int)chrono::duration_cast<chrono::seconds>(bayGio - thoiDiemBatDauGD).count();
            int conLai45 = 45 - giayTroiQua;

            if (conLai45 <= 0) {
                trangThai = LUONG_NGAM_DANG_QUET;
                thongTin->giayDemNguoc = 0;
                thongTin->thongBaoStatus = "[LUONG NGAM] Quet 45s khong thay lai. Tiep tuc cau.";
                continue;
            }

            cv::Mat anhGoc = BoDieuKhien::ChupManHinh(thongTin->h_game);
            if (anhGoc.empty()) continue;

            cv::Point p1 = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_DieuKien_1, roiDieuKienChuyenMap, 0.8);
            cv::Point p2 = BoDieuKhien::TimAnhTrongVung(anhGoc, anh_DieuKien_2, roiDieuKienChuyenMap, 0.8);

            if (p1.x != -1 || p2.x != -1) {
                thongTin->yeuCauNgatCau = true; // Kích hoạt cờ ngắt tuyến tính cho hàm chính
                thongTin->giayDemNguoc = 0;
                trangThai = LUONG_NGAM_DANG_QUET;
                thongTin->thongBaoStatus = "[LUONG NGAM] XAC NHAN! Phat lenh doi map sau luot cau nay!";
            }
        }
    }
}




























// hàm chuẩn 
void AutoSanThoiTietCauCa3(ThongTinTool* thongTin) {
    thongTin->trangThaiCauCa = TT_CHECK_THOI_TIET;

    // =========================================================================
    // BƯỚC INIT: Kiểm tra vị trí thực tế TRƯỚC khi vào vòng lặp chính
    // Lý do: mapHienTai mặc định = -1, nếu không check trước thì logic switch
    //        ở TT_CHECK_THOI_TIET sẽ ra quyết định sai khi so sánh với mapMuonSan
    // =========================================================================
    thongTin->thongBaoStatus = "[INIT] Dang xac dinh vi tri ban dau...";
    while (thongTin->dangChay) {
        KiemTraLaiViTri(thongTin);
        if (thongTin->mapHienTai != -1) break; // Đã biết vị trí → vào vòng chính
        this_thread::sleep_for(chrono::milliseconds(1000)); // Nếu đang load map thì chờ rồi thử lại
    }

    // =========================================================================
    // VÒNG LẶP CHÍNH
    // =========================================================================
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(200));

        if (!thongTin->dangChay) continue;

        // Nếu người dùng TẮT chế độ auto săn thời tiết → nhảy thẳng vào bật cần câu bình thường
        if (!thongTin->batAutoThoiTiet) {
            thongTin->trangThaiCauCa = TT_BAT_CAN;
            // QUAN TRỌNG: continue ở đây để switch bên dưới xử lý TT_BAT_CAN đúng ở lần lặp tiếp theo
            // (không nhảy vào switch giữa chừng với trạng thái chưa ổn định)
            continue;
        }

        switch (thongTin->trangThaiCauCa) {

            // ================================================================
            // TRẠNG THÁI 0: QUÉT BẢNG THỜI TIẾT & XÁC ĐỊNH MAP MỤC TIÊU
            // ================================================================
        case TT_CHECK_THOI_TIET: {
            thongTin->thongBaoStatus = "Dang quet thoi tiet va vi tri...";

            // Gọi 1 lần, hàm này không còn vòng while - cập nhật rồi trả về ngay
            CheckThoiTietVaViTri(thongTin);

            // ĐIỀU KIỆN TIẾP TỤC CHỜ: Thời tiết muốn săn chưa xuất hiện
            if (thongTin->thoiTietMuonSan != -1 &&
                thongTin->thoiTietSannDuoc != thongTin->thoiTietMuonSan) {
                thongTin->thongBaoStatus = "Thoi tiet can san chua xuat hien. Tiep tuc cho...";
                this_thread::sleep_for(chrono::milliseconds(2000)); // Đợi rồi quét lại
                continue;
            }

            // ĐIỀU KIỆN TIẾP TỤC CHỜ: Thời tiết đúng nhưng sai map
            if (thongTin->mapMuonSan != -1 &&
                thongTin->mapSanDuoc != thongTin->mapMuonSan) {
                thongTin->thongBaoStatus = "Thoi tiet dung nhung sai Map. Bo qua lan nay...";
                this_thread::sleep_for(chrono::milliseconds(2000));
                continue;
            }

            // PHÍM TẮT: Nếu đang đứng đúng map muốn săn → bỏ qua di chuyển, bật cần luôn
            if (thongTin->mapHienTai == thongTin->mapMuonSan) {
                thongTin->thongBaoStatus = "[BO QUA DOI MAP] Da dung san nha, nhay thang ve bat can!";
                thongTin->trangThaiCauCa = TT_BAT_CAN;
            }
            else {
                thongTin->trangThaiCauCa = TT_DI_CHUYEN_MAP;
            }
            break;
        }

                               // ================================================================
                               // TRẠNG THÁI 1: DI CHUYỂN LIÊN MAP (CÓ TRẠM TRUNG CHUYỂN PLAZA)
                               // ================================================================
        case TT_DI_CHUYEN_MAP: {
            thongTin->thongBaoStatus = "Dang phat lenh dieu phoi di chuyen lien ban do...";

            // NHÁNH 1: Đi Plaza (1) hoặc KTT (4) → Đi thẳng
            if (thongTin->mapMuonSan == 1 || thongTin->mapMuonSan == 4) {
                if (thongTin->mapMuonSan == 1) ChuyenToiMapPlaza(thongTin);
                else                           ChuyenToiMapKtt(thongTin);

                // Chờ nhân vật đến đúng map mục tiêu
                while (thongTin->dangChay) {
                    KiemTraLaiViTri(thongTin);
                    if (thongTin->mapHienTai == thongTin->mapMuonSan) break;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }
            }
            // NHÁNH 2: Đi KND (3) hoặc HOME (5) → BẮT BUỘC transit qua Plaza trước
            else if (thongTin->mapMuonSan == 3 || thongTin->mapMuonSan == 5) {
                thongTin->thongBaoStatus = "[TRUNG CHUYEN] Chay ve tram Plaza truoc...";
                ChuyenToiMapPlaza(thongTin);

                // Chờ đứng vững ở Plaza
                while (thongTin->dangChay) {
                    KiemTraLaiViTri(thongTin);
                    if (thongTin->mapHienTai == 1) break;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }

                // Từ Plaza đi tiếp đến map đích thực sự
                if (thongTin->mapMuonSan == 3) ChuyenSangMapKND(thongTin);
                else                           ChuyenVeMapHome(thongTin);

                // Xác nhận đã đến map đích
                while (thongTin->dangChay) {
                    KiemTraLaiViTri(thongTin);
                    if (thongTin->mapHienTai == thongTin->mapMuonSan) break;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }
            }

            thongTin->trangThaiCauCa = TT_SETUP_VITRI;
            break;
        }

                             // ================================================================
                             // TRẠNG THÁI 2: SETUP VỊ TRÍ AUTO PATH RA ĐIỂM CÂU
                             // ================================================================
        case TT_SETUP_VITRI: {
            thongTin->thongBaoStatus = "Da den dung map muc tieu! Dang chay auto path ra diem cau...";

            if (thongTin->mapHienTai == 1) SetUpCauMapPlazaHaiDang(thongTin);
            else if (thongTin->mapHienTai == 3) SetUpCauCaMapKnd(thongTin);
            else if (thongTin->mapHienTai == 4) SetUpCauMapKtt(thongTin);
            else if (thongTin->mapHienTai == 5) SetUpCauCaMapHome(thongTin);

            thongTin->trangThaiCauCa = TT_BAT_CAN;
            break;
        }

                           // ================================================================
                           // TRẠNG THÁI 3: KIỂM TRA VÀ SPAM BẬT CẦN CÂU (Delay 1 giây/lần)
                           // ================================================================
        case TT_BAT_CAN: {
            thongTin->thongBaoStatus = "Dang kiem tra trang thai can cau...";

            while (thongTin->dangChay) {
                CheckTrangThaiCanCau_HSV_TestLoop(thongTin);

                if (thongTin->thongBaoStatus == "Cam can") {
                    break; // Cần đã lên tay → Thoát loop
                }

                // Chưa bật → Spam click tọa độ mồi câu
                BoDieuKhien::BamChuot(thongTin->h_game, 304, 509);
                this_thread::sleep_for(chrono::seconds(1)); // Delay đúng 1 giây
            }

            thongTin->trangThaiCauCa = TT_CAU_CA_PRO;
            break;
        }

                       // ================================================================
                       // TRẠNG THÁI 4: ĐANG CÂU - Lắng nghe lệnh ngắt từ luồng ngầm
                       // Bot đứng câu và auto câu xử lý phần còn lại.
                       // Nhiệm vụ ở đây: nếu luồng ngầm phát lệnh → tắt cần → quay về check thời tiết
                       // ================================================================
        case TT_CAU_CA_PRO: {
            if (thongTin->yeuCauNgatCau) {
                thongTin->thongBaoStatus = "[ALARM] Luong ngam phat lenh! Dang spam cat can de doi map...";

                // Spam bấm tắt cần cho đến khi HSV xác nhận cần đã cất
                while (thongTin->dangChay) {
                    BoDieuKhien::BamChuot(thongTin->h_game, 731, 408); // Tọa độ nút tắt cần
                    this_thread::sleep_for(chrono::milliseconds(300));

                    CheckTrangThaiCanCau_HSV_TestLoop(thongTin);
                    if (thongTin->thongBaoStatus == "Khong cam can") {
                        break; // Cất cần thành công
                    }
                }

                // Reset cờ hiệu, quay lại check thời tiết từ đầu
                thongTin->yeuCauNgatCau = false;
                thongTin->trangThaiCauCa = TT_CHECK_THOI_TIET;
                break;
            }

            // Không có lệnh ngắt → bot đang câu bình thường, không làm gì cả
            thongTin->thongBaoStatus = "THIEN THOI DIA LOI! Bot dang cau ca...";
            // Auto câu đã xử lý phần còn lại, luồng này chỉ chờ lệnh từ luồng ngầm
            break;
        }

        } // end switch
    } // end while
}




void ResetDuLieuAutoThoiTiet(ThongTinTool* thongTin) {
    thongTin->trangThaiCauCa = TT_CHECK_THOI_TIET;
    thongTin->yeuCauNgatCau = false;
    thongTin->giayDemNguoc = 0;
    thongTin->mapHienTai = -1;
    thongTin->thoiTietSannDuoc = -1;
    thongTin->mapSanDuoc = -1;

    // Khôi phục lại ý muốn của người dùng từ biến gốc
    thongTin->mapMuonSan = thongTin->luaChonMapGoc;

    thongTin->canResetLuongNgam.store(true);
    thongTin->thongBaoStatus = "[RESET] Da lam sach du lieu.";
}








void AutoSanThoiTietCauCa(ThongTinTool* thongTin) {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(200));

        // 1. Kiểm tra trạng thái dừng
        if (!thongTin->dangChay || !thongTin->batAutoThoiTiet) {
            continue;
        }

        switch (thongTin->trangThaiCauCa) {
        case TT_CHECK_THOI_TIET: {
            // Đợi dữ liệu ổn định sau khi reset
            if (thongTin->mapSanDuoc == -1) this_thread::sleep_for(chrono::milliseconds(1000));

            CheckThoiTietVaViTri(thongTin);

            // Kiểm tra thời tiết mục tiêu
            if (thongTin->thoiTietMuonSan != -1 && thongTin->thoiTietSannDuoc != thongTin->thoiTietMuonSan) {
                thongTin->thongBaoStatus = "Cho thoi tiet mong muon...";
                this_thread::sleep_for(chrono::milliseconds(2000));
                break;
            }

            // Logic lọc map: nếu đã chọn map cố định thì phải khớp, không thì săn tất cả trừ map 2
            bool mapHopLe = (thongTin->luaChonMapGoc != -1) ? (thongTin->mapSanDuoc == thongTin->luaChonMapGoc)
                : (thongTin->mapSanDuoc != 2 && thongTin->mapSanDuoc != -1);

            if (!mapHopLe) {
                thongTin->thongBaoStatus = "Map khong hop le, dang tim...";
                this_thread::sleep_for(chrono::milliseconds(2000));
                break;
            }

            // Quyết định: Cần di chuyển hay Setup luôn
            if (thongTin->mapHienTai == thongTin->mapSanDuoc) {
                thongTin->trangThaiCauCa = TT_SETUP_VITRI;
            }
            else {
                thongTin->mapMuonSan = thongTin->mapSanDuoc; // Cập nhật đích đến
                thongTin->trangThaiCauCa = TT_DI_CHUYEN_MAP;
            }
            break;
        }

        case TT_DI_CHUYEN_MAP: {
            thongTin->thongBaoStatus = "[DI CHUYEN] Bat dau hanh trinh...";
            ChuyenToiMapPlaza(thongTin);

            // 1. Đợi về Plaza (ID 1)
            int dem = 0;
            while (thongTin->dangChay && thongTin->mapHienTai != 1 && dem++ < 20) {
                KiemTraLaiViTri(thongTin);
                this_thread::sleep_for(chrono::milliseconds(800));
            }

            // 2. Đi tiếp đến map mục tiêu
            if (thongTin->mapHienTai == 1) {
                if (thongTin->mapMuonSan == 3)      ChuyenSangMapKND(thongTin);
                else if (thongTin->mapMuonSan == 4) ChuyenToiMapKtt(thongTin);
                else if (thongTin->mapMuonSan == 5) ChuyenVeMapHome(thongTin);

                // Đợi đến map đích thật sự
                dem = 0;
                while (thongTin->dangChay && thongTin->mapHienTai != thongTin->mapMuonSan && dem++ < 20) {
                    KiemTraLaiViTri(thongTin);
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }

                if (thongTin->mapHienTai == thongTin->mapMuonSan) {
                    thongTin->trangThaiCauCa = TT_SETUP_VITRI;
                }
                else {
                    ResetDuLieuAutoThoiTiet(thongTin);
                }
            }
            else {
                ResetDuLieuAutoThoiTiet(thongTin);
            }
            break;
        }

        case TT_SETUP_VITRI: {
            thongTin->thongBaoStatus = "Dang Setup vi tri cau...";
            if (thongTin->mapHienTai == 1)      SetUpCauMapPlazaHaiDang(thongTin);
            else if (thongTin->mapHienTai == 3) SetUpCauCaMapKnd(thongTin);
            else if (thongTin->mapHienTai == 4) SetUpCauMapKtt(thongTin);
            else if (thongTin->mapHienTai == 5) SetUpCauCaMapHome(thongTin);

            thongTin->trangThaiCauCa = TT_BAT_CAN;
            break;
        }

        case TT_BAT_CAN: {
            CheckTrangThaiCanCau_HSV_TestLoop(thongTin);
            giulaicacanthiet(thongTin);
            if (thongTin->thongBaoStatus == "Cam can") {
                thongTin->trangThaiCauCa = TT_CAU_CA_PRO;
            }
            else {
                BoDieuKhien::BamChuot(thongTin->h_game, 304, 509);
                this_thread::sleep_for(chrono::seconds(1));
            }
            break;
        }

        case TT_CAU_CA_PRO: {
            if (thongTin->yeuCauNgatCau.load()) {
                thongTin->thongBaoStatus = "Thoi tiet thay doi, cat can...";
                while (thongTin->dangChay) {
                    BoDieuKhien::BamChuot(thongTin->h_game, 731, 408);
                    this_thread::sleep_for(chrono::milliseconds(300));
                    CheckTrangThaiCanCau_HSV_TestLoop(thongTin);
                    if (thongTin->thongBaoStatus == "Khong cam can") break;
                }
                ResetDuLieuAutoThoiTiet(thongTin);
            }
            break;
        }
        }
    }
}



























void AutoSanThoiTietCauCa1(ThongTinTool* thongTin) {
    thongTin->trangThaiCauCa = TT_CHECK_THOI_TIET;

    while (true) {
        this_thread::sleep_for(chrono::milliseconds(200));
        if (!thongTin->dangChay) continue;

        // Chạy tự do tại chỗ nếu người dùng tắt chế độ săn thời tiết trên UI
        if (!thongTin->batAutoThoiTiet) {
            thongTin->trangThaiCauCa = TT_BAT_CAN;
        }

        switch (thongTin->trangThaiCauCa) {

            // =========================================================================
            // TRẠNG THÁI 0: QUÉT BẢNG ĐỐI CHIẾU THỜI TIẾT & BAN ĐỒ MỤC TIÊU
            // =========================================================================
        case TT_CHECK_THOI_TIET: {
            CheckThoiTietVaViTri(thongTin);

            if (thongTin->thoiTietMuonSan != -1 && thongTin->thoiTietSannDuoc != thongTin->thoiTietMuonSan) {
                thongTin->thongBaoStatus = "Thoi tiet can san chua xuất hien. Dang tiep tuc cho...";
                continue;
            }

            if (thongTin->mapMuonSan != -1 && thongTin->mapSanDuoc != thongTin->mapMuonSan) {
                thongTin->thongBaoStatus = "Thoi tiet dung nhung sai Map. Bo qua...";
                continue;
            }

            // BẪY TRÙNG MAP CỦA ÔNG GIÁO: Nếu đang đứng đúng map muốn săn -> Bật cần câu luôn 🚀
            if (thongTin->mapHienTai == thongTin->mapMuonSan) {
                thongTin->thongBaoStatus = "[BO QUA DOI MAP] Da dung san nha, nhay thang ve buoc bat can!";
                thongTin->trangThaiCauCa = TT_BAT_CAN;
            }
            else {
                thongTin->trangThaiCauCa = TT_DI_CHUYEN_MAP;
            }
            break;
        }

                               // =========================================================================
                               // TRẠNG THÁI 1: LOGIC DI CHUYỂN LIÊN MAP (CÓ TRẠM TRUNG CHUYỂN PLAZA)
                               // =========================================================================
        case TT_DI_CHUYEN_MAP: {
            thongTin->thongBaoStatus = "Dang phat lenh dieu phoi di chuyen lien ban do...";

            // NHÁNH 1: Đi Map Plaza (1) hoặc Khu Trung Tâm (4) -> Đi trực tiếp
            if (thongTin->mapMuonSan == 1 || thongTin->mapMuonSan == 4) {
                if (thongTin->mapMuonSan == 1) ChuyenToiMapPlaza(thongTin);
                else ChuyenToiMapKtt(thongTin);

                // Khóa loop kiểm tra vị trí thực tế cho tới khi khớp map mục tiêu
                while (thongTin->dangChay) {
                    KiemTraLaiViTri(thongTin);
                    if (thongTin->mapHienTai == thongTin->mapMuonSan) break;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }
            }
            // NHÁNH 2: Đi Khu Nghỉ Dưỡng (3) hoặc HOME (5) -> BẮT BUỘC TRUNG CHUYỂN QUA PLAZA
            else if (thongTin->mapMuonSan == 3 || thongTin->mapMuonSan == 5) {
                thongTin->thongBaoStatus = "[TRUNG CHUYEN] Chay ve tram Plaza truoc...";
                ChuyenToiMapPlaza(thongTin);

                // Check chặt chẽ bao giờ nhân vật đứng vững ở Plaza mới được rẽ tiếp
                while (thongTin->dangChay) {
                    KiemTraLaiViTri(thongTin);
                    if (thongTin->mapHienTai == 1) break;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }

                // Đã đáp xuống Plaza an toàn -> Bắt đầu bốc đầu sang map đích thực sự
                if (thongTin->mapMuonSan == 3) ChuyenSangMapKND(thongTin);
                else ChuyenVeMapHome(thongTin);

                // Check lần cuối xác nhận đã cập bến map mong muốn chưa
                while (thongTin->dangChay) {
                    KiemTraLaiViTri(thongTin);
                    if (thongTin->mapHienTai == thongTin->mapMuonSan) break;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }
            }

            thongTin->trangThaiCauCa = TT_SETUP_VITRI;
            break;
        }

                             // =========================================================================
                             // TRẠNG THÁI 2: SETUP VỊ TRÍ AUTO PATH (KÍCH HOẠT CHẠY RA ĐIỂM CÂU)
                             // =========================================================================
        case TT_SETUP_VITRI: {
            thongTin->thongBaoStatus = "Da den dung map mục tieu! Dang chay auto path ra diem cau...";

            if (thongTin->mapHienTai == 1) SetUpCauMapPlazaHaiDang(thongTin);
            else if (thongTin->mapHienTai == 3) SetUpCauCaMapKnd(thongTin);
            else if (thongTin->mapHienTai == 4) SetUpCauMapKtt(thongTin);
            else if (thongTin->mapHienTai == 5) SetUpCauCaMapHome(thongTin);

            thongTin->trangThaiCauCa = TT_BAT_CAN;
            break;
        }

                           // =========================================================================
                           // TRẠNG THÁI 3: KIỂM TRA VÀ SPAM BẬT CẦN CÂU (DELAY ĐÚNG 1 GIÂY NHƯ Ý ÔNG)
                           // =========================================================================
        case TT_BAT_CAN: {
            thongTin->thongBaoStatus = "Dang kiem tra trang thai can cau...";

            while (thongTin->dangChay) {
                CheckTrangThaiCanCau_HSV_TestLoop(thongTin); // Gọi hàm HSV gốc của ông

                if (thongTin->thongBaoStatus == "Cam can") {
                    break; // Cần đã lên tay -> Thoát vòng lặp vào guồng câu cá luôn
                }

                // Nếu chưa bật cần thì nhấn click mồi vào tọa độ quy định
                BoDieuKhien::BamChuot(thongTin->h_game, 304, 509);
                this_thread::sleep_for(chrono::seconds(1)); // Nhịp click delay đúng 1s theo yêu cầu
            }

            thongTin->trangThaiCauCa = TT_CAU_CA_PRO;
            break;
        }

                       // =========================================================================
                       // TRẠNG THÁI 4: VÀO VỊ TRÍ CÂU & LẮNG NGHE LỆNH THU CẦN TỪ LUỒNG NGẦM
                       // =========================================================================
        case TT_CAU_CA_PRO: {
            // Đang trong luồng băm cá thời tiết, liên tục check tín hiệu hủy lệnh từ luồng ngầm
            if (thongTin->yeuCauNgatCau) {
                thongTin->thongBaoStatus = "[ALARM] Luong ngam phat lenh! Dang spam cat can de doi map...";

                // Spam bấm nút cất cần cho tới khi hàm HSV xác nhận đã cất thành công
                while (thongTin->dangChay) {
                    BoDieuKhien::BamChuot(thongTin->h_game, 731, 408); // Spam tọa độ tắt cần
                    this_thread::sleep_for(chrono::milliseconds(300));

                    CheckTrangThaiCanCau_HSV_TestLoop(thongTin); // Gọi hàm HSV kiểm tra lại
                    if (thongTin->thongBaoStatus == "Khong cam can") {
                        break; // Cất cần thành công -> Ngừng spam
                    }
                }

                // Reset trạng thái cờ hiệu, kéo bot ngược lại trạng thái check bảng thời tiết từ đầu
                thongTin->yeuCauNgatCau = false;
                thongTin->trangThaiCauCa = TT_CHECK_THOI_TIET;
                break;
            }

            thongTin->thongBaoStatus = "THIÊN THỜI ĐỊA LỢI! Bot dang tap trung quet pixel giat ca...";
            // Biến xử lý băm chuột giật bong bóng cá cụ thể của ông chạy tại đây...
            break;
        }
        }
    }
}



void AutoSanThoiTietCauCa2(ThongTinTool* thongTin) {
    // Khởi tạo trạng thái ban đầu: Bắt buộc phải check vị trí và thời tiết trước
    thongTin->trangThaiCauCa = TT_CHECK_THOI_TIET;

    while (true) {
        this_thread::sleep_for(chrono::milliseconds(200));
        if (!thongTin->dangChay) continue;

        // Chạy tự do tại chỗ nếu người dùng TẮT chế độ săn thời tiết trên UI
        // CHỈ ép trạng thái nếu bot đang lẩn quẩn ở các bước check hoặc chuyển map
        if (!thongTin->batAutoThoiTiet && thongTin->trangThaiCauCa < TT_BAT_CAN) {
            thongTin->trangThaiCauCa = TT_BAT_CAN;
        }

        switch (thongTin->trangThaiCauCa) {

            // =========================================================================
            // TRẠNG THÁI 0: QUÉT BẢNG ĐỐI CHIẾU THỜI TIẾT & BAN ĐỒ MỤC TIÊU
            // =========================================================================
        case TT_CHECK_THOI_TIET: {
            // Hàm này sẽ giải quyết bài toán "Mới bật tool chưa biết vị trí ở đâu" của ông
            CheckThoiTietVaViTri(thongTin);

            if (thongTin->thoiTietMuonSan != -1 && thongTin->thoiTietSannDuoc != thongTin->thoiTietMuonSan) {
                thongTin->thongBaoStatus = "Thoi tiet can san chua xuat hien. Dang tiep tuc cho...";
                break; // Dùng break thay vi continue de tranh nghen luong, giam tai CPU
            }

            if (thongTin->mapMuonSan != -1 && thongTin->mapSanDuoc != thongTin->mapMuonSan) {
                thongTin->thongBaoStatus = "Thoi tiet dung nhung sai Map. Bo qua...";
                break;
            }

            // BẪY TRÙNG MAP: Nếu đang đứng đúng map muốn săn -> Bật cần câu luôn 🚀
            if (thongTin->mapHienTai == thongTin->mapMuonSan) {
                thongTin->thongBaoStatus = "[NE TRUNG MAP] Da dung vi tri, nhay thang ve buoc bat can!";
                thongTin->trangThaiCauCa = TT_BAT_CAN;
            }
            else {
                thongTin->trangThaiCauCa = TT_DI_CHUYEN_MAP;
            }
            break;
        }

                               // =========================================================================
                               // TRẠNG THÁI 1: LOGIC DI CHUYỂN LIÊN MAP (CÓ TRẠM TRUNG CHUYỂN PLAZA)
                               // =========================================================================
        case TT_DI_CHUYEN_MAP: {
            thongTin->thongBaoStatus = "Dang phat lenh dieu phoi di chuyen lien ban do...";

            // NHÁNH 1: Đi Map Plaza (1) hoặc Khu Trung Tâm (4) -> Đi trực tiếp
            if (thongTin->mapMuonSan == 1 || thongTin->mapMuonSan == 4) {
                if (thongTin->mapMuonSan == 1) ChuyenToiMapPlaza(thongTin);
                else ChuyenToiMapKtt(thongTin);

                // Khóa loop kiểm tra vị trí thực tế cho tới khi khớp map mục tiêu
                while (thongTin->dangChay) {
                    KiemTraLaiViTri(thongTin); // Hàm này phải cập nhật vào thongTin->mapHienTai
                    if (thongTin->mapHienTai == thongTin->mapMuonSan) break;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }
            }
            // NHÁNH 2: Đi Khu Nghỉ Dưỡng (3) hoặc HOME (5) -> BẮT BUỘC TRUNG CHUYỂN QUA PLAZA
            else if (thongTin->mapMuonSan == 3 || thongTin->mapMuonSan == 5) {
                thongTin->thongBaoStatus = "[TRUNG CHUYEN] Chay ve tram Plaza truoc...";
                ChuyenToiMapPlaza(thongTin);

                // Check chặt chẽ bao giờ nhân vật đứng vững ở Plaza (Map ID: 1) mới được rẽ tiếp
                while (thongTin->dangChay) {
                    KiemTraLaiViTri(thongTin);
                    if (thongTin->mapHienTai == 1) break;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }

                // Đã đáp xuống Plaza an toàn -> Bắt đầu bốc đầu sang map đích thực sự
                if (thongTin->mapMuonSan == 3) ChuyenSangMapKND(thongTin);
                else ChuyenVeMapHome(thongTin);

                // Check lần cuối xác nhận đã cập bến map mong muốn chưa
                while (thongTin->dangChay) {
                    KiemTraLaiViTri(thongTin);
                    if (thongTin->mapHienTai == thongTin->mapMuonSan) break;
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }
            }

            thongTin->trangThaiCauCa = TT_SETUP_VITRI;
            break;
        }

                             // =========================================================================
                             // TRẠNG THÁI 2: SETUP VỊ TRÍ AUTO PATH (KÍCH HOẠT CHẠY RA ĐIỂM CÂU)
                             // =========================================================================
        case TT_SETUP_VITRI: {
            thongTin->thongBaoStatus = "Da den dung map mục tieu! Dang chay auto path ra diem cau...";

            if (thongTin->mapHienTai == 1) SetUpCauMapPlazaHaiDang(thongTin);
            else if (thongTin->mapHienTai == 3) SetUpCauCaMapKnd(thongTin);
            else if (thongTin->mapHienTai == 4) SetUpCauMapKtt(thongTin);
            else if (thongTin->mapHienTai == 5) SetUpCauCaMapHome(thongTin);

            thongTin->trangThaiCauCa = TT_BAT_CAN;
            break;
        }

                           // =========================================================================
                           // TRẠNG THÁI 3: KIỂM TRA VÀ SPAM BẬT CẦN CÂU
                           // =========================================================================
        case TT_BAT_CAN: {
            thongTin->thongBaoStatus = "Dang kiem tra trang thai can cau...";

            while (thongTin->dangChay) {
                CheckTrangThaiCanCau_HSV_TestLoop(thongTin); // Gọi hàm HSV gốc của ông

                if (thongTin->thongBaoStatus == "Cam can") {
                    break; // Cần đã lên tay -> Thoát vòng lặp vào guồng câu cá luôn
                }

                // Nếu chưa bật cần thì nhấn click mồi vào tọa độ quy định
                BoDieuKhien::BamChuot(thongTin->h_game, 304, 509);
                this_thread::sleep_for(chrono::seconds(1)); // Nhịp click delay đúng 1s
            }

            thongTin->trangThaiCauCa = TT_CAU_CA_PRO;
            break;
        }

                       // =========================================================================
                       // TRẠNG THÁI 4: VÀO VỊ TRÍ CÂU & LẮNG NGHE LỆNH THU CẦN TỪ LUỒNG NGẦM
                       // =========================================================================
        case TT_CAU_CA_PRO: {
            // Đang trong luồng băm cá thời tiết, liên tục check tín hiệu hủy lệnh từ luồng ngầm
            if (thongTin->yeuCauNgatCau) {
                thongTin->thongBaoStatus = "[ALARM] Luong ngam phat lenh! Dang spam cat can de doi map...";

                // Spam bấm nút cất cần cho tới khi hàm HSV xác nhận đã cất thành công
                while (thongTin->dangChay) {
                    BoDieuKhien::BamChuot(thongTin->h_game, 731, 408); // Spam tọa độ tắt cần
                    this_thread::sleep_for(chrono::milliseconds(300));

                    CheckTrangThaiCanCau_HSV_TestLoop(thongTin);
                    if (thongTin->thongBaoStatus == "Khong cam can") {
                        break; // Cất cần thành công -> Ngừng spam
                    }
                }

                // Reset trạng thái cờ hiệu, kéo bot ngược lại trạng thái check bảng thời tiết từ đầu
                thongTin->yeuCauNgatCau = false;
                thongTin->trangThaiCauCa = TT_CHECK_THOI_TIET;
                break;
            }

            thongTin->thongBaoStatus = "THIÊN THỜI ĐỊA LỢI! Bot dang tap trung quet pixel giat ca...";

            // TODO: Chèn logic quét pixel nhận diện bóng cá/chấm than giật cá của ông ở đây.
            // Ví dụ: DoCauCaLogic(thongTin); 

            break;
        }
        }
    }
}












































































void LuongTuDongFarm(ThongTinTool* thongTin) {
    thongTin->hatTrongDaNho.Invalidate();
    thongTin->henTrongCay=0;
    thongTin->thoiGianMuaHatGanNhat = chrono::steady_clock::now() - chrono::seconds(120);
    thongTin->thoiGianMuaCongCuGanNhat = chrono::steady_clock::now() - chrono::seconds(300);
    

    // --- KHAI BÁO MÁY ĐẾM (Đặt trước vòng lặp để không bị reset oan) ---
    

    while (thongTin->dangChay) {
        thongTin->h_game = FindWindowExA(thongTin->h_cha, NULL, "RenderWindow", NULL);
        if (!thongTin->h_game) {
            thongTin->thongBaoStatus = "Loi: Khong thay Game!";
            this_thread::sleep_for(chrono::milliseconds(1000));
            continue;
        }

        cv::Mat anhChup = BoDieuKhien::ChupManHinh(thongTin->h_game);
        if (anhChup.empty()) continue;

        // --- MẮT BOT: SOI ĐÚNG TỌA ĐỘ ĐÃ CĂN (Giữ nguyên gốc của ông giáo) ---
        if (thongTin->hienMatBot) {
            cv::Mat anhHienThi;
            anhChup.copyTo(anhHienThi);
            string nameWindow = "MAT BOT - " + thongTin->tenTab;

            // ROI của Bán   màu xanh biển
            cv::rectangle(anhHienThi, cv::Rect(632, 272, 54, 43), cv::Scalar(255, 0, 0), 2);    // mặt cười 
            cv::rectangle(anhHienThi, cv::Rect(553, 467, 137, 45), cv::Scalar(255, 0, 0), 2);   // nút chọn 
            cv::rectangle(anhHienThi, cv::Rect(892, 5, 65, 54), cv::Scalar(255, 0, 0), 2);     // nút thoát 
            cv::rectangle(anhHienThi, cv::Rect(245, 397, 152, 32), cv::Scalar(255, 0, 0), 2);  // tên NPC
            cv::rectangle(anhHienThi, cv::Rect(440, 170, 75, 51), cv::Scalar(255, 0, 0), 2);   // NPC bán nông sản
            cv::rectangle(anhHienThi, cv::Rect(79, 83, 109, 431), cv::Scalar(255, 0, 0), 2);   // ROI hạt


            // roi tentim  màu hồng tím

            cv::rectangle(anhHienThi, cv::Rect(317, 232, 155, 155), cv::Scalar(255, 0, 255), 2);   // Vùng biểu tượng ở nhà
            cv::rectangle(anhHienThi, cv::Rect(362, 177, 45, 45), cv::Scalar(255, 0, 255), 2);   // Vùng nút thu hoạch trai
            cv::rectangle(anhHienThi, cv::Rect(772, 17, 50, 34), cv::Scalar(255, 0, 255), 2);   //  Loc.png
            cv::rectangle(anhHienThi, cv::Rect(494, 441, 140, 62), cv::Scalar(255, 0, 255), 2);   // ROI cai dat bo loc
            cv::rectangle(anhHienThi, cv::Rect(770, 477, 60, 60), cv::Scalar(255, 0, 255), 2);   // dau cong hai mau tim
            cv::rectangle(anhHienThi, cv::Rect(521, 378, 134, 61), cv::Scalar(255, 0, 255), 2);  // Vùng nút Xác nhận cuối cùng
            cv::rectangle(anhHienThi, cv::Rect(32, 382, 40, 40), cv::Scalar(255, 0, 255), 2);   // roi_NgoaiTru
            cv::rectangle(anhHienThi, cv::Rect(397, 21, 226, 55), cv::Scalar(255, 0, 255), 2);    //Vùng Lọc Nông Sản
            


            // roi bang thu trai màu xanh lá cây

            
            cv::rectangle(anhHienThi, cv::Rect(32, 111, 110, 322), cv::Scalar(0, 255, 0), 2);  
            cv::rectangle(anhHienThi, cv::Rect(207, 111, 110, 322), cv::Scalar(0, 255, 0), 2);   // ROI ten hat + x = 175
            cv::rectangle(anhHienThi, cv::Rect(382, 111, 110, 322), cv::Scalar(0, 255, 0), 2);
            cv::rectangle(anhHienThi, cv::Rect(557, 111, 110, 322), cv::Scalar(0, 255, 0), 2);
            cv::rectangle(anhHienThi, cv::Rect(732, 111, 110, 322), cv::Scalar(0, 255, 0), 2);


            // ROI của Mua hạt  màu xanh nhạt  
            cv::rectangle(anhHienThi, cv::Rect(819, 27, 63, 56), cv::Scalar(139, 0, 0), 2);

            cv::rectangle(anhHienThi, cv::Rect(642, 222, 90, 43), cv::Scalar(255, 255, 88), 2); // nút vào chỗ mua hạt 
            cv::rectangle(anhHienThi, cv::Rect(381, 167, 210, 71), cv::Scalar(255, 255, 88), 2);// NPC mua hạt

            // ROI Túi đầy
            cv::rectangle(anhHienThi, cv::Rect(340, 93, 287, 71), cv::Scalar(0, 0, 255), 2);


            // roi tìm tele
            cv::rectangle(anhHienThi, cv::Rect(128, 67, 491, 350), cv::Scalar(0, 0, 255), 2);

            cv::imshow("MAT BOT - " + thongTin->tenTab, anhHienThi);
            cv::waitKey(1);
        }
        else {
            // Nếu người dùng bỏ tích "Hien Mat Bot", ta phải dọn dẹp cửa sổ đó đi
            string nameWindow = "MAT BOT - " + thongTin->tenTab;
            // Kiểm tra xem cửa sổ có đang mở không trước khi xóa để tránh lỗi
            if (cv::getWindowProperty(nameWindow, cv::WND_PROP_VISIBLE) >= 1) {
                cv::destroyWindow(nameWindow);
            }
        }

        // --- BƯỚC 1: QUÉT TÚI ĐẦY ---
        cv::Rect roiCheckTui(340, 93, 287, 71);
        cv::Rect safeRoi = roiCheckTui & cv::Rect(0, 0, anhChup.cols, anhChup.rows);
        bool tuiDay = false;
        if (safeRoi.width > 0 && safeRoi.height > 0) {
            cv::Mat vungSoi = anhChup(safeRoi);
            cv::Point p = BoDieuKhien::TimAnhTrongVung(vungSoi, "tui_day.png", cv::Rect(0, 0, vungSoi.cols, vungSoi.rows), 0.7);
            if (p.x != -1) tuiDay = true;
        }

        // --- BƯỚC 2: LOGIC ƯU TIÊN BÁN ---
        if (thongTin->kichHoatBan && tuiDay) {
            if (thongTin->trangThaiHienTai != STATE_SELLING) {
                thongTin->thongBaoStatus = "Tui day! Dang di ban...";
                thongTin->trangThaiHienTai = STATE_SELLING;

                ThucHienDiBan(thongTin);

                // Wait for the updated farm inventory.


                thongTin->thongBaoStatus = "Nghi 5s doi game cap nhat...";
                this_thread::sleep_for(chrono::seconds(5));
                thongTin->trangThaiHienTai = STATE_IDLE;
                continue;
            }
        }

        // Plant existing selected seeds before optional harvesting/shop travel.
        // A full bag still gets selling priority above; START resets this timer.
        if (thongTin->kichHoatTrongCay && GetTickCount64() >= thongTin->henTrongCay) {
            FarmPlantSelected(thongTin);
            if(!thongTin->dangChay)break;
        }
        if (thongTin->kichHoatThuHoachNhanh) {
            ThuHoachTenTim(thongTin);
            if(!PlantWait(thongTin,10000))break;
        }

        // --- BƯỚC 3: KIỂM TRA GIỜ MUA HÀNG ---
        // --- BƯỚC 3: KIỂM TRA GIỜ MUA HÀNG (TÁCH BIỆT ĐỘC LẬP) ---
        auto bayGio = chrono::steady_clock::now();

        // 3.1. LOGIC MUA HẠT
        if (thongTin->kichHoatMuaHat) {
            int giayTroiQuaHat = chrono::duration_cast<chrono::seconds>(bayGio - thongTin->thoiGianMuaHatGanNhat).count();
            if (giayTroiQuaHat >= 120) { // 2 phút

                thongTin->thongBaoStatus = "Den gio: Dang di mua hat...";
                thongTin->trangThaiHienTai = STATE_BUYING_SEEDS;

                ThucHienMuaHat(thongTin);
                if (thongTin->dangChay && thongTin->kichHoatTrongCay) FarmPlantSelected(thongTin);

                thongTin->thoiGianMuaHatGanNhat = chrono::steady_clock::now(); // Reset mốc mua hạt

                thongTin->trangThaiHienTai = STATE_IDLE;
                continue;
            }
        }

        // 3.2. LOGIC MUA CÔNG CỤ
        if (thongTin->kichHoatMuaCongCu) {
            int giayTroiQuaCu = chrono::duration_cast<chrono::seconds>(bayGio - thongTin->thoiGianMuaCongCuGanNhat).count();
            if (giayTroiQuaCu >= 300) { // 5 phút

                thongTin->thongBaoStatus = "Den gio: Dang di mua cong cu...";
                thongTin->trangThaiHienTai = STATE_BUYING_TOOLS;

                ThucHienMuaCongCu(thongTin);

                thongTin->thoiGianMuaCongCuGanNhat = chrono::steady_clock::now(); // Reset mốc mua công cụ

                thongTin->trangThaiHienTai = STATE_IDLE;
                continue;
            }
        }

        thongTin->trangThaiHienTai = STATE_IDLE;
        this_thread::sleep_for(chrono::milliseconds(1500));
    }
}
int main(int, char**) {
    // Luon tim anh/tai nguyen canh file exe, ke ca khi mo tu Terminal o thu muc khac.
    char exePath[MAX_PATH] = {};
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) > 0) {
        std::string thuMucExe(exePath);
        size_t viTriGachCuoi = thuMucExe.find_last_of("\\/");
        if (viTriGachCuoi != std::string::npos) {
            SetCurrentDirectoryA(thuMucExe.substr(0, viTriGachCuoi).c_str());
        }
    }

    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, XuLyTinHieuCuaSo, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"FarmerPro", NULL };
    ::RegisterClassExW(&wc);

    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"BOT FARMER X DLee v0.1", WS_OVERLAPPEDWINDOW, 100, 100, 620, 820, NULL, NULL, wc.hInstance, NULL);

    if (!KhoiTaoThietBiD3D(hwnd)) { DonDepThietBiD3D(); return 1; }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(thietBiD3D, boDieuKhienD3D);

    while (true) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg); ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) goto cleanup;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // --- BẮT ĐẦU GIAO DIỆN CHÍNH ---
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGui::Begin("BOT FARMER X DLee v0.1", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // 1. NÚT QUÉT TỔNG LỰC
        if (ImGui::Button("QUET TAT CA LDPLAYER", ImVec2(-1, 35))) {
            for (auto t : danhSachTabs) t->dangChay = false;
            this_thread::sleep_for(chrono::milliseconds(150));
            for (auto t : danhSachTabs) t->dangChay = false;
            danhSachTabs.clear();

            HWND hwndLD = NULL;
            while ((hwndLD = FindWindowExA(NULL, hwndLD, "LDPlayerMainFrame", NULL)) != NULL) {
                ThongTinTool* tabMoi = new ThongTinTool();
                tabMoi->h_cha = hwndLD;
                tabMoi->h_game = FindWindowExA(hwndLD, NULL, "RenderWindow", NULL);
                char title[256]; GetWindowTextA(hwndLD, title, 256);
                tabMoi->tenTab = string(title);
                FarmLoadPreferences(*tabMoi);
                tabMoi->thongBaoStatus = "San sang";
                danhSachTabs.push_back(tabMoi);
            }
        }
        ImGui::Separator();

        // --- THANH TAB CHÍNH TỔNG QUAN ---
        if (ImGui::BeginTabBar("MainSystemTabs")) {

            // TAB 1: QUẢN LÝ BOT (DASHBOARD)
            if (ImGui::BeginTabItem("Dashboard")) {
                if (!danhSachTabs.empty()) {
                    if (ImGui::BeginTabBar("ManagerTabs")) {
                        for (int i = 0; i < (int)danhSachTabs.size(); i++) {
                            ThongTinTool* tab = danhSachTabs[i];
                            if (ImGui::BeginTabItem(tab->tenTab.c_str())) {
                                tabDangChon = i;
                                bool luaChonDaDoi = false;
                                if (ImGui::Button("LUU LUA CHON", ImVec2(140, 25))) FarmSavePreferences(*tab);
                                ImGui::SameLine();
                                ImGui::TextWrapped("%s", tab->thongBaoLuu.c_str());
                                ImGui::TextColored(ImVec4(0, 1, 1, 1), "STATUS: %s", tab->thongBaoStatus.c_str());

                                if (!tab->dangChay) {
                                    if (ImGui::Button("START BOT", ImVec2(-1, 40))) {
                                        if (tab->h_game) { tab->dangChay = true; thread(LuongTuDongFarm, tab).detach(); }
                                        else { tab->thongBaoStatus = "Loi: Khong thay Render!"; }
                                    }
                                }
                                else {
                                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                                    if (ImGui::Button("STOP BOT", ImVec2(-1, 40))) tab->dangChay = false;
                                    ImGui::PopStyleColor();
                                }
                                ImGui::Separator();
                                // --- CẤU HÌNH FARM ---
                                luaChonDaDoi |= ImGui::Checkbox("Auto BAN (sau thu hoach / khi day tui)", &tab->kichHoatBan);
                                luaChonDaDoi |= ImGui::Checkbox("Auto THU HOACH (TEN TIM / thuong)", &tab->kichHoatThuHoachNhanh);

                                if (tab->kichHoatBan || tab->kichHoatThuHoachNhanh) {
                                    // --- CHỌN CHẾ ĐỘ THU HOẠCH TEN TIM ---
                                    ImGui::Text("Che do thu hoach:");
                                    luaChonDaDoi |= ImGui::Checkbox("Thu hoach bang TEN TIM (dong loat)", &tab->thuHoachBangTenTim);
                                    ImGui::TextWrapped(tab->thuHoachBangTenTim
                                        ? "TEN TIM: bam dong loat va xac nhan; khong tu chuyen thu tung trai."
                                        : "Thu tung trai: bo chon TEN TIM de dung cac nut thu hoach thuong.");
                                    luaChonDaDoi |= ImGui::RadioButton("Chon trai", &tab->modeThuHoachTenTim, CHE_DO_CHON_HAT);
                                    ImGui::SameLine();
                                    luaChonDaDoi |= ImGui::RadioButton("Hai ALL", &tab->modeThuHoachTenTim, CHE_DO_HAI_ALL);
                                    ImGui::SameLine();
                                    luaChonDaDoi |= ImGui::RadioButton("Hai loc bo bien the", &tab->modeThuHoachTenTim, CHE_DO_HAI_LOC_BIEN_THE);

                                    // Chi che do CHON HAT moi can danh sach tick tung loai hat.
                                    // HAI ALL va HAI LOC BIEN THE khong can chon tung loai.
                                    if (tab->modeThuHoachTenTim == CHE_DO_CHON_HAT) {
                                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
                                        if (ImGui::CollapsingHeader(" DANH SACH QUA CAN THAO TAC")) {
                                            ImGui::PopStyleColor();
                                            if (ImGui::BeginChild("ScrollRegion", ImVec2(0, 180), true)) {
                                                ImGui::Columns(2, "ListQuaColumns");
                                                for (int j = 0; j < SO_NONG_SAN; j++) {
                                                    luaChonDaDoi |= ImGui::Checkbox(ds_hat_gionghai[j], &tab->cacHatDuocChon[j]);
                                                    ImGui::NextColumn();
                                                }
                                                ImGui::Columns(1);
                                            }
                                            ImGui::EndChild(); // LUÔN gọi, dù BeginChild tra ve true hay false
                                        }
                                        else { ImGui::PopStyleColor(); }
                                    }
                                    else if (tab->modeThuHoachTenTim == CHE_DO_HAI_ALL) {
                                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Che do HAI ALL: hai het, ke ca bien the.");
                                    }
                                    else {
                                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Che do LOC BIEN THE: hai het, tru bien the.");
                                    }
                                }
                                ImGui::Separator();

                                // --- TRONG CAY: danh sach rieng, mac dinh trong ---
                                ImGui::BeginDisabled(tab->dangChay);
                                luaChonDaDoi |= ImGui::Checkbox("Auto TRONG CAY (hat trong balo)", &tab->kichHoatTrongCay);
                                if (tab->kichHoatTrongCay) {
                                    ImGui::TextWrapped("Chi trong hat da chon. Khong co trong balo: bo qua. Mua xong hat trong danh sach mua thi trong tiep; khong tu mua loai khac.");
                                    ImGui::TextWrapped("Quet ten, so luong va vi tri hat mot luot. Trong hat dang cam truoc, sau do theo vi tri balo; doi hat khong tim lai toan bo.");
                                    if(ImGui::Button("QUET LAI HAT TRONG BALO",ImVec2(-1,25))) {
                                        tab->dangChay=true;
                                        std::thread([tab](){
                                            tab->hatTrongDaNho.Invalidate();
                                            bool ready=PlantEnsureInventory(tab);
                                            PlantCloseBag(tab);
                                            if(ready) {
                                                int found=0;for(const auto& seed:tab->hatTrongDaNho.snapshot.seeds)found+=seed.present;
                                                tab->thongBaoStatus="Da nho balo: "+std::to_string(found)+" loai hat; cac hat khac duoc nho la thieu";
                                            } else tab->thongBaoStatus="Chua quet duoc balo; dong hoi thoai/menu game roi thu lai";
                                            tab->dangChay=false;
                                        }).detach();
                                    }
                                    ImGui::Text("Balo: %s | So luot quet: %u",tab->hatTrongDaNho.valid?"da nho":"chua quet",tab->hatTrongDaNho.scans.load());
                                    std::string khoHat,luotTrong;
                                    {std::lock_guard<std::mutex> lock(tab->trongThongTinMutex);khoHat=tab->ketQuaBalo;luotTrong=tab->ketQuaTrong;}
                                    ImGui::TextWrapped("Luot trong gan nhat: %s",luotTrong.c_str());
                                    ImGui::Text("Vuon: %d luong | Kiem tra: %d/%d diem | Da trong: %d",tab->vuonSoLuong.load(),tab->vuonDaKiemTra.load(),tab->vuonSoDiem.load(),tab->vuonDaTrong.load());
                                    if(ImGui::CollapsingHeader("HAT DA NHO TRONG BALO"))ImGui::TextWrapped("%s",khoHat.c_str());
                                    if(ImGui::Button("DAT LAI TIEN DO VUON",ImVec2(-1,25))) {
                                        tab->soDoVuon.Clear();tab->vuonSoLuong=tab->vuonSoDiem=tab->vuonDaKiemTra=tab->vuonDaTrong=0;
                                        tab->thongBaoStatus="Da dat lai tien do; luot trong sau se khao sat vuon moi";
                                    }
                                    if (ImGui::CollapsingHeader(" DANH SACH HAT TRONG", ImGuiTreeNodeFlags_DefaultOpen)) {
                                        if (ImGui::BeginChild("VungHatTrong", ImVec2(0, 160), true)) {
                                            ImGui::Columns(2, "HatTrongCols");
                                            for (int h = 0; h < SO_HAT_TRONG; ++h) {
                                                ImGui::PushID(1000+h);
                                                luaChonDaDoi |= ImGui::Checkbox(ds_hat_trong[h], &tab->cacHatCanTrong[h]);
                                                ImGui::PopID();
                                                ImGui::NextColumn();
                                            }
                                            ImGui::Columns(1);
                                        }
                                        ImGui::EndChild();
                                    }
                                }
                                ImGui::EndDisabled();
                                ImGui::Separator();

                                // --- MUA ĐỒ ---
                                luaChonDaDoi |= ImGui::Checkbox("Auto MUA HAT (Moi 2p)", &tab->kichHoatMuaHat);
                                if (tab->kichHoatMuaHat) {
                                    if (ImGui::CollapsingHeader(" DANH SACH MUA HAT")) {
                                        if (ImGui::BeginChild("VungCuonHat", ImVec2(0, 120), true)) {
                                            ImGui::Columns(2, "MuaHatCols");
                                            for (int k = 0; k < SO_HAT_MUA; k++) { luaChonDaDoi |= ImGui::Checkbox(ds_hat_giongmua[k], &tab->cacHatCanMua[k]); ImGui::NextColumn(); }
                                            ImGui::Columns(1);
                                        }
                                        ImGui::EndChild(); // LUÔN gọi, dù BeginChild tra ve true hay false
                                    }
                                }

                                luaChonDaDoi |= ImGui::Checkbox("Auto MUA CONG CU 5p/lan", &tab->kichHoatMuaCongCu);
                                if (tab->kichHoatMuaCongCu) {
                                    if (ImGui::CollapsingHeader(" DANH SACH MUA CONG CU")) {
                                        if (ImGui::BeginChild("VungCuonCC", ImVec2(0, 100), true)) {
                                            ImGui::Columns(2, "MuaCCCols");
                                            for (int l = 0; l < 8; l++) { luaChonDaDoi |= ImGui::Checkbox(ds_cong_cu[l], &tab->cacCongCuCanMua[l]); ImGui::NextColumn(); }
                                            ImGui::Columns(1);
                                        }
                                        ImGui::EndChild(); // LUÔN gọi, dù BeginChild tra ve true hay false
                                    }
                                }
                                luaChonDaDoi |= ImGui::Checkbox("Show MAT BOT", &tab->hienMatBot);
                                if (luaChonDaDoi) FarmSavePreferences(*tab);
                                ImGui::EndTabItem();
                            }
                        }
                        ImGui::EndTabBar();
                    }
                }
                else {
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Hay bam Quet LDPlayer de bat dau!");
                }
                ImGui::EndTabItem();
            }

            // TAB 2: KIỂM TRA HỆ THỐNG
            if (ImGui::BeginTabItem("He Thong")) {
                if (ImGui::BeginChild("CheckFileLogs")) {
                    ImGui::Columns(2, "LogCols");
                    ImGui::Text("ANH HAT"); ImGui::NextColumn();
                    ImGui::Text("CHU QUA"); ImGui::NextColumn();
                    for (int i = 0; i < SO_NONG_SAN; i++) {
                        if (i >= SO_HAT_MUA) ImGui::Text("[%d] Chi thu hoach", i + 1);
                        else if (FileTonTai(ds_anh_hat[i])) ImGui::Text("[%d] OK", i + 1);
                        else ImGui::TextColored(ImVec4(1, 0, 0, 1), "[%d] MISS!", i + 1);
                        ImGui::NextColumn();
                        if (FileTonTai(ds_anh_chu_qua[i])) ImGui::Text("[%d] OK", i + 1);
                        else ImGui::Text("[%d] OCR ten trai", i + 1);
                        ImGui::NextColumn();
                    }
                    ImGui::Columns(1);
                }
                ImGui::EndChild(); // LUÔN gọi, dù BeginChild tra ve true hay false
                ImGui::EndTabItem();
            }

            // TAB 3: MOD CHỨC NĂNG RIÊNG
            // ====================================================================
// ====================================================================
// KHÚC NÀY NÉM ĐÈ LẠI VÀO TAB 3 TRONG GIAO DIỆN IMGUI (KHÔNG MẤT NÚT CŨ)
// ====================================================================
// TAB 3: MOD CHỨC NĂNG RIÊNG
            // =========================================================================
            // TAB 3: MOD CHỨC NĂNG RIÊNG - NƠI NÂNG CẤP HỆ THỐNG SĂN THỜI TIẾT ĐA LUỒNG 🚀
            // =========================================================================
            if (ImGui::BeginTabItem("MOD CHUC NANG RIENG")) {
                if (!danhSachTabs.empty()) {
                    if (ImGui::BeginTabBar("ModVipTabs")) {
                        for (int i = 0; i < (int)danhSachTabs.size(); i++) {
                            ThongTinTool* tab = danhSachTabs[i];
                            if (ImGui::BeginTabItem(tab->tenTab.c_str())) {
                                tabDangChon = i;
                                ImGui::PushID(i);

                                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "CHE DO: TU DONG DIEU PHOI NANG CAO");
                                ImGui::Separator();

                                // --- BẢNG THEO DÕI TRẠNG THÁI REALTIME (MONITOR) ---
                                ImGui::Text("Log chuoi hoat dong: "); ImGui::SameLine();
                                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", tab->thongBaoStatus.c_str());

                                // Hiển thị chi tiết bộ đếm FSM hữu hạn nếu đang chạy chế độ Thời Tiết
                                if (tab->dangChay && tab->batAutoThoiTiet) {
                                    string textTrangThaiFSM = "Khoi tao luong...";
                                    if (tab->trangThaiHienTai == 0) textTrangThaiFSM = "Quet thoi tiet & Check vi tri map";
                                    else if (tab->trangThaiHienTai == 1) textTrangThaiFSM = "Dang chuyen lien map (Trung chuyen Plaza)";
                                    else if (tab->trangThaiHienTai == 2) textTrangThaiFSM = "Auto path chay ra toa do vung cau";
                                    else if (tab->trangThaiHienTai == 3) textTrangThaiFSM = "Kich hoat bat can (Delay hoan thien 1s)";
                                    else if (tab->trangThaiHienTai == 4) textTrangThaiFSM = "Trong tran cau (Cho tin hieu luong ngam)";

                                    ImGui::Text("May trang thai FSM: "); ImGui::SameLine();
                                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", textTrangThaiFSM.c_str());

                                    // Hiển thị đồng hồ đếm ngược của bộ lọc nhiễu luồng ngầm
                                    int conLai = tab->giayDemNguoc.load();
                                    if (conLai > 0) {
                                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "🔥 LICH TRINH DIEU MAP: Rút can sau %ds", conLai);
                                    }
                                    else {
                                        ImGui::Text("Bo loc luong ngam: "); ImGui::SameLine();
                                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "An toan (Thoi tiet dep)");
                                    }
                                }
                                ImGui::Separator();

                                // --- KHỐI ĐIỀU KHIỂN HOẠT ĐỘNG ---
                                if (!tab->dangChay) {
                                    // Bảng Checkbox cấu hình Săn Thời Tiết đa luồng thông minh
                                    if (ImGui::Checkbox("BAT AUTO SAN THOI TIET & DOI MAP", &tab->batAutoThoiTiet)) FarmSavePreferences(*tab);

                                    if (tab->batAutoThoiTiet) {
                                        const char* mangTenMaps[] = { "Bo Qua Loc Map", "Plaza (1)", "Khu Nghi Duong (3)", "Khu Trung Tam (4)", "Nha Rieng (5)" };
                                        int mangGiaTriMaps[] = { -1, 1, 3, 4, 5 };
                                        int mapHienTaiIdx = 0;

                                        for (int m = 0; m < 5; m++) {
                                            if (tab->mapMuonSan == mangGiaTriMaps[m]) { mapHienTaiIdx = m; break; }
                                        }

                                        ImGui::SetNextItemWidth(220);
                                        if (ImGui::Combo("Ban do muon san", &mapHienTaiIdx, mangTenMaps, 5)) {
                                            tab->mapMuonSan = mangGiaTriMaps[mapHienTaiIdx];
                                            tab->luaChonMapGoc = tab->mapMuonSan;
                                            FarmSavePreferences(*tab);
                                        }
                                        ImGui::Spacing();

                                        // Nút kích hoạt băm luồng chính và luồng ngầm chạy song song
                                        if (ImGui::Button("START BOT SAN THOI TIET", ImVec2(-1, 40))) {
                                            if (tab->h_game) {
                                                tab->dangChay = true;
                                                tab->thongBaoStatus = "Dang khoi tao luong quan ly thoi tiet...";
                                                std::thread(LuongNgamCanhGioiThoiTiet, tab).detach();
                                                std::thread(AutoSanThoiTietCauCa, tab).detach();
                                            }
                                            else {
                                                tab->thongBaoStatus = "Loi: Khong tim thay handle RenderWindow!";
                                            }
                                        }
                                    }
                                    else {
                                        // 1. NÚT CHẠY LUỒNG TELEPORT TRUYỀN THỐNG CỦA ÔNG
                                        if (ImGui::Button("START TELEPORT", ImVec2(-1, 40))) {
                                            if (tab->h_game) {
                                                tab->dangChay = true;
                                                tab->thongBaoStatus = "Dang chay MOD TELEPORT...";
                                                std::thread(LuongTeleport, tab).detach();
                                            }
                                            else {
                                                tab->thongBaoStatus = "Loi: Khong tim thay handle RenderWindow!";
                                            }
                                        }
                                    }

                                    ImGui::Separator();

                                }
                                else {
                                    // Nếu bot đang chạy (bất kỳ chế độ nào), hiện nút STOP chung để dừng hẳn
                                    if (ImGui::Button("STOP MOD", ImVec2(-1, 45))) {
                                        tab->dangChay = false;
                                        tab->thongBaoStatus = "Da dung mod.";
                                    }
                                }
                                ImGui::PopID();
                                ImGui::EndTabItem();
                            }
                        }
                        ImGui::EndTabBar();
                    }
                }
                else {
                    ImGui::Text("Hay quet LDPlayer!");
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar(); // Đóng MainSystemTabs
        }
        ImGui::End(); // Đóng Dashboard Window

        // --- PHẦN RENDER PHẢI NẰM ĐỘC LẬP NGOÀI MỌI KHỐI IF CỦA IMGUI ---
        ImGui::Render();
        const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        boDieuKhienD3D->OMSetRenderTargets(1, &hinhAnhDich, NULL);
        boDieuKhienD3D->ClearRenderTargetView(hinhAnhDich, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        chuoiTraoDoi->Present(1, 0);
    }

cleanup:
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    DonDepThietBiD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}


    // --- DIRECTX HELPERS ---
    bool KhoiTaoThietBiD3D(HWND hWnd) {
        DXGI_SWAP_CHAIN_DESC sd; ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2; sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1; sd.Windowed = TRUE; sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        D3D_FEATURE_LEVEL fl;
        if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &sd, &chuoiTraoDoi, &thietBiD3D, &fl, &boDieuKhienD3D) != S_OK) return false;
        TaoHinhAnhDich(); return true;
    }
    void TaoHinhAnhDich() { ID3D11Texture2D* p; chuoiTraoDoi->GetBuffer(0, IID_PPV_ARGS(&p)); thietBiD3D->CreateRenderTargetView(p, NULL, &hinhAnhDich); p->Release(); }
    void DonDepHinhAnhDich() { if (hinhAnhDich) { hinhAnhDich->Release(); hinhAnhDich = NULL; } }
    void DonDepThietBiD3D() { DonDepHinhAnhDich(); if (chuoiTraoDoi) chuoiTraoDoi->Release(); if (boDieuKhienD3D) boDieuKhienD3D->Release(); if (thietBiD3D) thietBiD3D->Release(); }
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WINAPI XuLyTinHieuCuaSo(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
        switch (msg) {
        case WM_SIZE: if (thietBiD3D != NULL && wParam != SIZE_MINIMIZED) { DonDepHinhAnhDich(); chuoiTraoDoi->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0); TaoHinhAnhDich(); } return 0;
        case WM_DESTROY: ::PostQuitMessage(0); return 0;
        }
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }
