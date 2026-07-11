#include "Room.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

// ============================================================
// Room.cpp - Triển khai lớp Phòng chiếu
// ============================================================

// ---- Constructor mặc định ----
Room::Room()
    : maPhong(""), tenPhong(""), loaiPhong(LoaiPhong::HAI_D),
      soHang(0), soGheMoiHang(0), tongSoGhe(0),
      dangHoatDong(true), maCinema("") {}

// ---- Constructor có tham số ----
Room::Room(const std::string& maPhong, const std::string& tenPhong,
           LoaiPhong loaiPhong, int soHang, int soGheMoiHang,
           const std::string& maCinema)
    : maPhong(maPhong), tenPhong(tenPhong), loaiPhong(loaiPhong),
      soHang(soHang), soGheMoiHang(soGheMoiHang),
      tongSoGhe(soHang * soGheMoiHang),
      dangHoatDong(true), maCinema(maCinema) {

    // Tự động cấu hình hàng VIP và Couple dựa trên tổng số hàng
    // Ví dụ 10 hàng (A-J):
    //   A, B, C, D  → Thường
    //   E, F, G     → VIP (giữa phòng, view tốt nhất)
    //   H, I        → Thường phía sau
    //   J           → Couple (hàng cuối)

    // 2 hàng cuối là Couple
    for (int i = soHang - 2; i < soHang; ++i) {
        hangCouple.push_back('A' + i);
    }

    // 3 hàng giữa (khoảng 40%-70% từ trên) là VIP
    int batDauVIP = soHang / 3;
    for (int i = batDauVIP; i < batDauVIP + 3 && i < soHang - 2; ++i) {
        hangVIP.push_back('A' + i);
    }
}

Room::~Room() {}

// ============================================================
// Xác định loại ghế dựa trên hàng
// ============================================================
LoaiGhe Room::xacDinhLoaiGhe(char hang) const {
    // Kiểm tra hàng Couple
    for (char h : hangCouple) {
        if (h == hang) return LoaiGhe::COUPLE;
    }
    // Kiểm tra hàng VIP
    for (char h : hangVIP) {
        if (h == hang) return LoaiGhe::VIP;
    }
    return LoaiGhe::THUONG;
}

// ============================================================
// Khởi tạo sơ đồ ghế tự động
// ============================================================
void Room::khoiTaoGhe(double giaCoSo) {
    danhSachGhe.clear();
    danhSachGhe.reserve(tongSoGhe);

    for (int hang = 0; hang < soHang; ++hang) {
        char kyHieuHang = 'A' + hang;
        LoaiGhe loai = xacDinhLoaiGhe(kyHieuHang);

        for (int so = 1; so <= soGheMoiHang; ++so) {
            // Tạo mã ghế: A01, A02, ..., B01, ...
            std::ostringstream oss;
            oss << kyHieuHang << std::setw(2) << std::setfill('0') << so;
            std::string maGhe = oss.str();

            danhSachGhe.emplace_back(maGhe, kyHieuHang, so, loai, giaCoSo);
        }
    }
    std::cout << "✅ Khởi tạo " << danhSachGhe.size() << " ghế cho phòng "
              << tenPhong << "\n";
}

// ============================================================
// Lấy danh sách ghế còn trống
// ============================================================
std::vector<Seat*> Room::layDanhSachGheTrong() {
    std::vector<Seat*> gheTrong;
    for (auto& ghe : danhSachGhe) {
        if (ghe.laTrong()) {
            gheTrong.push_back(&ghe);
        }
    }
    return gheTrong;
}

// ============================================================
// Đếm số ghế còn trống
// ============================================================
int Room::demGheTrong() const {
    int dem = 0;
    for (const auto& ghe : danhSachGhe) {
        if (ghe.laTrong()) ++dem;
    }
    return dem;
}

// ============================================================
// Kiểm tra phòng đã đầy chưa
// ============================================================
bool Room::kiemTraSucChua() const {
    return demGheTrong() == 0;
}

// ============================================================
// Tìm ghế theo mã
// ============================================================
Seat* Room::timGheBangMa(const std::string& maGhe) {
    for (auto& ghe : danhSachGhe) {
        if (ghe.getMaGhe() == maGhe) {
            return &ghe;
        }
    }
    return nullptr;  // Không tìm thấy
}

// ============================================================
// Đặt nhiều ghế cùng lúc (kiểm tra hết rồi mới đặt)
// ============================================================
bool Room::datNhieuGhe(const std::vector<std::string>& danhSachMaGhe,
                       const std::string& maDon) {
    // Bước 1: Kiểm tra TẤT CẢ ghế có trống không
    for (const auto& maGhe : danhSachMaGhe) {
        Seat* ghe = timGheBangMa(maGhe);
        if (!ghe) {
            std::cout << "❌ Không tìm thấy ghế: " << maGhe << "\n";
            return false;
        }
        if (!ghe->coTheDat()) {
            std::cout << "❌ Ghế " << maGhe << " đã được đặt!\n";
            return false;
        }
    }

    // Bước 2: Đặt tất cả ghế
    for (const auto& maGhe : danhSachMaGhe) {
        timGheBangMa(maGhe)->datGhe(maDon);
    }
    return true;
}

// ============================================================
// Hủy nhiều ghế
// ============================================================
bool Room::huyNhieuGhe(const std::vector<std::string>& danhSachMaGhe) {
    for (const auto& maGhe : danhSachMaGhe) {
        Seat* ghe = timGheBangMa(maGhe);
        if (ghe) ghe->huyDatGhe();
    }
    return true;
}

// ============================================================
// Reset tất cả ghế (dùng sau khi suất chiếu kết thúc)
// ============================================================
void Room::resetGhe() {
    for (auto& ghe : danhSachGhe) {
        if (!ghe.laKhoa()) {  // Không reset ghế đang khóa
            ghe.huyDatGhe();
        }
    }
    std::cout << "🔄 Đã reset tất cả ghế trong phòng " << tenPhong << "\n";
}

// ============================================================
// Hiển thị SƠ ĐỒ GHẾ trực quan trong console
// ============================================================
void Room::hienThiSoDo() const {
    int chieuRongManHinh = soGheMoiHang * 4 + 10;

    std::cout << "\n";
    // In tiêu đề phòng
    std::cout << "╔";
    for (int i = 0; i < chieuRongManHinh; ++i) std::cout << "═";
    std::cout << "╗\n";

    std::cout << "║" << std::setw(chieuRongManHinh / 2 + 8)
              << "🎬 MÀN ẢNH PHIM 🎬"
              << std::setw(chieuRongManHinh / 2 - 7) << "" << "║\n";

    std::cout << "╚";
    for (int i = 0; i < chieuRongManHinh; ++i) std::cout << "═";
    std::cout << "╝\n\n";

    // In số ghế header
    std::cout << "     ";
    for (int so = 1; so <= soGheMoiHang; ++so) {
        std::cout << std::setw(4) << so;
    }
    std::cout << "\n";
    std::cout << "     ";
    for (int so = 0; so < soGheMoiHang; ++so) std::cout << "----";
    std::cout << "\n";

    // In từng hàng ghế
    for (int hang = 0; hang < soHang; ++hang) {
        char kyHieuHang = 'A' + hang;
        LoaiGhe loai = xacDinhLoaiGhe(kyHieuHang);

        // Nhãn hàng và loại ghế
        std::cout << " " << kyHieuHang << " ";
        if (loai == LoaiGhe::VIP)    std::cout << "V";
        else if (loai == LoaiGhe::COUPLE) std::cout << "C";
        else std::cout << " ";
        std::cout << " ";

        // In các ghế trong hàng
        for (int so = 1; so <= soGheMoiHang; ++so) {
            // Tìm ghế trong danh sách
            bool timThay = false;
            for (const auto& ghe : danhSachGhe) {
                if (ghe.getHang() == kyHieuHang && ghe.getSoThuTu() == so) {
                    std::cout << ghe.kyHieuSoDo() << " ";
                    timThay = true;
                    break;
                }
            }
            if (!timThay) std::cout << "[?] ";
        }
        std::cout << "  " << kyHieuHang << "\n";
    }

    // Chú thích
    std::cout << "\n  📌 Chú thích: [_]=Trống  [X]=Đã bán  [~]=Đang giữ  ";
    std::cout << "[V]=VIP  [♥]=Couple  [■]=Khóa\n";
    std::cout << "  Ghế trống: " << demGheTrong() << "/" << tongSoGhe << "\n\n";
}

// ============================================================
// Thống kê ghế
// ============================================================
void Room::hienThiThongKe() const {
    int thuong = 0, vip = 0, couple = 0;
    int trongThuong = 0, trongVip = 0, trongCouple = 0;

    for (const auto& ghe : danhSachGhe) {
        switch (ghe.getLoaiGhe()) {
            case LoaiGhe::THUONG:
                thuong++;
                if (ghe.laTrong()) trongThuong++;
                break;
            case LoaiGhe::VIP:
                vip++;
                if (ghe.laTrong()) trongVip++;
                break;
            case LoaiGhe::COUPLE:
                couple++;
                if (ghe.laTrong()) trongCouple++;
                break;
            default: break;
        }
    }

    std::cout << "\n📊 THỐNG KÊ PHÒNG: " << tenPhong << "\n";
    std::cout << "─────────────────────────────────────\n";
    std::cout << "  Thường : " << trongThuong << "/" << thuong << " trống\n";
    std::cout << "  VIP    : " << trongVip    << "/" << vip    << " trống\n";
    std::cout << "  Couple : " << trongCouple << "/" << couple << " trống\n";
    std::cout << "  Tổng   : " << demGheTrong() << "/" << tongSoGhe << " trống\n";
    std::cout << "─────────────────────────────────────\n";
}

std::string Room::loaiPhongToString(LoaiPhong loai) {
    switch (loai) {
        case LoaiPhong::HAI_D:   return "2D";
        case LoaiPhong::BA_D:    return "3D";
        case LoaiPhong::IMAX:    return "IMAX";
        case LoaiPhong::FOUR_DX: return "4DX";
        case LoaiPhong::PREMIUM: return "Premium Recliner";
        case LoaiPhong::DOLBY:   return "Dolby Atmos";
        default:                  return "Không xác định";
    }
}
