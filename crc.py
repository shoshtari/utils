from typing import List


def xor(a, b):
    if len(a) != len(b):
        raise ValueError(f"a and b must have same size, {len(a)=}, {len(b)=}")
    n = len(a)
    c = [a[i] ^ b[i] for i in range(n)]

    return c


def calc(data: List, p: List) -> List:
    k = len(p) - 1
    n = len(data)

    data = list(data)
    data += [0] * k
    p = list(p)

    q = []
    for i in range(n): 
        if data[i] == 1:
            q.append(1)
            data[i:i + k + 1] = xor(data[i:i + k + 1], p)
        else:
            q.append(0)

    print(data)
    print(q)


data = list("10011011100")
p = list("11001")

data = [int(i) for i in data]
p = [int(i) for i in p]
calc(data, p)
