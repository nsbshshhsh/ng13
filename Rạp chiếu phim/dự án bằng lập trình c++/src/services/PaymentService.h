#pragma once
#ifndef PAYMENTSERVICE_H
#define PAYMENTSERVICE_H

#include "../models/Booking.h"
#include <string>
#include <iostream>

// ============================================================
// PaymentService - Dịch vụ thanh toán và hoàn tiền
// Xử lý: thanh toán, sinh QR, hoàn tiền, lịch sử giao dịch
// ============================================================
class PaymentService {
public:
    PaymentService();
    ~PaymentService();

    /**
     * @brief Xử lý thanh toán cho đơn hàng
     * Kết nối cổng thanh toán (MoMo/VNPay/ZaloPay)
     * @param booking Đơn hàng cần thanh toán
     * @param pttt Phương thức thanh toán
     * @return true nếu thanh toán thành công
     */
    bool xuLyThanhToan(Booking& booking, PhuongThucThanhToan pttt);

    /**
     * @brief Tạo mã QR thanh toán VNPay/MoMo
     * @param soTien Số tiền cần thanh toán
     * @param maDon Mã đơn hàng
     * @return Chuỗi URL hoặc payload mã QR
     */
    std::string taoMaQRThanhToan(double soTien, const std::string& maDon);

    /**
     * @brief Xử lý hoàn tiền cho khách khi hủy vé
     * Tự động tính % hoàn dựa trên thời gian còn lại trước chiếu
     *
     * Chính sách:
     *   Hủy trước 24h → Hoàn 100%
     *   Hủy trong 1h-24h → Hoàn 50%
     *   Hủy < 1h trước chiếu → Không hoàn
     *
     * @param booking Đơn hàng cần hoàn tiền
     * @param gioTruocChieu Số giờ còn lại trước suất chiếu
     * @return Số tiền đã hoàn (đồng)
     */
    double xuLyHoanTien(Booking& booking, double gioTruocChieu);

    /**
     * @brief Xác nhận giao dịch thanh toán từ cổng trả về
     * @param maGiaoDich Mã giao dịch từ payment gateway
     * @return true nếu giao dịch hợp lệ
     */
    bool xacNhanThanhToan(const std::string& maGiaoDich);

    /**
     * @brief Kiểm tra trạng thái thanh toán
     * @param maDon Mã đơn cần kiểm tra
     * @return Chuỗi mô tả trạng thái
     */
    std::string kiemTraTrangThai(const std::string& maDon);

    /**
     * @brief Hiển thị thông tin QR Code ASCII art
     * @param noidungQR Nội dung mã QR
     */
    void hienThiQRCode(const std::string& noidungQR) const;

    /**
     * @brief Tính phí dịch vụ dựa trên phương thức thanh toán
     * Tiền mặt: 0% | Thẻ: 1.5% | Ví điện tử: 0%
     * @param soTien Số tiền gốc
     * @param pttt Phương thức thanh toán
     * @return Phí dịch vụ (đồng)
     */
    double tinhPhiDichVu(double soTien, PhuongThucThanhToan pttt) const;

private:
    /**
     * @brief Ghi log giao dịch
     */
    void ghiLogGiaoDich(const std::string& maDon,
                        double soTien,
                        bool thanhCong,
                        const std::string& ghiChu = "");
};

#endif // PAYMENTSERVICE_H
