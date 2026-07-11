#pragma once
#ifndef MOVIESCHEDULE_H
#define MOVIESCHEDULE_H

#include "Showtime.h"
#include "Movie.h"
#include "Room.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

// ============================================================
// Lớp MovieSchedule - Quản lý lịch chiếu bộ phim
// Lớp trung gian: gán phim + phòng + giờ → tạo suất chiếu
// ============================================================
class MovieSchedule {
private:
    std::vector<Showtime>  danhSachSuatChieu; // Toàn bộ suất chiếu trong hệ thống
    std::vector<Movie*>    danhSachPhim;      // Tham chiếu các phim đã thêm
    std::vector<Room*>     danhSachPhong;     // Tham chiếu các phòng

public:
    // ----- Constructor & Destructor -----
    MovieSchedule();
    ~MovieSchedule();

    // ----- Đăng ký phim và phòng -----
    void themPhimVaoHeThong(Movie* phim);
    void themPhongVaoHeThong(Room* phong);

    // ----- Phương thức cốt lõi -----

    /**
     * @brief Thêm bộ phim vào lịch chiếu (tạo suất chiếu mới)
     * Gán phim + phòng + giờ chiếu thành 1 suất (Showtime)
     *
     * @param maPhim       Mã phim cần xếp lịch
     * @param maPhong      Mã phòng chiếu
     * @param thoiGianBD   Giờ bắt đầu (YYYY-MM-DD HH:MM)
     * @param giaVeCoSo    Giá vé cơ sở cho suất này
     * @param ngonNgu      Ngôn ngữ (Phụ đề Việt / Thuyết minh / Tiếng Anh)
     * @param laSuatDacBiet true nếu là suất premier/đặc biệt
     * @return Mã suất chiếu mới tạo (rỗng nếu thất bại)
     */
    std::string themBophimVaoLich(const std::string& maPhim,
                                   const std::string& maPhong,
                                   const std::string& thoiGianBD,
                                   double giaVeCoSo = 85000.0,
                                   const std::string& ngonNgu = "Phụ đề Việt",
                                   bool laSuatDacBiet = false);

    /**
     * @brief Xóa suất chiếu khỏi lịch
     * @param maSuatChieu Mã suất chiếu cần xóa
     * @return true nếu xóa thành công
     */
    bool xoaSuatChieu(const std::string& maSuatChieu);

    /**
     * @brief Lấy lịch chiếu của 1 phim cụ thể
     * @param maPhim Mã phim cần xem lịch
     * @return Vector pointer các suất chiếu của phim đó
     */
    std::vector<Showtime*> layLichChieuTheoPhim(const std::string& maPhim);

    /**
     * @brief Lấy lịch chiếu theo ngày
     * @param ngay Ngày cần xem (YYYY-MM-DD)
     * @return Vector pointer các suất chiếu trong ngày đó
     */
    std::vector<Showtime*> layLichChieuTheoNgay(const std::string& ngay);

    /**
     * @brief Lấy lịch chiếu theo phòng
     * @param maPhong Mã phòng chiếu
     * @return Vector pointer các suất chiếu trong phòng đó
     */
    std::vector<Showtime*> layLichChieuTheoPhong(const std::string& maPhong);

    /**
     * @brief Tìm suất chiếu theo mã
     * @param maSuatChieu Mã suất chiếu cần tìm
     * @return Pointer đến suất chiếu, nullptr nếu không tìm thấy
     */
    Showtime* timSuatChieuBangMa(const std::string& maSuatChieu);

    /**
     * @brief Lấy tất cả suất chiếu đang bán vé
     */
    std::vector<Showtime*> layTatCaSuatDangBan();

    /**
     * @brief Kiểm tra phòng có bị trùng lịch không
     * Phòng không thể chiếu 2 phim cùng lúc
     * @param maPhong Mã phòng cần kiểm tra
     * @param thoiGianBD Giờ bắt đầu muốn thêm
     * @param thoiLuongPhut Thời lượng phim (phút)
     * @return true nếu phòng bị trùng lịch
     */
    bool kiemTraTrungLich(const std::string& maPhong,
                           const std::string& thoiGianBD,
                           int thoiLuongPhut) const;

    /**
     * @brief Hiển thị toàn bộ lịch chiếu (dạng bảng)
     */
    void hienThiToanBoLichChieu() const;

    /**
     * @brief Hiển thị lịch chiếu theo ngày với format đẹp
     * @param ngay Ngày cần xem (YYYY-MM-DD)
     */
    void hienThiLichChieuTheoNgay(const std::string& ngay) const;

    /**
     * @brief Lấy số lượng suất chiếu trong hệ thống
     */
    int getSoSuatChieu() const { return (int)danhSachSuatChieu.size(); }

    const std::vector<Showtime>& getDanhSachSuatChieu() const {
        return danhSachSuatChieu;
    }
    std::vector<Showtime>& getDanhSachSuatChieuRef() {
        return danhSachSuatChieu;
    }
    std::vector<Movie*>& getDanhSachPhimRef() {
        return danhSachPhim;
    }
    std::vector<Room*>& getDanhSachPhongRef() {
        return danhSachPhong;
    }

private:
    // Tạo mã suất chiếu tự động
    std::string sinhMaSuatChieu();

    // Tìm phim theo mã (trong danh sách đã đăng ký)
    Movie* timPhimBangMa(const std::string& maPhim);

    // Tìm phòng theo mã
    Room* timPhongBangMa(const std::string& maPhong);
};

#endif // MOVIESCHEDULE_H
