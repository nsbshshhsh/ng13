#include "Seat.h"
#include <iostream>
#include <iomanip>
#include <sstream>

// ============================================================
// Seat.cpp - Triển khai lớp Ghế ngồi
// ============================================================

// ---- Hệ số giá theo loại ghế ----
static const double HE_SO_THUONG   = 1.0;
static const double HE_SO_VIP      = 1.5;
static const double HE_SO_COUPLE   = 2.0;
static const double HE_SO_DISABLED = 1.0;

// ---- Constructor mặc định ----
Seat::Seat()
    : maGhe(""), hang('A'), soThuTu(1),
      loaiGhe(LoaiGhe::THUONG), trangThai(TrangThaiGhe::TRONG),
      giaGhe(0.0), maDatGhe("") {}

// ---- Constructor có tham số ----
Seat::Seat(const std::string& maGhe, char hang, int soThuTu,
           LoaiGhe loaiGhe, double giaCoSo)
    : maGhe(maGhe), hang(hang), soThuTu(soThuTu),
      loaiGhe(loaiGhe), trangThai(TrangThaiGhe::TRONG), maDatGhe("") {
    // Tính giá ghế dựa trên loại
    giaGhe = tinhGia(loaiGhe, giaCoSo);
}

Seat::~Seat() {}

// ============================================================
// Tính giá ghế dựa trên loại và giá cơ sở
// ============================================================
double Seat::tinhGia(LoaiGhe loai, double giaCoSo) {
    double heSo = HE_SO_THUONG;
    switch (loai) {
        case LoaiGhe::VIP:      heSo = HE_SO_VIP;      break;
        case LoaiGhe::COUPLE:   heSo = HE_SO_COUPLE;   break;
        case LoaiGhe::DISABLED: heSo = HE_SO_DISABLED; break;
        default: break; // THUONG
    }
    return giaCoSo * heSo;
}

// ============================================================
// Đặt ghế - chuyển sang DA_DAT
// ============================================================
bool Seat::datGhe(const std::string& maDon) {
    if (!coTheDat()) {
        std::cout << "❌ Ghế " << maGhe << " không thể đặt! Trạng thái: "
                  << trangThaiToString(trangThai) << "\n";
        return false;
    }
    trangThai = TrangThaiGhe::DA_DAT;
    maDatGhe  = maDon;
    std::cout << "✅ Đã giữ ghế " << maGhe << " cho đơn " << maDon << "\n";
    return true;
}

// ============================================================
// Hủy đặt ghế - trả về TRONG
// ============================================================
bool Seat::huyDatGhe() {
    if (trangThai == TrangThaiGhe::DA_BAN) {
        std::cout << "❌ Ghế " << maGhe << " đã được bán, không thể hủy!\n";
        return false;
    }
    if (trangThai == TrangThaiGhe::TRONG) {
        std::cout << "⚠️  Ghế " << maGhe << " đã trống rồi!\n";
        return false;
    }
    trangThai = TrangThaiGhe::TRONG;
    maDatGhe  = "";
    std::cout << "↩️  Đã giải phóng ghế " << maGhe << "\n";
    return true;
}

// ============================================================
// Xác nhận bán (sau thanh toán thành công)
// ============================================================
bool Seat::xacNhanBan() {
    if (trangThai != TrangThaiGhe::DA_DAT) {
        std::cout << "❌ Ghế chưa được đặt, không thể xác nhận bán!\n";
        return false;
    }
    trangThai = TrangThaiGhe::DA_BAN;
    return true;
}

// ============================================================
// Khóa và mở khóa ghế
// ============================================================
void Seat::khoaGhe() {
    trangThai = TrangThaiGhe::KHOA;
    std::cout << "🔒 Đã khóa ghế " << maGhe << "\n";
}

void Seat::moKhoaGhe() {
    trangThai = TrangThaiGhe::TRONG;
    std::cout << "🔓 Đã mở khóa ghế " << maGhe << "\n";
}

// ============================================================
// Ký hiệu hiển thị trên sơ đồ ghế trong console
// ============================================================
std::string Seat::kyHieuSoDo() const {
    switch (trangThai) {
        case TrangThaiGhe::TRONG:
            if (loaiGhe == LoaiGhe::VIP)    return "[V]";
            if (loaiGhe == LoaiGhe::COUPLE) return "[♥]";
            return "[_]";
        case TrangThaiGhe::DA_DAT:    return "[~]";  // Đang giữ chỗ
        case TrangThaiGhe::DA_BAN:    return "[X]";  // Đã bán
        case TrangThaiGhe::KHOA:      return "[■]";  // Khóa/hỏng
        case TrangThaiGhe::DANG_CHON: return "[●]";  // Đang chọn
        default: return "[?]";
    }
}

// ============================================================
// Chuyển enum sang chuỗi
// ============================================================
std::string Seat::loaiGheToString(LoaiGhe loai) {
    switch (loai) {
        case LoaiGhe::THUONG:   return "Thường";
        case LoaiGhe::VIP:      return "VIP";
        case LoaiGhe::COUPLE:   return "Couple (Sweetbox)";
        case LoaiGhe::DISABLED: return "Người khuyết tật";
        default:                return "Không xác định";
    }
}

std::string Seat::trangThaiToString(TrangThaiGhe tt) {
    switch (tt) {
        case TrangThaiGhe::TRONG:      return "Trống";
        case TrangThaiGhe::DA_DAT:     return "Đã đặt";
        case TrangThaiGhe::DA_BAN:     return "Đã bán";
        case TrangThaiGhe::KHOA:       return "Khóa";
        case TrangThaiGhe::DANG_CHON:  return "Đang chọn";
        default:                        return "Không xác định";
    }
}

// ============================================================
// Hiển thị thông tin chi tiết ghế
// ============================================================
void Seat::hienThiThongTin() const {
    std::cout << "  Mã ghế   : " << maGhe                          << "\n";
    std::cout << "  Vị trí   : Hàng " << hang << " - Ghế " << soThuTu << "\n";
    std::cout << "  Loại     : " << loaiGheToString(loaiGhe)        << "\n";
    std::cout << "  Trạng thái: " << trangThaiToString(trangThai)   << "\n";
    std::cout << "  Giá      : " << std::fixed << std::setprecision(0)
              << giaGhe << "đ\n";
    if (!maDatGhe.empty())
    std::cout << "  Đơn đặt  : " << maDatGhe                       << "\n";
}
