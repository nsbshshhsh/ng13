#include "CinemaRoom.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

// ============================================================
// CinemaRoom.cpp - Triển khai lớp Rạp chiếu phim
// ============================================================

CinemaRoom::CinemaRoom()
    : maCinema(""), tenRap(""), diaChi(""), thanhPho(""),
      thuongHieu(""), gioMoCua("08:00"), gioDongCua("23:30"),
      dangHoatDong(true) {}

CinemaRoom::CinemaRoom(const std::string& maCinema,
                       const std::string& tenRap,
                       const std::string& diaChi,
                       const std::string& thanhPho,
                       const std::string& thuongHieu)
    : maCinema(maCinema), tenRap(tenRap), diaChi(diaChi),
      thanhPho(thanhPho), thuongHieu(thuongHieu),
      gioMoCua("08:00"), gioDongCua("23:30"), dangHoatDong(true) {}

CinemaRoom::~CinemaRoom() {}

// ============================================================
// Thêm phòng bằng đối tượng Room
// ============================================================
bool CinemaRoom::themPhong(const Room& phong) {
    // Kiểm tra mã phòng đã tồn tại chưa
    for (const auto& p : danhSachPhong) {
        if (p.getMaPhong() == phong.getMaPhong()) {
            std::cout << "❌ Mã phòng " << phong.getMaPhong()
                      << " đã tồn tại trong rạp!\n";
            return false;
        }
    }
    danhSachPhong.push_back(phong);
    std::cout << "✅ Đã thêm phòng " << phong.getTenPhong()
              << " vào rạp " << tenRap << "\n";
    return true;
}

// ============================================================
// Thêm phòng với thông số (overload tiện lợi)
// ============================================================
bool CinemaRoom::themPhong(const std::string& maPhong,
                            const std::string& tenPhong,
                            LoaiPhong loaiPhong,
                            int soHang, int soGheMoiHang,
                            double giaVeCoSo) {
    // Kiểm tra trùng mã
    for (const auto& p : danhSachPhong) {
        if (p.getMaPhong() == maPhong) {
            std::cout << "❌ Mã phòng " << maPhong << " đã tồn tại!\n";
            return false;
        }
    }

    // Tạo phòng mới
    Room phongMoi(maPhong, tenPhong, loaiPhong, soHang, soGheMoiHang, maCinema);
    phongMoi.khoiTaoGhe(giaVeCoSo);  // Khởi tạo ghế tự động
    danhSachPhong.push_back(phongMoi);

    std::cout << "✅ Thêm phòng thành công!\n";
    std::cout << "   Mã phòng  : " << maPhong  << "\n";
    std::cout << "   Tên phòng : " << tenPhong << "\n";
    std::cout << "   Loại      : " << Room::loaiPhongToString(loaiPhong) << "\n";
    std::cout << "   Sức chứa  : " << soHang * soGheMoiHang << " ghế\n";
    return true;
}

// ============================================================
// Xóa phòng
// ============================================================
bool CinemaRoom::xoaPhong(const std::string& maPhong) {
    auto it = std::remove_if(danhSachPhong.begin(), danhSachPhong.end(),
        [&](const Room& p) { return p.getMaPhong() == maPhong; });

    if (it == danhSachPhong.end()) {
        std::cout << "❌ Không tìm thấy phòng: " << maPhong << "\n";
        return false;
    }
    danhSachPhong.erase(it, danhSachPhong.end());
    std::cout << "✅ Đã xóa phòng: " << maPhong << "\n";
    return true;
}

// ============================================================
// Tìm phòng theo mã
// ============================================================
Room* CinemaRoom::timPhongBangMa(const std::string& maPhong) {
    for (auto& p : danhSachPhong) {
        if (p.getMaPhong() == maPhong) return &p;
    }
    return nullptr;
}

// ============================================================
// Lấy danh sách phòng đang hoạt động
// ============================================================
std::vector<Room*> CinemaRoom::layPhongHoatDong() {
    std::vector<Room*> ketQua;
    for (auto& p : danhSachPhong) {
        if (p.isDangHoatDong()) ketQua.push_back(&p);
    }
    return ketQua;
}

// ============================================================
// Lấy phòng theo loại
// ============================================================
std::vector<Room*> CinemaRoom::layPhongTheoLoai(LoaiPhong loai) {
    std::vector<Room*> ketQua;
    for (auto& p : danhSachPhong) {
        if (p.getLoaiPhong() == loai) ketQua.push_back(&p);
    }
    return ketQua;
}

// ============================================================
// Tính tổng sức chứa toàn rạp
// ============================================================
int CinemaRoom::tinhTongSucChua() const {
    int tong = 0;
    for (const auto& p : danhSachPhong) {
        tong += p.getTongSoGhe();
    }
    return tong;
}

// ============================================================
// Kiểm tra có phòng IMAX không
// ============================================================
bool CinemaRoom::coPhongIMAX() const {
    for (const auto& p : danhSachPhong) {
        if (p.getLoaiPhong() == LoaiPhong::IMAX) return true;
    }
    return false;
}

// ============================================================
// Hiển thị thông tin rạp
// ============================================================
void CinemaRoom::hienThiThongTin() const {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║         THÔNG TIN RẠP CHIẾU PHIM            ║\n";
    std::cout << "╠══════════════════════════════════════════════╣\n";
    std::cout << "  🎭 " << tenRap << " [" << thuongHieu << "]\n";
    std::cout << "─────────────────────────────────────────────\n";
    std::cout << "  Mã rạp    : " << maCinema      << "\n";
    std::cout << "  Địa chỉ   : " << diaChi        << "\n";
    std::cout << "  Thành phố : " << thanhPho      << "\n";
    std::cout << "  Hotline   : " << (soDienThoai.empty() ? "Chưa cập nhật" : soDienThoai) << "\n";
    std::cout << "  Giờ mở    : " << gioMoCua << " - " << gioDongCua << "\n";
    std::cout << "  Số phòng  : " << danhSachPhong.size() << " phòng\n";
    std::cout << "  Sức chứa  : " << tinhTongSucChua() << " ghế\n";
    std::cout << "  IMAX      : " << (coPhongIMAX() ? "✅ Có" : "❌ Không") << "\n";
    std::cout << "  Trạng thái: " << (dangHoatDong ? "🟢 Đang hoạt động" : "🔴 Tạm đóng") << "\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";
}

// ============================================================
// Hiển thị danh sách phòng
// ============================================================
void CinemaRoom::hienThiDanhSachPhong() const {
    std::cout << "\n📋 DANH SÁCH PHÒNG - " << tenRap << "\n";
    std::cout << std::left
              << std::setw(8)  << "Mã"
              << std::setw(20) << "Tên phòng"
              << std::setw(18) << "Loại"
              << std::setw(10) << "Sức chứa"
              << std::setw(10) << "Trống"
              << "Trạng thái\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& phong : danhSachPhong) {
        std::cout << std::left
                  << std::setw(8)  << phong.getMaPhong()
                  << std::setw(20) << phong.getTenPhong()
                  << std::setw(18) << Room::loaiPhongToString(phong.getLoaiPhong())
                  << std::setw(10) << phong.getTongSoGhe()
                  << std::setw(10) << phong.demGheTrong()
                  << (phong.isDangHoatDong() ? "🟢 Hoạt động" : "🔴 Bảo trì") << "\n";
    }
}
