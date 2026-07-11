#pragma once
#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <vector>
#include "../models/Movie.h"
#include "../models/Customer.h"
#include "../models/Admin.h"
#include "../models/Booking.h"
#include "../models/MovieSchedule.h"
#include "../models/CinemaRoom.h"

class DatabaseManager {
private:
    static std::string folderPath;
    static std::vector<std::string> parseCSVLine(const std::string& line);
    static std::string escapeCSV(const std::string& field);
    static void ensureDirectoryExists();
    static int safeStoi(const std::string& s, int def = 0);
    static double safeStod(const std::string& s, double def = 0.0);

public:
    static void setFolderPath(const std::string& path);
    static std::string getFolderPath();

    static bool fileExists(const std::string& filename);

    // Lưu dữ liệu
    static void saveMovies(const std::vector<Movie>& movies);
    static void saveRooms(const std::vector<Room>& rooms);
    static void saveUsers(const std::vector<Customer>& customers, const std::vector<Admin>& admins);
    static void saveShowtimes(const std::vector<Showtime>& showtimes);
    static void saveBookingsAndTickets(const std::vector<Booking>& bookings);

    // Đọc dữ liệu
    static std::vector<Movie> loadMovies();
    static std::vector<Room> loadRooms(const std::string& maCinema = "RAP001");
    static void loadUsers(std::vector<Customer>& customers, std::vector<Admin>& admins);
    static std::vector<Showtime> loadShowtimes(std::vector<Movie>& movies, std::vector<Room>& rooms);
    static std::vector<Booking> loadBookingsAndTickets(std::vector<Customer>& customers, const std::vector<Showtime>& showtimes);

    // Đồng bộ toàn bộ hệ thống
    static void loadAllData(std::vector<Movie>& movies, 
                            CinemaRoom& rap, 
                            std::vector<Customer>& customers, 
                            std::vector<Admin>& admins, 
                            std::vector<Booking>& bookings, 
                            MovieSchedule& schedule);

    static void saveAllData(const std::vector<Movie>& movies, 
                            const CinemaRoom& rap, 
                            const std::vector<Customer>& customers, 
                            const std::vector<Admin>& admins, 
                            const std::vector<Booking>& bookings, 
                            const MovieSchedule& schedule);

    // Khởi tạo dữ liệu mẫu nếu chưa có file
    static void seedDefaultData(std::vector<Movie>& movies, 
                                CinemaRoom& rap, 
                                std::vector<Customer>& customers, 
                                std::vector<Admin>& admins, 
                                MovieSchedule& schedule);
};

#endif // DATABASE_MANAGER_H
