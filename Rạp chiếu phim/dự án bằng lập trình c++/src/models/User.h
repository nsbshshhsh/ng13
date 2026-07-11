#pragma once
#ifndef USER_H
#define USER_H

#include <string>
#include <iostream>

// ============================================================
// Lớp User - Người dùng cơ sở (Base class)
// Tất cả các loại người dùng đều kế thừa từ lớp này
// ============================================================
class User {
protected:
    int         id;             // Mã định danh người dùng (tự tăng)
    std::string hoTen;          // Họ và tên đầy đủ
    std::string email;          // Email (dùng để đăng nhập)
    std::string soDienThoai;    // Số điện thoại liên hệ
    std::string matKhau;        // Mật khẩu (đã mã hóa SHA-256)
    std::string vaiTro;         // Vai trò: "customer" | "admin" | "manager"
    bool        dangOnline;     // Trạng thái đăng nhập hiện tại

public:
    // ----- Constructor & Destructor -----
    User();
    User(int id, const std::string& hoTen, const std::string& email,
         const std::string& soDienThoai, const std::string& matKhau,
         const std::string& vaiTro = "customer");
    virtual ~User();

    // ----- Getter (Lấy dữ liệu) -----
    int         getId()           const { return id; }
    std::string getHoTen()        const { return hoTen; }
    std::string getEmail()        const { return email; }
    std::string getSoDienThoai()  const { return soDienThoai; }
    std::string getVaiTro()       const { return vaiTro; }
    bool        isDangOnline()    const { return dangOnline; }
    std::string getMatKhau()      const { return matKhau; }

    // ----- Setter (Cập nhật dữ liệu) -----
    void setHoTen(const std::string& ten)     { hoTen = ten; }
    void setEmail(const std::string& mail)    { email = mail; }
    void setSoDienThoai(const std::string& sdt) { soDienThoai = sdt; }
    void setId(int newId)                     { id = newId; }
    void setMatKhauHashed(const std::string& hashedPw) { matKhau = hashedPw; }
    void setVaiTro(const std::string& vt)              { vaiTro = vt; }

    // ----- Phương thức cốt lõi -----

    /**
     * @brief Đăng nhập với email + mật khẩu
     * @param emailInput Email nhập vào
     * @param matKhauInput Mật khẩu nhập vào
     * @return true nếu đăng nhập thành công
     */
    virtual bool dangNhap(const std::string& emailInput,
                          const std::string& matKhauInput);

    /**
     * @brief Đăng xuất khỏi hệ thống
     */
    virtual void dangXuat();

    /**
     * @brief Cập nhật thông tin cá nhân (họ tên, SĐT)
     * @param hoTenMoi Họ tên mới (để trống nếu không thay đổi)
     * @param sdtMoi SĐT mới (để trống nếu không thay đổi)
     */
    virtual void capNhatThongTin(const std::string& hoTenMoi,
                                  const std::string& sdtMoi);

    /**
     * @brief Đổi mật khẩu
     * @param matKhauCu Mật khẩu hiện tại để xác thực
     * @param matKhauMoi Mật khẩu mới
     * @return true nếu đổi thành công
     */
    virtual bool doiMatKhau(const std::string& matKhauCu,
                             const std::string& matKhauMoi);

    /**
     * @brief Hiển thị thông tin người dùng ra màn hình
     */
    virtual void hienThiThongTin() const;

    /**
     * @brief Mã hóa mật khẩu bằng thuật toán đơn giản (SHA-like)
     * @param matKhau Mật khẩu gốc
     * @return Chuỗi đã mã hóa
     */
    static std::string maHoaMatKhau(const std::string& matKhau);

    /**
     * @brief Kiểm tra mật khẩu có khớp không
     */
    bool kiemTraMatKhau(const std::string& matKhauNhap) const;

    // Phương thức ảo thuần túy - các lớp con phải override
    virtual std::string loaiNguoiDung() const = 0;
};

#endif // USER_H
