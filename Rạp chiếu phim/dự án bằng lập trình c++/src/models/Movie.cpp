#include "Movie.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

// ============================================================
// Movie.cpp - Triển khai lớp Phim
// ============================================================

// ---- Constructor mặc định ----
Movie::Movie()
    : maPhim(""), tenPhim(""), theLoai(""), thoiLuong(0),
      gioiHanTuoi(0), diemDanhGia(0.0), soLuotDanhGia(0), dangChieu(false) {}

// ---- Constructor có tham số ----
Movie::Movie(const std::string& maPhim, const std::string& tenPhim,
             const std::string& theLoai, int thoiLuong,
             const std::string& ngayKhoiChieu, const std::string& moTa,
             int gioiHanTuoi)
    : maPhim(maPhim), tenPhim(tenPhim), theLoai(theLoai),
      thoiLuong(thoiLuong), ngayKhoiChieu(ngayKhoiChieu), moTa(moTa),
      gioiHanTuoi(gioiHanTuoi), diemDanhGia(0.0), soLuotDanhGia(0),
      dangChieu(true), ngonNgu("Tiếng Anh"), daoDien(""), dienVienChinh(""),
      poster("default_poster.jpg"), trailerUrl("") {}

Movie::~Movie() {}

// ============================================================
// Chuyển thời lượng phút sang "X giờ Y phút"
// ============================================================
std::string Movie::formatThoiLuong() const {
    int gio = thoiLuong / 60;
    int phut = thoiLuong % 60;
    std::ostringstream oss;
    if (gio > 0) oss << gio << " giờ ";
    if (phut > 0) oss << phut << " phút";
    return oss.str();
}

// ============================================================
// Nhãn giới hạn tuổi theo chuẩn Việt Nam
// ============================================================
std::string Movie::getNhanGioiHanTuoi() const {
    switch (gioiHanTuoi) {
        case 0:  return "P  (Mọi lứa tuổi)";
        case 13: return "T13 (Từ 13 tuổi)";
        case 16: return "T16 (Từ 16 tuổi)";
        case 18: return "T18 (Từ 18 tuổi)";
        default: return "Không xác định";
    }
}

// ============================================================
// Kiểm tra phim đang chiếu dựa vào ngày hiện tại
// ============================================================
bool Movie::kiemTraDangChieu() const {
    // Lấy ngày hiện tại theo định dạng YYYY-MM-DD
    time_t t = time(nullptr);
    tm* tmInfo = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", tmInfo);
    std::string ngayHienTai(buf);

    // Kiểm tra ngày hiện tại có trong khoảng chiếu không
    bool sauKhoiChieu = (ngayHienTai >= ngayKhoiChieu);
    bool truocKetThuc = ngayKetThucChieu.empty() || 
                        (ngayHienTai <= ngayKetThucChieu);
    
    return sauKhoiChieu && truocKetThuc && dangChieu;
}

// ============================================================
// Thêm đánh giá mới (cập nhật điểm trung bình)
// ============================================================
void Movie::themDanhGia(double diem) {
    if (diem < 1.0 || diem > 10.0) {
        std::cout << "❌ Điểm đánh giá phải từ 1.0 đến 10.0!\n";
        return;
    }
    // Tính điểm trung bình cộng mới
    diemDanhGia = (diemDanhGia * soLuotDanhGia + diem) / (soLuotDanhGia + 1);
    soLuotDanhGia++;
    std::cout << "✅ Cảm ơn bạn đã đánh giá! Điểm TB hiện tại: "
              << std::fixed << std::setprecision(1) << diemDanhGia << "/10\n";
}

// ============================================================
// Kiểm tra phù hợp độ tuổi
// ============================================================
bool Movie::kiemTraPhuHopTuoi(int tuoiKhachHang) const {
    if (gioiHanTuoi == 0) return true;   // Mọi lứa tuổi
    return tuoiKhachHang >= gioiHanTuoi;
}

// ============================================================
// Hiển thị thông tin đầy đủ của phim
// ============================================================
void Movie::hienThiThongTin() const {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║            THÔNG TIN PHIM                           ║\n";
    std::cout << "╠══════════════════════════════════════════════════════╣\n";
    std::cout << "  🎬 " << tenPhim << " (" << tenPhimGoc << ")\n";
    std::cout << "──────────────────────────────────────────────────────\n";
    std::cout << "  Mã phim     : " << maPhim                << "\n";
    std::cout << "  Thể loại    : " << theLoai               << "\n";
    std::cout << "  Thời lượng  : " << formatThoiLuong()     << "\n";
    std::cout << "  Đạo diễn    : " << daoDien               << "\n";
    std::cout << "  Diễn viên   : " << dienVienChinh         << "\n";
    std::cout << "  Ngôn ngữ    : " << ngonNgu               << "\n";
    std::cout << "  Giới hạn    : " << getNhanGioiHanTuoi()  << "\n";
    std::cout << "  Khởi chiếu  : " << ngayKhoiChieu         << "\n";
    if (!ngayKetThucChieu.empty())
    std::cout << "  Kết thúc    : " << ngayKetThucChieu      << "\n";
    std::cout << "  Đánh giá    : " << std::fixed << std::setprecision(1)
              << diemDanhGia << "/10 (" << soLuotDanhGia << " lượt)\n";
    std::cout << "  Trạng thái  : " << (kiemTraDangChieu() ? "🟢 Đang chiếu" : "🔴 Ngừng chiếu") << "\n";
    std::cout << "──────────────────────────────────────────────────────\n";
    std::cout << "  Nội dung:\n  " << moTa << "\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
}

// ============================================================
// Hiển thị thông tin tóm tắt (dùng trong bảng danh sách)
// ============================================================
void Movie::hienThiTomTat() const {
    std::cout << std::left
              << std::setw(10) << maPhim
              << std::setw(35) << tenPhim
              << std::setw(15) << theLoai
              << std::setw(12) << formatThoiLuong()
              << std::setw(6)  << getNhanGioiHanTuoi().substr(0,3)
              << std::fixed << std::setprecision(1)
              << diemDanhGia << "/10\n";
}
