#include "Showtime.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

// ============================================================
// Showtime.cpp - Triển khai lớp Suất chiếu
// ============================================================

// ---- Constructor mặc định ----
Showtime::Showtime()
    : maSuatChieu(""), phim(nullptr), phongChieu(nullptr),
      thoiGianBatDau(""), thoiGianKetThuc(""), ngayChieu(""),
      giaVeCoSo(85000.0), trangThai(TrangThaiSuatChieu::CHUA_MO_BAN),
      laSuatDacBiet(false), ngonNguPhim("Phụ đề Việt"), maCinema("") {}

// ---- Constructor có tham số ----
Showtime::Showtime(const std::string& maSuatChieu,
                   Movie* phim,
                   Room*  phongChieu,
                   const std::string& thoiGianBatDau,
                   double giaVeCoSo,
                   const std::string& ngonNguPhim)
    : maSuatChieu(maSuatChieu), phim(phim), phongChieu(phongChieu),
      thoiGianBatDau(thoiGianBatDau), giaVeCoSo(giaVeCoSo),
      trangThai(TrangThaiSuatChieu::DANG_BAN),
      laSuatDacBiet(false), ngonNguPhim(ngonNguPhim) {
    // Tách ngày từ chuỗi "YYYY-MM-DD HH:MM"
    if (thoiGianBatDau.size() >= 10) {
        ngayChieu = thoiGianBatDau.substr(0, 10);
    }
    // Tự tính giờ kết thúc
    thoiGianKetThuc = tinhGioKetThuc();

    // Cập nhật giá ghế trong phòng theo giá mới
    if (phongChieu) {
        phongChieu->khoiTaoGhe(giaVeCoSo);
    }
}

Showtime::~Showtime() {}

// ============================================================
// Tính giờ kết thúc = giờ bắt đầu + thời lượng phim
// ============================================================
std::string Showtime::tinhGioKetThuc() const {
    if (!phim || thoiGianBatDau.size() < 16) return "";

    // Tách giờ và phút từ "YYYY-MM-DD HH:MM"
    int gio  = std::stoi(thoiGianBatDau.substr(11, 2));
    int phut = std::stoi(thoiGianBatDau.substr(14, 2));

    // Cộng thêm thời lượng phim (+ 15 phút quảng cáo)
    int tongPhut = gio * 60 + phut + phim->getThoiLuong() + 15;

    int gioKetThuc  = (tongPhut / 60) % 24;
    int phutKetThuc = tongPhut % 60;

    std::ostringstream oss;
    oss << thoiGianBatDau.substr(0, 10) << " "
        << std::setw(2) << std::setfill('0') << gioKetThuc << ":"
        << std::setw(2) << std::setfill('0') << phutKetThuc;
    return oss.str();
}

// ============================================================
// Kiểm tra có phải suất cuối ngày (sau 22:00)
// ============================================================
bool Showtime::laSuatCuoiNgay() const {
    if (thoiGianBatDau.size() < 16) return false;
    int gio = std::stoi(thoiGianBatDau.substr(11, 2));
    return gio >= 22;
}

// ============================================================
// Đếm ghế còn trống
// ============================================================
int Showtime::kiemTraGheTrong() const {
    if (!phongChieu) return 0;
    return phongChieu->demGheTrong();
}

// ============================================================
// Kiểm tra có thể đặt vé không
// ============================================================
bool Showtime::coTheDatVe() const {
    return trangThai == TrangThaiSuatChieu::DANG_BAN &&
           kiemTraGheTrong() > 0;
}

// ============================================================
// Tính giá vé cụ thể cho từng loại ghế
// Áp dụng hệ số: ghế + suất đặc biệt
// ============================================================
double Showtime::tinhGiaVe(LoaiGhe loaiGhe) const {
    double gia = Seat::tinhGia(loaiGhe, giaVeCoSo);

    // Suất đặc biệt (premier, đêm khuya): tăng 20%
    if (laSuatDacBiet) gia *= 1.2;

    // Suất cuối ngày (sau 22h): tăng thêm 10%
    if (laSuatCuoiNgay()) gia *= 1.1;

    return gia;
}

// ============================================================
// Chuyển enum trạng thái sang chuỗi
// ============================================================
std::string Showtime::trangThaiToString(TrangThaiSuatChieu tt) {
    switch (tt) {
        case TrangThaiSuatChieu::CHUA_MO_BAN: return "Chưa mở bán";
        case TrangThaiSuatChieu::DANG_BAN:    return "🟢 Đang bán vé";
        case TrangThaiSuatChieu::SAP_CHIEU:   return "⏰ Sắp chiếu";
        case TrangThaiSuatChieu::DANG_CHIEU:  return "🎬 Đang chiếu";
        case TrangThaiSuatChieu::DA_CHIEU:    return "✅ Đã chiếu";
        case TrangThaiSuatChieu::HUY:         return "❌ Đã hủy";
        default:                               return "Không xác định";
    }
}

// ============================================================
// Hiển thị thông tin chi tiết suất chiếu
// ============================================================
void Showtime::hienThiChiTiet() const {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║            CHI TIẾT SUẤT CHIẾU                      ║\n";
    std::cout << "╠══════════════════════════════════════════════════════╣\n";
    std::cout << "  Mã suất    : " << maSuatChieu << "\n";

    if (phim) {
        std::cout << "  🎬 Phim    : " << phim->getTenPhim();
        if (!phim->getTenPhimGoc().empty())
            std::cout << " (" << phim->getTenPhimGoc() << ")";
        std::cout << "\n";
        std::cout << "  Thể loại   : " << phim->getTheLoai() << "\n";
        std::cout << "  Thời lượng : " << phim->formatThoiLuong() << "\n";
        std::cout << "  Giới hạn   : " << phim->getNhanGioiHanTuoi() << "\n";
    }

    std::cout << "──────────────────────────────────────────────────────\n";

    if (phongChieu) {
        std::cout << "  🏢 Phòng   : " << phongChieu->getTenPhong()
                  << " (" << Room::loaiPhongToString(phongChieu->getLoaiPhong()) << ")\n";
    }

    std::cout << "  🕐 Bắt đầu : " << thoiGianBatDau  << "\n";
    std::cout << "  🕐 Kết thúc: " << thoiGianKetThuc << "\n";
    std::cout << "  Ngôn ngữ   : " << ngonNguPhim     << "\n";
    if (laSuatDacBiet)
        std::cout << "  ⭐ Suất đặc biệt (Premier)\n";

    std::cout << "──────────────────────────────────────────────────────\n";
    std::cout << "  Ghế còn    : " << kiemTraGheTrong() << " ghế\n";
    std::cout << "  Giá (Thường): "
              << std::fixed << std::setprecision(0) << tinhGiaVe(LoaiGhe::THUONG) << "đ\n";
    std::cout << "  Giá (VIP)  : "
              << std::fixed << std::setprecision(0) << tinhGiaVe(LoaiGhe::VIP)    << "đ\n";
    std::cout << "  Giá (Couple): "
              << std::fixed << std::setprecision(0) << tinhGiaVe(LoaiGhe::COUPLE) << "đ\n";
    std::cout << "  Trạng thái : " << trangThaiToString(trangThai) << "\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
}

// ============================================================
// Hiển thị tóm tắt (cho bảng danh sách)
// ============================================================
void Showtime::hienThiTomTat() const {
    std::string tenPhim = phim ? phim->getTenPhim() : "N/A";
    std::string tenPhong = phongChieu ? phongChieu->getTenPhong() : "N/A";

    std::cout << std::left
              << std::setw(14) << maSuatChieu
              << std::setw(30) << tenPhim.substr(0, 28)
              << std::setw(12) << tenPhong.substr(0, 10)
              << std::setw(18) << thoiGianBatDau
              << std::setw(6)  << kiemTraGheTrong()
              << std::setw(12) << std::fixed << std::setprecision(0) << giaVeCoSo
              << trangThaiToString(trangThai) << "\n";
}
