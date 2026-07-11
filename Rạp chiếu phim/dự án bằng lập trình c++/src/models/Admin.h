#pragma once
#ifndef ADMIN_H
#define ADMIN_H

#include "User.h"
#include <string>
#include <vector>
#include <map>

// ============================================================
// Lớp Admin - Quản trị viên (Kế thừa từ User)
// Có quyền quản lý phim, lịch chiếu, và xem báo cáo
// ============================================================

// Enum định nghĩa các mức quyền hạn
enum class QuyenHan {
    STAFF       = 1,  // Nhân viên: xem dữ liệu, hỗ trợ khách
    MANAGER     = 2,  // Quản lý: thêm/sửa phim, quản lý lịch chiếu
    SUPER_ADMIN = 3   // Siêu quản trị: toàn quyền, xóa dữ liệu
};

class Admin : public User {
private:
    QuyenHan    quyenHan;       // Mức quyền hạn của tài khoản này
    std::string phongBan;       // Phòng ban đảm nhiệm
    std::string maNhanVien;     // Mã nhân viên nội bộ

public:
    // ----- Constructor & Destructor -----
    Admin();
    Admin(int id, const std::string& hoTen, const std::string& email,
          const std::string& soDienThoai, const std::string& matKhau,
          QuyenHan quyenHan = QuyenHan::STAFF,
          const std::string& phongBan = "Vận hành");
    virtual ~Admin();

    // ----- Getter -----
    QuyenHan    getQuyenHan()    const { return quyenHan; }
    std::string getPhongBan()    const { return phongBan; }
    std::string getMaNhanVien()  const { return maNhanVien; }

    // ----- Setter -----
    void setQuyenHan(QuyenHan qh) { quyenHan = qh; }
    void setPhongBan(const std::string& pb) { phongBan = pb; }
    void setMaNhanVien(const std::string& mnv) { maNhanVien = mnv; }

    /**
     * @brief Kiểm tra admin có đủ quyền thực hiện hành động không
     * @param quyenYeuCau Mức quyền tối thiểu cần có
     * @return true nếu đủ quyền
     */
    bool kiemTraQuyen(QuyenHan quyenYeuCau) const;

    // ============================================================
    // QUẢN LÝ PHIM (yêu cầu MANAGER trở lên)
    // ============================================================

    /**
     * @brief Thêm phim mới vào hệ thống
     * @param tenPhim Tên bộ phim
     * @param theLoai Thể loại (Hành động, Tình cảm, Kinh dị...)
     * @param thoiLuong Thời lượng tính bằng phút
     * @param moTa Mô tả nội dung phim
     * @param gioiHanTuoi Giới hạn độ tuổi (0, 13, 16, 18)
     * @param ngayKhoiChieu Ngày bắt đầu chiếu (YYYY-MM-DD)
     * @return Mã phim mới tạo
     */
    std::string themPhim(const std::string& tenPhim,
                         const std::string& theLoai,
                         int thoiLuong,
                         const std::string& moTa,
                         int gioiHanTuoi,
                         const std::string& ngayKhoiChieu);

    /**
     * @brief Sửa thông tin phim đã có
     * @param maPhim Mã phim cần sửa
     * @param truong Trường cần sửa ("tenPhim", "moTa", "theLoai"...)
     * @param giaTri Giá trị mới
     * @return true nếu sửa thành công
     */
    bool suaPhim(const std::string& maPhim,
                 const std::string& truong,
                 const std::string& giaTri);

    /**
     * @brief Xóa phim khỏi hệ thống (chỉ SUPER_ADMIN)
     * Không thể xóa phim đang có lịch chiếu trong tương lai
     * @param maPhim Mã phim cần xóa
     * @return true nếu xóa thành công
     */
    bool xoaPhim(const std::string& maPhim);

    // ============================================================
    // QUẢN LÝ LỊCH CHIẾU
    // ============================================================

    /**
     * @brief Thêm suất chiếu mới
     * @param maPhim Mã phim
     * @param maPhong Mã phòng chiếu
     * @param thoiGianBatDau Giờ bắt đầu (YYYY-MM-DD HH:MM)
     * @param giaVeCoSo Giá vé cơ sở (đồng)
     * @return Mã suất chiếu mới
     */
    std::string themLichChieu(const std::string& maPhim,
                               const std::string& maPhong,
                               const std::string& thoiGianBatDau,
                               double giaVeCoSo);

    /**
     * @brief Sửa thông tin suất chiếu
     * @param maSuatChieu Mã suất chiếu cần sửa
     * @param truong Trường cần sửa
     * @param giaTri Giá trị mới
     * @return true nếu sửa thành công
     */
    bool suaLichChieu(const std::string& maSuatChieu,
                      const std::string& truong,
                      const std::string& giaTri);

    /**
     * @brief Hủy suất chiếu (và hoàn tiền cho tất cả khách đã đặt)
     */
    bool huyLichChieu(const std::string& maSuatChieu);

    // ============================================================
    // THỐNG KÊ DOANH THU
    // ============================================================

    /**
     * @brief Xem báo cáo doanh thu theo ngày
     * @param ngay Ngày cần xem (YYYY-MM-DD)
     */
    void thongKeDoanhThuTheoNgay(const std::string& ngay) const;

    /**
     * @brief Xem báo cáo doanh thu theo tháng
     * @param thang Tháng (1-12)
     * @param nam Năm (YYYY)
     */
    void thongKeDoanhThuTheoThang(int thang, int nam) const;

    /**
     * @brief Xem báo cáo doanh thu theo phim
     * @param maPhim Mã phim (để trống = xem tất cả)
     */
    void thongKeDoanhThuTheoPhim(const std::string& maPhim = "") const;

    /**
     * @brief Xem tổng quan doanh thu (Dashboard)
     */
    void thongKeTongQuan() const;

    /**
     * @brief Xem danh sách tất cả người dùng (chỉ SUPER_ADMIN)
     */
    void xemDanhSachNguoiDung() const;

    /**
     * @brief Khóa/mở khóa tài khoản người dùng
     */
    bool khoaTaiKhoan(int userId);

    /**
     * @brief Hiển thị thông tin admin (override)
     */
    void hienThiThongTin() const override;

    // Override phương thức thuần ảo
    std::string loaiNguoiDung() const override { return "Quản trị viên"; }

    // ----- Hàm hỗ trợ -----
    static std::string quyenHanToString(QuyenHan quyen);
};

#endif // ADMIN_H
