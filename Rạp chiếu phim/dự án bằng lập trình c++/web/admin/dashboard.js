// ══════════════════════════════════════════════════════════
// dashboard.js – Admin Dashboard specific logic connected to C++ Backend
// ══════════════════════════════════════════════════════════
const formatCurrency = fmtCurrency;

document.addEventListener('DOMContentLoaded', async () => {
  await window.dbInitialized; // Chờ C++ backend nạp dữ liệu
  checkAdminAuth();
  initDashboard();
});

function checkAdminAuth() {
  const user = AuthService.currentUser();
  if (!user || user.role !== 'admin') {
    window.location.href = '../index.html';
    return;
  }
  const nameEl = document.getElementById('adminName');
  if (nameEl) nameEl.textContent = user.name.split(' ').pop();
  const avatarEl = document.getElementById('adminAvatar');
  if (avatarEl) avatarEl.textContent = user.name.charAt(user.name.lastIndexOf(' ') + 1);
}

// ── GET INTEGRATED DATA FROM C++ API ──
async function getBookingsFromAPI() {
  try {
    const ticketsRes = await fetch(resolveRootPath() + 'api/tickets?userId=0');
    const ticketsFromApi = await ticketsRes.json();
    
    const VIP_ROWS = ['D','E','F'];
    const COUPLE_ROWS = ['H','I'];
    
    return ticketsFromApi.map(b => {
      const seats = b.tickets.map(t => {
        let type = 'normal';
        if (VIP_ROWS.includes(t.seatId.charAt(0))) type = 'vip';
        else if (COUPLE_ROWS.includes(t.seatId.charAt(0))) type = 'couple';
        return { id: t.seatId, type, price: t.price };
      });
      
      let localStatus = 'confirmed';
      if (b.status.includes('hủy') || b.status.includes('hoàn tiền') || b.status.includes('Hủy') || b.status.includes('Hoàn')) {
        localStatus = 'cancelled';
      }
      
      let payMethod = 'vnpay';
      if ((b.payMethod || '').includes('MoMo')) payMethod = 'momo';
      else if ((b.payMethod || '').includes('ZaloPay')) payMethod = 'zalopay';
      else if ((b.payMethod || '').includes('Tiền mặt')) payMethod = 'cash';
      else if ((b.payMethod || '').includes('Thẻ')) payMethod = 'card';
      
      return {
        id: b.id,
        code: b.bookingCode,
        userName: b.userName || 'Khách',
        movie: b.movieTitle,
        seats: seats,
        total: b.total,
        payment: payMethod,
        status: localStatus
      };
    });
  } catch (err) {
    console.error("Lỗi lấy vé:", err);
    return [];
  }
}

// ── SECTION: DASHBOARD ──
async function initDashboard() {
  const content = document.getElementById('adminContent');
  if (!content) return;

  // Render HTML structure of dashboard
  content.innerHTML = `
    <!-- KPI CARDS -->
    <div class="kpi-grid">
      <div class="kpi-card">
        <div class="kpi-label">💰 Doanh thu hệ thống</div>
        <div class="kpi-value" id="kpiRevenue">0đ</div>
        <div class="kpi-change up">↑ Hoạt động ổn định</div>
      </div>
      <div class="kpi-card">
        <div class="kpi-label">🎟️ Vé đã bán</div>
        <div class="kpi-value" id="kpiTickets">0</div>
        <div class="kpi-change up">↑ Đang bán trực tuyến</div>
      </div>
      <div class="kpi-card">
        <div class="kpi-label">👥 Khách hàng</div>
        <div class="kpi-value" id="kpiCustomers">0</div>
        <div class="kpi-change up">↑ Thành viên đăng ký</div>
      </div>
      <div class="kpi-card">
        <div class="kpi-label">🎬 Suất chiếu</div>
        <div class="kpi-value" id="kpiShowtimes">0</div>
        <div class="kpi-change">Đang hiển thị lịch</div>
      </div>
    </div>

    <!-- CHARTS -->
    <div class="charts-grid">
      <div class="chart-card">
        <h3>📈 Doanh thu 7 ngày gần nhất</h3>
        <div class="bar-chart" id="revenueChart" style="display:block;height:250px;"></div>
        <div style="display:flex;justify-content:center;gap:16px;margin-top:12px;font-size:0.75rem;color:var(--text-400)">
          <span style="color:var(--text-400)">Đơn vị: triệu đồng (Dữ liệu thực từ hệ thống)</span>
        </div>
      </div>
      <div class="chart-card">
        <h3>🏆 Top phim doanh thu cao</h3>
        <div class="movie-rank" id="movieRanking"></div>
      </div>
    </div>

    <!-- RECENT BOOKINGS TABLE -->
    <div class="table-card">
      <div class="table-header">
        <h3>🎟️ Đơn đặt vé gần nhất</h3>
        <button onclick="showSection('bookings')" class="btn-outline" style="font-size:0.8rem;padding:6px 12px">Xem tất cả</button>
      </div>
      <table>
        <thead>
          <tr>
            <th>Mã đơn</th>
            <th>Khách hàng</th>
            <th>Phim</th>
            <th>Ghế</th>
            <th>Tổng tiền</th>
            <th>Thanh toán</th>
            <th>Trạng thái</th>
          </tr>
        </thead>
        <tbody id="bookingTable">
          <tr><td colspan="7" style="text-align:center;color:var(--text-400);padding:40px">Đang tải dữ liệu...</td></tr>
        </tbody>
      </table>
    </div>
  `;

  try {
    // 1. Tải dữ liệu tổng quan từ C++ API
    const dashboardRes = await fetch(resolveRootPath() + 'api/dashboard');
    const dashboardData = await dashboardRes.json();
    
    document.getElementById('kpiRevenue').textContent   = fmtCurrency(dashboardData.totalRevenue);
    document.getElementById('kpiTickets').textContent   = dashboardData.ticketsSold;
    document.getElementById('kpiCustomers').textContent = dashboardData.totalCustomers;
    document.getElementById('kpiShowtimes').textContent = ShowtimeService.getAll().length;
    
    // 2. Tải tất cả các đơn đặt vé từ backend C++
    const bookings = await getBookingsFromAPI();

    renderRevenueChart();
    renderMovieRanking();
    renderBookingTable(bookings);

  } catch (err) {
    console.error("Lỗi khởi tạo dashboard:", err);
  }
}

async function renderRevenueChart() {
  const chartContainer = document.getElementById('revenueChart');
  if (!chartContainer) return;

  try {
    const res = await fetch(resolveRootPath() + 'api/admin-chart');
    const data = await res.json();
    
    // Create canvas
    chartContainer.innerHTML = '<canvas id="revenueCanvas" style="width:100%;height:100%"></canvas>';
    const ctx = document.getElementById('revenueCanvas').getContext('2d');
    
    // Convert to millions
    const revenueInMillions = data.revenue.map(r => r / 1000000);

    new Chart(ctx, {
      type: 'line',
      data: {
        labels: data.labels,
        datasets: [{
          label: 'Doanh thu (Triệu VNĐ)',
          data: revenueInMillions,
          borderColor: '#e50914',
          backgroundColor: 'rgba(229, 9, 20, 0.2)',
          borderWidth: 3,
          tension: 0.4,
          fill: true,
          pointBackgroundColor: '#e50914',
          pointBorderColor: '#fff',
          pointRadius: 5
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { display: false }
        },
        scales: {
          y: {
            beginAtZero: true,
            grid: { color: 'rgba(255, 255, 255, 0.1)' },
            ticks: { color: '#94a3b8' }
          },
          x: {
            grid: { display: false },
            ticks: { color: '#94a3b8' }
          }
        }
      }
    });
  } catch(e) {
    console.error("Error drawing chart", e);
    chartContainer.innerHTML = '<div style="color:var(--text-400)">Không thể tải biểu đồ</div>';
  }
}

function renderMovieRanking() {
  const el = document.getElementById('movieRanking');
  if (!el) return;
  const movies = [
    { name: "Avengers: Secret Wars", pct: 100 },
    { name: "Doraemon: Phiêu Lưu",   pct: 72 },
    { name: "Gia Đình Là Số 1",       pct: 58 },
    { name: "Vong Bóng Tối",          pct: 35 },
  ];

  el.innerHTML = movies.map((m, i) => `
    <div class="rank-item">
      <div class="rank-num">#${i+1}</div>
      <div class="rank-bar-wrap">
        <div class="rank-name">${m.name}</div>
        <div class="rank-bar-bg">
          <div class="rank-bar-fill" style="width:${m.pct}%"></div>
        </div>
      </div>
      <div class="rank-pct">${m.pct}%</div>
    </div>`).join('');
}

function renderBookingTable(bookings) {
  const tbody = document.getElementById('bookingTable');
  if (!tbody) return;
  if (bookings.length === 0) {
    tbody.innerHTML = '<tr><td colspan="7" style="text-align:center;color:var(--text-400);padding:40px">Chưa có đơn hàng nào</td></tr>';
    return;
  }

  tbody.innerHTML = bookings.slice(-10).reverse().map(b => `
    <tr>
      <td style="font-weight:600;color:var(--primary)">${b.id}</td>
      <td>${b.userName}</td>
      <td>${b.movie?.substr(0,20) || '—'}${b.movie?.length > 20 ? '...' : ''}</td>
      <td>${b.seats?.map(s => s.id).join(', ') || '—'}</td>
      <td style="font-weight:600">${fmtCurrency(b.total)}</td>
      <td>${paymentLabel(b.payment)}</td>
      <td><span class="badge ${b.status}">${statusLabel(b.status)}</span></td>
    </tr>`).join('');
}

function paymentLabel(pt) {
  const map = { vnpay: '🏦 VNPay', momo: '💜 MoMo', zalopay: '💙 ZaloPay', cash: '💵 Tiền mặt', card: '💳 Thẻ' };
  return map[pt] || pt;
}
function statusLabel(s) {
  const map = { confirmed: '✅ Thành công', cancelled: '❌ Đã hủy', pending: '⏳ Chờ xử lý' };
  return map[s] || s;
}


// ── SECTION: MOVIES MANAGEMENT (CRUD) ──
function initMoviesSection() {
  const content = document.getElementById('adminContent');
  if (!content) return;

  const movies = MovieService.getAll();
  const listRows = movies.map(m => `
    <tr>
      <td style="font-weight:600;color:var(--primary)">${m.id}</td>
      <td>
        <div style="display:flex;align-items:center;gap:12px">
          <div class="admin-movie-poster" style="width:45px;height:65px;border-radius:6px;overflow:hidden;flex-shrink:0;background:var(--bg-600);display:flex;align-items:center;justify-content:center;box-shadow:var(--shadow-sm);border:1px solid var(--glass-border); font-size:1.5rem">
            ${getPosterHTML(m.poster)}
          </div>
          <strong>${m.title}</strong>
        </div>
      </td>
      <td>${m.genre}</td>
      <td>${m.duration} phút</td>
      <td>${m.releaseDate}</td>
      <td><span style="padding:2px 8px;border-radius:4px;background:${m.ageLimit===0?'#1db954':'#e50914'};font-size:0.75rem;font-weight:700">${m.ageLimit===0?'P':'T'+m.ageLimit}</span></td>
      <td>⭐ ${m.rating}</td>
      <td>
        <div style="display:flex;gap:8px">
          <button class="btn-outline" style="padding:4px 10px;font-size:0.75rem" onclick="openEditMovieModal('${m.id}')">✏️ Sửa</button>
          <button class="btn-danger" style="padding:4px 10px;font-size:0.75rem" onclick="deleteMovie('${m.id}')">🗑️ Xóa</button>
        </div>
      </td>
    </tr>
  `).join('');

  content.innerHTML = `
    <div style="display:flex;justify-content:between;align-items:center;margin-bottom:24px">
      <h2 style="color:var(--text-100)">🎬 Danh Sách Phim</h2>
      <button class="btn-primary" style="margin-left:auto;padding:10px 18px" onclick="openAddMovieModal()">➕ Thêm Phim Mới</button>
    </div>
    
    <div class="table-card">
      <table>
        <thead>
          <tr>
            <th>Mã Phim</th>
            <th>Tên Phim</th>
            <th>Thể Loại</th>
            <th>Thời Lượng</th>
            <th>Khởi Chiếu</th>
            <th>Giới Hạn</th>
            <th>Đánh Giá</th>
            <th>Hành Động</th>
          </tr>
        </thead>
        <tbody>
          ${listRows || '<tr><td colspan="8" style="text-align:center;color:var(--text-400);padding:40px">Không có phim nào trong hệ thống.</td></tr>'}
        </tbody>
      </table>
    </div>

    <!-- MOVIE MODAL -->
    <div class="modal-overlay" id="movieModal">
      <div class="modal" style="max-width:540px">
        <button class="modal-close" onclick="closeMovieModal()">✕</button>
        <h2 class="modal-title" id="movieModalTitle">Thêm Phim Mới</h2>
        <input type="hidden" id="formMovieId"/>
        <div class="form-group"><label>Tên phim</label><input type="text" id="formMovieTitle" class="form-input" placeholder="Tên phim tiếng Việt"/></div>
        <div class="form-group"><label>Poster</label>
          <div style="display:flex;gap:8px">
            <input type="text" id="formMoviePoster" class="form-input" placeholder="Link ảnh hoặc icon (vd: 🎬)"/>
            <button type="button" class="btn-outline" style="flex-shrink:0; padding:0 15px" onclick="document.getElementById('formMoviePosterFile').click()">Tải ảnh từ máy</button>
            <input type="file" id="formMoviePosterFile" accept="image/*" style="display:none;" onchange="handlePosterUpload(event)"/>
          </div>
        </div>
        <div class="form-group"><label>Thể loại</label><input type="text" id="formMovieGenre" class="form-input" placeholder="Hành động / Tình cảm..."/></div>
        <div class="form-group"><label>Thời lượng (phút)</label><input type="number" id="formMovieDuration" class="form-input" placeholder="120" min="1"/></div>
        <div class="form-group"><label>Mô tả ngắn</label><textarea id="formMovieDesc" class="form-input" placeholder="Nội dung phim..." style="height:80px;font-family:inherit"></textarea></div>
        <div class="form-group"><label>Giới hạn độ tuổi</label>
          <select id="formMovieAge" class="form-input" style="background:var(--bg-700)">
            <option value="0">P – Mọi lứa tuổi</option>
            <option value="13">T13 – Từ 13 tuổi trở lên</option>
            <option value="16">T16 – Từ 16 tuổi trở lên</option>
            <option value="18">T18 – Từ 18 tuổi trở lên</option>
          </select>
        </div>
        <div class="form-group"><label>Ngày khởi chiếu</label><input type="date" id="formMovieDate" class="form-input"/></div>
        <button class="btn-submit" onclick="submitMovieForm()" id="btnSubmitMovie">Lưu Phim</button>
      </div>
    </div>
  `;
}

let currentPosterBase64 = null;
window.handlePosterUpload = function(event) {
  const file = event.target.files[0];
  if (file) {
    const reader = new FileReader();
    reader.onload = function(e) {
      currentPosterBase64 = e.target.result;
      document.getElementById('formMoviePoster').value = "[Ảnh tải lên từ thiết bị]";
    };
    reader.readAsDataURL(file);
  }
};

function openAddMovieModal() {
  document.getElementById('movieModalTitle').textContent = "➕ Thêm Phim Mới";
  document.getElementById('formMovieId').value = "";
  document.getElementById('formMovieTitle').value = "";
  document.getElementById('formMoviePoster').value = "🎬";
  document.getElementById('formMoviePosterFile').value = "";
  currentPosterBase64 = null;
  document.getElementById('formMovieGenre').value = "";
  document.getElementById('formMovieDuration').value = "120";
  document.getElementById('formMovieDesc').value = "";
  document.getElementById('formMovieAge').value = "0";
  document.getElementById('formMovieDate').value = todayStr();
  document.getElementById('movieModal').classList.add('open');
}

function openEditMovieModal(id) {
  const m = MovieService.getById(id);
  if (!m) return;
  document.getElementById('movieModalTitle').textContent = "✏️ Cập Nhật Thông Tin Phim";
  document.getElementById('formMovieId').value = m.id;
  document.getElementById('formMovieTitle').value = m.title;
  document.getElementById('formMoviePoster').value = m.poster || "🎬";
  document.getElementById('formMoviePosterFile').value = "";
  currentPosterBase64 = null;
  document.getElementById('formMovieGenre').value = m.genre;
  document.getElementById('formMovieDuration').value = m.duration;
  document.getElementById('formMovieDesc').value = m.desc || "";
  document.getElementById('formMovieAge').value = m.ageLimit.toString();
  document.getElementById('formMovieDate').value = m.releaseDate;
  document.getElementById('movieModal').classList.add('open');
}

function closeMovieModal() {
  document.getElementById('movieModal').classList.remove('open');
}

async function submitMovieForm() {
  const id = document.getElementById('formMovieId').value;
  const title = document.getElementById('formMovieTitle').value.trim();
  let poster = document.getElementById('formMoviePoster').value.trim() || "🎬";
  let posterBase64 = null;
  if (poster === "[Ảnh tải lên từ thiết bị]") {
    posterBase64 = currentPosterBase64;
  }
  const genre = document.getElementById('formMovieGenre').value.trim();
  const duration = parseInt(document.getElementById('formMovieDuration').value);
  const desc = document.getElementById('formMovieDesc').value.trim();
  const ageLimit = parseInt(document.getElementById('formMovieAge').value);
  const releaseDate = document.getElementById('formMovieDate').value;

  if (!title || !genre || !duration || !releaseDate) {
    showToast("⚠️ Vui lòng nhập đầy đủ thông tin!", "error");
    return;
  }

  const btn = document.getElementById('btnSubmitMovie');
  btn.textContent = "⏳ Đang xử lý..."; btn.disabled = true;

  try {
    let res;
    if (id) {
      // SỬA PHIM (gọi API /api/update-movie của C++)
      res = await fetch(resolveRootPath() + 'api/update-movie', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ movieId: id, title, genre, duration, desc, ageLimit, releaseDate, poster, posterBase64 })
      });
    } else {
      // THÊM PHIM (gọi API /api/add-movie của C++)
      res = await fetch(resolveRootPath() + 'api/add-movie', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ title, genre, duration, desc, ageLimit, releaseDate, poster, posterBase64 })
      });
    }
    const data = await res.json();
    if (data.ok) {
      showToast(id ? "✅ Đã sửa phim thành công!" : "✅ Đã thêm phim mới thành công!", "success");
      closeMovieModal();
      await initAppDB(); // Tải lại DB cache
      initMoviesSection(); // Vẽ lại
    } else {
      showToast("❌ Lỗi: " + data.msg, "error");
    }
  } catch (err) {
    console.error(err);
    showToast("❌ Lỗi kết nối đến server backend C++!", "error");
  } finally {
    btn.textContent = "Lưu Phim"; btn.disabled = false;
  }
}

async function deleteMovie(id) {
  if (!confirm("⚠️ Bạn có chắc muốn XÓA phim này? Hành động này sẽ hủy tất cả suất chiếu liên quan!")) return;
  try {
    const res = await fetch(resolveRootPath() + 'api/delete-movie', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ movieId: id })
    });
    const data = await res.json();
    if (data.ok) {
      showToast("✅ Đã xóa phim thành công!", "success");
      await initAppDB();
      initMoviesSection();
    } else {
      showToast("❌ Lỗi: " + data.msg, "error");
    }
  } catch (err) {
    console.error(err);
    showToast("❌ Lỗi kết nối server!", "error");
  }
}


// ── SECTION: SHOWTIMES MANAGEMENT ──
function initShowtimesSection() {
  const content = document.getElementById('adminContent');
  if (!content) return;

  const showtimes = ShowtimeService.getAll();
  const listRows = showtimes.map(s => {
    const movie = MovieService.getById(s.movieId);
    const room  = RoomService.getById(s.roomId);
    return `
      <tr>
        <td style="font-weight:600;color:var(--primary)">${s.id}</td>
        <td><strong>${movie?.title || s.movieId}</strong></td>
        <td><strong>${room?.name || s.roomId}</strong> (${room?.type || ''})</td>
        <td>${s.date} lúc <strong>${s.time}</strong></td>
        <td>${fmtCurrency(s.price)}</td>
        <td>${s.lang}</td>
        <td><span class="badge ${s.status}">${s.status === 'selling' ? '✅ Đang bán' : '❌ Đóng'}</span></td>
        <td>
          <button class="btn-danger" style="padding:4px 10px;font-size:0.75rem" onclick="deleteShowtime('${s.id}')">🗑️ Hủy suất</button>
        </td>
      </tr>
    `;
  }).join('');

  const movies = MovieService.getAll();
  const rooms = RoomService.getAll();

  const movieOptions = movies.map(m => `<option value="${m.id}">${m.title}</option>`).join('');
  const roomOptions  = rooms.map(r => `<option value="${r.id}">${r.name} (${r.type})</option>`).join('');

  content.innerHTML = `
    <div style="display:flex;justify-content:between;align-items:center;margin-bottom:24px">
      <h2 style="color:var(--text-100)">📅 Quản Lý Lịch Chiếu</h2>
      <button class="btn-primary" style="margin-left:auto;padding:10px 18px" onclick="openAddShowtimeModal()">➕ Thêm Suất Chiếu</button>
    </div>
    
    <div class="table-card">
      <table>
        <thead>
          <tr>
            <th>Mã Suất</th>
            <th>Bộ Phim</th>
            <th>Phòng Chiếu</th>
            <th>Thời Gian Chiếu</th>
            <th>Giá Vé Cơ Sở</th>
            <th>Ngôn Ngữ</th>
            <th>Trạng Thái</th>
            <th>Hành Động</th>
          </tr>
        </thead>
        <tbody>
          ${listRows || '<tr><td colspan="8" style="text-align:center;color:var(--text-400);padding:40px">Không có suất chiếu nào.</td></tr>'}
        </tbody>
      </table>
    </div>

    <!-- SHOWTIME MODAL -->
    <div class="modal-overlay" id="showtimeModal">
      <div class="modal" style="max-width:480px">
        <button class="modal-close" onclick="closeShowtimeModal()">✕</button>
        <h2 class="modal-title">📅 Thêm Suất Chiếu Mới</h2>
        <div class="form-group"><label>Chọn phim</label><select id="formStMovie" class="form-input" style="background:var(--bg-700)">${movieOptions}</select></div>
        <div class="form-group"><label>Chọn phòng chiếu</label><select id="formStRoom" class="form-input" style="background:var(--bg-700)">${roomOptions}</select></div>
        <div class="form-group"><label>Ngày chiếu</label><input type="date" id="formStDate" class="form-input"/></div>
        <div class="form-group"><label>Giờ chiếu</label><input type="time" id="formStTime" class="form-input" value="09:00"/></div>
        <div class="form-group"><label>Giá vé cơ sở (đ)</label><input type="number" id="formStPrice" class="form-input" value="80000" min="0" step="5000"/></div>
        <div class="form-group"><label>Ngôn ngữ</label><input type="text" id="formStLang" class="form-input" value="Phụ đề Việt"/></div>
        <button class="btn-submit" onclick="submitShowtimeForm()" id="btnSubmitShowtime">Tạo Suất Chiếu</button>
      </div>
    </div>
  `;
}

function openAddShowtimeModal() {
  document.getElementById('formStDate').value = todayStr();
  document.getElementById('showtimeModal').classList.add('open');
}

function closeShowtimeModal() {
  document.getElementById('showtimeModal').classList.remove('open');
}

async function submitShowtimeForm() {
  const movieId = document.getElementById('formStMovie').value;
  const roomId  = document.getElementById('formStRoom').value;
  const date    = document.getElementById('formStDate').value;
  const time    = document.getElementById('formStTime').value;
  const price   = parseFloat(document.getElementById('formStPrice').value);
  const lang    = document.getElementById('formStLang').value.trim();

  if (!movieId || !roomId || !date || !time || !price || !lang) {
    showToast("⚠️ Vui lòng nhập đầy đủ các trường!", "error");
    return;
  }

  // Ghép datetime dạng YYYY-MM-DD HH:MM
  const datetime = `${date} ${time}`;

  const btn = document.getElementById('btnSubmitShowtime');
  btn.textContent = "⏳ Đang tạo..."; btn.disabled = true;

  try {
    const res = await ShowtimeService.add({ movieId, roomId, datetime, price, lang });
    if (res.ok) {
      showToast("✅ Đã tạo suất chiếu mới thành công!", "success");
      closeShowtimeModal();
      initShowtimesSection(); // vẽ lại bảng
    } else {
      showToast("❌ Lỗi: " + res.msg, "error");
    }
  } catch (err) {
    console.error(err);
    showToast("❌ Lỗi kết nối đến server backend C++!", "error");
  } finally {
    btn.textContent = "Tạo Suất Chiếu"; btn.disabled = false;
  }
}

async function deleteShowtime(id) {
  if (!confirm("⚠️ Bạn có chắc muốn HỦY suất chiếu này? Ghế đã đặt cho suất này cũng sẽ bị hủy!")) return;
  try {
    const res = await ShowtimeService.delete(id);
    if (res.ok) {
      showToast("✅ Đã hủy suất chiếu thành công!", "success");
      initShowtimesSection();
    } else {
      showToast("❌ Lỗi: " + res.msg, "error");
    }
  } catch (err) {
    console.error(err);
    showToast("❌ Lỗi kết nối server!", "error");
  }
}


// ── SECTION: CUSTOMERS MANAGEMENT ──
async function initCustomersSection() {
  const content = document.getElementById('adminContent');
  if (!content) return;

  content.innerHTML = `
    <h2 style="color:var(--text-100);margin-bottom:24px">👥 Quản Lý Khách Hàng</h2>
    <div class="table-card">
      <table>
        <thead>
          <tr>
            <th>ID</th>
            <th>Họ và Tên</th>
            <th>Email</th>
            <th>Số Điện Thoại</th>
            <th>Hạng Thành Viên</th>
            <th>Điểm Tích Lũy</th>
            <th>Tổng Chi Tiêu</th>
          </tr>
        </thead>
        <tbody id="customersTableBody">
          <tr><td colspan="7" style="text-align:center;color:var(--text-400);padding:40px">Đang tải danh sách từ C++...</td></tr>
        </tbody>
      </table>
    </div>
  `;

  try {
    const res = await fetch(resolveRootPath() + 'api/customers');
    const customers = await res.json();
    
    const tbody = document.getElementById('customersTableBody');
    if (!tbody) return;

    if (customers.length === 0) {
      tbody.innerHTML = '<tr><td colspan="7" style="text-align:center;color:var(--text-400);padding:40px">Không có khách hàng nào</td></tr>';
      return;
    }

    tbody.innerHTML = customers.map(c => `
      <tr>
        <td style="font-weight:600;color:var(--primary)">${c.id}</td>
        <td><strong>${c.name}</strong></td>
        <td>${c.email}</td>
        <td>${c.phone}</td>
        <td><span style="font-size:0.95rem;margin-right:4px">${getTierBadge(c.tier)}</span><strong style="color:${getTierColor(c.tier)}">${c.tier.toUpperCase()}</strong></td>
        <td><strong style="color:var(--gold)">${c.points} ⭐</strong></td>
        <td style="font-weight:600">${fmtCurrency(c.totalSpent)}</td>
      </tr>
    `).join('');
  } catch (err) {
    console.error(err);
    showToast("❌ Không thể kết nối server để tải danh sách khách hàng!", "error");
  }
}

// ── SECTION: ROOMS ──
function initRoomsSection() {
  const content = document.getElementById('adminContent');
  if (!content) return;

  const rooms = RoomService.getAll();
  const listRows = rooms.map(r => `
    <tr>
      <td style="font-weight:600;color:var(--primary)">${r.id}</td>
      <td><strong>${r.name}</strong></td>
      <td>${r.type}</td>
      <td>${r.rows} hàng</td>
      <td>${r.seatsPerRow} ghế/hàng</td>
      <td>${r.rows * r.seatsPerRow} ghế</td>
      <td>${fmtCurrency(r.basePrice)}</td>
    </tr>
  `).join('');

  content.innerHTML = `
    <h2 style="color:var(--text-100);margin-bottom:24px">🏢 Danh Sách Phòng Chiếu</h2>
    <div class="table-card">
      <table>
        <thead>
          <tr>
            <th>Mã Phòng</th>
            <th>Tên Phòng</th>
            <th>Loại Phòng</th>
            <th>Số Hàng Ghế</th>
            <th>Ghế/Hàng</th>
            <th>Tổng Số Ghế</th>
            <th>Giá Vé Cơ Sở</th>
          </tr>
        </thead>
        <tbody>
          ${listRows || '<tr><td colspan="7" style="text-align:center;color:var(--text-400);padding:40px">Chưa có phòng chiếu</td></tr>'}
        </tbody>
      </table>
    </div>
  `;
}

// ── SECTION: BOOKINGS (TẤT CẢ VÉ ĐÃ ĐẶT HỆ THỐNG) ──
async function initBookingsSection() {
  const content = document.getElementById('adminContent');
  if (!content) return;

  content.innerHTML = `
    <h2 style="color:var(--text-100);margin-bottom:24px">🎟️ Quản Lý Đơn Đặt Vé</h2>
    <div class="table-card">
      <table>
        <thead>
          <tr>
            <th>Mã Đơn</th>
            <th>Mã Code</th>
            <th>Khách Hàng</th>
            <th>Bộ Phim</th>
            <th>Danh Sách Ghế</th>
            <th>Tổng Tiền</th>
            <th>Phương Thức</th>
            <th>Trạng Thái</th>
          </tr>
        </thead>
        <tbody id="allBookingsTableBody">
          <tr><td colspan="8" style="text-align:center;color:var(--text-400);padding:40px">Đang tải danh sách vé từ C++...</td></tr>
        </tbody>
      </table>
    </div>
  `;

  try {
    const bookings = await getBookingsFromAPI();
    const tbody = document.getElementById('allBookingsTableBody');
    if (!tbody) return;

    if (bookings.length === 0) {
      tbody.innerHTML = '<tr><td colspan="8" style="text-align:center;color:var(--text-400);padding:40px">Không có đơn đặt vé nào</td></tr>';
      return;
    }

    tbody.innerHTML = bookings.slice().reverse().map(b => `
      <tr>
        <td style="font-weight:600;color:var(--primary)">${b.id}</td>
        <td style="font-weight:700;letter-spacing:1px">${b.code}</td>
        <td><strong>${b.userName}</strong></td>
        <td>${b.movie}</td>
        <td><span style="font-family:monospace;font-weight:600">${b.seats.map(s => s.id).join(', ')}</span></td>
        <td style="font-weight:600">${fmtCurrency(b.total)}</td>
        <td>${paymentLabel(b.payment)}</td>
        <td><span class="badge ${b.status}">${statusLabel(b.status)}</span></td>
      </tr>
    `).join('');
  } catch (err) {
    console.error(err);
    showToast("❌ Không thể tải danh sách đặt vé!", "error");
  }
}

// ── SECTION: REPORTS ──
async function initReportsSection() {
  const content = document.getElementById('adminContent');
  if (!content) return;

  try {
    const dashboardRes = await fetch(resolveRootPath() + 'api/dashboard');
    const dbData = await dashboardRes.json();

    content.innerHTML = `
      <h2 style="color:var(--text-100);margin-bottom:24px">📈 Báo Cáo Doanh Thu & Hệ Thống</h2>
      
      <div class="kpi-grid">
        <div class="kpi-card" style="background:rgba(29,185,84,0.08)">
          <div class="kpi-label">💰 Tổng Doanh Thu Thu Được</div>
          <div class="kpi-value" style="color:var(--green)">${fmtCurrency(dbData.totalRevenue)}</div>
        </div>
        <div class="kpi-card">
          <div class="kpi-label">🎟️ Tổng Số Vé Đã Bán</div>
          <div class="kpi-value">${dbData.ticketsSold} vé</div>
        </div>
        <div class="kpi-card">
          <div class="kpi-label">🎥 Phim Đang Trình Chiếu</div>
          <div class="kpi-value">${dbData.activeMovies} phim</div>
        </div>
        <div class="kpi-card">
          <div class="kpi-label">👥 Số Lượng Khách Hàng</div>
          <div class="kpi-value">${dbData.totalCustomers} tài khoản</div>
        </div>
      </div>

      <div class="table-card" style="margin-top:32px;padding:24px">
        <h3 style="color:var(--text-100);margin-bottom:16px">📊 Thống kê chi tiết OOP C++</h3>
        <p style="color:var(--text-300);font-size:0.9rem;line-height:1.6">
          Toàn bộ các đơn hàng ở bảng trên được tính toán và xử lý trực tiếp bởi các lớp nghiệp vụ trong backend C++. 
          Khi một khách hàng thực hiện đặt vé, lớp <code>Booking</code> sẽ tự động tính toán chi phí ghế, áp dụng giảm giá theo hạng thành viên 
          (lấy từ lớp <code>Customer</code>) hoặc giảm giá điểm thưởng, sau đó lưu trạng thái đơn hàng. 
          Các chỉ số doanh thu và vé được thống kê động thông qua API của chương trình C++.
        </p>
      </div>
    `;
  } catch (err) {
    console.error(err);
    showToast("❌ Lỗi tải dữ liệu báo cáo!", "error");
  }
}


// ── MENU NAVIGATION ──
function showSection(name) {
  document.querySelectorAll('.sidebar-item').forEach(i => i.classList.remove('active'));
  
  // Highlighting active sidebar item (matching onclick text)
  const items = Array.from(document.querySelectorAll('.sidebar-item'));
  const foundItem = items.find(item => item.getAttribute('onclick')?.includes(`'${name}'`));
  if (foundItem) foundItem.classList.add('active');

  const titles = {
    dashboard: '📊 Dashboard - Tổng quan hệ thống',
    movies: '🎬 Quản lý Phim',
    showtimes: '📅 Quản lý Lịch chiếu',
    rooms: '🏢 Quản lý Phòng chiếu',
    bookings: '🎟️ Quản lý Đơn đặt vé',
    customers: '👥 Quản lý Khách hàng',
    reports: '📈 Báo cáo Doanh thu'
  };
  
  const titleEl = document.getElementById('pageTitle');
  if (titleEl) titleEl.textContent = titles[name] || name;

  if (name === 'dashboard') initDashboard();
  else if (name === 'movies') initMoviesSection();
  else if (name === 'showtimes') initShowtimesSection();
  else if (name === 'rooms') initRoomsSection();
  else if (name === 'bookings') initBookingsSection();
  else if (name === 'customers') initCustomersSection();
  else if (name === 'reports') initReportsSection();
}

function logout() {
  AuthService.logout();
  window.location.href = '../index.html';
}

// Expose functions to window for onclick callbacks
Object.assign(window, {
  showSection, logout, openAddMovieModal, openEditMovieModal, closeMovieModal,
  submitMovieForm, deleteMovie, openAddShowtimeModal, closeShowtimeModal,
  submitShowtimeForm, deleteShowtime
});
