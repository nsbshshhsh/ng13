"""Chuẩn hoá lại ID suất chiếu về định dạng ST-YYYYMMDD-XXXXX"""
import csv, os, re

path = 'data/showtimes.csv'
if not os.path.exists(path):
    print("Không tìm thấy file")
    exit()

rows = list(csv.reader(open(path, encoding='utf-8')))
seen = {}   # old_id -> new_id mapping
new_rows = []
counter = {}  # date -> current counter

for r in rows:
    if not r: continue
    old_id = r[0]
    # Lấy ngày từ ID (format: ST-YYYYMMDD-XXXXX hoặc dạng lạ)
    m = re.match(r'ST-(\d{8})-', old_id)
    if m:
        date_part = m.group(1)
    else:
        date_part = '20260619'
    
    if old_id not in seen:
        # Tạo ID mới theo format chuẩn
        if date_part not in counter:
            counter[date_part] = 1
        else:
            counter[date_part] += 1
        new_id = f"ST-{date_part}-{counter[date_part]:05d}"
        seen[old_id] = new_id
    
    r[0] = seen[old_id]
    new_rows.append(r)

with open(path, 'w', newline='', encoding='utf-8') as f:
    csv.writer(f).writerows(new_rows)

print(f"Đã chuẩn hoá {len(new_rows)} suất chiếu:")
for old, new in seen.items():
    print(f"  {old} -> {new}")
