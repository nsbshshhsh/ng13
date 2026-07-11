#include "Ticket.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <functional>

// ============================================================
// Ticket.cpp - Triển khai lớp Vé xem phim
// ============================================================

// ---- Constructor mặc định ----
Ticket::Ticket()
    : maVe(""), suatChieu(nullptr), ghe(nullptr),
      giaVe(0.0), trangThai(TrangThaiVe::CHO_THANH_TOAN),
      maQR(""), ngayTao(""), tenKhachHang(""), emailKhachHang(""), maDon("") {}

// ---- Constructor có tham số ----
Ticket::Ticket(const std::string& maVe,
               Showtime* suatChieu,
               Seat* ghe,
               const std::string& tenKhachHang,
               const std::string& emailKhachHang,
               const std::string& maDon)
    : maVe(maVe), suatChieu(suatChieu), ghe(ghe),
      trangThai(TrangThaiVe::CHO_THANH_TOAN),
      tenKhachHang(tenKhachHang), emailKhachHang(emailKhachHang),
      maDon(maDon) {
    // Lưu ngày tạo vé
    time_t t = time(nullptr);
    tm* tmInfo = localtime(&t);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmInfo);
    ngayTao = std::string(buf);

    // Tự tính giá vé
    giaVe = tinhGiaVe();

    // Tạo mã QR
    maQR = taoMaQR();
}

Ticket::~Ticket() {}

// ============================================================
// Tạo mã vé tự động: TK-YYYYMMDD-XXXX
// ============================================================
std::string Ticket::taoMaVeMoi(int soThuTu) {
    time_t t = time(nullptr);
    tm* tmInfo = localtime(&t);
    char ngay[9];
    strftime(ngay, sizeof(ngay), "%Y%m%d", tmInfo);

    std::ostringstream oss;
    oss << "TK-" << ngay << "-"
        << std::setw(4) << std::setfill('0') << soThuTu;
    return oss.str();
}

// ============================================================
// Tính giá vé
// ============================================================
double Ticket::tinhGiaVe() {
    if (!suatChieu || !ghe) return 0.0;
    giaVe = suatChieu->tinhGiaVe(ghe->getLoaiGhe());
    return giaVe;
}

// ============================================================
// Tạo mã QR (mã hóa thông tin vé thành chuỗi unique)
// Trong thực tế sẽ dùng thư viện QR như libqrencode
// ============================================================
std::string Ticket::taoMaQR() {
    // Kết hợp các thông tin quan trọng của vé
    std::string duLieuQR = maVe + "|";
    duLieuQR += (suatChieu ? suatChieu->getMaSuatChieu() : "N/A") + "|";
    duLieuQR += (ghe ? ghe->getMaGhe() : "N/A") + "|";
    duLieuQR += ngayTao + "|";
    duLieuQR += maDon;

    // Tạo hash của dữ liệu (dùng std::hash)
    std::size_t hashVal = std::hash<std::string>{}(duLieuQR);
    std::ostringstream oss;
    oss << "QR-" << std::hex << std::uppercase << std::setw(12)
        << std::setfill('0') << hashVal;
    maQR = oss.str();
    return maQR;
}

// ============================================================
// Chuyển trạng thái sang chuỗi
// ============================================================
std::string Ticket::trangThaiToString(TrangThaiVe tt) {
    switch (tt) {
        case TrangThaiVe::CHO_THANH_TOAN: return "⏳ Chờ thanh toán";
        case TrangThaiVe::DA_THANH_TOAN:  return "✅ Đã thanh toán";
        case TrangThaiVe::DA_SU_DUNG:     return "🎬 Đã sử dụng";
        case TrangThaiVe::DA_HUY:         return "❌ Đã hủy";
        case TrangThaiVe::HET_HAN:        return "⌛ Hết hạn";
        default:                           return "Không xác định";
    }
}

// ============================================================
// Kiểm tra vé có hợp lệ để vào xem
// ============================================================
bool Ticket::laHopLe() const {
    return trangThai == TrangThaiVe::DA_THANH_TOAN;
}

// ============================================================
// Sử dụng vé (quét QR tại rạp)
// ============================================================
bool Ticket::suDungVe() {
    if (!laHopLe()) {
        std::cout << "❌ Vé không hợp lệ! Trạng thái: "
                  << trangThaiToString(trangThai) << "\n";
        return false;
    }
    trangThai = TrangThaiVe::DA_SU_DUNG;
    std::cout << "✅ Quét vé thành công! Chúc xem phim vui vẻ! 🎬\n";
    return true;
}

// ============================================================
// Hủy vé
// ============================================================
bool Ticket::huyVe() {
    if (trangThai == TrangThaiVe::DA_SU_DUNG) {
        std::cout << "❌ Vé đã được sử dụng, không thể hủy!\n";
        return false;
    }
    if (trangThai == TrangThaiVe::DA_HUY) {
        std::cout << "⚠️  Vé này đã được hủy trước đó!\n";
        return false;
    }
    trangThai = TrangThaiVe::DA_HUY;
    // Giải phóng ghế
    if (ghe) ghe->huyDatGhe();
    std::cout << "✅ Đã hủy vé: " << maVe << "\n";
    return true;
}

// ============================================================
// IN VÉ - Mô phỏng vé vật lý/điện tử
// ============================================================
void Ticket::inVe() const {
    std::string tenPhim   = "N/A", thoiGian = "N/A", tenPhong = "N/A";
    std::string maGhe     = "N/A", loaiGhe  = "N/A";

    if (suatChieu) {
        thoiGian = suatChieu->getThoiGianBatDau();
        if (suatChieu->getPhim())
            tenPhim = suatChieu->getPhim()->getTenPhim();
        if (suatChieu->getPhongChieu())
            tenPhong = suatChieu->getPhongChieu()->getTenPhong();
    }
    if (ghe) {
        maGhe   = ghe->getMaGhe();
        loaiGhe = Seat::loaiGheToString(ghe->getLoaiGhe());
    }

    // Mô phỏng thẻ vé đẹp mắt
    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────────┐\n";
    std::cout << "│          🎬  VÉ XEM PHIM ĐIỆN TỬ  🎬        │\n";
    std::cout << "├─────────────────────────────────────────────┤\n";
    std::cout << "│ Mã vé    : " << std::left << std::setw(33) << maVe       << "│\n";
    std::cout << "│ Phim     : " << std::left << std::setw(33) << tenPhim.substr(0, 31) << "│\n";
    std::cout << "│ Giờ chiếu: " << std::left << std::setw(33) << thoiGian   << "│\n";
    std::cout << "│ Phòng    : " << std::left << std::setw(33) << tenPhong   << "│\n";
    std::cout << "│ Ghế      : " << std::left << std::setw(10) << maGhe
              << "(" << std::setw(22) << loaiGhe << ")│\n";
    std::cout << "│ Khách    : " << std::left << std::setw(33) << tenKhachHang.substr(0, 31) << "│\n";
    std::cout << "├─────────────────────────────────────────────┤\n";
    std::cout << "│ Giá vé   : " << std::left << std::setw(33)
              << (std::to_string((int)giaVe) + "đ")  << "│\n";
    std::cout << "│ Trạng thái: " << std::left << std::setw(32)
              << Ticket::trangThaiToString(trangThai)  << "│\n";
    std::cout << "├─────────────────────────────────────────────┤\n";
    std::cout << "│ Mã QR    : " << std::left << std::setw(33) << maQR       << "│\n";
    std::cout << "│ Đặt vé   : " << std::left << std::setw(33) << ngayTao    << "│\n";
    std::cout << "├─────────────────────────────────────────────┤\n";
    std::cout << "│  ██████  ██████  ██████  ██████  ██████     │\n";
    std::cout << "│  ██  ██  ██      ██  ██  ██  ██  ██  ██     │\n";
    std::cout << "│  ██████  ████    ██████  ██████  ██████     │\n";
    std::cout << "│  [Quét mã QR tại quầy rạp để vào xem]      │\n";
    std::cout << "└─────────────────────────────────────────────┘\n";
}

// ============================================================
// In vé ngắn (cho xác nhận email/SMS)
// ============================================================
void Ticket::inVeNgan() const {
    std::string tenPhim = suatChieu && suatChieu->getPhim() ?
                          suatChieu->getPhim()->getTenPhim() : "N/A";
    std::cout << "[" << maVe << "] " << tenPhim
              << " | Ghế: " << (ghe ? ghe->getMaGhe() : "N/A")
              << " | Giá: " << std::fixed << std::setprecision(0) << giaVe << "đ"
              << " | " << trangThaiToString(trangThai) << "\n";
}
