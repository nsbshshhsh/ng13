/**
 * ================================================================
 * CineMax - app.js  (CORE ENGINE)
 * Toàn bộ logic tương đương với code C++ được port sang JS:
 *  - User / Customer / Admin
 *  - Movie / Room / Showtime / Seat
 *  - Booking / Ticket / PaymentService / MovieSchedule
 * ================================================================
 */

// ─────────────────────────────────────────────────────────────────
// CONSTANTS (tương đương enum C++)
// ─────────────────────────────────────────────────────────────────
const SeatType     = { NORMAL: 'normal', VIP: 'vip', COUPLE: 'couple' };
const SeatStatus   = { AVAILABLE: 'available', BOOKED: 'booked', SOLD: 'sold', MAINTENANCE: 'maintenance' };
const RoomType     = { HAI_D: '2D', BA_D: '3D', IMAX: 'IMAX', PREMIUM: 'Premium', FOUR_DX: '4DX' };
const ShowStatus   = { UPCOMING: 'upcoming', SELLING: 'selling', FULL: 'full', ENDED: 'ended', CANCELLED: 'cancelled' };
const BookingStatus= { PENDING: 'pending', CONFIRMED: 'confirmed', CANCELLED: 'cancelled', REFUNDED: 'refunded' };
const PayMethod    = { CASH: 'cash', CARD: 'card', MOMO: 'momo', VNPAY: 'vnpay', ZALOPAY: 'zalopay', POINTS: 'points' };
const MemberTier   = { BRONZE: 'Bronze', SILVER: 'Silver', GOLD: 'Gold', DIAMOND: 'Diamond' };

// ─────────────────────────────────────────────────────────────────
// SEAT PRICE MULTIPLIERS (tương đương Seat::tinhGia())
// ─────────────────────────────────────────────────────────────────
const SEAT_MULTIPLIER = { normal: 1.0, vip: 1.5, couple: 2.0 };
const MEMBER_DISCOUNT  = { Bronze: 0, Silver: 0.05, Gold: 0.10, Diamond: 0.20 };
const POINTS_PER_VND   = 10000;   // 10.000đ = 1 điểm
const VND_PER_POINT    = 1000;    // 1 điểm = 1.000đ

// ─────────────────────────────────────────────────────────────────
// REFUND POLICY (tương đương PaymentService::tinhTienHoan())
// > 24h  → hoàn 100%
// 1-24h  → hoàn 50%
// < 1h   → hoàn 0%
// ─────────────────────────────────────────────────────────────────
function calcRefundRate(hoursBeforeShow) {
  if (hoursBeforeShow > 24)  return 1.00;
  if (hoursBeforeShow >= 1)  return 0.50;
  return 0.00;
}

// ─────────────────────────────────────────────────────────────────
// MEMBER TIER (tương đương Customer::capNhatHang())
// ─────────────────────────────────────────────────────────────────
function normalizeTier(tierStr) {
  if (!tierStr) return 'Bronze';
  const lower = tierStr.toLowerCase();
  if (lower.includes('diamond')) return 'Diamond';
  if (lower.includes('gold')) return 'Gold';
  if (lower.includes('silver')) return 'Silver';
  return 'Bronze';
}
function getMemberTier(totalSpent) {
  if (totalSpent >= 20_000_000) return MemberTier.DIAMOND;
  if (totalSpent >= 5_000_000)  return MemberTier.GOLD;
  if (totalSpent >= 1_000_000)  return MemberTier.SILVER;
  return MemberTier.BRONZE;
}
function getTierBadge(tier) {
  const norm = normalizeTier(tier);
  return { Bronze: '🥉', Silver: '🥈', Gold: '🥇', Diamond: '💎' }[norm] || '🥉';
}
function getTierColor(tier) {
  const norm = normalizeTier(tier);
  return { Bronze: '#cd7f32', Silver: '#c0c0c0', Gold: '#f5c518', Diamond: '#88c6f0' }[norm] || '#cd7f32';
}

// ─────────────────────────────────────────────────────────────────
// QR CODE GENERATOR (tương đương Ticket::taoMaQR())
// ─────────────────────────────────────────────────────────────────
function generateQRCode(ticketId) {
  const data = ticketId + '|' + Date.now();
  let hash = 0;
  for (let i = 0; i < data.length; i++) {
    hash = Math.imul(31, hash) + data.charCodeAt(i) | 0;
  }
  return 'QR-' + Math.abs(hash).toString(16).toUpperCase().padStart(12, '0');
}

// ─────────────────────────────────────────────────────────────────
// BOOKING CODE (tương đương Booking::sinhMaDatChoNgauNhien())
// ─────────────────────────────────────────────────────────────────
function generateBookingCode() {
  const chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
  return 'CIN-' + Array.from({ length: 6 }, () => chars[Math.floor(Math.random() * chars.length)]).join('');
}

// ─────────────────────────────────────────────────────────────────
// BOOKING ID (tương đương Booking::sinhMaDon())
// ─────────────────────────────────────────────────────────────────
function generateBookingId() {
  const d = new Date().toISOString().slice(0, 10).replace(/-/g, '');
  const seq = String(Math.floor(Math.random() * 9999) + 1).padStart(4, '0');
  return `BK-${d}-${seq}`;
}

function generateTicketId(seq) {
  const d = new Date().toISOString().slice(0, 10).replace(/-/g, '');
  return `TK-${d}-${String(seq).padStart(4, '0')}`;
}

// ─────────────────────────────────────────────────────────────────
// FORMATTERS
// ─────────────────────────────────────────────────────────────────
function fmtCurrency(n) {
  return new Intl.NumberFormat('vi-VN').format(n) + 'đ';
}
function fmtDuration(min) {
  const h = Math.floor(min / 60), m = min % 60;
  return h ? `${h}g${m ? m + 'p' : ''}` : `${m}p`;
}
function fmtDate(iso) {
  if (!iso) return '';
  const d = new Date(iso);
  return d.toLocaleDateString('vi-VN', { weekday: 'short', day: '2-digit', month: '2-digit', year: 'numeric' });
}
function getPosterHTML(posterStr) {
  if (!posterStr) return '🎬';
  if (posterStr.includes('.') || posterStr.startsWith('http') || posterStr.startsWith('images/')) {
    let src = posterStr;
    if (posterStr.startsWith('images/')) {
      src = resolveRootPath() + posterStr;
    }
    return `<img src="${src}" alt="Poster" />`;
  }
  return posterStr;
}
function todayStr() {
  return new Date().toISOString().slice(0, 10);
}

// ─────────────────────────────────────────────────────────────────
// DATABASE GATEWAY - Kết nối trực tiếp với backend C++ (qua Python)
// ─────────────────────────────────────────────────────────────────
let _dbCache = {
  movies: [],
  showtimes: [],
  rooms: [],
  soldSeats: {},
  bookingsById: {}
};

async function refreshSoldSeatCache(showtimeIds = null) {
  const ids = Array.isArray(showtimeIds) && showtimeIds.length
    ? showtimeIds
    : _dbCache.showtimes.map(s => s.id);

  await Promise.all(ids.map(async (showtimeId) => {
    try {
      const res = await fetch(resolveRootPath() + `api/sold-seats?showtimeId=${showtimeId}`);
      const soldSeats = await res.json();
      _dbCache.soldSeats[showtimeId] = soldSeats;
    } catch (err) {
      console.error(`Không thể tải ghế đã bán cho suất ${showtimeId}:`, err);
      _dbCache.soldSeats[showtimeId] = _dbCache.soldSeats[showtimeId] || [];
    }
  }));
}

async function initAppDB() {
  try {
    const root = resolveRootPath();
    const [movies, showtimes, rooms] = await Promise.all([
      fetch(root + 'api/movies').then(r => r.json()),
      fetch(root + 'api/showtimes').then(r => r.json()),
      fetch(root + 'api/rooms').then(r => r.json())
    ]);
    _dbCache.movies = movies;
    _dbCache.showtimes = showtimes;
    _dbCache.rooms = rooms;
    _dbCache.soldSeats = {};
    console.log("C++ Backend Data loaded successfully!");
  } catch (err) {
    console.error("Lỗi kết nối C++ Backend API:", err);
  }
}
window.dbInitialized = initAppDB();

// ─────────────────────────────────────────────────────────────────
// AUTH SERVICE (Kết nối --api-login và --api-register của C++)
// ─────────────────────────────────────────────────────────────────
const AuthService = {
  currentUser() {
    try {
      return JSON.parse(localStorage.getItem('cm_currentUser')) || JSON.parse(localStorage.getItem('cinemax_user')) || null;
    } catch {
      return null;
    }
  },

  async login(email, password) {
    try {
      const res = await fetch(resolveRootPath() + 'api/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ email, password })
      });
      const data = await res.json();
      if (data.ok) {
        // Đồng bộ lưu trữ cả 2 key cho các trang cũ/mới tương thích
        localStorage.setItem('cm_currentUser', JSON.stringify(data.user));
        localStorage.setItem('cinemax_user', JSON.stringify(data.user));
        return { ok: true, user: data.user };
      } else {
        return { ok: false, msg: data.msg || 'Email hoặc mật khẩu không đúng!' };
      }
    } catch (err) {
      return { ok: false, msg: 'Không thể kết nối đến server!' };
    }
  },

  logout() {
    localStorage.removeItem('cm_currentUser');
    localStorage.removeItem('cinemax_user');
  },

  async register(name, email, phone, password) {
    try {
      const res = await fetch(resolveRootPath() + 'api/register', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ name, email, phone, password })
      });
      const data = await res.json();
      if (data.ok) {
        return { ok: true, user: data.user };
      } else {
        return { ok: false, msg: data.msg || 'Đăng ký thất bại!' };
      }
    } catch (err) {
      return { ok: false, msg: 'Không thể kết nối đến server!' };
    }
  },

  updateProfile(userId, patch) {
    // Để nguyên để hỗ trợ thay đổi UI tạm nếu cần, hoặc kết nối API nếu C++ hỗ trợ.
    // Vì C++ không có api cập nhật trực tiếp profile qua dòng lệnh, ta giữ cục bộ.
    const user = AuthService.currentUser();
    if (user && user.id === userId) {
      const updated = { ...user, ...patch };
      localStorage.setItem('cm_currentUser', JSON.stringify(updated));
      localStorage.setItem('cinemax_user', JSON.stringify(updated));
      return updated;
    }
  }
};

// ─────────────────────────────────────────────────────────────────
// MOVIE SERVICE (Đọc từ bộ nhớ đệm đã nạp từ C++ backend)
// ─────────────────────────────────────────────────────────────────
const MovieService = {
  getAll: ()           => _dbCache.movies,
  getById: (id)        => _dbCache.movies.find(m => m.id === id) || null,
  async add(movie) {
    try {
      const res = await fetch(resolveRootPath() + 'api/add-movie', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(movie)
      });
      const data = await res.json();
      if (data.ok) {
        await initAppDB(); // Tải lại danh sách
        return { ok: true, id: data.maPhim };
      }
      return { ok: false, msg: data.msg };
    } catch (err) {
      return { ok: false, msg: 'Lỗi kết nối server' };
    }
  },
  update(id, patch) {
    // Cục bộ
  },
  delete(id) {
    // Cục bộ
  }
};

// ─────────────────────────────────────────────────────────────────
// ROOM SERVICE (Đọc từ bộ nhớ đệm đã nạp từ C++ backend)
// ─────────────────────────────────────────────────────────────────
const RoomService = {
  getAll:    ()      => _dbCache.rooms,
  getById:   (id)    => _dbCache.rooms.find(r => r.id === id) || null,
};

// ─────────────────────────────────────────────────────────────────
// SHOWTIME SERVICE (Đọc từ bộ nhớ đệm & tải động trạng thái ghế C++)
// ─────────────────────────────────────────────────────────────────
const ShowtimeService = {
  getAll: ()       => _dbCache.showtimes,
  getById: (id)    => _dbCache.showtimes.find(s => s.id === id) || null,
  getByDate: (d)   => _dbCache.showtimes.filter(s => s.date === d),
  getByMovie: (mid)=> _dbCache.showtimes.filter(s => s.movieId === mid),

  calcSeatPrice(showtimeId, seatType) {
    const suat = ShowtimeService.getById(showtimeId);
    if (!suat) return 0;
    const room = RoomService.getById(suat.roomId);
    const base = room ? room.basePrice : (suat.price || 100000); // fallback về giá showtime
    return Math.round(base * (SEAT_MULTIPLIER[seatType] || 1.0));
  },

  getSoldSeats(showtimeId) {
    return _dbCache.soldSeats[showtimeId] || [];
  },

  setSoldSeats(showtimeId, seats) {
    _dbCache.soldSeats[showtimeId] = Array.isArray(seats) ? seats : [];
  },

  async addSoldSeats(showtimeId, seatIds) {
    const current = new Set(ShowtimeService.getSoldSeats(showtimeId));
    (seatIds || []).forEach(seatId => current.add(seatId));
    ShowtimeService.setSoldSeats(showtimeId, [...current]);
  },

  async freeSoldSeats(showtimeId, seatIds) {
    const current = new Set(ShowtimeService.getSoldSeats(showtimeId));
    (seatIds || []).forEach(seatId => current.delete(seatId));
    ShowtimeService.setSoldSeats(showtimeId, [...current]);
  },

  countAvailable(showtimeId) {
    const suat = ShowtimeService.getById(showtimeId);
    if (!suat) return 0;
    const room = RoomService.getById(suat.roomId);
    const total = room ? room.rows * room.seatsPerRow : 117; // fallback 9x13
    const sold  = ShowtimeService.getSoldSeats(showtimeId).length;
    return total - sold;
  },

  async add(showtime) {
    try {
      const res = await fetch(resolveRootPath() + 'api/add-showtime', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(showtime)
      });
      const data = await res.json();
      if (data.ok) {
        await initAppDB();
        return { ok: true, id: data.maSuat };
      }
      return { ok: false, msg: data.msg };
    } catch (err) {
      return { ok: false, msg: 'Lỗi kết nối server' };
    }
  },

  async delete(id) {
    try {
      const res = await fetch(resolveRootPath() + 'api/delete-showtime', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ showtimeId: id })
      });
      const data = await res.json();
      if (data.ok) {
        await initAppDB();
        return { ok: true };
      }
      return { ok: false, msg: data.msg };
    } catch (err) {
      return { ok: false, msg: 'Lỗi kết nối' };
    }
  }
};

// ─────────────────────────────────────────────────────────────────
// BOOKING SERVICE (Gọi API của C++ backend)
// ─────────────────────────────────────────────────────────────────
const BookingService = {
  // Lấy danh sách bookings qua API /api/tickets?userId=...
  async getByUser(uid) {
    try {
      const res = await fetch(resolveRootPath() + `api/tickets?userId=${uid}`);
      return await res.json();
    } catch (err) {
      console.error(err);
      return [];
    }
  },

  async create({ userId, showtimeId, seats, payMethod, pointsUsed = 0 }) {
    try {
      // payMethod: chuyển tên string (vnpay, momo,...) sang số tương đương enum C++
      // PhuongThucThanhToan C++:
      //   TIEN_MAT = 0, THE_NGAN_HANG = 1, MOMO = 2, VNPAY = 3, ZALOPAY = 4, DIEM_TICH_LUY = 5
      const methodMap = { cash: 0, card: 1, momo: 2, vnpay: 3, zalopay: 4, points: 5 };
      const payVal = methodMap[payMethod] ?? 3; // Mặc định vnpay

      const res = await fetch(resolveRootPath() + 'api/book', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          userId: userId,
          showtimeId: showtimeId,
          seats: seats.map(s => s.id), // Chỉ gửi list ID ghế cho backend C++
          payMethod: payVal,
          pointsUsed: pointsUsed
        })
      });
      
      const data = await res.json();
      if (data.ok) {
        const seatIds = seats.map(s => s.id);
        await ShowtimeService.addSoldSeats(showtimeId, seatIds);

        const user = AuthService.currentUser();
        if (user) {
          const updatedTotalSpent = (Number(user.totalSpent) || 0) + Number(data.total || 0);
          const updatedUser = {
            ...user,
            points: Number(data.points ?? user.points ?? 0),
            totalSpent: updatedTotalSpent,
            tier: getMemberTier(updatedTotalSpent),
          };
          localStorage.setItem('cm_currentUser', JSON.stringify(updatedUser));
          localStorage.setItem('cinemax_user', JSON.stringify(updatedUser));
        }
        
        // Trả về cấu trúc tương thích với frontend UI
        const booking = {
          bookingCode: data.bookingCode,
          id: data.bookingId,
          total: data.total,
          payMethod: payMethod,
          showtimeId: showtimeId,
          seats: seats
        };
        _dbCache.bookingsById[booking.id] = booking;
        return { ok: true, booking, updatedUser: AuthService.currentUser() };
      } else {
        return { ok: false, msg: data.msg || 'Đặt vé thất bại!' };
      }
    } catch (err) {
      console.error(err);
      return { ok: false, msg: 'Lỗi kết nối server!' };
    }
  },

  async cancel(bookingId) {
    try {
      const res = await fetch(resolveRootPath() + 'api/cancel', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ bookingId })
      });
      const data = await res.json();
      if (data.ok) {
        const booking = _dbCache && _dbCache.bookingsById ? _dbCache.bookingsById[bookingId] : null;
        if (booking?.showtimeId) {
          await ShowtimeService.freeSoldSeats(booking.showtimeId, booking.seats?.map(s => s.id) || []);
          delete _dbCache.bookingsById[bookingId];
        } else {
          await refreshSoldSeatCache();
        }
        return { ok: true, refundAmount: data.refundAmount };
      } else {
        return { ok: false, msg: data.msg || 'Hủy vé thất bại!' };
      }
    } catch (err) {
      console.error(err);
      return { ok: false, msg: 'Lỗi kết nối server!' };
    }
  }
};

// ─────────────────────────────────────────────────────────────────
// NAVBAR HELPER (dùng chung mọi trang)
// ─────────────────────────────────────────────────────────────────
function initNavbar() {
  const navbar = document.getElementById('navbar');
  if (navbar) {
    window.addEventListener('scroll', () => {
      navbar.classList.toggle('scrolled', window.scrollY > 30);
    });
    // trigger once
    navbar.classList.toggle('scrolled', window.scrollY > 30);
  }

  refreshNavAuth();

  // hamburger
  const ham = document.getElementById('hamburger');
  if (ham) ham.onclick = () => document.getElementById('navLinks')?.classList.toggle('open');
}

function refreshNavAuth() {
  const user = AuthService.currentUser();
  const loginBtn    = document.getElementById('loginBtn');
  const logoutBtn   = document.getElementById('logoutBtn');
  const userInfo    = document.getElementById('userInfo');
  const myTicketsBtn= document.getElementById('myTicketsBtn');
  const adminBtn    = document.getElementById('adminBtn');

  if (user) {
    loginBtn    && (loginBtn.style.display    = 'none');
    logoutBtn   && (logoutBtn.style.display   = 'inline-flex');
    userInfo    && (userInfo.style.display    = 'flex',
                    userInfo.innerHTML = `${getTierBadge(user.tier)} ${user.name.split(' ').pop()} <small style="color:var(--text-400)">(${user.points}đ⭐)</small>`);
    myTicketsBtn && (myTicketsBtn.style.display = 'inline-flex');
    if (adminBtn) adminBtn.style.display = user.role === 'admin' ? 'inline-flex' : 'none';
  } else {
    loginBtn    && (loginBtn.style.display    = 'inline-flex');
    logoutBtn   && (logoutBtn.style.display   = 'none');
    userInfo    && (userInfo.style.display    = 'none');
    myTicketsBtn && (myTicketsBtn.style.display = 'none');
    if (adminBtn) adminBtn.style.display = 'none';
  }
}

// ─────────────────────────────────────────────────────────────────
// AUTH MODAL (dùng chung)
// ─────────────────────────────────────────────────────────────────
function openLoginModal()  { const m = document.getElementById('loginModal'); if(m){ m.classList.add('open'); document.body.style.overflow='hidden'; } }
function closeLoginModal() { const m = document.getElementById('loginModal'); if(m){ m.classList.remove('open'); document.body.style.overflow=''; } }

function switchTab(tab) {
  document.getElementById('loginForm')  .style.display = tab==='login'    ? 'block' : 'none';
  document.getElementById('registerForm').style.display = tab==='register' ? 'block' : 'none';
  document.getElementById('loginTab')   .classList.toggle('active', tab==='login');
  document.getElementById('registerTab').classList.toggle('active', tab==='register');
}

function fillDemo(email, pw) {
  document.getElementById('loginEmail').value    = email;
  document.getElementById('loginPassword').value = pw;
}

function togglePassword(id) {
  const el = document.getElementById(id);
  el.type = el.type === 'password' ? 'text' : 'password';
}

async function handleLogin() {
  const email = document.getElementById('loginEmail').value.trim();
  const pw    = document.getElementById('loginPassword').value;
  const err   = document.getElementById('loginError');

  if (!email || !pw) { err.textContent = '⚠️ Vui lòng nhập đầy đủ!'; return; }

  const res = await AuthService.login(email, pw);
  if (!res.ok) { err.textContent = '❌ ' + res.msg; return; }

  closeLoginModal();
  refreshNavAuth();
  showToast(`✅ Chào mừng, ${res.user.name.split(' ').pop()}!`, 'success');

  if (res.user.role === 'admin') {
    setTimeout(() => window.location.href = resolveAdminPath(), 600);
  }
}

async function handleRegister() {
  const name = document.getElementById('regName').value.trim();
  const email= document.getElementById('regEmail').value.trim();
  const phone= document.getElementById('regPhone').value.trim();
  const pw   = document.getElementById('regPassword').value;
  const err  = document.getElementById('regError');

  if (!name||!email||!phone||!pw) { err.textContent='⚠️ Vui lòng nhập đầy đủ!'; return; }

  const res = await AuthService.register(name, email, phone, pw);
  if (!res.ok) { err.textContent = '❌ ' + res.msg; return; }

  showToast('✅ Đăng ký thành công! Hãy đăng nhập.', 'success');
  switchTab('login');
  document.getElementById('loginEmail').value = email;
}

function logout() {
  AuthService.logout();
  refreshNavAuth();
  showToast('👋 Đã đăng xuất!', 'success');
  setTimeout(() => window.location.href = resolveRootPath() + 'index.html', 600);
}

// Tính đường dẫn gốc (hỗ trợ cả giao thức file:// mở cục bộ)
function resolveRootPath() {
  // Nếu mở trực tiếp file HTML bằng file:// thì vẫn gọi về server local.
  if (window.location.protocol === 'file:') {
    return 'http://196.169.3.176:8085/';
  }
  // Khi mở bằng IP LAN, domain, ngrok/Cloudflare Tunnel... API sẽ đi theo đúng host hiện tại.
  return `${window.location.protocol}//${window.location.host}/`;
}
function resolveAdminPath() {
  return resolveRootPath() + 'admin/dashboard.html';
}

// ─────────────────────────────────────────────────────────────────
// TOAST NOTIFICATION
// ─────────────────────────────────────────────────────────────────
function showToast(msg, type='info') {
  let t = document.getElementById('_toast');
  if (!t) {
    t = document.createElement('div'); t.id = '_toast';
    t.style.cssText = 'position:fixed;bottom:24px;right:24px;z-index:99999;padding:14px 24px;border-radius:12px;font-size:.9rem;font-weight:600;max-width:360px;transform:translateX(140%);transition:.35s cubic-bezier(.34,1.56,.64,1);color:#fff;box-shadow:0 8px 32px rgba(0,0,0,.4)';
    document.body.appendChild(t);
  }
  t.textContent = msg;
  t.style.background = type==='success' ? '#1db954' : type==='error' ? '#e50914' : '#2d2d3a';
  requestAnimationFrame(() => { t.style.transform = 'translateX(0)'; });
  clearTimeout(t._timer);
  t._timer = setTimeout(() => { t.style.transform = 'translateX(140%)'; }, 3500);
}

// ─────────────────────────────────────────────────────────────────
// INIT (chạy khi load trang)
// ─────────────────────────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', async () => {
  await window.dbInitialized;
  initNavbar();

  // Đóng modal khi click ngoài
  document.getElementById('loginModal')?.addEventListener('click', e => {
    if (e.target.id === 'loginModal') closeLoginModal();
  });
});

// Expose globals
Object.assign(window, {
  openLoginModal, closeLoginModal, switchTab, fillDemo, togglePassword,
  handleLogin, handleRegister, logout, showToast, fmtCurrency, fmtDuration, fmtDate,
  AuthService, MovieService, RoomService, ShowtimeService, BookingService,
  generateBookingCode, generateQRCode, getMemberTier, getTierBadge, getTierColor, normalizeTier, resolveRootPath,
  calcRefundRate, SEAT_MULTIPLIER, MEMBER_DISCOUNT, POINTS_PER_VND, VND_PER_POINT,
  refreshSoldSeatCache, _dbCache, initAppDB,
});
