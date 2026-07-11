# -*- coding: utf-8 -*-
"""
Dời toàn bộ lịch chiếu (showtimes.csv) thêm 5 ngày (24/6 -> 29/6 logic chung:
mọi suất chiếu +5 ngày: 19->24, 20->25, 21->26, 22->27, 23->28, 25->30).

Đồng thời:
  - Cập nhật lại mã suất chiếu (ST-YYYYMMDD-XXXXX) theo ngày mới, giữ đúng
    định dạng sinh mã mà chương trình C++ dùng (MovieSchedule.cpp).
  - Cập nhật các tham chiếu showtime_id cũ -> mới trong tickets.csv.
  - Giữ nguyên line ending CRLF (\r\n) như file gốc.
"""
import csv
import io
from datetime import datetime, timedelta

DAYS_SHIFT = 5
SHOWTIMES_PATH = 'data/showtimes.csv'
TICKETS_PATH = 'data/tickets.csv'


def shift_date_str(date_str, fmt):
    dt = datetime.strptime(date_str, fmt)
    return (dt + timedelta(days=DAYS_SHIFT)).strftime(fmt)


def process_showtimes():
    with open(SHOWTIMES_PATH, encoding='utf-8', newline='') as f:
        rows = list(csv.reader(f))

    id_map = {}  # old_id -> new_id
    counter = {}  # new_date_part -> next counter
    new_rows = []

    for r in rows:
        if not r:
            continue
        old_id = r[0]
        start_dt = r[3]   # 2026-06-19 09:00
        end_dt = r[4]     # 2026-06-19 10:55
        date_only = r[5]  # 2026-06-19

        new_start = shift_date_str(start_dt, '%Y-%m-%d %H:%M')
        new_end = shift_date_str(end_dt, '%Y-%m-%d %H:%M')
        new_date_only = shift_date_str(date_only, '%Y-%m-%d')
        new_date_part = new_date_only.replace('-', '')

        counter[new_date_part] = counter.get(new_date_part, 0) + 1
        new_id = f"ST-{new_date_part}-{counter[new_date_part]:05d}"

        id_map[old_id] = new_id

        r[0] = new_id
        r[3] = new_start
        r[4] = new_end
        r[5] = new_date_only
        new_rows.append(r)

    buf = io.StringIO()
    writer = csv.writer(buf, lineterminator='\r\n')
    writer.writerows(new_rows)
    with open(SHOWTIMES_PATH, 'w', encoding='utf-8', newline='') as f:
        f.write(buf.getvalue())

    return id_map


def process_tickets(id_map):
    with open(TICKETS_PATH, encoding='utf-8', newline='') as f:
        rows = list(csv.reader(f))

    new_rows = []
    updated = 0
    for r in rows:
        if not r:
            continue
        if len(r) > 2 and r[2] in id_map:
            r[2] = id_map[r[2]]
            updated += 1
        new_rows.append(r)

    buf = io.StringIO()
    writer = csv.writer(buf, lineterminator='\r\n')
    writer.writerows(new_rows)
    with open(TICKETS_PATH, 'w', encoding='utf-8', newline='') as f:
        f.write(buf.getvalue())

    return updated


if __name__ == '__main__':
    id_map = process_showtimes()
    print(f"Đã dời {len(id_map)} suất chiếu (+{DAYS_SHIFT} ngày):")
    for old, new in id_map.items():
        print(f"  {old} -> {new}")

    updated = process_tickets(id_map)
    print(f"\nĐã đồng bộ {updated} dòng trong tickets.csv theo showtime_id mới.")
