n = int(input())
events = []
for _ in range(n):
    a, b = input().split('-')
    h1, m1 = map(int, a.split(':'))
    h2, m2 = map(int, b.split(':'))
    s = h1 * 60 + m1
    e = h2 * 60 + m2
    events.append((s, 1))   # شروع بازه
    events.append((e, -1))  # پایان بازه

# مرتب‌سازی: اگر دو رویداد هم‌زمان باشند، پایان (-1) قبل از شروع (+1) پردازش می‌شود
events.sort(key=lambda x: (x[0], x[1]))

# گذر اول: پیدا کردن بیشترین تعداد هم‌پوشانی
current = 0
max_overlap = 0
for _, delta in events:
    current += delta
    if current > max_overlap:
        max_overlap = current

# گذر دوم: استخراج بازه‌های بین هر دو رویداد که دقیقا هم‌پوشانی == max_overlap دارند
intervals = []
current = 0
prev_time = None
for time, delta in events:
    if prev_time is not None and time > prev_time:
        if current == max_overlap:
            intervals.append((prev_time, time))
    current += delta
    prev_time = time

# چاپ آخرین بازه با بیشترین هم‌پوشانی
s, e = intervals[-1]
h1, m1 = divmod(s, 60)
h2, m2 = divmod(e, 60)
print(f'{h1:02}:{m1:02}-{h2:02}:{m2:02}')