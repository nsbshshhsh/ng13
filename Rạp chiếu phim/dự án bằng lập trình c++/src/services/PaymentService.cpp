#include "PaymentService.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <fstream>

// ============================================================
// PaymentService.cpp - Triển khai dịch vụ thanh toán
// ============================================================

PaymentService::PaymentService() {}
PaymentService::~PaymentService() {}

// ============================================================
// Xử lý thanh toán
// ============================================================
bool PaymentService::xuLyThanhToan(Booking& booking,
                                    PhuongThucThanhToan pttt) {
    booking.setPhuongThucThanhToan(pttt);
    double soTien = booking.getSoTienThucTe();

    std::cout << "\n💳 XỬ LÝ THANH TOÁN\n";
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  Mã đơn    : " << booking.getMaDon()     << "\n";
    std::cout << "  Số tiền   : " << std::fixed << std::setprecision(0)
              << soTien << "đ\n";

    // Tính phí dịch vụ
    double phiDV = tinhPhiDichVu(soTien, pttt);
    if (phiDV > 0) {
        std::cout << "  Phí DV    : " << phiDV << "đ (1.5%)\n";
        std::cout << "  Tổng cộng : " << (soTien + phiDV) << "đ\n";
    }

    std::cout << "  Phương thức: " << Booking::ptttToString(pttt) << "\n";

    // Hiển thị QR nếu thanh toán điện tử
    if (pttt == PhuongThucThanhToan::MOMO ||
        pttt == PhuongThucThanhToan::VNPAY ||
        pttt == PhuongThucThanhToan::ZALOPAY) {
        std::string maQR = taoMaQRThanhToan(soTien + phiDV, booking.getMaDon());
        std::cout << "\n📱 QUÉT MÃ QR ĐỂ THANH TOÁN:\n";
        hienThiQRCode(maQR);
        std::cout << "\n  ⏳ Đang chờ xác nhận thanh toán...\n";
    }

    std::cout << "═══════════════════════════════════════\n";

    // Mô phỏng kết quả thanh toán thành công
    bool thanhCong = booking.thanhToan();

    // Ghi log
    ghiLogGiaoDich(booking.getMaDon(), soTien + phiDV, thanhCong);

    return thanhCong;
}

// ============================================================
// Tạo mã QR thanh toán
// ============================================================
std::string PaymentService::taoMaQRThanhToan(double soTien,
                                              const std::string& maDon) {
    // Trong thực tế: gọi API VNPay/MoMo để lấy URL mã QR
    // Demo: tạo chuỗi giả lập
    std::ostringstream oss;
    oss << "https://payment.cinema.vn/qr/"
        << maDon << "/"
        << std::fixed << std::setprecision(0) << soTien
        << "/" << time(nullptr);
    return oss.str();
}

// ============================================================
// Hiển thị QR Code (ASCII art mô phỏng)
// ============================================================
void PaymentService::hienThiQRCode(const std::string& noidungQR) const {
    std::cout << "\n  ┌──────────────────────────────┐\n";
    std::cout << "  │  ▄▄▄▄▄  ▄  ▄▄▄  ▄▄▄▄▄  ▄▄▄ │\n";
    std::cout << "  │  █   █ ▄█▄ ▄██  █   █ ▄██  │\n";
    std::cout << "  │  █▄▄▄█  █  ▄█▄  █▄▄▄█  █▄▄ │\n";
    std::cout << "  │  ▄▄▄▄▄ ▄▄▄ █ █  ▄▄▄▄▄ █    │\n";
    std::cout << "  │  █████ ▄▄▄ ▄█▄  █████ █▄▄  │\n";
    std::cout << "  │  ▄▄▄▄▄  █  ███  ▄▄▄▄▄  █▄▄ │\n";
    std::cout << "  │  █   █ ▄█▄  █   █   █  ▄██ │\n";
    std::cout << "  │  █▄▄▄█  █  ▄█▄  █▄▄▄█ █▄▄▄ │\n";
    std::cout << "  └──────────────────────────────┘\n";
    // Cắt URL hiển thị cho gọn
    std::string hienThi = noidungQR.length() > 40 ?
                           noidungQR.substr(0, 37) + "..." : noidungQR;
    std::cout << "  URL: " << hienThi << "\n";
}

// ============================================================
// XỬ LÝ HOÀN TIỀN - Chính sách rõ ràng
// ============================================================
double PaymentService::xuLyHoanTien(Booking& booking, double gioTruocChieu) {
    std::cout << "\n💰 XỬ LÝ HOÀN TIỀN\n";
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  Mã đơn    : " << booking.getMaDon() << "\n";
    std::cout << "  Số tiền TT: " << std::fixed << std::setprecision(0)
              << booking.getSoTienThucTe() << "đ\n";
    std::cout << "  Giờ trước chiếu: " << gioTruocChieu << "h\n";
    std::cout << "───────────────────────────────────────\n";

    // Tính số tiền hoàn
    double tienHoan = booking.tinhTienHoan(gioTruocChieu);

    if (tienHoan > 0) {
        std::cout << "  ✅ Số tiền hoàn: " << tienHoan << "đ\n";
        std::cout << "  📅 Thời gian hoàn: 3-5 ngày làm việc\n";
        std::cout << "  🏦 Hoàn về: " << Booking::ptttToString(booking.getPhuongThucThanhToan()) << "\n";

        // Ghi log hoàn tiền
        ghiLogGiaoDich(booking.getMaDon() + "_REFUND", -tienHoan, true, "Hoàn tiền hủy vé");
    } else {
        std::cout << "  ❌ Không đủ điều kiện hoàn tiền\n";
        std::cout << "  📋 Lý do: Hủy vé dưới 1 giờ trước giờ chiếu\n";
    }

    std::cout << "═══════════════════════════════════════\n";
    return tienHoan;
}

// ============================================================
// Xác nhận giao dịch (từ webhook payment gateway)
// ============================================================
bool PaymentService::xacNhanThanhToan(const std::string& maGiaoDich) {
    // Trong thực tế: verify chữ ký HMAC từ VNPay/MoMo
    std::cout << "🔍 Xác nhận giao dịch: " << maGiaoDich << "\n";
    // Mô phỏng: luôn thành công
    std::cout << "✅ Giao dịch hợp lệ!\n";
    return true;
}

// ============================================================
// Kiểm tra trạng thái thanh toán
// ============================================================
std::string PaymentService::kiemTraTrangThai(const std::string& maDon) {
    // Trong thực tế: query DB hoặc gọi API payment gateway
    return "COMPLETED";
}

// ============================================================
// Tính phí dịch vụ
// Tiền mặt: 0% | Thẻ: 1.5% | Ví điện tử: 0%
// ============================================================
double PaymentService::tinhPhiDichVu(double soTien,
                                      PhuongThucThanhToan pttt) const {
    switch (pttt) {
        case PhuongThucThanhToan::THE_NGAN_HANG:
            return soTien * 0.015;  // 1.5% phí giao dịch thẻ
        default:
            return 0.0;  // Các phương thức khác miễn phí
    }
}

// ============================================================
// Ghi log giao dịch vào file
// ============================================================
void PaymentService::ghiLogGiaoDich(const std::string& maDon,
                                     double soTien,
                                     bool thanhCong,
                                     const std::string& ghiChu) {
    std::ofstream logFile("data/payment_log.txt", std::ios::app);
    if (!logFile.is_open()) return;

    time_t t = time(nullptr);
    tm* tmInfo = localtime(&t);
    char thoiGian[20];
    strftime(thoiGian, sizeof(thoiGian), "%Y-%m-%d %H:%M:%S", tmInfo);

    logFile << "[" << thoiGian << "] "
            << "MaDon=" << maDon << " | "
            << "SoTien=" << std::fixed << std::setprecision(0) << soTien << "d | "
            << "KetQua=" << (thanhCong ? "THANH_CONG" : "THAT_BAI");
    if (!ghiChu.empty()) logFile << " | GhiChu=" << ghiChu;
    logFile << "\n";

    logFile.close();
}
