// ══════════════════════════════════════════════════════════
// index.js – Homepage specific logic connected to C++ Backend
// ══════════════════════════════════════════════════════════

function smoothTo(id) {
  event?.preventDefault();
  document.getElementById(id)?.scrollIntoView({ behavior:'smooth' });
}

// Particles
function createParticles() {
  const c = document.getElementById('particles');
  if (!c) return;
  c.innerHTML = ''; // Clear old
  for (let i = 0; i < 28; i++) {
    const p = document.createElement('div');
    p.className = 'particle';
    p.style.cssText = `left:${Math.random()*100}%;--duration:${5+Math.random()*10}s;--delay:${Math.random()*-15}s;width:${2+Math.random()*3}px;height:${2+Math.random()*3}px;`;
    c.appendChild(p);
  }
}

// Counter animation
function animateCounters() {
  document.querySelectorAll('.stat-number[data-count]').forEach(el => {
    const target = +el.dataset.count, dur = 1800, start = performance.now();
    const run = t => {
      const p = Math.min((t - start)/dur, 1);
      el.textContent = Math.floor((1-Math.pow(1-p,3))*target).toLocaleString('vi-VN');
      if (p < 1) requestAnimationFrame(run);
    };
    new IntersectionObserver(entries => {
      if (entries[0].isIntersecting) { requestAnimationFrame(run); entries[0].target._io?.disconnect(); }
    }).observe(el);
  });
}

// Genre tabs
function renderGenreTabs() {
  const movies = MovieService.getAll();
  const genres = ['Tất cả', ...new Set(movies.map(m => m.genre.split('/')[0]))];
  const el = document.getElementById('genreTabs');
  if (!el) return;
  el.innerHTML = genres.map((g, i) => `
    <button class="date-tab${i===0?' active':''}" style="padding:8px 18px;font-size:.85rem"
      onclick="filterMovies('${g}',this)">${g}</button>`).join('');
}

function filterMovies(genre, btn) {
  document.querySelectorAll('#genreTabs .date-tab').forEach(b => b.classList.remove('active'));
  btn.classList.add('active');
  renderMovieGrid(genre === 'Tất cả' ? null : genre);
}

// Movie grid
function renderMovieGrid(genre = null) {
  const grid = document.getElementById('moviesGrid');
  if (!grid) return;
  let movies = MovieService.getAll();
  if (genre) movies = movies.filter(m => m.genre.includes(genre));
  movies = movies.slice(0, 6); // Chỉ hiện 6 phim trên trang chủ

  grid.innerHTML = movies.map((m, i) => `
    <div class="movie-card" style="animation-delay:${i*0.07}s" onclick="window.location.href='booking.html?movie=${m.id}'">
      <div class="movie-poster">
        <div class="movie-poster-placeholder">${getPosterHTML(m.poster)}</div>
        ${m.status ? `<div class="movie-badge ${m.status}">${m.status==='hot'?'🔥 HOT':'✨ MỚI'}</div>` : ''}
        <div class="movie-rating">⭐ ${m.rating.toFixed(1)}</div>
        <div class="movie-overlay">
          <button class="btn-book-now" onclick="event.stopPropagation();window.location.href='booking.html?movie=${m.id}'">🎟️ Đặt vé ngay</button>
        </div>
      </div>
      <div class="movie-info">
        <div class="movie-title">${m.title}</div>
        <div class="movie-meta">
          <span class="movie-genre">${m.genre}</span>
          <span class="movie-age">${m.ageLimit===0?'P':'T'+m.ageLimit}</span>
          <span>⏱ ${fmtDuration(m.duration)}</span>
        </div>
      </div>
    </div>`).join('');
}

// Date tabs
function renderDateTabs() {
  const el = document.getElementById('dateTabs'); if (!el) return;
  el.innerHTML = ''; // Clear old
  const days = ['CN','T2','T3','T4','T5','T6','T7'];
  const months = ['Th1','Th2','Th3','Th4','Th5','Th6','Th7','Th8','Th9','Th10','Th11','Th12'];
  for (let i = 0; i < 7; i++) {
    const d = new Date(); d.setDate(d.getDate()+i);
    const ds = d.toISOString().slice(0,10);
    const tab = document.createElement('button');
    tab.className = 'date-tab' + (i===0?' active':'');
    tab.innerHTML = `<span class="day-name">${i===0?'Hôm nay':days[d.getDay()]}</span><span class="day-num">${d.getDate()}</span><span class="day-name">${months[d.getMonth()]}</span>`;
    tab.onclick = () => {
      document.querySelectorAll('#dateTabs .date-tab').forEach(t=>t.classList.remove('active'));
      tab.classList.add('active');
      renderSchedule(ds);
    };
    el.appendChild(tab);
  }
  renderSchedule(new Date().toISOString().slice(0,10));
}

// Schedule grid
function renderSchedule(date) {
  const grid = document.getElementById('scheduleGrid'); if (!grid) return;
  const suats = ShowtimeService.getByDate(date);
  if (!suats.length) {
    grid.innerHTML = '<p style="text-align:center;color:var(--text-400);padding:40px">Không có suất chiếu nào trong ngày này.</p>';
    return;
  }
  // Group by movie
  const grouped = {};
  suats.forEach(s => { (grouped[s.movieId] = grouped[s.movieId]||[]).push(s); });

  grid.innerHTML = Object.entries(grouped).map(([mid, arr]) => {
    const m = MovieService.getById(mid);
    if (!m) return '';
    const times = arr.map(s => {
      const avail = ShowtimeService.countAvailable(s.id);
      const lowClass = avail < 15 ? 'style="border-color:#e50914"' : '';
      return `<button class="time-btn" ${lowClass} onclick="window.location.href='booking.html?showtime=${s.id}'">
        <span>${s.time}</span>
        <span class="time-type">${s.special?'⭐ PREMIER':ShowtimeService.getById(s.id)?RoomService.getById(ShowtimeService.getById(s.id).roomId)?.type||'':''}</span>
        <span class="time-seats">${avail} ghế còn</span>
      </button>`;
    }).join('');

    return `<div class="schedule-movie-row">
      <div class="schedule-movie-header">
        <div class="schedule-poster">${getPosterHTML(m.poster)}</div>
        <div class="schedule-movie-info">
          <div class="schedule-movie-name">${m.title}</div>
          <div class="schedule-movie-meta">
            <span>⏱ ${fmtDuration(m.duration)}</span>
            <span>${m.genre}</span>
            <span>⭐ ${m.rating}</span>
            <span style="color:${m.ageLimit===0?'#1db954':'#e50914'}">${m.ageLimit===0?'Mọi lứa tuổi':'T'+m.ageLimit+'+'}</span>
          </div>
        </div>
        <a href="booking.html?movie=${m.id}" class="btn-primary" style="flex-shrink:0;font-size:.8rem;padding:8px 16px">Đặt vé</a>
      </div>
      <div class="schedule-times">${times}</div>
    </div>`;
  }).join('');
}

document.addEventListener('DOMContentLoaded', async () => {
  await window.dbInitialized;
  createParticles();
  animateCounters();
  renderGenreTabs();
  renderMovieGrid();
  renderDateTabs();
});
