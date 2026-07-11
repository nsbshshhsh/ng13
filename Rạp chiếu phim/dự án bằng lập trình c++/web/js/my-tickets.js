// ══════════════════════════════════════════════════════════
// my-tickets.js – My tickets specific logic connected to C++ Backend
// ══════════════════════════════════════════════════════════
let currentCancelId = null;
let allBookings = [];
const VIP_ROWS    = ['D','E','F'];
const COUPLE_ROWS = ['H','I'];

document.addEventListener('DOMContentLoaded', async () => {
  await window.dbInitialized; // Chờ C++ backend nạp dữ liệu
  checkAuth();
  loadTickets();
});

function checkAuth() {
  const user = AuthService.currentUser();
  const userInfo  = document.getElementById('userInfo');
  const loginBtn  = document.getElementById('loginBtn');
  const logoutBtn = document.getElementById('logoutBtn');

  if (user) {
    if (userInfo) {
      userInfo.style.display  = 'flex';
      userInfo.innerHTML      = `${getTierBadge(user.tier)} ${user.name.split(' ').pop()} <small style="color:var(--text-400)">(${user.points}đ⭐)</small>`;
    }
    if (loginBtn) loginBtn.style.display  = 'none';
    if (logoutBtn) logoutBtn.style.display = 'inline-flex';
  } else {
    window.location.href = 'index.html';
  }
}

async function loadTickets(filter = 'all') {
  const user = AuthService.currentUser();
  if (!user) return;

  try {
    const res = await fetch(resolveRootPath() + `api/tickets?userId=${user.id}`);
    const ticketsFromApi = await res.json();
    
    // Ánh xạ dữ liệu C++ sang dạng tương thích với giao diện
    allBookings = ticketsFromApi.map(b => {
      const seats = b.tickets.map(t => {
        let type = 'normal';
        if (VIP_ROWS.includes(t.seatId.charAt(0))) type = 'vip';
        else if (COUPLE_ROWS.includes(t.seatId.charAt(0))) type = 'couple';
        return { id: t.seatId, type, price: t.price };
      });
      
      let localStatus = 'confirmed'; // mặc định confirmed (Sắp chiếu)
      if (b.status.includes('hủy') || b.status.includes('hoàn tiền') || b.status.includes('Hủy') || b.status.includes('Hoàn')) {
        localStatus = 'cancelled';
      }
      
      return {
        id: b.id,
        code: b.bookingCode,
        userId: user.id,
        showtimeId: b.showtimeId,
        movie: b.movieTitle,
        payment: b.payMethod,
        showtime: {
          id: b.showtimeId,
          date: b.showDate,
          time: b.showTime,
          roomName: b.roomName,
          roomId: b.roomId,
          type: b.roomName.includes('IMAX') ? 'IMAX' : b.roomName.includes('3D') ? '3D' : b.roomName.includes('Premium') ? 'Premium' : '2D',
        },
        seats: seats,
        total: b.total,
        status: localStatus
      };
    });

    if (window._dbCache) {
      window._dbCache.bookingsById = window._dbCache.bookingsById || {};
      allBookings.forEach(b => {
        window._dbCache.bookingsById[b.id] = b;
      });
    }

    const filtered = filter === 'all' ? allBookings : allBookings.filter(b => b.status === filter);
    renderTickets(filtered);
  } catch (err) {
    console.error("Lỗi tải vé:", err);
    showToast("❌ Lỗi tải danh sách vé từ backend C++!", "error");
  }
}

function filterTickets(status, el) {
  document.querySelectorAll('.tab-item').forEach(t => t.classList.remove('active'));
  el.classList.add('active');
  loadTickets(status);
}

function renderTickets(bookings) {
  const list = document.getElementById('ticketsList');
  if (!list) return;

  if (bookings.length === 0) {
    list.innerHTML = `
      <div class="empty-state">
        <div class="empty-icon">🎟️</div>
        <h3>Chưa có vé nào</h3>
        <p>Đặt vé để tận hưởng những bộ phim hay nhất!</p>
        <a href="movies.html" class="btn-primary" style="display:inline-flex">🎬 Xem phim ngay</a>
      </div>`;
    return;
  }

  list.innerHTML = bookings.map(b => createTicketCardHTML(b)).join('');
}

function createTicketCardHTML(b) {
  const statusMap = {
    confirmed: { label: '✅ Sắp chiếu', cls: 'confirmed' },
    used:      { label: '🎬 Đã xem',    cls: 'used' },
    cancelled: { label: '❌ Đã hủy',    cls: 'cancelled' }
  };
  const st = statusMap[b.status] || statusMap['confirmed'];
  const seatsHtml = (b.seats || []).map(s =>
    `<span class="ticket-seat-badge ${s.type}">${s.id}</span>`
  ).join('');
  const canCancel = b.status === 'confirmed';
  const suat = b.showtime || {};

  return `
    <div class="ticket-card" id="ticket-${b.id}">
      <div class="ticket-body">
        <div class="ticket-left">
          <div class="ticket-movie">${b.movie}</div>
          <div class="ticket-meta">
            <div class="ticket-meta-item">
              <label>Ngày chiếu</label>
              <span>${suat.date || '—'}</span>
            </div>
            <div class="ticket-meta-item">
              <label>Giờ chiếu</label>
              <span>${suat.time || '—'}</span>
            </div>
            <div class="ticket-meta-item">
              <label>Phòng</label>
              <span>${suat.roomName || '—'}</span>
            </div>
            <div class="ticket-meta-item">
              <label>Loại</label>
              <span>${suat.type || '—'}</span>
            </div>
            <div class="ticket-meta-item">
              <label>Thanh toán</label>
              <span>${b.payment || '—'}</span>
            </div>
          </div>
          <div class="ticket-seats">${seatsHtml}</div>
          <div class="ticket-footer">
            <div class="ticket-total">${fmtCurrency(b.total)}</div>
            <span class="ticket-status ${st.cls}">${st.label}</span>
            ${canCancel ? `<button class="btn-cancel-ticket" onclick="openCancelModal('${b.id}', ${b.total})">Hủy vé</button>` : ''}
          </div>
        </div>
        <div class="ticket-right">
          <div class="ticket-qr">📱</div>
          <div class="booking-code-small">${b.code}</div>
          <span style="font-size:0.7rem;color:var(--text-400)">Mã đặt chỗ</span>
        </div>
      </div>
    </div>`;
}

function openCancelModal(bookingId, total) {
  currentCancelId = bookingId;
  const booking = allBookings.find(b => b.id === bookingId);
  let refund = total;
  if (booking) {
    const showDateTime = new Date(`${booking.showtime.date}T${booking.showtime.time}:00`);
    const now = new Date();
    const hoursLeft = (showDateTime - now) / 3600000;
    const rate = calcRefundRate(hoursLeft);
    refund = Math.round(total * rate);
  }
  const refundAmountEl = document.getElementById('refundAmount');
  if (refundAmountEl) refundAmountEl.textContent = fmtCurrency(refund);
  const modal = document.getElementById('cancelModal');
  if (modal) modal.classList.add('open');
  document.body.style.overflow = 'hidden';
}

function closeCancelModal() {
  const modal = document.getElementById('cancelModal');
  if (modal) modal.classList.remove('open');
  document.body.style.overflow = '';
  currentCancelId = null;
}

async function executeCancel() {
  if (!currentCancelId) return;
  try {
    const res = await BookingService.cancel(currentCancelId);
    if (res.ok) {
      showToast(`✅ Đã hủy vé thành công! Đã hoàn lại ${fmtCurrency(res.refundAmount)}.`, 'success');
      checkAuth();
      loadTickets();
    } else {
      showToast('❌ ' + res.msg, 'error');
    }
  } catch (err) {
    console.error(err);
    showToast('❌ Không thể kết nối server!', 'error');
  }
  closeCancelModal();
}

function logout() {
  AuthService.logout();
  window.location.href = 'index.html';
}

// Expose globals so HTML can call them
Object.assign(window, {
  filterTickets, openCancelModal, closeCancelModal, executeCancel, logout
});
