def min_deletions_to_substring(a, b):
    nA, nB = len(a), len(b)
    best = nA
    for i in range(nA):
        if a[i] != b[0]:
            continue
        pos = i
        ok = True
        for k in range(1, nB):
            j = a.find(b[k], pos + 1)
            if j == -1:
                ok = False
                break
            pos = j
        if ok:
            cost = (pos - i + 1) - nB
            best = min(best, cost)
    return best if best != nA else 0

n = int(input())
results = []
for _ in range(n):
    a, b = input().strip().split()
    results.append(min_deletions_to_substring(a, b))

for res in results:
    print(res)