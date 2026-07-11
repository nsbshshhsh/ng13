#include "MovieSchedule.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <random>

// ============================================================
// MovieSchedule.cpp - Triển khai quản lý lịch chiếu
// ============================================================

MovieSchedule::MovieSchedule() {}
MovieSchedule::~MovieSchedule() {}

// ============================================================
// Đăng ký phim và phòng vào hệ thống lịch chiếu
// ============================================================
void MovieSchedule::themPhimVaoHeThong(Movie* phim) {
    if (phim) danhSachPhim.push_back(phim);
}

void MovieSchedule::themPhongVaoHeThong(Room* phong) {
    if (phong) danhSachPhong.push_back(phong);
}

// ============================================================
// Sinh mã suất chiếu tự động: ST-YYYYMMDD-XXXXX
// ============================================================
std::string MovieSchedule::sinhMaSuatChieu() {
    time_t t = time(nullptr);
    tm* tmInfo = localtime(&t);
    char ngay[9];
    strftime(ngay, sizeof(ngay), "%Y%m%d", tmInfo);
    std::string prefix = "ST-" + std::string(ngay) + "-";

    int maxDem = 0;
    for (const auto& suat : danhSachSuatChieu) {
        if (suat.getMaSuatChieu().find(prefix) == 0) {
            std::string stDem = suat.getMaSuatChieu().substr(prefix.length());
            try {
                int hDem = std::stoi(stDem);
                if (hDem > maxDem) maxDem = hDem;
            } catch(...) {}
        }
    }

    std::ostringstream oss;
    oss << prefix << std::setw(5) << std::setfill('0') << (maxDem + 1);
    return oss.str();
}

// ============================================================
// Tìm phim theo mã (trong danh sách đã đăng ký)
// ============================================================
Movie* MovieSchedule::timPhimBangMa(const std::string& maPhim) {
    for (auto* p : danhSachPhim) {
        if (p && p->getMaPhim() == maPhim) return p;
    }
    return nullptr;
}

// ============================================================
// Tìm phòng theo mã
// ============================================================
Room* MovieSchedule::timPhongBangMa(const std::string& maPhong) {
    for (auto* p : danhSachPhong) {
        if (p && p->getMaPhong() == maPhong) return p;
    }
    return nullptr;
}

// ============================================================
// Kiểm tra trùng lịch phòng chiếu
// Phòng không thể chiếu 2 phim cùng 1 khung giờ
// ============================================================
bool MovieSchedule::kiemTraTrungLich(const std::string& maPhong,
                                      const std::string& thoiGianBD,
                                      int thoiLuongPhut) const {
    // Tính giờ bắt đầu và kết thúc của suất mới (số phút từ 00:00)
    auto parseThoiGian = [](const std::string& tg) -> int {
        if (tg.size() < 16) return -1;
        int gio  = std::stoi(tg.substr(11, 2));
        int phut = std::stoi(tg.substr(14, 2));
        return gio * 60 + phut;
    };

    int batDauMoi   = parseThoiGian(thoiGianBD);
    int ketThucMoi  = batDauMoi + thoiLuongPhut + 30; // +30 phút dọn phòng
    std::string ngayMoi = thoiGianBD.substr(0, 10);

    for (const auto& suat : danhSachSuatChieu) {
        // Chỉ kiểm tra cùng phòng và cùng ngày
        if (suat.getPhongChieu() == nullptr) continue;
        if (suat.getPhongChieu()->getMaPhong() != maPhong) continue;
        if (suat.getNgayChieu() != ngayMoi) continue;
        if (suat.getTrangThai() == TrangThaiSuatChieu::HUY) continue;

        int batDauCu   = parseThoiGian(suat.getThoiGianBatDau());
        int thoiLuongCu = suat.getPhim() ? suat.getPhim()->getThoiLuong() : 120;
        int ketThucCu  = batDauCu + thoiLuongCu + 30;

        // Kiểm tra có giao nhau không (overlap)
        if (batDauMoi < ketThucCu && ketThucMoi > batDauCu) {
            return true;  // Trùng lịch!
        }
    }
    return false;
}

// ============================================================
// THÊM BỘ PHIM VÀO LỊCH CHIẾU (tạo Showtime mới)
// ============================================================
std::string MovieSchedule::themBophimVaoLich(const std::string& maPhim,
                                               const std::string& maPhong,
                                               const std::string& thoiGianBD,
                                               double giaVeCoSo,
                                               const std::string& ngonNgu,
                                               bool laSuatDacBiet) {
    // Bước 1: Tìm phim
    Movie* phim = timPhimBangMa(maPhim);
    if (!phim) {
        std::cout << "❌ Không tìm thấy phim: " << maPhim
                  << "\n   Hãy dùng themPhimVaoHeThong() trước!\n";
        return "";
    }

    // Bước 2: Tìm phòng
    Room* phong = timPhongBangMa(maPhong);
    if (!phong) {
        std::cout << "❌ Không tìm thấy phòng: " << maPhong
                  << "\n   Hãy dùng themPhongVaoHeThong() trước!\n";
        return "";
    }

    // Bước 3: Kiểm tra phim có đang chiếu không
    if (!phim->kiemTraDangChieu()) {
        std::cout << "⚠️  Phim '" << phim->getTenPhim()
                  << "' không nằm trong thời gian công chiếu!\n";
        // Vẫn cho phép thêm (admin có thể muốn lên lịch trước)
    }

    // Bước 4: Kiểm tra trùng lịch phòng
    if (kiemTraTrungLich(maPhong, thoiGianBD, phim->getThoiLuong())) {
        std::cout << "❌ Phòng " << maPhong
                  << " bị TRÙNG LỊCH tại giờ " << thoiGianBD << "!\n";
        return "";
    }

    // Bước 5: Tạo suất chiếu mới
    std::string maSuatMoi = sinhMaSuatChieu();
    Showtime suatMoi(maSuatMoi, phim, phong, thoiGianBD, giaVeCoSo, ngonNgu);
    suatMoi.setLaSuatDacBiet(laSuatDacBiet);

    // Khởi tạo lại ghế phòng với giá mới
    phong->khoiTaoGhe(giaVeCoSo);

    danhSachSuatChieu.push_back(suatMoi);

    std::cout << "✅ Đã thêm suất chiếu vào lịch!\n";
    std::cout << "   Mã suất  : " << maSuatMoi << "\n";
    std::cout << "   Phim     : " << phim->getTenPhim() << "\n";
    std::cout << "   Phòng    : " << phong->getTenPhong() << "\n";
    std::cout << "   Giờ chiếu: " << thoiGianBD << "\n";
    std::cout << "   Ngôn ngữ : " << ngonNgu << "\n";
    if (laSuatDacBiet) std::cout << "   ⭐ Suất đặc biệt\n";

    return maSuatMoi;
}

// ============================================================
// Xóa suất chiếu
// ============================================================
bool MovieSchedule::xoaSuatChieu(const std::string& maSuatChieu) {
    auto it = std::find_if(danhSachSuatChieu.begin(), danhSachSuatChieu.end(),
        [&](const Showtime& s) { return s.getMaSuatChieu() == maSuatChieu; });

    if (it == danhSachSuatChieu.end()) {
        std::cout << "❌ Không tìm thấy suất chiếu: " << maSuatChieu << "\n";
        return false;
    }
    danhSachSuatChieu.erase(it);
    std::cout << "✅ Đã xóa suất chiếu: " << maSuatChieu << "\n";
    return true;
}

// ============================================================
// Tìm suất chiếu theo mã
// ============================================================
Showtime* MovieSchedule::timSuatChieuBangMa(const std::string& maSuatChieu) {
    for (auto& s : danhSachSuatChieu) {
        if (s.getMaSuatChieu() == maSuatChieu) return &s;
    }
    return nullptr;
}

// ============================================================
// Lấy lịch chiếu theo phim
// ============================================================
std::vector<Showtime*> MovieSchedule::layLichChieuTheoPhim(const std::string& maPhim) {
    std::vector<Showtime*> ketQua;
    for (auto& s : danhSachSuatChieu) {
        if (s.getPhim() && s.getPhim()->getMaPhim() == maPhim) {
            ketQua.push_back(&s);
        }
    }
    return ketQua;
}

// ============================================================
// Lấy lịch chiếu theo ngày
// ============================================================
std::vector<Showtime*> MovieSchedule::layLichChieuTheoNgay(const std::string& ngay) {
    std::vector<Showtime*> ketQua;
    for (auto& s : danhSachSuatChieu) {
        if (s.getNgayChieu() == ngay) {
            ketQua.push_back(&s);
        }
    }
    return ketQua;
}

// ============================================================
// Lấy lịch chiếu theo phòng
// ============================================================
std::vector<Showtime*> MovieSchedule::layLichChieuTheoPhong(const std::string& maPhong) {
    std::vector<Showtime*> ketQua;
    for (auto& s : danhSachSuatChieu) {
        if (s.getPhongChieu() && s.getPhongChieu()->getMaPhong() == maPhong) {
            ketQua.push_back(&s);
        }
    }
    return ketQua;
}

// ============================================================
// Lấy tất cả suất đang bán
// ============================================================
std::vector<Showtime*> MovieSchedule::layTatCaSuatDangBan() {
    std::vector<Showtime*> ketQua;
    for (auto& s : danhSachSuatChieu) {
        if (s.coTheDatVe()) {
            ketQua.push_back(&s);
        }
    }
    return ketQua;
}

// ============================================================
// Hiển thị toàn bộ lịch chiếu
// ============================================================
void MovieSchedule::hienThiToanBoLichChieu() const {
    std::cout << "\n📅 TOÀN BỘ LỊCH CHIẾU (" << danhSachSuatChieu.size() << " suất)\n";
    std::cout << std::left
              << std::setw(16) << "Mã suất"
              << std::setw(28) << "Phim"
              << std::setw(12) << "Phòng"
              << std::setw(18) << "Giờ bắt đầu"
              << std::setw(8)  << "Ghế còn"
              << "Trạng thái\n";
    std::cout << std::string(100, '─') << "\n";

    for (const auto& s : danhSachSuatChieu) {
        s.hienThiTomTat();
    }
}

// ============================================================
// Hiển thị lịch chiếu theo ngày
// ============================================================
void MovieSchedule::hienThiLichChieuTheoNgay(const std::string& ngay) const {
    std::cout << "\n📅 LỊCH CHIẾU NGÀY: " << ngay << "\n";
    std::cout << std::string(100, '═') << "\n";

    bool coSuat = false;
    for (const auto& s : danhSachSuatChieu) {
        if (s.getNgayChieu() == ngay) {
            s.hienThiTomTat();
            coSuat = true;
        }
    }
    if (!coSuat) {
        std::cout << "  (Không có suất chiếu nào trong ngày này)\n";
    }
    std::cout << std::string(100, '═') << "\n";
}
