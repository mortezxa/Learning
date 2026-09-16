from collections import deque

n, k = map(int, input().split())
graph = [[] for _ in range(n + 1)]

while True:
    try:
        u, v, w = map(int, input().split())
        graph[u].append((v, w))
    except:
        break

dist = [float('inf')] * (n + 1)
dist[k] = 0
q = deque()
q.append(k)

while q:
    u = q.popleft()
    for v, w in graph[u]:
        if dist[u] + w < dist[v]:
            dist[v] = dist[u] + w
            q.append(v)

res = max(dist[1:])
print(-1 if res == float('inf') else res)