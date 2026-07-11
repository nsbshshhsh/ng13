#pragma once
#ifndef ROOM_H
#define ROOM_H

#include "Seat.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

// ============================================================
// Lớp Room - Phòng chiếu phim
// Chứa danh sách ghế và quản lý layout sơ đồ ghế
// ============================================================

// Enum loại phòng chiếu
enum class LoaiPhong {
    HAI_D,      // Phòng 2D thông thường
    BA_D,       // Phòng 3D
    IMAX,       // Phòng IMAX (màn hình lớn, âm thanh Dolby)
    FOUR_DX,    // Phòng 4DX (ghế chuyển động + hiệu ứng)
    PREMIUM,    // Phòng Premium (ghế nằm/recliner)
    DOLBY       // Phòng Dolby Atmos
};

class Room {
private:
    std::string          maPhong;       // Mã phòng (VD: P01, IMAX01)
    std::string          tenPhong;      // Tên phòng (VD: Phòng 1, IMAX Hall)
    LoaiPhong            loaiPhong;     // Loại phòng chiếu
    int                  soHang;        // Số hàng ghế (VD: 10)
    int                  soGheMoiHang;  // Số ghế mỗi hàng (VD: 15)
    int                  tongSoGhe;     // Tổng ghế = soHang * soGheMoiHang
    std::vector<Seat>    danhSachGhe;   // Danh sách tất cả ghế trong phòng
    bool                 dangHoatDong;  // Phòng đang hoạt động hay bảo trì
    std::string          maCinema;      // Thuộc rạp chiếu nào

    // Cấu hình layout: hàng nào là VIP/Couple
    // VD: {"D", "E", "F"} = hàng D, E, F là ghế VIP
    std::vector<char>    hangVIP;
    std::vector<char>    hangCouple;    // Thường là 2 hàng cuối

public:
    // ----- Constructor & Destructor -----
    Room();
    Room(const std::string& maPhong, const std::string& tenPhong,
         LoaiPhong loaiPhong, int soHang, int soGheMoiHang,
         const std::string& maCinema = "RAP001");
    ~Room();

    // ----- Getter -----
    std::string       getMaPhong()      const { return maPhong; }
    std::string       getTenPhong()     const { return tenPhong; }
    LoaiPhong         getLoaiPhong()    const { return loaiPhong; }
    int               getTongSoGhe()    const { return tongSoGhe; }
    int               getSoHang()       const { return soHang; }
    int               getSoGheMoiHang() const { return soGheMoiHang; }
    bool              isDangHoatDong()  const { return dangHoatDong; }
    std::string       getMaCinema()     const { return maCinema; }
    std::vector<Seat>& getDanhSachGhe()        { return danhSachGhe; }
    const std::vector<Seat>& getDanhSachGhe() const { return danhSachGhe; }

    // ----- Setter -----
    void setDangHoatDong(bool status) { dangHoatDong = status; }
    void setMaCinema(const std::string& ma) { maCinema = ma; }

    // ----- Phương thức cốt lõi -----

    /**
     * @brief Khởi tạo sơ đồ ghế tự động theo số hàng và số ghế/hàng
     * Tự động phân loại: 2 hàng cuối = Couple, 3 hàng giữa = VIP, còn lại = Thường
     * @param giaCoSo Giá vé cơ sở để tính giá từng loại ghế
     */
    void khoiTaoGhe(double giaCoSo = 85000.0);

    /**
     * @brief Lấy danh sách các ghế còn trống
     * @return Vector chứa các ghế trống
     */
    std::vector<Seat*> layDanhSachGheTrong();

    /**
     * @brief Đếm số ghế còn trống
     */
    int demGheTrong() const;

    /**
     * @brief Kiểm tra phòng đã đầy chưa
     * @return true nếu không còn ghế trống
     */
    bool kiemTraSucChua() const;

    /**
     * @brief Tìm ghế theo mã ghế
     * @param maGhe Mã ghế cần tìm (VD: "A01")
     * @return Pointer đến ghế, nullptr nếu không tìm thấy
     */
    Seat* timGheBangMa(const std::string& maGhe);

    /**
     * @brief Hiển thị sơ đồ ghế trực quan trong console
     * Màn ảnh ở trên, các hàng ghế phía dưới
     */
    void hienThiSoDo() const;

    /**
     * @brief Đặt nhiều ghế cùng lúc
     * @param danhSachMaGhe Danh sách mã ghế cần đặt
     * @param maDon Mã đơn đặt vé
     * @return true nếu tất cả ghế đặt thành công
     */
    bool datNhieuGhe(const std::vector<std::string>& danhSachMaGhe,
                     const std::string& maDon);

    /**
     * @brief Hủy nhiều ghế cùng lúc
     */
    bool huyNhieuGhe(const std::vector<std::string>& danhSachMaGhe);

    /**
     * @brief Reset tất cả ghế về trạng thái trống (sau mỗi suất chiếu)
     */
    void resetGhe();

    /**
     * @brief Hiển thị thống kê ghế của phòng
     */
    void hienThiThongKe() const;

    // ----- Hàm tiện ích -----
    static std::string loaiPhongToString(LoaiPhong loai);

private:
    // Xác định loại ghế dựa trên vị trí hàng
    LoaiGhe xacDinhLoaiGhe(char hang) const;
};

#endif // ROOM_H
