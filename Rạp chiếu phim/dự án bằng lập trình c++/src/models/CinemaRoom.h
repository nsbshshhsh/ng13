#pragma once
#ifndef CINEMAROOM_H
#define CINEMAROOM_H

#include "Room.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

// ============================================================
// Lớp CinemaRoom - Rạp chiếu phim (Cinema / Multiplex)
// Quản lý toàn bộ rạp: tên, địa chỉ, nhiều phòng chiếu
// Đây là lớp chứa nhiều Room bên trong
// ============================================================
class CinemaRoom {
private:
    std::string        maCinema;      // Mã rạp (VD: CGV_HN01)
    std::string        tenRap;        // Tên rạp (VD: CGV Vincom Bà Triệu)
    std::string        diaChi;        // Địa chỉ đầy đủ
    std::string        thanhPho;      // Thành phố/Tỉnh
    std::string        soDienThoai;   // Hotline rạp
    std::string        email;         // Email liên hệ
    std::string        gioMoCua;      // Giờ mở cửa (VD: "08:00")
    std::string        gioDongCua;    // Giờ đóng cửa (VD: "23:30")
    std::string        thuongHieu;    // Thương hiệu (CGV, Lotte, BHD, Galaxy...)
    bool               dangHoatDong;  // Rạp đang hoạt động hay đóng cửa
    std::vector<Room>  danhSachPhong; // Danh sách các phòng chiếu

public:
    // ----- Constructor & Destructor -----
    CinemaRoom();
    CinemaRoom(const std::string& maCinema,
               const std::string& tenRap,
               const std::string& diaChi,
               const std::string& thanhPho,
               const std::string& thuongHieu = "Generic");
    ~CinemaRoom();

    // ----- Getter -----
    std::string         getMaCinema()     const { return maCinema; }
    std::string         getTenRap()       const { return tenRap; }
    std::string         getDiaChi()       const { return diaChi; }
    std::string         getThanhPho()     const { return thanhPho; }
    std::string         getSoDienThoai()  const { return soDienThoai; }
    std::string         getThuongHieu()   const { return thuongHieu; }
    std::string         getGioMoCua()     const { return gioMoCua; }
    std::string         getGioDongCua()   const { return gioDongCua; }
    bool                isDangHoatDong()  const { return dangHoatDong; }
    int                 getSoPhong()      const { return (int)danhSachPhong.size(); }
    std::vector<Room>&  getDanhSachPhong()       { return danhSachPhong; }
    const std::vector<Room>& getDanhSachPhong() const { return danhSachPhong; }

    // ----- Setter -----
    void setSoDienThoai(const std::string& sdt) { soDienThoai = sdt; }
    void setEmail(const std::string& mail)       { email = mail; }
    void setGioMoCua(const std::string& gio)     { gioMoCua = gio; }
    void setGioDongCua(const std::string& gio)   { gioDongCua = gio; }
    void setDangHoatDong(bool ht)                { dangHoatDong = ht; }

    // ----- Phương thức quản lý phòng -----

    /**
     * @brief Thêm phòng chiếu mới vào rạp
     * @param phong Đối tượng Room cần thêm
     * @return true nếu thêm thành công (mã phòng chưa tồn tại)
     */
    bool themPhong(const Room& phong);

    /**
     * @brief Thêm phòng chiếu với thông số cụ thể (overload tiện lợi)
     * @param maPhong Mã phòng
     * @param tenPhong Tên phòng
     * @param loaiPhong Loại phòng (2D/3D/IMAX...)
     * @param soHang Số hàng ghế
     * @param soGheMoiHang Số ghế mỗi hàng
     * @param giaVeCoSo Giá vé cơ sở để khởi tạo ghế
     * @return true nếu thêm thành công
     */
    bool themPhong(const std::string& maPhong,
                   const std::string& tenPhong,
                   LoaiPhong loaiPhong,
                   int soHang, int soGheMoiHang,
                   double giaVeCoSo = 85000.0);

    /**
     * @brief Xóa phòng chiếu khỏi rạp
     * @param maPhong Mã phòng cần xóa
     * @return true nếu xóa thành công
     */
    bool xoaPhong(const std::string& maPhong);

    /**
     * @brief Tìm phòng theo mã
     * @param maPhong Mã phòng cần tìm
     * @return Pointer đến phòng, nullptr nếu không tìm thấy
     */
    Room* timPhongBangMa(const std::string& maPhong);

    /**
     * @brief Lấy danh sách phòng đang hoạt động
     */
    std::vector<Room*> layPhongHoatDong();

    /**
     * @brief Lấy danh sách phòng theo loại (2D, 3D, IMAX...)
     */
    std::vector<Room*> layPhongTheoLoai(LoaiPhong loai);

    /**
     * @brief Tính tổng sức chứa của toàn bộ rạp
     */
    int tinhTongSucChua() const;

    /**
     * @brief Hiển thị thông tin rạp phim
     */
    void hienThiThongTin() const;

    /**
     * @brief Hiển thị danh sách tất cả phòng
     */
    void hienThiDanhSachPhong() const;

    /**
     * @brief Kiểm tra rạp có phòng loại IMAX không
     */
    bool coPhongIMAX() const;
};

#endif // CINEMAROOM_H
